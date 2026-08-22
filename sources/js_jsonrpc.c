/*
 * FreeSWITCH Modular Media Switching Software Library / Soft-Switch Application
 * Copyright (C) 2005-2014, Anthony Minessale II <anthm@freeswitch.org>
 *
 * Version: MPL 1.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Module Contributor(s):
 *  aks  https://akstel.org
 *
 *
 * json-rpc v1.0 protocol implementation
 * compatible with qooxdoo backends
 *
 */
#include "js_jsonrpc.h"

#define CLASS_NAME              "JsonRPC"
#define PROP_URL                1
#define PROP_PROXY              2
#define PROP_REQ_TIMEOUT        4
#define PROP_CONN_TIMEOUT       5
#define PROP_CREDENTIALS        6
#define PROP_PROXY_CREDENTIALS  7
#define PROP_REQ_ID             8
#define PROP_ENABLE_EXCEPTIONS  9

static void js_jsonrpc_finalizer(JSRuntime *rt, JSValue val);

static JSValue new_local_exception(JSContext *ctx, uint32_t origin, uint32_t code, char *message, bool asException) {
    JSValue result;

    if(!asException) {
        result = JS_NewObject(ctx);
        JS_SetPropertyStr(ctx, result, "error", JS_TRUE);
        JS_SetPropertyStr(ctx, result, "code", JS_NewInt32(ctx, code));
        JS_SetPropertyStr(ctx, result, "origin", JS_NewInt32(ctx, origin));
        JS_SetPropertyStr(ctx, result, "message", message ? JS_NewString(ctx, message) : JS_NULL);
    } else {
        if(zstr(message)) {
            result = JS_ThrowTypeError(ctx, "Error %d:%d", origin, code);
        } else {
            result = JS_ThrowTypeError(ctx, "Error %d:%d (%s)", origin, code, message);
        }
    }

    return result;
}

