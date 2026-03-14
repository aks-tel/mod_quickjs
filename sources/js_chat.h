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
#ifndef JS_CHAT_H
#define JS_CHAT_H
#include "mod_quickjs.h"

#define JS_CHAT_QUEUE_SIZE 1024

typedef struct {
    switch_memory_pool_t    *pool;
    switch_mutex_t          *mutex;
    switch_queue_t          *inq;
} js_chat_t;

typedef struct {
    char        *from;
    char        *body;
    uint32_t    body_len;
} js_chat_message_t;

JSClassID js_chat_get_classid(JSContext *ctx);
JSClassID js_chat_get_classid2(JSRuntime *rt);
switch_status_t js_chat_class_register(JSContext *ctx, JSValue global_obj, JSClassID class_id);

switch_status_t js_chat_message_free(js_chat_message_t **msg);
switch_status_t js_chat_message_alloc(js_chat_message_t **msg, const char *from, const char *body, uint32_t body_len);

#endif


