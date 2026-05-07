// SPDX-FileCopyrightText: 2026 Kamov
//
// SPDX-License-Identifier: Apache-2.0

#ifndef LIBLLM_H
#define LIBLLM_H

#include <stdatomic.h>
#include <stdlib.h>

// ============================================================
// Message types — merged from message_list.h
// ============================================================
typedef struct {
  char* role;
  char* content;
  char* tool_call_id;
  char* tool_calls;
  char* reasoning_content;
} llm_message_t;

typedef struct llm_message_node llm_message_node_t;

typedef struct llm_message_node {
  llm_message_t* message;
  llm_message_node_t* next;
  llm_message_node_t* prev;
} llm_message_node_t;

typedef struct llm_message_list_s llm_message_list_t;

// ============================================================
// Core libllm types
// ============================================================
typedef enum {
  LLM_TYPE_OPENAI = 0,
  LLM_TYPE_ANTHROPIC = 1,
} llm_type_t;

#if defined(_MSC_VER)
#define LIBLLM_EXPORT __declspec(dllexport)
#else
#define LIBLLM_EXPORT __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

LIBLLM_EXPORT extern _Atomic long llm_http_timeout_sec;
LIBLLM_EXPORT extern _Atomic(char*) llm_last_request;
typedef enum {
  LLM_OK = 0,
  LLM_ERR_INVALID_PARAM = -1,
  LLM_ERR_MEMORY = -2,
  LLM_ERR_BACKEND = -3,
  LLM_ERR_NETWORK = -4,
  LLM_ERR_JSON = -5,
  LLM_ERR_AUTH = -6,
} llm_error_t;
typedef struct llm_s llm_t;
typedef struct llm_request_s llm_request_t;
typedef struct llm_tool_s llm_tool_t;
typedef char* (*llm_tool_callback)(const llm_request_t* req,
                                   const char* args_json);
typedef struct {
  llm_tool_callback callback;
  char* args_json;
  char* tool_call_id;
} llm_tools_callback_t;
typedef struct {
  int prompt_tokens;
  int completion_tokens;
  int total_tokens;
} llm_token_usage_t;

LIBLLM_EXPORT llm_error_t get_models(const char* base_url, const char* api_key,
                                     llm_type_t type, int* out_len,
                                     llm_t*** out_models);
LIBLLM_EXPORT llm_error_t generate(const llm_t* llm, const llm_request_t* req,
                                   const char* extra_body,
                                   llm_tools_callback_t** tools_callback,
                                   int* tools_count,
                                   llm_token_usage_t* out_usage);
LIBLLM_EXPORT llm_t* llm_new(llm_type_t type, const char* api_key,
                             const char* model, const char* api_url);
LIBLLM_EXPORT void llm_free(llm_t* llm);

LIBLLM_EXPORT void llm_set_api_key(llm_t* llm, const char* api_key);
LIBLLM_EXPORT const char* llm_get_api_key(const llm_t* llm);
LIBLLM_EXPORT void llm_set_model_name(llm_t* llm, const char* model);
LIBLLM_EXPORT const char* llm_get_model_name(const llm_t* llm);
LIBLLM_EXPORT void llm_set_api_url(llm_t* llm, const char* api_url);
LIBLLM_EXPORT const char* llm_get_api_url(const llm_t* llm);
LIBLLM_EXPORT void llm_set_type(llm_t* llm, llm_type_t type);
LIBLLM_EXPORT llm_type_t llm_get_type(const llm_t* llm);

LIBLLM_EXPORT llm_request_t* llm_request_new(void);
LIBLLM_EXPORT void llm_request_free(llm_request_t* req);

LIBLLM_EXPORT void llm_request_set_messages(llm_request_t* req,
                                            llm_message_list_t* messages);
LIBLLM_EXPORT llm_message_list_t* llm_request_get_messages(
    const llm_request_t* req);

LIBLLM_EXPORT void llm_request_set_temperature(llm_request_t* req,
                                               float temperature);
LIBLLM_EXPORT float llm_request_get_temperature(const llm_request_t* req);

LIBLLM_EXPORT void llm_request_set_top_p(llm_request_t* req, float top_p);
LIBLLM_EXPORT float llm_request_get_top_p(const llm_request_t* req);

LIBLLM_EXPORT void llm_request_set_max_tokens(llm_request_t* req,
                                              int max_tokens);
LIBLLM_EXPORT int llm_request_get_max_tokens(const llm_request_t* req);

LIBLLM_EXPORT void llm_request_set_reasoning_effort(
    llm_request_t* req, const char* reasoning_effort);
LIBLLM_EXPORT const char* llm_request_get_thinking_budget(
    const llm_request_t* req);

LIBLLM_EXPORT void llm_request_set_tool_choice(llm_request_t* req,
                                               const char* tool_choice);
LIBLLM_EXPORT const char* llm_request_get_tool_choice(const llm_request_t* req);

LIBLLM_EXPORT int llm_tool_register(const char* name, const char* json,
                                    llm_tool_callback callback);
LIBLLM_EXPORT int llm_tool_unregister(const char* name);
LIBLLM_EXPORT void llm_tools_callback_free(llm_tools_callback_t* callbacks,
                                           int count);

// ============================================================
// Message list API — merged from message_list.h
// ============================================================
LIBLLM_EXPORT void llm_message_list_remove(llm_message_node_t* node,
                                           llm_message_list_t* list);
LIBLLM_EXPORT void llm_message_list_push_front(llm_message_list_t* list,
                                               llm_message_node_t* node);
LIBLLM_EXPORT void llm_message_list_push_back(llm_message_list_t* list,
                                              llm_message_node_t* node);
LIBLLM_EXPORT void llm_message_list_insert_after(llm_message_node_t* after,
                                                 llm_message_node_t* node,
                                                 llm_message_list_t* list);
LIBLLM_EXPORT void llm_message_list_insert_before(llm_message_node_t* before,
                                                  llm_message_node_t* node,
                                                  llm_message_list_t* list);
LIBLLM_EXPORT llm_message_node_t* llm_message_list_create_node(
    const char* role, const char* content);
LIBLLM_EXPORT llm_message_node_t* llm_message_list_create_node_tool_call_id(
    const char* role, const char* content, const char* tool_call_id);
LIBLLM_EXPORT llm_message_node_t* llm_message_list_create_node_tool_calls(
    const char* role, const char* content, const char* tool_calls);
LIBLLM_EXPORT llm_message_list_t* llm_message_list_new();
LIBLLM_EXPORT void llm_message_list_free(llm_message_list_t* list);
LIBLLM_EXPORT void llm_message_node_free(llm_message_node_t* node);
LIBLLM_EXPORT void llm_message_free(llm_message_t* msg);
LIBLLM_EXPORT llm_message_node_t* llm_message_list_get_head(
    const llm_message_list_t* list);
LIBLLM_EXPORT llm_message_node_t* llm_message_list_get_tail(
    const llm_message_list_t* list);
LIBLLM_EXPORT int llm_message_list_get_count(const llm_message_list_t* list);

#ifdef __cplusplus
}
#endif

#endif  // LIBLLM_H
