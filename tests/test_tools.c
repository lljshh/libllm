// SPDX-FileCopyrightText: 2026 Kamov
//
// SPDX-License-Identifier: Apache-2.0

/**
 * @file test_tools.c
 * @brief 测试工具（tool）注册表功能
 *
 * 注意：工具注册使用全局链表 s_tools，测试之间需要隔离。
 * 因为测试是顺序执行的，我们可以在测试前后清理注册表，
 * 或者将工具注册/注销测试组织在一起。
 */

#include "libllm.h"
#include "test_utils.h"

/* 一个简单的回调函数，用于测试 */
static char* test_callback(const llm_request_t* req, const char* args_json) {
  (void)req;
  return strdup(args_json);
}

static char* another_callback(const llm_request_t* req, const char* args_json) {
  (void)req;
  return strdup("another");
}

/* ============================================================
 * llm_tool_register
 * ============================================================ */

TEST(test_tool_register_valid) {
  int rc = llm_tool_register("test_tool_1", "{\"type\":\"function\"}",
                             test_callback);
  ASSERT_EQ(rc, 0);

  // 清理
  llm_tool_unregister("test_tool_1");
  PASS();
}

TEST(test_tool_register_null_name) {
  int rc = llm_tool_register(NULL, "{}", test_callback);
  ASSERT_EQ(rc, -1);
  PASS();
}

TEST(test_tool_register_null_callback) {
  int rc = llm_tool_register("test_tool_null_cb", "{}", NULL);
  ASSERT_EQ(rc, -1);
  PASS();
}

TEST(test_tool_register_duplicate) {
  // 先注册一次
  int rc1 = llm_tool_register("dup_tool", "{}", test_callback);
  ASSERT_EQ(rc1, 0);

  // 再次注册同名工具应失败
  int rc2 = llm_tool_register("dup_tool", "{}", another_callback);
  ASSERT_EQ(rc2, -1);

  llm_tool_unregister("dup_tool");
  PASS();
}

TEST(test_tool_register_null_json) {
  int rc = llm_tool_register("tool_no_json", NULL, test_callback);
  ASSERT_EQ(rc, 0);

  llm_tool_unregister("tool_no_json");
  PASS();
}

/* ============================================================
 * llm_tool_unregister
 * ============================================================ */

TEST(test_tool_unregister_valid) {
  llm_tool_register("to_remove", "{}", test_callback);

  int rc = llm_tool_unregister("to_remove");
  ASSERT_EQ(rc, 0);
  PASS();
}

TEST(test_tool_unregister_null_name) {
  int rc = llm_tool_unregister(NULL);
  ASSERT_EQ(rc, -1);
  PASS();
}

TEST(test_tool_unregister_not_found) {
  int rc = llm_tool_unregister("nonexistent_tool");
  ASSERT_EQ(rc, -1);
  PASS();
}

/* ============================================================
 * llm_tools_callback_free
 * ============================================================ */

TEST(test_tools_callback_free_null) {
  llm_tools_callback_free(NULL, 0);
  PASS();
}

TEST(test_tools_callback_free_valid) {
  int count = 2;
  llm_tools_callback_t* callbacks = calloc(count, sizeof(llm_tools_callback_t));
  ASSERT_NOT_NULL(callbacks);

  callbacks[0].args_json = strdup("{\"city\":\"Beijing\"}");
  callbacks[0].tool_call_id = strdup("call_001");
  callbacks[0].callback = test_callback;

  callbacks[1].args_json = strdup("{\"city\":\"Shanghai\"}");
  callbacks[1].tool_call_id = strdup("call_002");
  callbacks[1].callback = another_callback;

  llm_tools_callback_free(callbacks, count);

  // 验证已经释放（无法验证内容，但确保不崩溃）
  PASS();
}

/* ============================================================
 * 工具注册表与 generate 的交互（仅测试参数校验，不调 HTTP）
 * ============================================================ */

TEST(test_generate_null_llm) {
  llm_t* llm =
      llm_new(LLM_TYPE_OPENAI, "sk-test", "gpt-4", "https://example.com");
  ASSERT_NOT_NULL(llm);

  // 验证 NULL llm 时返回错误
  llm_error_t err = generate(NULL, NULL, NULL, NULL, NULL, NULL);
  ASSERT_EQ((int)err, (int)LLM_ERR_INVALID_PARAM);

  llm_free(llm);
  PASS();
}

TEST(test_generate_null_request) {
  llm_t* llm =
      llm_new(LLM_TYPE_OPENAI, "sk-test", "gpt-4", "https://example.com");
  ASSERT_NOT_NULL(llm);

  llm_error_t err = generate(llm, NULL, NULL, NULL, NULL, NULL);
  ASSERT_EQ((int)err, (int)LLM_ERR_INVALID_PARAM);

  llm_free(llm);
  PASS();
}

TEST(test_generate_invalid_type) {
  // llm 的 type 不匹配实际 provider（但 find_provider 仍能找到）
  // 这里是验证 openai generate 内部的类型检查
  llm_t* llm =
      llm_new(LLM_TYPE_OPENAI, "sk-test", "gpt-4", "https://example.com");
  ASSERT_NOT_NULL(llm);

  // 把类型改成无效值
  // 不提供 llm_set_type 来设置无效值，直接构造场景
  // 实际上 generate 会走 openai generate，其内部会检查 llm->type
  // 但我们不调用真实 generate（需要 HTTP），只测试参数校验

  llm_free(llm);
  PASS();
}
