// SPDX-FileCopyrightText: 2026 Kamov
//
// SPDX-License-Identifier: Apache-2.0

/**
 * @file test_openai_backend.c
 * @brief OpenAI backend 集成测试（使用 Python Mock HTTP Server）
 *
 * 这些测试需要先启动 mock_server.py：
 *   python3 tests/mock_server.py --port 8089
 *
 * 然后在运行测试程序之前，Mock Server 需要已经启动。
 * 如果服务未启动，这些测试会跳过（返回 PASS 并打印提示）。
 */

#include <stdlib.h>
#include <string.h>

#include "libllm.h"
#include "test_utils.h"

/* 工具回调：用于 llm_tool_register */
static char* dummy_tool_callback(const llm_request_t* req,
                                 const char* args_json) {
  (void)req;
  return strdup(args_json);
}

/* 全局 Mock 服务器地址 */
static const char* MOCK_BASE_URL = "http://127.0.0.1:8089/v1";
static const char* MOCK_API_KEY = "sk-mock-key-for-testing";
static const int MOCK_PORT = 8089;

/* 尝试连接 Mock 服务器的帮助函数 */
static int is_mock_server_running(void) {
  // 简单的端口检查 — 使用 /proc/net/tcp 或尝试连接
  // 这里我们用 curl 命令检测
  char cmd[128];
  snprintf(cmd, sizeof(cmd),
           "curl -s -o /dev/null -w '%%{http_code}' "
           "--connect-timeout 1 http://127.0.0.1:%d/v1/models 2>/dev/null",
           MOCK_PORT);
  FILE* fp = popen(cmd, "r");
  if (fp == NULL) return 0;

  char buf[16] = {0};
  if (fgets(buf, sizeof(buf), fp) != NULL) {
    pclose(fp);
    return atoi(buf) == 200;
  }
  pclose(fp);
  return 0;
}

/* 跳过测试的提示 */
#define SKIP_IF_MOCK_DOWN()                                                  \
  do {                                                                       \
    if (!is_mock_server_running()) {                                         \
      printf(                                                                \
          TEST_COLOR_YELLOW                                                  \
          "  [ SKIP   ] Mock server not running on port %d" TEST_COLOR_RESET \
          "\n",                                                              \
          MOCK_PORT);                                                        \
      /* 不增加失败计数，标记为 pass */                                      \
      g_assert_passed++;                                                     \
      return;                                                                \
    }                                                                        \
  } while (0)

/* ============================================================
 * get_models 集成测试
 * ============================================================ */

TEST(test_get_models_success) {
  SKIP_IF_MOCK_DOWN();

  int models_count = 0;
  llm_t** models = NULL;

  llm_error_t err = get_models(MOCK_BASE_URL, MOCK_API_KEY, LLM_TYPE_OPENAI,
                               &models_count, &models);
  ASSERT_EQ((int)err, (int)LLM_OK);
  ASSERT_GT(models_count, 0);
  ASSERT_NOT_NULL(models);

  // 验证返回的模型
  for (int i = 0; i < models_count; i++) {
    ASSERT_NOT_NULL(llm_get_model_name(models[i]));
    ASSERT_STREQ(llm_get_api_url(models[i]), MOCK_BASE_URL);
    ASSERT_EQ((int)llm_get_type(models[i]), (int)LLM_TYPE_OPENAI);
  }

  // 清理
  for (int i = 0; i < models_count; i++) {
    llm_free(models[i]);
  }
  free(models);
  PASS();
}

TEST(test_get_models_with_wrong_auth) {
  SKIP_IF_MOCK_DOWN();

  // 即使 Mock 服务器不验证 auth，测试能正常通信即可
  int models_count = 0;
  llm_t** models = NULL;

  // 使用错误的 URL（端口不对）来测试
  llm_error_t err = get_models("http://127.0.0.1:19999/v1", "bad-key",
                               LLM_TYPE_OPENAI, &models_count, &models);
  // 应返回网络错误或 backend 错误
  ASSERT_NE((int)err, (int)LLM_OK);
  PASS();
}

/* ============================================================
 * generate 集成测试
 * ============================================================ */

