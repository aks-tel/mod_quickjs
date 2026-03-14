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
#include "js_jsonrpc.h"

#define CONTENT_TYPE_HDR "Content-type: application/json; charset=utf-8"

typedef struct {
    char    *data;
    uint32_t data_len;
} send_buffer_t;

static size_t curl_io_write_callback(char *buffer, size_t size, size_t nitems, void *user_data) {
    switch_buffer_t *recv_buffer = (switch_buffer_t *)user_data;
    size_t len = (size * nitems);

    if(len > 0 && recv_buffer) {
        switch_buffer_write(recv_buffer, buffer, len);
    }

    return len;
}

static size_t curl_io_read_callback(char *buffer, size_t size, size_t nitems, void *user_data) {
    send_buffer_t *send_buffer = (send_buffer_t *)user_data;
    size_t nmax = (size * nitems);
    size_t ncur = (send_buffer->data_len > nmax) ? nmax : send_buffer->data_len;

    memmove(buffer, send_buffer->data, ncur);
    send_buffer->data += ncur;
    send_buffer->data_len -= ncur;

    return ncur;
}

switch_status_t js_jsonrpc_curl_perform(js_jsonrpc_t *js_jsonrpc, char *data, uint32_t data_len) {
    switch_status_t status = SWITCH_STATUS_SUCCESS;
    CURL *curl_handle = NULL;
    switch_curl_slist_t *headers = NULL;
    send_buffer_t send_buffer = { 0 };
    switch_CURLcode curl_ret = 0;
    long http_resp = 0;

    send_buffer.data = data;
    send_buffer.data_len = data_len;

    curl_handle = switch_curl_easy_init();
    headers = switch_curl_slist_append(headers, CONTENT_TYPE_HDR);

    switch_curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, headers);
    switch_curl_easy_setopt(curl_handle, CURLOPT_POST, 1);
    switch_curl_easy_setopt(curl_handle, CURLOPT_NOSIGNAL, 1);
    switch_curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, (void *)data);
    switch_curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDSIZE, data_len);
    switch_curl_easy_setopt(curl_handle, CURLOPT_READDATA, (void *)&send_buffer);
    switch_curl_easy_setopt(curl_handle, CURLOPT_READFUNCTION, curl_io_read_callback);
    switch_curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)js_jsonrpc->curl_buffer);
    switch_curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, curl_io_write_callback);

    if(js_jsonrpc->connect_timeout > 0) {
        switch_curl_easy_setopt(curl_handle, CURLOPT_CONNECTTIMEOUT, js_jsonrpc->connect_timeout);
    }
    if(js_jsonrpc->connect_timeout > 0) {
        switch_curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, js_jsonrpc->connect_timeout);
    }

    if(js_jsonrpc->fl_ssl_url) {
        switch_curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0);
        switch_curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 0);
    }
    if(js_jsonrpc->proxy) {
        if(js_jsonrpc->proxy_credentials != NULL) {
            switch_curl_easy_setopt(curl_handle, CURLOPT_PROXYAUTH, CURLAUTH_BASIC);
            switch_curl_easy_setopt(curl_handle, CURLOPT_PROXYUSERPWD, js_jsonrpc->proxy_credentials);
        }
        if(strncasecmp(js_jsonrpc->proxy, "https", 5) == 0) {
            switch_curl_easy_setopt(curl_handle, CURLOPT_PROXY_SSL_VERIFYPEER, 0);
        }
        switch_curl_easy_setopt(curl_handle, CURLOPT_PROXY, js_jsonrpc->proxy);
    }

    if(js_jsonrpc->credentials) {
        if(js_jsonrpc->fl_auth_bearer) {
            curl_easy_setopt(curl_handle, CURLOPT_HTTPAUTH, CURLAUTH_BEARER);
            curl_easy_setopt(curl_handle, CURLOPT_XOAUTH2_BEARER, js_jsonrpc->credentials);
        } else {
            switch_curl_easy_setopt(curl_handle, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
            switch_curl_easy_setopt(curl_handle, CURLOPT_USERPWD, js_jsonrpc->credentials);
        }
    }

    headers = switch_curl_slist_append(headers, "Expect:");
    switch_curl_easy_setopt(curl_handle, CURLOPT_URL, js_jsonrpc->url);

    switch_buffer_zero(js_jsonrpc->curl_buffer);
    curl_ret = switch_curl_easy_perform(curl_handle);
    if(!curl_ret) {
        switch_curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &http_resp);
        if(!http_resp) { switch_curl_easy_getinfo(curl_handle, CURLINFO_HTTP_CONNECTCODE, &http_resp); }
    } else {
        http_resp = curl_ret;
    }

    if(http_resp != 200) {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "http-error=[%ld] (%s)\n", http_resp, js_jsonrpc->url);
        status = SWITCH_STATUS_FALSE;
    }

    if(switch_buffer_inuse(js_jsonrpc->curl_buffer) > 0) {
        switch_buffer_write(js_jsonrpc->curl_buffer, "\0", 1);
    }

    if(curl_handle) {
        switch_curl_easy_cleanup(curl_handle);
    }
    if(headers) {
        switch_curl_slist_free_all(headers);
    }

    return status;
}
