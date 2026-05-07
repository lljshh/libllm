// SPDX-FileCopyrightText: 2026 Kamov
//
// SPDX-License-Identifier: Apache-2.0

/**
 * @file main.c
 * @brief 测试入口 — 运行所有测试模块
 */

#include <stdio.h>

#include "test_utils.h"

/* 在 test_utils.h 中已包含：
 *   TEST(), RUN_TEST(), PRINT_TEST_RESULTS(), TEST_FAILED
 */

/* === 测试模块声明 === */
/* test_llm_lifecycle.c */ extern void test_llm_new_valid(void);
extern void test_llm_new_null_api_key(void);
extern void test_llm_new_null_model(void);
extern void test_llm_new_invalid_type(void);
extern void test_llm_new_null_api_url(void);
extern void test_llm_free_null(void);
extern void test_llm_set_get_api_key(void);
extern void test_llm_set_get_model_name(void);
extern void test_llm_set_get_api_url(void);
extern void test_llm_set_get_type(void);
extern void test_llm_get_null_safety(void);
extern void test_request_new_free(void);
extern void test_request_free_null(void);
extern void test_request_default_values(void);
extern void test_request_set_get_temperature(void);
extern void test_request_set_get_top_p(void);
extern void test_request_set_get_max_tokens(void);
extern void test_request_set_get_reasoning_effort(void);
extern void test_request_set_get_tool_choice(void);
extern void test_request_get_messages(void);
extern void test_request_null_safety(void);

/* test_message_list.c */ extern void test_list_new_free(void);
extern void test_list_free_null(void);
extern void test_create_node_valid(void);
extern void test_create_node_invalid_params(void);
extern void test_create_node_tool_call_id(void);
extern void test_create_node_tool_calls(void);
extern void test_push_back_single(void);
extern void test_push_back_multiple(void);
extern void test_push_front_single(void);
extern void test_push_front_multiple(void);
extern void test_insert_after(void);
extern void test_insert_after_tail(void);
extern void test_insert_before(void);
extern void test_insert_before_head(void);
extern void test_remove_head(void);
extern void test_remove_tail(void);
extern void test_remove_middle(void);
extern void test_remove_only_node(void);
extern void test_list_null_safety(void);
extern void test_message_free_null(void);
extern void test_list_operations_cleanup(void);

/* test_tools.c */ extern void test_tool_register_valid(void);
extern void test_tool_register_null_name(void);
extern void test_tool_register_null_callback(void);
extern void test_tool_register_duplicate(void);
extern void test_tool_register_null_json(void);
extern void test_tool_unregister_valid(void);
extern void test_tool_unregister_null_name(void);
extern void test_tool_unregister_not_found(void);
extern void test_tools_callback_free_null(void);
extern void test_tools_callback_free_valid(void);
extern void test_generate_null_llm(void);
extern void test_generate_null_request(void);
extern void test_generate_invalid_type(void);

/* test_internal.c */ extern void test_build_url_basic(void);
extern void test_build_url_with_trailing_slash(void);
extern void test_build_url_with_leading_slash(void);
extern void test_build_url_both_slashes(void);
extern void test_build_url_empty_base(void);
extern void test_build_url_empty_path(void);
extern void test_build_url_complex_path(void);
extern void test_build_auth_header_valid(void);
extern void test_build_auth_header_null(void);
extern void test_build_auth_header_empty(void);
extern void test_build_auth_header_long_key(void);
extern void test_provider_count(void);
extern void test_tool_find_null_name(void);
extern void test_tool_find_not_found(void);
extern void test_tool_find_found(void);
extern void test_get_models_null_base_url(void);
extern void test_get_models_null_api_key(void);
extern void test_get_models_invalid_type(void);
extern void test_providers_sentinel(void);

/* test_openai_backend.c */ extern void test_get_models_success(void);
extern void test_get_models_with_wrong_auth(void);
extern void test_generate_success(void);
extern void test_generate_with_tool_choice(void);
extern void test_generate_null_safety(void);

