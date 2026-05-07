// SPDX-FileCopyrightText: 2026 Kamov
//
// SPDX-License-Identifier: Apache-2.0

/**
 * @file test_internal.c
 * @brief 测试内部工具函数（不依赖网络）
 *
 * 为了测试内部函数，我们需要包含 libllm_internal.h。
 */

#include "test_utils.h"

// 需要访问内部函数声明
#include "libllm_internal.h"

/* ============================================================
 * build_url
 * ============================================================ */

TEST(test_build_url_basic) {
  char* url = build_url("https://api.openai.com/v1", "models");
  ASSERT_NOT_NULL(url);
  ASSERT_STREQ(url, "https://api.openai.com/v1/models");
  free(url);
  PASS();
}

TEST(test_build_url_with_trailing_slash) {
  char* url = build_url("https://api.openai.com/v1/", "models");
  ASSERT_NOT_NULL(url);
  ASSERT_STREQ(url, "https://api.openai.com/v1/models");
  free(url);
  PASS();
}

TEST(test_build_url_with_leading_slash) {
  char* url = build_url("https://api.openai.com/v1", "/models");
  ASSERT_NOT_NULL(url);
  ASSERT_STREQ(url, "https://api.openai.com/v1/models");
  free(url);
  PASS();
}

TEST(test_build_url_both_slashes) {
  char* url = build_url("https://api.openai.com/v1/", "/models");
  ASSERT_NOT_NULL(url);
  ASSERT_STREQ(url, "https://api.openai.com/v1//models");
  free(url);
  PASS();
}

TEST(test_build_url_empty_base) {
  // 空 base 仍能工作
  char* url = build_url("", "models");
  ASSERT_NOT_NULL(url);
  ASSERT_STREQ(url, "models");
  free(url);
  PASS();
}

TEST(test_build_url_empty_path) {
  char* url = build_url("https://example.com", "");
  ASSERT_NOT_NULL(url);
  ASSERT_STREQ(url, "https://example.com");
  free(url);
  PASS();
}

TEST(test_build_url_complex_path) {
  char* url = build_url("https://api.anthropic.com", "v1/messages");
  ASSERT_NOT_NULL(url);
  ASSERT_STREQ(url, "https://api.anthropic.com/v1/messages");
  free(url);
  PASS();
}

/* ============================================================
 * llm_internal_build_auth_header
 * ============================================================ */

TEST(test_build_auth_header_valid) {
  char* header = llm_internal_build_auth_header("sk-test-key-123");
  ASSERT_NOT_NULL(header);
  ASSERT_STREQ(header, "Authorization: Bearer sk-test-key-123");
  free(header);
  PASS();
}

TEST(test_build_auth_header_null) {
  char* header = llm_internal_build_auth_header(NULL);
  ASSERT_NULL(header);
  PASS();
}

TEST(test_build_auth_header_empty) {
  char* header = llm_internal_build_auth_header("");
  ASSERT_NOT_NULL(header);
  ASSERT_STREQ(header, "Authorization: Bearer ");
  free(header);
  PASS();
}

TEST(test_build_auth_header_long_key) {
  // 测试长 API key
  char long_key[1024];
  memset(long_key, 'a', sizeof(long_key) - 1);
  long_key[sizeof(long_key) - 1] = '\0';

  char* header = llm_internal_build_auth_header(long_key);
  ASSERT_NOT_NULL(header);

  // 验证以正确前缀开头
  ASSERT_EQ(strncmp(header, "Authorization: Bearer ",
                    strlen("Authorization: Bearer ")),
            0);

  // 验证总长度：前缀(21) + key(1023) + null
  size_t expected_len = strlen("Authorization: Bearer ") + strlen(long_key);
  ASSERT_EQ(strlen(header), expected_len);

  free(header);
  PASS();
}

/* ============================================================
 * llm_internal_provider_count
 * ============================================================ */

TEST(test_provider_count) {
  int count = llm_internal_provider_count();
  // 目前有 2 个 provider: OpenAI + Anthropic
  // sentinel 不计入
  ASSERT_EQ(count, 2);
  PASS();
}

/* ============================================================
 * llm_internal_tool_find
 * ============================================================ */

TEST(test_tool_find_null_name) {
  llm_tool_t* t = llm_internal_tool_find(NULL);
  ASSERT_NULL(t);
  PASS();
}

TEST(test_tool_find_not_found) {
  llm_tool_t* t = llm_internal_tool_find("nonexistent_tool");
  ASSERT_NULL(t);
  PASS();
}

static char* dummy_callback(const llm_request_t* req, const char* args_json) {
  (void)req;
  return strdup(args_json);
}

TEST(test_tool_find_found) {
  // 先注册一个工具
  int rc = llm_tool_register("findable_tool", "{}", dummy_callback);
  ASSERT_EQ(rc, 0);

  llm_tool_t* t = llm_internal_tool_find("findable_tool");
  ASSERT_NOT_NULL(t);
  ASSERT_STREQ(t->name, "findable_tool");

  llm_tool_unregister("findable_tool");
  PASS();
}

/* ============================================================
 * get_models 参数校验（不调用 HTTP）
 * ============================================================ */

TEST(test_get_models_null_base_url) {
  int out_len = 0;
  llm_t** out_models = NULL;

  llm_error_t err =
      get_models(NULL, "sk-test", LLM_TYPE_OPENAI, &out_len, &out_models);
  ASSERT_EQ((int)err, (int)LLM_ERR_INVALID_PARAM);
  PASS();
}

TEST(test_get_models_null_api_key) {
  int out_len = 0;
  llm_t** out_models = NULL;

  llm_error_t err = get_models("https://example.com", NULL, LLM_TYPE_OPENAI,
                               &out_len, &out_models);
  ASSERT_EQ((int)err, (int)LLM_ERR_INVALID_PARAM);
  PASS();
}

TEST(test_get_models_invalid_type) {
  int out_len = 0;
  llm_t** out_models = NULL;

  llm_error_t err = get_models("https://example.com", "sk-test", (llm_type_t)99,
                               &out_len, &out_models);
  ASSERT_EQ((int)err, (int)LLM_ERR_INVALID_PARAM);
  PASS();
}

/* ============================================================
 * llm_providers sentinel
 * ============================================================ */

TEST(test_providers_sentinel) {
  // 验证 provider 表以 sentinel 结束
  int i = 0;
  while (llm_providers[i].generate != NULL) {
    i++;
  }
  // 验证 sentinel 的 id 为 2（无效类型）
  ASSERT_EQ((int)llm_providers[i].id, 2);
  ASSERT_NULL(llm_providers[i].get_models);
  ASSERT_NULL(llm_providers[i].generate);
  PASS();
}
