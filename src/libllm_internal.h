// SPDX-FileCopyrightText: 2026 Kamov
//
// SPDX-License-Identifier: Apache-2.0

#ifndef LIBLLM_INTERNAL_H
#define LIBLLM_INTERNAL_H

/**
 * @file libllm_internal.h
 * @brief libllm 内部使用的私有类型与辅助函数。
 *
 * 此头文件 **不会** 被安装 / 导出到最终的 .so 中，仅供 src/ 下的
 * .c 文件内部使用。
 */

#include <cJSON.h>
#include <curl/curl.h>

#include "libllm.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef llm_error_t (*get_models_f)(const char* base_url, const char* api_key,
                                    int* out_len, llm_t*** out_models);
typedef llm_error_t (*generate_f)(const llm_t* llm, const llm_request_t* req,
                                  const char* extra_body,
                                  llm_tools_callback_t** tools_callback,
                                  int* tools_count,
                                  llm_token_usage_t* out_usage);

typedef struct llm_s {
  char* api_key;
  char* model;
  char* api_url;
  llm_type_t type;
} llm_t;

typedef struct llm_request_s {
  llm_message_list_t* messages;
  float temperature;
  float top_p;
  int max_tokens;
  char* reasoning_effort;
  char* response_format;
  char* tool_choice;
} llm_request_t;

typedef struct llm_provider_s {
  llm_type_t id;
  get_models_f get_models;
  generate_f generate;
} llm_provider_t;

extern const llm_provider_t llm_providers[];

typedef struct llm_message_list_s {
  llm_message_node_t* head;
  llm_message_node_t* tail;
  int count;
} llm_message_list_t;

struct llm_tool_s {
  char* name;
  char* json;
  llm_tool_callback callback;
  struct llm_tool_s* next;
};

extern llm_tool_t* s_tools;

// OpenAI
llm_error_t llm_internal_openai_get_models(const char* base_url,
                                           const char* api_key, int* out_len,
                                           llm_t*** out_models);
llm_error_t llm_internal_openai_generate(const llm_t* llm,
                                         const llm_request_t* req,
                                         const char* extra_body,
                                         llm_tools_callback_t** tools_callback,
                                         int* tools_count,
                                         llm_token_usage_t* out_usage);
// Anthropic
llm_error_t llm_internal_anthropic_get_models(const char* base_url,
                                              const char* api_key, int* out_len,
                                              llm_t*** out_models);
llm_error_t llm_internal_anthropic_generate(
    const llm_t* llm, const llm_request_t* req, const char* extra_body,
    llm_tools_callback_t** tools_callback, int* tools_count,
    llm_token_usage_t* out_usage);

CURLcode llm_internal_http_post(const char* url, char** header,
                                int header_length, const char* json_body,
                                int* out_status_code, char** out_body,
                                size_t* out_len);
CURLcode llm_internal_http_get(const char* url, char** header,
                               int header_length, int* out_status_code,
                               char** out_body, size_t* out_len);

char* llm_internal_build_auth_header(const char* api_key);

int llm_internal_provider_count(void);
char* build_url(const char* base, const char* path);

llm_tool_t* llm_internal_tool_find(const char* name);

#ifdef __cplusplus
}
#endif

#endif /* LIBLLM_INTERNAL_H */
