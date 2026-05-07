// SPDX-FileCopyrightText: 2026 Kamov
//
// SPDX-License-Identifier: Apache-2.0

#include "libllm.h"
#include "libllm_internal.h"

// ============================================================
// Anthropic provider — stub 实现（待后续完善）
// ============================================================

llm_error_t llm_internal_anthropic_get_models(const char* base_url,
                                              const char* api_key, int* out_len,
                                              llm_t*** out_models) {
  (void)base_url;
  (void)api_key;
  (void)out_len;
  (void)out_models;
  // TODO: 使用 libllm_internal_http_post() + cJSON 解析
  return LLM_ERR_BACKEND;
}

llm_error_t llm_internal_anthropic_generate(
    const llm_t* llm, const llm_request_t* req, const char* extra_body,
    llm_tools_callback_t** tools_callback, int* tools_count,
    llm_token_usage_t* out_usage) {
  (void)llm;
  (void)req;
  (void)extra_body;
  (void)tools_callback;
  (void)tools_count;
  (void)out_usage;
  // TODO: 使用 libllm_internal_http_post() + cJSON 构建请求/解析响应
  return LLM_ERR_BACKEND;
}
