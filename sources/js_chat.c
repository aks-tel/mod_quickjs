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
 */
#include "js_chat.h"

#define CLASS_NAME          "Chat"

#define CHAT_SANITY_CHECK() if (!js_chat || !js_chat->inq) { \
           return JS_ThrowTypeError(ctx, "Chat is not initialized"); \
        }

static JSValue js_chat_contructor(JSContext *ctx, JSValueConst new_target, int argc, JSValueConst *argv);
static void js_chat_finalizer(JSRuntime *rt, JSValue val);

static void queue_clean(switch_queue_t *queue) {
    js_chat_message_t *data = NULL;

    if(queue) {
        while(switch_queue_trypop(queue, (void *)&data) == SWITCH_STATUS_SUCCESS) {
            if(data) { js_chat_message_free(&data); }
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------------------
static JSValue js_chat_property_get(JSContext *ctx, JSValueConst this_val, int magic) {
    js_chat_t *js_chat = JS_GetOpaque2(ctx, this_val, js_chat_get_classid(ctx));

    if(!js_chat) {
        return JS_UNDEFINED;
    }

    return JS_UNDEFINED;
}

static JSValue js_chat_property_set(JSContext *ctx, JSValueConst this_val, JSValue val, int magic) {
    js_chat_t *js_chat = JS_GetOpaque2(ctx, this_val, js_chat_get_classid(ctx));

    if(!js_chat) {
        return JS_FALSE;
    }


    return JS_FALSE;
}

// getMessage()
static JSValue js_chat_get_message(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
    js_chat_t *js_chat = JS_GetOpaque2(ctx, this_val, js_chat_get_classid(ctx));
    js_chat_message_t *msg = NULL;
    JSValue ret_obj = JS_UNDEFINED;

    CHAT_SANITY_CHECK();

    if(switch_queue_trypop(js_chat->inq, (void *)&msg) == SWITCH_STATUS_SUCCESS) {
        if(msg) {
            ret_obj = JS_NewObject(ctx);
            JS_SetPropertyStr(ctx, ret_obj, "class",JS_NewString(ctx, "ChatMessage"));
            JS_SetPropertyStr(ctx, ret_obj, "from", msg->from ? JS_NewString(ctx, msg->from) : JS_NULL);
            JS_SetPropertyStr(ctx, ret_obj, "fromTitle", msg->from_title ? JS_NewString(ctx, msg->from_title) : JS_NULL);
            JS_SetPropertyStr(ctx, ret_obj, "body", msg->body ? JS_NewStringLen(ctx, msg->body, msg->body_len) : JS_NULL);

            js_chat_message_free(&msg);
        }
    }

    return ret_obj;
}

// send(protocol, to, message)
static JSValue js_chat_send(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
    js_chat_t *js_chat = JS_GetOpaque2(ctx, this_val, js_chat_get_classid(ctx));
    script_t *script = JS_GetRuntimeOpaque(JS_GetRuntime(ctx));
    const char *proto = NULL;
    const char *to = NULL;
    const char *msg = NULL;
    JSValue ret = JS_FALSE;

    CHAT_SANITY_CHECK();
    if(!script) {
        return JS_ThrowTypeError(ctx, "script == NULL");
    }

    if(argc < 3 || QJS_IS_NULL(argv[0]) || QJS_IS_NULL(argv[1]) || QJS_IS_NULL(argv[2])) {
        return JS_ThrowTypeError(ctx, "send(protocol, to, message)");
    }

    proto = JS_ToCString(ctx, argv[0]);
    to = JS_ToCString(ctx, argv[1]);
    msg = JS_ToCString(ctx, argv[2]);

    if(switch_core_chat_send_args(proto, "global", script->id, to, "", msg, NULL, "", SWITCH_TRUE) == SWITCH_STATUS_SUCCESS) {
        ret = JS_TRUE;
    }

    JS_FreeCString(ctx, proto);
    JS_FreeCString(ctx, to);
    JS_FreeCString(ctx, msg);

    return ret;
}

// sendEx(protocol, from, to, message)
static JSValue js_chat_send_ex(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
    js_chat_t *js_chat = JS_GetOpaque2(ctx, this_val, js_chat_get_classid(ctx));
    const char *proto = NULL;
    const char *from = NULL;
    const char *to = NULL;
    const char *msg = NULL;
    JSValue ret = JS_FALSE;

    CHAT_SANITY_CHECK();

    if(argc < 4 || QJS_IS_NULL(argv[0]) || QJS_IS_NULL(argv[1]) || QJS_IS_NULL(argv[2]) || QJS_IS_NULL(argv[3])) {
        return JS_ThrowTypeError(ctx, "sendEx(protocol, from, to, message)");
    }

    proto = JS_ToCString(ctx, argv[0]);
    from = JS_ToCString(ctx, argv[1]);
    to = JS_ToCString(ctx, argv[2]);
    msg = JS_ToCString(ctx, argv[3]);

    if(switch_core_chat_send_args(proto, "global", from, to, "", msg, NULL, "", SWITCH_TRUE) == SWITCH_STATUS_SUCCESS) {
        ret = JS_TRUE;
    }

    JS_FreeCString(ctx, proto);
    JS_FreeCString(ctx, from);
    JS_FreeCString(ctx, to);
    JS_FreeCString(ctx, msg);

    return JS_UNDEFINED;
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------------------
static JSClassDef js_chat_class = {
    CLASS_NAME,
    .finalizer = js_chat_finalizer,
};

static const JSCFunctionListEntry js_chat_proto_funcs[] = {
    JS_CFUNC_DEF("getMessage", 1, js_chat_get_message),
    JS_CFUNC_DEF("sendEx", 1, js_chat_send_ex),
    JS_CFUNC_DEF("send", 1, js_chat_send)
};

static void js_chat_finalizer(JSRuntime *rt, JSValue val) {
    js_chat_t *js_chat = JS_GetOpaque(val, js_chat_get_classid2(rt));
    script_t *script = JS_GetRuntimeOpaque(rt);

    if(!js_chat) {
        return;
    }

#ifdef MOD_QUICKJS_DEBUG
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "js-chat-finalizer: js_chat=%p, queue=%p\n", js_chat, js_chat->inq);
#endif

    if(js_chat->inq) {
        js_chat_message_t *data = NULL;
        while(switch_queue_trypop(js_chat->inq, (void *)&data) == SWITCH_STATUS_SUCCESS) {
            if(data) { js_chat_message_free(&data); }
        }
    }

    /* delete script queue ref */
    switch_mutex_lock(script->mutex_chat);
    script->chat_queue_ref = NULL;
    switch_mutex_unlock(script->mutex_chat);

    if(js_chat->pool) {
        switch_core_destroy_memory_pool(&js_chat->pool);
    }

    js_free_rt(rt, js_chat);
}

/* new(queueSize) */
static JSValue js_chat_contructor(JSContext *ctx, JSValueConst new_target, int argc, JSValueConst *argv) {
    script_t *script = JS_GetRuntimeOpaque(JS_GetRuntime(ctx));
    JSValue obj = JS_UNDEFINED;
    JSValue err = JS_UNDEFINED;
    JSValue proto;
    switch_status_t status;
    js_chat_t *js_chat = NULL;
    switch_memory_pool_t *pool = NULL;
    uint32_t qsize = 0;

    if(!script) {
        return JS_ThrowTypeError(ctx, "script == NULL");
    }
    if(script->chat_queue_ref) {
        return JS_ThrowTypeError(ctx, "Script has already registered chat interface");
    }

    if(argc > 0) {
        JS_ToUint32(ctx, &qsize, argv[0]);
    }

    js_chat = js_mallocz(ctx, sizeof(js_chat_t));
    if(!js_chat) {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "js_mallocz()\n");
        return JS_EXCEPTION;
    }

    if(switch_core_new_memory_pool(&pool) != SWITCH_STATUS_SUCCESS) {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "switch_core_new_memory_pool()\n");
        goto fail;
    }

    if(switch_mutex_init(&js_chat->mutex, SWITCH_MUTEX_NESTED, pool) != SWITCH_STATUS_SUCCESS) {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "switch_mutex_init()\n");
        goto fail;
    }

    switch_queue_create(&js_chat->inq, (qsize ? qsize : JS_CHAT_QUEUE_SIZE), pool);
    js_chat->pool = pool;

    proto = JS_GetPropertyStr(ctx, new_target, "prototype");
    if(JS_IsException(proto)) { goto fail; }

    obj = JS_NewObjectProtoClass(ctx, proto, js_chat_get_classid(ctx));
    JS_FreeValue(ctx, proto);
    if(JS_IsException(obj)) { goto fail; }

    JS_SetOpaque(obj, js_chat);

    /* update script queue ref */
    switch_mutex_lock(script->mutex_chat);
    script->chat_queue_ref = js_chat->inq;
    switch_mutex_unlock(script->mutex_chat);

#ifdef MOD_QUICKJS_DEBUG
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "js-chat-constructor: js_chat=%p, queue=%p\n", js_chat, js_chat->inq);
#endif

    return obj;

