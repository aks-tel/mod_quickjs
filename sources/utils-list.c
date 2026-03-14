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
#include <mod_quickjs.h>

typedef struct js_list_item_s {
    void                            *data;
    struct js_list_item_s           *next;
    js_list_item_data_destructor_h  dh;
} js_list_item_t;

struct js_list_s {
    uint32_t                        size;
    js_list_item_t                  *head;
    js_list_item_t                  *tail;
};

// -------------------------------------------------------------------------------------------------------
switch_status_t js_list_create(js_list_t **list) {
    js_list_t *list_local = NULL;

    switch_zmalloc(list_local, sizeof(js_list_t));
    list_local->size = 0;

    *list = list_local;
    return SWITCH_STATUS_SUCCESS;
}

switch_status_t js_list_destroy(js_list_t **list) {
    js_list_t *tmpl = list ? *list : NULL;

    if(tmpl) {
        struct js_list_item_s *t = NULL, *tt = NULL;

        t = tmpl->head;
        while(t != NULL) {
            tt = t; t = t->next;
            if(tt && tt->dh) tt->dh(tt->data);
            switch_safe_free(tt);
        }

        switch_safe_free(tmpl);
    }

    return SWITCH_STATUS_SUCCESS;
}

switch_status_t js_list_foreach(js_list_t *list, void (*callback)(uint32_t, void *, void *), void *udata) {
 js_list_item_t *target = NULL;

    if(!list || !callback) {
        return SWITCH_STATUS_FALSE;
    }
    if(!list->size) {
        return SWITCH_STATUS_SUCCESS;
    }

    target = list->head;
    for(uint32_t i = 0; i < list->size; i++) {
        if(!target) { break; }
        callback(i, target->data, udata);
        target = target->next;
    }

    return SWITCH_STATUS_SUCCESS;
}

switch_status_t js_list_add(js_list_t *list, void *data, js_list_item_data_destructor_h dh) {
    js_list_item_t *item = NULL;

    if(!list || !data) {
        return SWITCH_STATUS_FALSE;
    }

    switch_zmalloc(item, sizeof(js_list_item_t));

    item->dh = dh;
    item->data = data;
    item->next = NULL;

    if(!list->size) {
        list->head = item;
        list->tail = item;
        list->size = 1;
    } else {
        list->tail->next = item;
        list->tail = item;
        list->size = (list->size + 1);
    }

    return SWITCH_STATUS_SUCCESS;
}

