// SPDX-FileCopyrightText: 2026 Kamov
//
// SPDX-License-Identifier: Apache-2.0

/**
 * @file test_llm_lifecycle.c
 * @brief 测试 llm_t 和 llm_request_t 的创建、销毁、属性设置/获取
 */

#include "libllm.h"
#include "test_utils.h"

/* ============================================================
 * llm_new / llm_free
 * ============================================================ */

TEST(test_llm_new_valid) {
  llm_t* llm = llm_new(LLM_TYPE_OPENAI, "sk-test-key", "gpt-4",
                       "https://api.openai.com/v1");
  ASSERT_NOT_NULL(llm);
  llm_free(llm);
  PASS();
}

TEST(test_llm_new_null_api_key) {
  llm_t* llm = llm_new(LLM_TYPE_OPENAI, NULL, "gpt-4", NULL);
  ASSERT_NULL(llm);
  PASS();
}

TEST(test_llm_new_null_model) {
  llm_t* llm = llm_new(LLM_TYPE_OPENAI, "sk-test-key", NULL, NULL);
  ASSERT_NULL(llm);
  PASS();
}

TEST(test_llm_new_invalid_type) {
  llm_t* llm = llm_new((llm_type_t)99, "sk-test-key", "gpt-4", NULL);
  ASSERT_NULL(llm);
  PASS();
}

TEST(test_llm_new_null_api_url) {
  llm_t* llm = llm_new(LLM_TYPE_OPENAI, "sk-test-key", "gpt-4", NULL);
  ASSERT_NOT_NULL(llm);
  ASSERT_NULL(llm_get_api_url(llm));
  llm_free(llm);
  PASS();
}

TEST(test_llm_free_null) {
  // 释放 NULL 指针应该安全
  llm_free(NULL);
  PASS();
}

/* ============================================================
 * llm_set / get API
 * ============================================================ */

TEST(test_llm_set_get_api_key) {
  llm_t* llm = llm_new(LLM_TYPE_OPENAI, "original", "model", NULL);
  ASSERT_NOT_NULL(llm);

  ASSERT_STREQ(llm_get_api_key(llm), "original");

  llm_set_api_key(llm, "new-key");
  ASSERT_STREQ(llm_get_api_key(llm), "new-key");

  // 设置 NULL 应该无变化
  llm_set_api_key(llm, NULL);
  ASSERT_STREQ(llm_get_api_key(llm), "new-key");

  llm_free(llm);
  PASS();
}

TEST(test_llm_set_get_model_name) {
  llm_t* llm = llm_new(LLM_TYPE_OPENAI, "key", "gpt-4", NULL);
  ASSERT_NOT_NULL(llm);

  ASSERT_STREQ(llm_get_model_name(llm), "gpt-4");

  llm_set_model_name(llm, "gpt-4-turbo");
  ASSERT_STREQ(llm_get_model_name(llm), "gpt-4-turbo");

  llm_set_model_name(llm, NULL);
  ASSERT_STREQ(llm_get_model_name(llm), "gpt-4-turbo");

  llm_free(llm);
  PASS();
}

TEST(test_llm_set_get_api_url) {
  llm_t* llm = llm_new(LLM_TYPE_OPENAI, "key", "model", "https://example.com");
  ASSERT_NOT_NULL(llm);

  ASSERT_STREQ(llm_get_api_url(llm), "https://example.com");

  llm_set_api_url(llm, "https://new.example.com");
  ASSERT_STREQ(llm_get_api_url(llm), "https://new.example.com");

  llm_set_api_url(llm, NULL);
  ASSERT_NULL(llm_get_api_url(llm));

  llm_free(llm);
  PASS();
}

TEST(test_llm_set_get_type) {
  llm_t* llm = llm_new(LLM_TYPE_ANTHROPIC, "key", "claude-3", NULL);
  ASSERT_NOT_NULL(llm);

  ASSERT_EQ((int)llm_get_type(llm), (int)LLM_TYPE_ANTHROPIC);

  llm_set_type(llm, LLM_TYPE_OPENAI);
  ASSERT_EQ((int)llm_get_type(llm), (int)LLM_TYPE_OPENAI);

  llm_free(llm);
  PASS();
}

TEST(test_llm_get_null_safety) {
  // 所有 getter 对 NULL 输入应返回安全值
  ASSERT_NULL(llm_get_api_key(NULL));
  ASSERT_NULL(llm_get_model_name(NULL));
  ASSERT_NULL(llm_get_api_url(NULL));
  ASSERT_EQ((int)llm_get_type(NULL), 0);

  // 所有 setter 对 NULL 输入应安全无操作
  llm_set_api_key(NULL, "key");
  llm_set_model_name(NULL, "model");
  llm_set_api_url(NULL, "url");
  llm_set_type(NULL, LLM_TYPE_OPENAI);

  PASS();
}