fail:
    if(js_chat) {
        if(js_chat->inq) {

        }
    }
    if(pool) {
        switch_core_destroy_memory_pool(&pool);
    }
    if(js_chat) {
        js_free(ctx, js_chat);
    }
    JS_FreeValue(ctx, obj);
    return (JS_IsUndefined(err) ? JS_EXCEPTION : err);
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------------------
// Public
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------
JSClassID js_chat_get_classid2(JSRuntime *rt) {
    script_t *script = JS_GetRuntimeOpaque(rt);
    switch_assert(script);
    return script->class_id_chat;
}
JSClassID js_chat_get_classid(JSContext *ctx) {
    return  js_chat_get_classid2(JS_GetRuntime(ctx));
}

switch_status_t js_chat_class_register(JSContext *ctx, JSValue global_obj, JSClassID class_id) {
    JSValue obj_proto, obj_class;
    script_t *script = JS_GetRuntimeOpaque(JS_GetRuntime(ctx));

    switch_assert(script);

    if(JS_IsRegisteredClass(JS_GetRuntime(ctx), class_id)) {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Class with id (%d) already registered!\n", class_id);
        return SWITCH_STATUS_FALSE;
    }

    JS_NewClassID(&class_id);
    JS_NewClass(JS_GetRuntime(ctx), class_id, &js_chat_class);
    script->class_id_chat = class_id;

#ifdef MOD_QUICKJS_DEBUG
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "Class registered [%s / %d]\n", CLASS_NAME, class_id);
#endif

    obj_proto = JS_NewObject(ctx);
    JS_SetPropertyFunctionList(ctx, obj_proto, js_chat_proto_funcs, QJS_ARRAY_SIZE(js_chat_proto_funcs));

    obj_class = JS_NewCFunction2(ctx, js_chat_contructor, CLASS_NAME, 1, JS_CFUNC_constructor, 0);
    JS_SetConstructor(ctx, obj_class, obj_proto);
    JS_SetClassProto(ctx, class_id, obj_proto);

    JS_SetPropertyStr(ctx, global_obj, CLASS_NAME, obj_class);

    return SWITCH_STATUS_SUCCESS;
}

switch_status_t js_chat_message_free(js_chat_message_t **msg) {
    js_chat_message_t *msg_ref = msg ? *msg : NULL;

    if(msg_ref) {
        switch_safe_free(msg_ref->from);
        switch_safe_free(msg_ref->body);
        switch_safe_free(msg_ref);
        *msg =  NULL;
    }

    return SWITCH_STATUS_SUCCESS;
}

switch_status_t js_chat_message_alloc(js_chat_message_t **msg, const char *from, const char *from_title, const char *body, uint32_t body_len) {
    js_chat_message_t *msg_local = NULL;

    switch_zmalloc(msg_local, sizeof(js_chat_message_t));
    msg_local->from = !zstr(from) ? strdup(from) : NULL;
    msg_local->from_title = !zstr(from_title) ? strdup(from_title) : NULL;

    if(!zstr(body) && body_len) {
        switch_malloc(msg_local->body, body_len);
        memcpy(msg_local->body, body, body_len);
        msg_local->body_len = body_len;
    }

    *msg = msg_local;
    return SWITCH_STATUS_SUCCESS;
}