int main(void) {
  printf("\n" TEST_COLOR_CYAN
         "========================================" TEST_COLOR_RESET "\n");
  printf(TEST_COLOR_CYAN "  libllm Test Suite" TEST_COLOR_RESET "\n");
  printf(TEST_COLOR_CYAN
         "========================================" TEST_COLOR_RESET "\n\n");

  /* ========================
   * LLM 生命周期测试
   * ======================== */
  printf(TEST_COLOR_YELLOW "--- LLM Lifecycle Tests ---" TEST_COLOR_RESET "\n");
  RUN_TEST(test_llm_new_valid);
  RUN_TEST(test_llm_new_null_api_key);
  RUN_TEST(test_llm_new_null_model);
  RUN_TEST(test_llm_new_invalid_type);
  RUN_TEST(test_llm_new_null_api_url);
  RUN_TEST(test_llm_free_null);
  RUN_TEST(test_llm_set_get_api_key);
  RUN_TEST(test_llm_set_get_model_name);
  RUN_TEST(test_llm_set_get_api_url);
  RUN_TEST(test_llm_set_get_type);
  RUN_TEST(test_llm_get_null_safety);
  RUN_TEST(test_request_new_free);
  RUN_TEST(test_request_free_null);
  RUN_TEST(test_request_default_values);
  RUN_TEST(test_request_set_get_temperature);
  RUN_TEST(test_request_set_get_top_p);
  RUN_TEST(test_request_set_get_max_tokens);
  RUN_TEST(test_request_set_get_reasoning_effort);
  RUN_TEST(test_request_set_get_tool_choice);
  RUN_TEST(test_request_get_messages);
  RUN_TEST(test_request_null_safety);

  /* ========================
   * 消息列表测试
   * ======================== */
  printf(TEST_COLOR_YELLOW "\n--- Message List Tests ---" TEST_COLOR_RESET
                           "\n");
  RUN_TEST(test_list_new_free);
  RUN_TEST(test_list_free_null);
  RUN_TEST(test_create_node_valid);
  RUN_TEST(test_create_node_invalid_params);
  RUN_TEST(test_create_node_tool_call_id);
  RUN_TEST(test_create_node_tool_calls);
  RUN_TEST(test_push_back_single);
  RUN_TEST(test_push_back_multiple);
  RUN_TEST(test_push_front_single);
  RUN_TEST(test_push_front_multiple);
  RUN_TEST(test_insert_after);
  RUN_TEST(test_insert_after_tail);
  RUN_TEST(test_insert_before);
  RUN_TEST(test_insert_before_head);
  RUN_TEST(test_remove_head);
  RUN_TEST(test_remove_tail);
  RUN_TEST(test_remove_middle);
  RUN_TEST(test_remove_only_node);
  RUN_TEST(test_list_null_safety);
  RUN_TEST(test_message_free_null);
  RUN_TEST(test_list_operations_cleanup);

  /* ========================
   * 工具注册表测试
   * ======================== */
  printf(TEST_COLOR_YELLOW "\n--- Tool Registry Tests ---" TEST_COLOR_RESET
                           "\n");
  RUN_TEST(test_tool_register_valid);
  RUN_TEST(test_tool_register_null_name);
  RUN_TEST(test_tool_register_null_callback);
  RUN_TEST(test_tool_register_duplicate);
  RUN_TEST(test_tool_register_null_json);
  RUN_TEST(test_tool_unregister_valid);
  RUN_TEST(test_tool_unregister_null_name);
  RUN_TEST(test_tool_unregister_not_found);
  RUN_TEST(test_tools_callback_free_null);
  RUN_TEST(test_tools_callback_free_valid);
  RUN_TEST(test_generate_null_llm);
  RUN_TEST(test_generate_null_request);
  RUN_TEST(test_generate_invalid_type);

  /* ========================
   * 内部工具函数测试
   * ======================== */
  printf(TEST_COLOR_YELLOW "\n--- Internal Utils Tests ---" TEST_COLOR_RESET
                           "\n");
  RUN_TEST(test_build_url_basic);
  RUN_TEST(test_build_url_with_trailing_slash);
  RUN_TEST(test_build_url_with_leading_slash);
  RUN_TEST(test_build_url_both_slashes);
  RUN_TEST(test_build_url_empty_base);
  RUN_TEST(test_build_url_empty_path);
  RUN_TEST(test_build_url_complex_path);
  RUN_TEST(test_build_auth_header_valid);
  RUN_TEST(test_build_auth_header_null);
  RUN_TEST(test_build_auth_header_empty);
  RUN_TEST(test_build_auth_header_long_key);
  RUN_TEST(test_provider_count);
  RUN_TEST(test_tool_find_null_name);
  RUN_TEST(test_tool_find_not_found);
  RUN_TEST(test_tool_find_found);
  RUN_TEST(test_get_models_null_base_url);
  RUN_TEST(test_get_models_null_api_key);
  RUN_TEST(test_get_models_invalid_type);
  RUN_TEST(test_providers_sentinel);

  /* ========================
   * OpenAI Backend 集成测试
   * (需要 Mock 服务器)
   * ======================== */
  printf(TEST_COLOR_YELLOW
         "\n--- OpenAI Backend Integration Tests ---" TEST_COLOR_RESET "\n");
  RUN_TEST(test_get_models_success);
  RUN_TEST(test_get_models_with_wrong_auth);
  RUN_TEST(test_generate_success);
  RUN_TEST(test_generate_with_tool_choice);
  RUN_TEST(test_generate_null_safety);

  /* ========================
   * 结果摘要
   * ======================== */
  PRINT_TEST_RESULTS();
  return TEST_FAILED ? 1 : 0;
}
