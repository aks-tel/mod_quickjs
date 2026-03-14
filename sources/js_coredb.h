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
#ifndef JS_COREDB_H
#define JS_COREDB_H
#include "mod_quickjs.h"

typedef struct {
    char                    *name;
    switch_core_db_t        *db;
    switch_core_db_stmt_t   *stmt;
    switch_memory_pool_t    *pool;
    JSContext               *ctx;
    JSValue                 callback;
} js_coredb_t;

JSClassID js_coredb_get_classid(JSContext *ctx);
JSClassID js_coredb_get_classid2(JSRuntime *rt);
switch_status_t js_coredb_class_register(JSContext *ctx, JSValue global_obj, JSClassID class_id);

#endif


