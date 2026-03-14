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
#ifndef JS_JSONRPC_H
#define JS_JSONRPC_H
#include "mod_quickjs.h"
#include "curl_hlp.h"

typedef struct {
    switch_memory_pool_t    *pool;
    switch_buffer_t         *curl_buffer;
    char                    *url;
    char                    *credentials;
    char                    *proxy_credentials;
    char                    *proxy;
    uint32_t                request_id;
    uint32_t                connect_timeout;
    uint32_t                request_timeout;
    uint8_t                 fl_auth_bearer;
    uint8_t                 fl_enable_exceptions;
    uint8_t                 fl_destroying;
    uint8_t                 fl_ssl_url;
} js_jsonrpc_t;

/* js_jsonrpc.c */
JSClassID js_jsonrpc_get_classid(JSContext *ctx);
JSClassID js_jsonrpc_get_classid2(JSRuntime *rt);
switch_status_t js_jsonrpc_class_register(JSContext *ctx, JSValue global_obj, JSClassID class_id);

switch_status_t js_jsonrpc_curl_perform(js_jsonrpc_t *js_jsonrpc, char *data, uint32_t data_len);

#endif