/* ============================================================
 * llm_request_new / llm_request_free
 * ============================================================ */

TEST(test_request_new_free) {
  llm_request_t* req = llm_request_new();
  ASSERT_NOT_NULL(req);
  llm_request_free(req);
  PASS();
}

TEST(test_request_free_null) {
  llm_request_free(NULL);
  PASS();
}

TEST(test_request_default_values) {
  llm_request_t* req = llm_request_new();
  ASSERT_NOT_NULL(req);

  ASSERT_EQ((int)(llm_request_get_temperature(req) * 100), 0);
  ASSERT_EQ((int)(llm_request_get_top_p(req) * 100), 0);
  ASSERT_EQ(llm_request_get_max_tokens(req), 0);
  ASSERT_NULL(llm_request_get_thinking_budget(req));
  ASSERT_NULL(llm_request_get_tool_choice(req));

  llm_request_free(req);
  PASS();
}

TEST(test_request_set_get_temperature) {
  llm_request_t* req = llm_request_new();
  ASSERT_NOT_NULL(req);

  llm_request_set_temperature(req, 0.7f);
  ASSERT_EQ((int)(llm_request_get_temperature(req) * 10), 7);

  llm_request_set_temperature(req, 0.0f);
  ASSERT_EQ((int)(llm_request_get_temperature(req) * 10), 0);

  llm_request_free(req);
  PASS();
}

TEST(test_request_set_get_top_p) {
  llm_request_t* req = llm_request_new();
  ASSERT_NOT_NULL(req);

  llm_request_set_top_p(req, 0.9f);
  ASSERT_EQ((int)(llm_request_get_top_p(req) * 10), 9);

  llm_request_free(req);
  PASS();
}

TEST(test_request_set_get_max_tokens) {
  llm_request_t* req = llm_request_new();
  ASSERT_NOT_NULL(req);

  llm_request_set_max_tokens(req, 4096);
  ASSERT_EQ(llm_request_get_max_tokens(req), 4096);

  llm_request_set_max_tokens(req, 0);
  ASSERT_EQ(llm_request_get_max_tokens(req), 0);

  llm_request_set_max_tokens(req, -1);
  ASSERT_EQ(llm_request_get_max_tokens(req), -1);

  llm_request_free(req);
  PASS();
}

TEST(test_request_set_get_reasoning_effort) {
  llm_request_t* req = llm_request_new();
  ASSERT_NOT_NULL(req);

  llm_request_set_reasoning_effort(req, "high");
  ASSERT_STREQ(llm_request_get_thinking_budget(req), "high");

  llm_request_set_reasoning_effort(req, NULL);
  ASSERT_NULL(llm_request_get_thinking_budget(req));

  llm_request_free(req);
  PASS();
}

TEST(test_request_set_get_tool_choice) {
  llm_request_t* req = llm_request_new();
  ASSERT_NOT_NULL(req);

  llm_request_set_tool_choice(req, "auto");
  ASSERT_STREQ(llm_request_get_tool_choice(req), "auto");

  llm_request_set_tool_choice(req, "none");
  ASSERT_STREQ(llm_request_get_tool_choice(req), "none");

  llm_request_set_tool_choice(req, NULL);
  ASSERT_NULL(llm_request_get_tool_choice(req));

  llm_request_free(req);
  PASS();
}

TEST(test_request_get_messages) {
  llm_request_t* req = llm_request_new();
  ASSERT_NOT_NULL(req);

  llm_message_list_t* messages = llm_request_get_messages(req);
  ASSERT_NOT_NULL(messages);
  ASSERT_EQ(llm_message_list_get_count(messages), 0);

  llm_request_free(req);
  PASS();
}

TEST(test_request_null_safety) {
  // 所有 setter 对 NULL req 安全
  llm_request_set_temperature(NULL, 0.5f);
  llm_request_set_top_p(NULL, 0.5f);
  llm_request_set_max_tokens(NULL, 100);
  llm_request_set_reasoning_effort(NULL, "high");
  llm_request_set_tool_choice(NULL, "auto");
  llm_request_set_messages(NULL, NULL);

  // 所有 getter 对 NULL req 返回安全值
  ASSERT_EQ((int)(llm_request_get_temperature(NULL) * 100), 0);
  ASSERT_EQ((int)(llm_request_get_top_p(NULL) * 100), 0);
  ASSERT_EQ(llm_request_get_max_tokens(NULL), 0);
  ASSERT_NULL(llm_request_get_thinking_budget(NULL));
  ASSERT_NULL(llm_request_get_tool_choice(NULL));
  ASSERT_NULL(llm_request_get_messages(NULL));

  PASS();
}
