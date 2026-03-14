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
#include "js_session.h"

uint32_t js_session_take(js_session_t *session) {
    uint32_t status = false;

    if(!session) { return false; }

    switch_mutex_lock(session->mutex);
    if(session->fl_ready) {
        status = true;
        session->wlock++;
    }
    switch_mutex_unlock(session->mutex);

    return status;
}

void js_session_release(js_session_t *session) {
    switch_assert(session);

    switch_mutex_lock(session->mutex);
    if(session->wlock) { session->wlock--; }
    switch_mutex_unlock(session->mutex);
}


