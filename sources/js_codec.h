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
#ifndef JS_CODEC_H
#define JS_CODEC_H
#include "mod_quickjs.h"

typedef struct {
    uint8_t                 fl_can_destroy;
    uint32_t                samplerate;
    uint32_t                channels;
    uint32_t                ptime;
    uint32_t                flags;
    const char              *name;
    switch_memory_pool_t    *pool;
    switch_codec_t          *codec;
    switch_codec_t          codec_base;
} js_codec_t;

JSClassID js_codec_get_classid(JSContext *ctx);
JSClassID js_codec_get_classid2(JSRuntime *rt);
switch_status_t js_codec_class_register(JSContext *ctx, JSValue global_obj, JSClassID class_id);

JSValue js_codec_from_session_wcodec(JSContext *ctx, switch_core_session_t *session);
JSValue js_codec_from_session_rcodec(JSContext *ctx, switch_core_session_t *session);


#endif