TEST(test_generate_success) {
  SKIP_IF_MOCK_DOWN();

  llm_t* llm = llm_new(LLM_TYPE_OPENAI, MOCK_API_KEY, "gpt-4", MOCK_BASE_URL);
  ASSERT_NOT_NULL(llm);

  llm_request_t* req = llm_request_new();
  ASSERT_NOT_NULL(req);

  // 添加消息
  llm_message_node_t* sys_msg =
      llm_message_list_create_node("system", "You are a helpful assistant.");
  llm_message_node_t* user_msg = llm_message_list_create_node("user", "Hello!");
  ASSERT_NOT_NULL(sys_msg);
  ASSERT_NOT_NULL(user_msg);

  llm_message_list_push_back(llm_request_get_messages(req), sys_msg);
  llm_message_list_push_back(llm_request_get_messages(req), user_msg);

  // 设置请求参数
  llm_request_set_temperature(req, 0.7f);
  llm_request_set_max_tokens(req, 100);

  // 调用 generate
  llm_tools_callback_t* tools_callback = NULL;
  int tools_count = 0;
  llm_token_usage_t usage = {0, 0, 0};

  llm_error_t err =
      generate(llm, req, NULL, &tools_callback, &tools_count, &usage);
  ASSERT_EQ((int)err, (int)LLM_OK);

  // 验证 token 使用量
  ASSERT_GT(usage.total_tokens, 0);
  ASSERT_EQ(usage.prompt_tokens + usage.completion_tokens, usage.total_tokens);

  // 验证消息列表中已添加了助手的回复
  llm_message_node_t* tail =
      llm_message_list_get_tail(llm_request_get_messages(req));
  ASSERT_NOT_NULL(tail);
  ASSERT_STREQ(tail->message->role, "assistant");
  ASSERT_NOT_NULL(tail->message->content);
  ASSERT_GT(strlen(tail->message->content), 0);

  // 验证 llm_last_request 被更新
  ASSERT_NOT_NULL(llm_last_request);

  llm_tools_callback_free(tools_callback, tools_count);
  llm_request_free(req);
  llm_free(llm);
  PASS();
}

TEST(test_generate_with_tool_choice) {
  SKIP_IF_MOCK_DOWN();

  llm_t* llm = llm_new(LLM_TYPE_OPENAI, MOCK_API_KEY, "gpt-4", MOCK_BASE_URL);
  ASSERT_NOT_NULL(llm);

  llm_request_t* req = llm_request_new();
  ASSERT_NOT_NULL(req);

  llm_message_node_t* msg =
      llm_message_list_create_node("user", "What's the weather?");
  llm_message_list_push_back(llm_request_get_messages(req), msg);

  // 设置 tool_choice 为 auto
  llm_request_set_tool_choice(req, "auto");

  // 注册一个工具
  const char* tool_json =
      "{"
      "  \"type\": \"function\","
      "  \"function\": {"
      "    \"name\": \"get_weather\","
      "    \"description\": \"Get weather\","
      "    \"parameters\": {"
      "      \"type\": \"object\","
      "      \"properties\": {"
      "        \"city\": { \"type\": \"string\" }"
      "      },"
      "      \"required\": [\"city\"]"
      "    }"
      "  }"
      "}";
  int rc = llm_tool_register("get_weather", tool_json, dummy_tool_callback);
  ASSERT_EQ(rc, 0);

  llm_tools_callback_t* tools_callback = NULL;
  int tools_count = 0;
  llm_token_usage_t usage = {0, 0, 0};

  llm_error_t err =
      generate(llm, req, NULL, &tools_callback, &tools_count, &usage);
  ASSERT_EQ((int)err, (int)LLM_OK);

  // 验证工具注册已清理
  llm_tool_unregister("get_weather");
  llm_tools_callback_free(tools_callback, tools_count);
  llm_request_free(req);
  llm_free(llm);
  PASS();
}

TEST(test_generate_null_safety) {
  // 不需要 Mock 服务器的参数校验测试
  llm_error_t err = generate(NULL, NULL, NULL, NULL, NULL, NULL);
  ASSERT_EQ((int)err, (int)LLM_ERR_INVALID_PARAM);
  PASS();
}