static JSValue error_obj_to_exception(JSContext *ctx, JSValue error, bool asException) {
    uint32_t origin = 0, code = 0;
    const char *message = NULL;
    JSValue error_code;
    JSValue error_origin;
    JSValue error_message;
    JSValue result = JS_NULL;

    error_code = JS_GetPropertyStr(ctx, error, "code");
    error_origin = JS_GetPropertyStr(ctx, error, "origin");
    error_message = JS_GetPropertyStr(ctx, error, "message");

    if(!QJS_IS_NULL(error_code) && JS_IsNumber(error_code)) {
        JS_ToUint32(ctx, &code, error_code);
    }
    if(!QJS_IS_NULL(error_origin) && JS_IsNumber(error_origin)) {
        JS_ToUint32(ctx, &origin, error_origin);
    }
    if(!QJS_IS_NULL(error_message) && JS_IsString(error_message)) {
        message = JS_ToCString(ctx, error_message);
    }

    if(!asException) {
        result = JS_NewObject(ctx);
        JS_SetPropertyStr(ctx, result, "error", JS_TRUE);
        JS_SetPropertyStr(ctx, result, "code", JS_NewInt32(ctx, code));
        JS_SetPropertyStr(ctx, result, "origin", JS_NewInt32(ctx, origin));
        JS_SetPropertyStr(ctx, result, "message", message ? JS_NewString(ctx, message) : JS_NULL);
    } else {
        if(zstr(message)) {
            result = JS_ThrowTypeError(ctx, "Error %d:%d", origin, code);
        } else {
            result = JS_ThrowTypeError(ctx, "Error %d:%d (%s)", origin, code, message);
        }
    }

    JS_FreeCString(ctx, message);
    return result;
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------------------
static JSValue js_jsonrpc_property_get(JSContext *ctx, JSValueConst this_val, int magic) {
    js_jsonrpc_t *js_jsonrpc = JS_GetOpaque2(ctx, this_val, js_jsonrpc_get_classid(ctx));

    if(!js_jsonrpc) {
        return JS_UNDEFINED;
    }

    switch(magic) {
        case PROP_URL: {
            if(!zstr(js_jsonrpc->url)) {
                return JS_NewString(ctx, js_jsonrpc->url);
            }
            return JS_UNDEFINED;
        }
        case PROP_PROXY: {
            if(!zstr(js_jsonrpc->proxy)) {
               return JS_NewString(ctx, js_jsonrpc->proxy);
            }
            return JS_UNDEFINED;
        }
        case PROP_CREDENTIALS: {
            if(!zstr(js_jsonrpc->credentials)) {
               return JS_NewString(ctx, js_jsonrpc->credentials);
            }
            return JS_UNDEFINED;
        }
        case PROP_PROXY_CREDENTIALS: {
            if(!zstr(js_jsonrpc->proxy_credentials)) {
               return JS_NewString(ctx, js_jsonrpc->proxy_credentials);
            }
            return JS_UNDEFINED;
        }
        case PROP_CONN_TIMEOUT: {
            return JS_NewInt32(ctx, js_jsonrpc->connect_timeout);
        }
        case PROP_REQ_TIMEOUT: {
            return JS_NewInt32(ctx, js_jsonrpc->request_timeout);
        }
        case PROP_REQ_ID: {
            return JS_NewInt32(ctx, js_jsonrpc->request_id);
        }
        case PROP_ENABLE_EXCEPTIONS: {
            return (js_jsonrpc->fl_enable_exceptions ? JS_TRUE : JS_FALSE);
        }
    }

    return JS_UNDEFINED;
}

static JSValue js_jsonrpc_property_set(JSContext *ctx, JSValueConst this_val, JSValue val, int magic) {
    js_jsonrpc_t *js_jsonrpc = JS_GetOpaque2(ctx, this_val, js_jsonrpc_get_classid(ctx));
    const char *str = NULL;
    int copy = 1, success = 1;

    if(!js_jsonrpc) {
        return JS_UNDEFINED;
    }

    switch(magic) {
        case PROP_URL: {
            if(QJS_IS_NULL(val)) { return JS_FALSE; }
            str = JS_ToCString(ctx, val);
            if(!zstr(js_jsonrpc->url)) { copy = strcmp(js_jsonrpc->url, str); }
            if(copy) {
                js_jsonrpc->url = switch_core_strdup(js_jsonrpc->pool, str);
                js_jsonrpc->fl_ssl_url = !strncasecmp(js_jsonrpc->url, "https", 5);
            }
            JS_FreeCString(ctx, str);
            return JS_TRUE;
        }
        case PROP_PROXY: {
            if(QJS_IS_NULL(val)) {
                js_jsonrpc->proxy = NULL;
            } else {
                str = JS_ToCString(ctx, val);
                if(!zstr(js_jsonrpc->proxy)) { copy = strcmp(js_jsonrpc->proxy, str); }
                if(copy) { js_jsonrpc->proxy = switch_core_strdup(js_jsonrpc->pool, str); }
                JS_FreeCString(ctx, str);
            }
            return JS_TRUE;
        }
        case PROP_CREDENTIALS: {
            if(QJS_IS_NULL(val)) {
                js_jsonrpc->credentials = NULL;
            } else {
                str = JS_ToCString(ctx, val);
                if(!zstr(js_jsonrpc->credentials)) { copy = strcmp(js_jsonrpc->credentials, str); }
                if(copy) { js_jsonrpc->credentials = switch_core_strdup(js_jsonrpc->pool, str); }
                js_jsonrpc->fl_auth_bearer = !strchr(js_jsonrpc->credentials, ':');
                JS_FreeCString(ctx, str);
            }
            return JS_TRUE;
        }
        case PROP_PROXY_CREDENTIALS: {
            if(QJS_IS_NULL(val)) {
                js_jsonrpc->proxy_credentials = NULL;
            } else {
                str = JS_ToCString(ctx, val);
                if(!zstr(js_jsonrpc->proxy_credentials)) { copy = strcmp(js_jsonrpc->proxy_credentials, str); }
                if(copy) { js_jsonrpc->proxy_credentials = switch_core_strdup(js_jsonrpc->pool, str); }
                JS_FreeCString(ctx, str);
            }
            return JS_TRUE;
        }
        case PROP_REQ_TIMEOUT: {
            JS_ToUint32(ctx, &js_jsonrpc->request_timeout, val);
            return JS_TRUE;
        }
        case PROP_CONN_TIMEOUT: {
            JS_ToUint32(ctx, &js_jsonrpc->connect_timeout, val);
            return JS_TRUE;
        }
        case PROP_ENABLE_EXCEPTIONS: {
            js_jsonrpc->fl_enable_exceptions = JS_ToBool(ctx, val);
            return JS_TRUE;
        }
    }

    return JS_FALSE;
}

/**
 ** {error : true, ....}
 **/
static JSValue js_jsonrpc_is_error(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
    js_jsonrpc_t *js_jsonrpc = JS_GetOpaque2(ctx, this_val, js_jsonrpc_get_classid(ctx));
    JSValue error;

    if(!js_jsonrpc || js_jsonrpc->fl_destroying) {
        return JS_ThrowTypeError(ctx, "Context destroyed");
    }

    if(argc < 1) {
        return JS_ThrowTypeError(ctx, "isError(callResult)");
    }

    if(QJS_IS_NULL(argv[0]) || !JS_IsObject(argv[0])) {
        return JS_FALSE;
    }

    error = JS_GetPropertyStr(ctx, argv[0], "error");
    if(!QJS_IS_NULL(error) && JS_IsBool(error)) {
        return JS_TRUE;
    }

    return JS_FALSE;
}

/**
 ** perform(service, method, rpcArgsAsArray)
 **/
static JSValue js_jsonrpc_perform_call(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
    js_jsonrpc_t *js_jsonrpc = JS_GetOpaque2(ctx, this_val, js_jsonrpc_get_classid(ctx));
    switch_status_t status = SWITCH_STATUS_SUCCESS;
    const char *js_str = NULL;
    const void *result_ptr = NULL;
    uint32_t result_len = 0;
    JSValue req_obj, rsp_obj, str_obj, err_obj;
    JSValue rrr_obj = JS_NULL;
    JSValue ret_obj = JS_NULL;

    if(!js_jsonrpc || js_jsonrpc->fl_destroying) {
        return JS_ThrowTypeError(ctx, "Context destroyed");
    }

    if(argc < 2 || QJS_IS_NULL(argv[0]) || QJS_IS_NULL(argv[1])) {
        return JS_ThrowTypeError(ctx, "perform(service, method, [rpcArgsAsArray]");
    }

    req_obj = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, req_obj, "id", JS_NewInt32(ctx, js_jsonrpc->request_id++));
    JS_SetPropertyStr(ctx, req_obj, "service", JS_DupValue(ctx, argv[0]));
    JS_SetPropertyStr(ctx, req_obj, "method", JS_DupValue(ctx, argv[1]));

    if(argc > 2) {
        JS_SetPropertyStr(ctx, req_obj, "params", JS_DupValue(ctx, argv[2]));
    } else {
        JS_SetPropertyStr(ctx, req_obj, "params", JS_NULL);
    }

    str_obj = JS_JSONStringify(ctx, req_obj, JS_NULL, JS_NULL);
    if(JS_IsException(str_obj)) {
        js_ctx_dump_error(NULL, ctx);
        JS_ResetUncatchableError(ctx);
        ret_obj = new_local_exception(ctx, 0, 0, "JSON.Stringify() failed", js_jsonrpc->fl_enable_exceptions);
        goto out;
    }

    js_str = JS_ToCString(ctx, str_obj);
    if(zstr(js_str)) {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "(js_str == null)\n");
        ret_obj = new_local_exception(ctx, 0, 0, "Runtime error (js_str == null)", js_jsonrpc->fl_enable_exceptions);
        goto out;
    }

    status = js_jsonrpc_curl_perform(js_jsonrpc, (char *)js_str, strlen(js_str));
    result_len = switch_buffer_peek_zerocopy(js_jsonrpc->curl_buffer, &result_ptr);
    if(status != SWITCH_STATUS_SUCCESS) {
        if(result_len) {
            switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Server response (%s)\n", (char *)result_ptr);
        }
        ret_obj = new_local_exception(ctx, 0, 0, "Unable to perform request (transport error)", js_jsonrpc->fl_enable_exceptions);
        goto out;
    }

    if(result_len < 10) {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Malformed server response (%s)\n", (char *)result_ptr);
        ret_obj = new_local_exception(ctx, 0, 0, "Malformed server response", js_jsonrpc->fl_enable_exceptions);
        goto out;
    }

    rsp_obj = JS_ParseJSON(ctx, (char *)result_ptr, result_len - 1, "<input>"); // -1 for extra '\0'
    if(JS_IsException(rsp_obj)) {
        js_ctx_dump_error(NULL, ctx);
        JS_ResetUncatchableError(ctx);
        ret_obj = new_local_exception(ctx, 0, 0, "Unable to parse json", js_jsonrpc->fl_enable_exceptions);
        goto out;
    }
    if(QJS_IS_NULL(rsp_obj) || !JS_IsObject(rsp_obj)) {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Malformed server response (%s)\n", (char *)result_ptr);
        ret_obj = new_local_exception(ctx, 0, 0, "Malformed server response", js_jsonrpc->fl_enable_exceptions);
        goto out;
    }

    err_obj = JS_GetPropertyStr(ctx, rsp_obj, "error");
    rrr_obj = JS_GetPropertyStr(ctx, rsp_obj, "result");

    if(!QJS_IS_NULL(err_obj)) {
        if(JS_IsObject(err_obj)) {
            ret_obj = error_obj_to_exception(ctx, err_obj, js_jsonrpc->fl_enable_exceptions);
        } else {
            switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Malformed error object (%s)\n", (char *)result_ptr);
            ret_obj = new_local_exception(ctx, 0, 0, "Some remote error", js_jsonrpc->fl_enable_exceptions);
        }
        goto out;
    }

    if(!QJS_IS_NULL(rrr_obj)) {
        ret_obj = JS_DupValue(ctx, rrr_obj);
    }

