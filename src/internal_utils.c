// SPDX-FileCopyrightText: 2026 Kamov
//
// SPDX-License-Identifier: Apache-2.0

#include <stdatomic.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "libllm_internal.h"
#define LLM_MAX_RESPONSE_BYTES (10 * 1024 * 1024L)
// 线程的全局超时
_Atomic long llm_http_timeout_sec = 30L;

typedef struct {
  char* data;
  size_t len;
} write_cb_buf;

static size_t write_callback(void* contents, size_t size, size_t nmemb,
                             void* userp) {
  if (nmemb > 0 && size > SIZE_MAX / nmemb) {
    return 0;
  }
  size_t total = size * nmemb;
  write_cb_buf* buf = (write_cb_buf*)userp;
  if (total > SIZE_MAX - buf->len - 1) {
    return 0;
  }
  char* tmp = realloc(buf->data, buf->len + total + 1);
  if (tmp == NULL) {
    return 0;
  }

  buf->data = tmp;
  memcpy(buf->data + buf->len, contents, total);
  buf->len += total;
  buf->data[buf->len] = '\0';
  return total;
}

static void setup_curl_handle(CURL* curl, write_cb_buf* buf) {
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, buf);
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);

  long t = llm_http_timeout_sec;
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, t);
  curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, t);

  curl_easy_setopt(curl, CURLOPT_MAXFILESIZE_LARGE, LLM_MAX_RESPONSE_BYTES);

  curl_easy_setopt(curl, CURLOPT_USERAGENT, "libllm/1.0");
}

CURLcode llm_internal_http_post(const char* url, char** header,
                                int header_length, const char* json_body,
                                int* out_status_code, char** out_body,
                                size_t* out_len) {
  CURL* curl = curl_easy_init();
  if (curl == NULL) return CURLE_FAILED_INIT;

  write_cb_buf buf = {NULL, 0};
  struct curl_slist* headers = NULL;

  for (int i = 0; i < header_length; i++) {
    if (header[i] != NULL) {
      headers = curl_slist_append(headers, header[i]);
    }
  }

  setup_curl_handle(curl, &buf);

  curl_easy_setopt(curl, CURLOPT_URL, url);
  curl_easy_setopt(curl, CURLOPT_POST, 1L);
  curl_easy_setopt(curl, CURLOPT_COPYPOSTFIELDS, json_body);
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

  CURLcode rc = curl_easy_perform(curl);

  if (out_status_code != NULL) {
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, out_status_code);
  }

  curl_slist_free_all(headers);
  curl_easy_cleanup(curl);

  if (rc != CURLE_OK) {
    free(buf.data);
    *out_body = NULL;
    *out_len = 0;
    return rc;
  }

  *out_body = buf.data ? buf.data : strdup("");
  *out_len = buf.len;
  return CURLE_OK;
}

CURLcode llm_internal_http_get(const char* url, char** header,
                               int header_length, int* out_status_code,
                               char** out_body, size_t* out_len) {
  CURL* curl = curl_easy_init();
  if (curl == NULL) return CURLE_FAILED_INIT;

  write_cb_buf buf = {NULL, 0};
  struct curl_slist* headers = NULL;

  for (int i = 0; i < header_length; i++) {
    if (header[i] != NULL) {
      headers = curl_slist_append(headers, header[i]);
    }
  }

  setup_curl_handle(curl, &buf);

  curl_easy_setopt(curl, CURLOPT_URL, url);
  curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

  CURLcode rc = curl_easy_perform(curl);

  /* 获取 HTTP 状态码 */
  if (out_status_code != NULL) {
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, out_status_code);
  }

  curl_slist_free_all(headers);
  curl_easy_cleanup(curl);

  if (rc != CURLE_OK) {
    free(buf.data);
    *out_body = NULL;
    *out_len = 0;
    return rc;
  }

  *out_body = buf.data ? buf.data : strdup("");
  *out_len = buf.len;
  return CURLE_OK;
}

char* llm_internal_build_auth_header(const char* api_key) {
  if (api_key == NULL) return NULL;

  const char prefix[] = "Authorization: Bearer ";
  size_t len = sizeof(prefix) - 1 + strlen(api_key) + 1;
  char* header = malloc(len);
  if (header == NULL) return NULL;

  int written = snprintf(header, len, "%s%s", prefix, api_key);
  if (written < 0 || (size_t)written != len - 1) {
    free(header);
    return NULL;
  }
  return header;
}

inline char* build_url(const char* base, const char* path) {
  size_t base_len = strlen(base);
  size_t path_len = strlen(path);

  int need_slash = 0;
  if (base_len > 0 && base[base_len - 1] != '/' && path_len > 0 &&
      path[0] != '/') {
    need_slash = 1;
  }

  size_t full_len = base_len + path_len + need_slash + 1;
  char* full = malloc(full_len);
  if (!full) return NULL;

  strcpy(full, base);
  if (need_slash) strcat(full, "/");
  strcat(full, path);
  return full;
}