out:
    JS_FreeCString(ctx, js_str);
    JS_FreeValue(ctx, str_obj);
    JS_FreeValue(ctx, req_obj);
    JS_FreeValue(ctx, rrr_obj);
    JS_FreeValue(ctx, err_obj);
    JS_FreeValue(ctx, rsp_obj);
    return ret_obj;
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------------------
static JSClassDef js_jsonrpc_class = {
    CLASS_NAME,
    .finalizer = js_jsonrpc_finalizer,
};

static const JSCFunctionListEntry js_jsonrpc_proto_funcs[] = {
    JS_CGETSET_MAGIC_DEF("url", js_jsonrpc_property_get, js_jsonrpc_property_set, PROP_URL),
    JS_CGETSET_MAGIC_DEF("requestId", js_jsonrpc_property_get, js_jsonrpc_property_set, PROP_REQ_ID),
    JS_CGETSET_MAGIC_DEF("requestTimeout", js_jsonrpc_property_get, js_jsonrpc_property_set, PROP_REQ_TIMEOUT),
    JS_CGETSET_MAGIC_DEF("connectTimeout", js_jsonrpc_property_get, js_jsonrpc_property_set, PROP_CONN_TIMEOUT),
    JS_CGETSET_MAGIC_DEF("credentials", js_jsonrpc_property_get, js_jsonrpc_property_set, PROP_CREDENTIALS),
    JS_CGETSET_MAGIC_DEF("proxy", js_jsonrpc_property_get, js_jsonrpc_property_set, PROP_PROXY),
    JS_CGETSET_MAGIC_DEF("proxyCredentials", js_jsonrpc_property_get, js_jsonrpc_property_set, PROP_PROXY_CREDENTIALS),
    JS_CGETSET_MAGIC_DEF("enableExceptions", js_jsonrpc_property_get, js_jsonrpc_property_set, PROP_ENABLE_EXCEPTIONS),
    //
    JS_CFUNC_DEF("perform", 1, js_jsonrpc_perform_call),
    JS_CFUNC_DEF("isError", 1, js_jsonrpc_is_error)
};

static void js_jsonrpc_finalizer(JSRuntime *rt, JSValue val) {
    js_jsonrpc_t *js_jsonrpc = JS_GetOpaque(val, js_jsonrpc_get_classid2(rt));
    switch_memory_pool_t *pool = (js_jsonrpc ? js_jsonrpc->pool : NULL);
    uint8_t fl_wloop = true;

    if(!js_jsonrpc || js_jsonrpc->fl_destroying) {
        return;
    }

    js_jsonrpc->fl_destroying = true;

    if(js_jsonrpc->curl_buffer) {
        switch_buffer_destroy(&js_jsonrpc->curl_buffer);
    }
    if(pool) {
        switch_core_destroy_memory_pool(&pool);
    }

#ifdef MOD_QUICKJS_DEBUG
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "js-jsonrpc-finalizer: js_jsonrpc=%p (destroyed)\n", js_jsonrpc);
#endif

    js_free_rt(rt, js_jsonrpc);
}

/*
 * new JsonRPC(url, [username or apikey, password or null, connectTimeout, requestTimeout])
*/
static JSValue js_jsonrpc_contructor(JSContext *ctx, JSValueConst new_target, int argc, JSValueConst *argv) {
    JSValue obj = JS_UNDEFINED;
    JSValue err = JS_UNDEFINED;
    JSValue proto;
    js_jsonrpc_t *js_jsonrpc = NULL;
    switch_buffer_t *curl_buffer = NULL;
    switch_memory_pool_t *pool = NULL;
    const char *url = NULL;
    const char *username = NULL;
    const char *password = NULL;
    uint32_t con_timeout = 0;
    uint32_t req_timeout = 0;

    if(argc < 1 || QJS_IS_NULL(argv[0])) {
        return JS_ThrowTypeError(ctx, "JsonRPC(url, [username or apikey, password or null, connectTimeout, requestTimeout])");
    }

    url = JS_ToCString(ctx, argv[0]);

    if(argc > 1 && !QJS_IS_NULL(argv[1])) {
        username = JS_ToCString(ctx, argv[1]);
    }
    if(argc > 2 && !QJS_IS_NULL(argv[2])) {
        password = JS_ToCString(ctx, argv[2]);
    }
    if(argc > 3) {
        JS_ToUint32(ctx, &con_timeout, argv[3]);
    }
    if(argc > 4) {
        JS_ToUint32(ctx, &req_timeout, argv[4]);
    }

    if(switch_core_new_memory_pool(&pool) != SWITCH_STATUS_SUCCESS) {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "switch_core_new_memory_pool()\n");
        goto fail;
    }

    if(switch_buffer_create_dynamic(&curl_buffer, 1024, 2048, 8192) != SWITCH_STATUS_SUCCESS) {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "switch_buffer_create_dynamic()\n");
        goto fail;
    }

    js_jsonrpc = js_mallocz(ctx, sizeof(js_jsonrpc_t));
    if(!js_jsonrpc) {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "js_mallocz()\n");
        goto fail;
    }

    if(!zstr(username)) {
        if(zstr(password)) {
            js_jsonrpc->credentials = switch_core_strdup(pool, username);
        } else {
            js_jsonrpc->credentials = switch_core_sprintf(pool, "%s:%s", username, password);
        }
    }

    js_jsonrpc->pool = pool;
    js_jsonrpc->url = switch_core_strdup(pool, url);
    js_jsonrpc->curl_buffer = curl_buffer;
    js_jsonrpc->connect_timeout = (con_timeout ? con_timeout : 10);
    js_jsonrpc->request_timeout = req_timeout;
    js_jsonrpc->fl_auth_bearer = (username && !password);
    js_jsonrpc->fl_destroying = false;
    js_jsonrpc->fl_enable_exceptions = false;
    js_jsonrpc->fl_ssl_url = !strncasecmp(url, "https", 5);

    proto = JS_GetPropertyStr(ctx, new_target, "prototype");
    if(JS_IsException(proto)) { goto fail; }

    obj = JS_NewObjectProtoClass(ctx, proto, js_jsonrpc_get_classid(ctx));
    JS_FreeValue(ctx, proto);
    if(JS_IsException(obj)) { goto fail; }

    JS_SetOpaque(obj, js_jsonrpc);

    JS_FreeCString(ctx, url);
    JS_FreeCString(ctx, username);
    JS_FreeCString(ctx, password);

#ifdef MOD_QUICKJS_DEBUG
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "js-jsonrpc-constructor: js_jsonrpc=%p\n", js_jsonrpc);
#endif

    return obj;
fail:
    if(js_jsonrpc) {
        js_free(ctx, js_jsonrpc);
    }
    if(curl_buffer) {
        switch_buffer_destroy(&curl_buffer);
    }
    if(pool) {
        switch_core_destroy_memory_pool(&pool);
    }
    JS_FreeValue(ctx, obj);
    JS_FreeCString(ctx, url);
    JS_FreeCString(ctx, username);
    JS_FreeCString(ctx, password);

    return (JS_IsUndefined(err) ? JS_EXCEPTION : err);
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------------------
// Public
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------
JSClassID js_jsonrpc_get_classid2(JSRuntime *rt) {
    script_t *script = JS_GetRuntimeOpaque(rt);
    switch_assert(script);
    return script->class_id_jsonrpc;
}
JSClassID js_jsonrpc_get_classid(JSContext *ctx) {
    return  js_jsonrpc_get_classid2(JS_GetRuntime(ctx));
}

switch_status_t js_jsonrpc_class_register(JSContext *ctx, JSValue global_obj, JSClassID class_id) {
    JSValue obj_proto, obj_class;
    script_t *script = JS_GetRuntimeOpaque(JS_GetRuntime(ctx));

    switch_assert(script);

    if(JS_IsRegisteredClass(JS_GetRuntime(ctx), class_id)) {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Class with id (%d) already registered!\n", class_id);
        return SWITCH_STATUS_FALSE;
    }

    JS_NewClassID(&class_id);
    JS_NewClass(JS_GetRuntime(ctx), class_id, &js_jsonrpc_class);
    script->class_id_jsonrpc = class_id;

#ifdef MOD_QUICKJS_DEBUG
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "Class registered [%s / %d]\n", CLASS_NAME, class_id);
#endif

    obj_proto = JS_NewObject(ctx);
    JS_SetPropertyFunctionList(ctx, obj_proto, js_jsonrpc_proto_funcs, QJS_ARRAY_SIZE(js_jsonrpc_proto_funcs));

    obj_class = JS_NewCFunction2(ctx, js_jsonrpc_contructor, CLASS_NAME, 1, JS_CFUNC_constructor, 0);
    JS_SetConstructor(ctx, obj_class, obj_proto);
    JS_SetClassProto(ctx, class_id, obj_proto);

    JS_SetPropertyStr(ctx, global_obj, CLASS_NAME, obj_class);

    return SWITCH_STATUS_SUCCESS;
}



