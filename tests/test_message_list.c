// SPDX-FileCopyrightText: 2026 Kamov
//
// SPDX-License-Identifier: Apache-2.0

/**
 * @file test_message_list.c
 * @brief 测试双向链表消息列表的全部操作
 */

#include "libllm.h"
#include "test_utils.h"

/* ============================================================
 * llm_message_list_new / free
 * ============================================================ */

TEST(test_list_new_free) {
  llm_message_list_t* list = llm_message_list_new();
  ASSERT_NOT_NULL(list);
  ASSERT_EQ(llm_message_list_get_count(list), 0);
  ASSERT_NULL(llm_message_list_get_head(list));
  ASSERT_NULL(llm_message_list_get_tail(list));
  llm_message_list_free(list);
  PASS();
}

TEST(test_list_free_null) {
  llm_message_list_free(NULL);
  PASS();
}

/* ============================================================
 * llm_message_node_t 创建
 * ============================================================ */

TEST(test_create_node_valid) {
  llm_message_node_t* node =
      llm_message_list_create_node("user", "Hello, world!");
  ASSERT_NOT_NULL(node);
  ASSERT_STREQ(node->message->role, "user");
  ASSERT_STREQ(node->message->content, "Hello, world!");
  ASSERT_NULL(node->message->tool_call_id);
  ASSERT_NULL(node->message->tool_calls);
  ASSERT_NULL(node->next);
  ASSERT_NULL(node->prev);
  llm_message_node_free(node);
  PASS();
}

TEST(test_create_node_invalid_params) {
  ASSERT_NULL(llm_message_list_create_node(NULL, "content"));
  ASSERT_NULL(llm_message_list_create_node("user", NULL));
  ASSERT_NULL(llm_message_list_create_node(NULL, NULL));
  PASS();
}

TEST(test_create_node_tool_call_id) {
  llm_message_node_t* node = llm_message_list_create_node_tool_call_id(
      "tool", "{\"result\":\"ok\"}", "call_123");
  ASSERT_NOT_NULL(node);
  ASSERT_STREQ(node->message->role, "tool");
  ASSERT_STREQ(node->message->content, "{\"result\":\"ok\"}");
  ASSERT_STREQ(node->message->tool_call_id, "call_123");
  llm_message_node_free(node);
  PASS();
}

TEST(test_create_node_tool_calls) {
  const char* tc = "[{\"function\":{\"name\":\"get_weather\"}}]";
  llm_message_node_t* node =
      llm_message_list_create_node_tool_calls("assistant", "", tc);
  ASSERT_NOT_NULL(node);
  ASSERT_STREQ(node->message->role, "assistant");
  ASSERT_STREQ(node->message->tool_calls, tc);
  llm_message_node_free(node);
  PASS();
}

/* ============================================================
 * Push front / back 操作
 * ============================================================ */

TEST(test_push_back_single) {
  llm_message_list_t* list = llm_message_list_new();
  llm_message_node_t* n1 = llm_message_list_create_node("user", "hi");
  llm_message_list_push_back(list, n1);

  ASSERT_EQ(llm_message_list_get_count(list), 1);
  ASSERT_EQ(llm_message_list_get_head(list), n1);
  ASSERT_EQ(llm_message_list_get_tail(list), n1);

  llm_message_list_free(list);
  PASS();
}

TEST(test_push_back_multiple) {
  llm_message_list_t* list = llm_message_list_new();
  llm_message_node_t* n1 = llm_message_list_create_node("user", "msg1");
  llm_message_node_t* n2 = llm_message_list_create_node("user", "msg2");
  llm_message_node_t* n3 = llm_message_list_create_node("assistant", "resp");

  llm_message_list_push_back(list, n1);
  llm_message_list_push_back(list, n2);
  llm_message_list_push_back(list, n3);

  ASSERT_EQ(llm_message_list_get_count(list), 3);
  ASSERT_EQ(llm_message_list_get_head(list), n1);
  ASSERT_EQ(llm_message_list_get_tail(list), n3);

  // 验证链表连接
  ASSERT_NULL(n1->prev);
  ASSERT_EQ(n1->next, n2);
  ASSERT_EQ(n2->prev, n1);
  ASSERT_EQ(n2->next, n3);
  ASSERT_EQ(n3->prev, n2);
  ASSERT_NULL(n3->next);

  // 验证内容顺序
  ASSERT_STREQ(n1->message->content, "msg1");
  ASSERT_STREQ(n2->message->content, "msg2");
  ASSERT_STREQ(n3->message->content, "resp");

  llm_message_list_free(list);
  PASS();
}

TEST(test_push_front_single) {
  llm_message_list_t* list = llm_message_list_new();
  llm_message_node_t* n1 = llm_message_list_create_node("user", "first");
  llm_message_list_push_front(list, n1);

  ASSERT_EQ(llm_message_list_get_count(list), 1);
  ASSERT_EQ(llm_message_list_get_head(list), n1);
  ASSERT_EQ(llm_message_list_get_tail(list), n1);

  llm_message_list_free(list);
  PASS();
}

TEST(test_push_front_multiple) {
  llm_message_list_t* list = llm_message_list_new();
  llm_message_node_t* n1 = llm_message_list_create_node("user", "A");
  llm_message_node_t* n2 = llm_message_list_create_node("user", "B");
  llm_message_node_t* n3 = llm_message_list_create_node("user", "C");

  llm_message_list_push_front(list, n1);  // list: A
  llm_message_list_push_front(list, n2);  // list: B -> A
  llm_message_list_push_front(list, n3);  // list: C -> B -> A

  ASSERT_EQ(llm_message_list_get_count(list), 3);
  ASSERT_EQ(llm_message_list_get_head(list), n3);
  ASSERT_EQ(llm_message_list_get_tail(list), n1);

  // 验证顺序：C(head) -> B -> A(tail)
  ASSERT_STREQ(n3->message->content, "C");
  ASSERT_STREQ(n3->next->message->content, "B");
  ASSERT_STREQ(n3->next->next->message->content, "A");

  llm_message_list_free(list);
  PASS();
}

/* ============================================================
 * Insert after / before
 * ============================================================ */

TEST(test_insert_after) {
  llm_message_list_t* list = llm_message_list_new();
  llm_message_node_t* n1 = llm_message_list_create_node("user", "first");
  llm_message_node_t* n2 = llm_message_list_create_node("user", "third");
  llm_message_node_t* n_ins = llm_message_list_create_node("user", "second");

  llm_message_list_push_back(list, n1);
  llm_message_list_push_back(list, n2);
  llm_message_list_insert_after(n1, n_ins, list);

  ASSERT_EQ(llm_message_list_get_count(list), 3);
  ASSERT_STREQ(n1->next->message->content, "second");
  ASSERT_STREQ(n_ins->next->message->content, "third");
  ASSERT_EQ(n_ins->prev, n1);

  llm_message_list_free(list);
  PASS();
}

TEST(test_insert_after_tail) {
  llm_message_list_t* list = llm_message_list_new();
  llm_message_node_t* n1 = llm_message_list_create_node("user", "first");
  llm_message_node_t* n2 = llm_message_list_create_node("user", "new_tail");

  llm_message_list_push_back(list, n1);
  llm_message_list_insert_after(n1, n2, list);

  ASSERT_EQ(llm_message_list_get_tail(list), n2);
  ASSERT_EQ(llm_message_list_get_count(list), 2);

  llm_message_list_free(list);
  PASS();
}

TEST(test_insert_before) {
  llm_message_list_t* list = llm_message_list_new();
  llm_message_node_t* n1 = llm_message_list_create_node("user", "first");
  llm_message_node_t* n2 = llm_message_list_create_node("user", "second");
  llm_message_node_t* n_ins = llm_message_list_create_node("user", "inserted");

  llm_message_list_push_back(list, n1);
  llm_message_list_push_back(list, n2);
  llm_message_list_insert_before(n2, n_ins, list);

  ASSERT_EQ(llm_message_list_get_count(list), 3);
  ASSERT_STREQ(n1->next->message->content, "inserted");
  ASSERT_STREQ(n_ins->next->message->content, "second");
  ASSERT_EQ(n_ins->prev, n1);

  llm_message_list_free(list);
  PASS();
}

TEST(test_insert_before_head) {
  llm_message_list_t* list = llm_message_list_new();
  llm_message_node_t* n1 = llm_message_list_create_node("user", "original");
  llm_message_node_t* n2 = llm_message_list_create_node("user", "new_head");

  llm_message_list_push_back(list, n1);
  llm_message_list_insert_before(n1, n2, list);

  ASSERT_EQ(llm_message_list_get_head(list), n2);
  ASSERT_EQ(llm_message_list_get_count(list), 2);

  llm_message_list_free(list);
  PASS();
}

/* ============================================================
 * Remove 操作
 * ============================================================ */

TEST(test_remove_head) {
  llm_message_list_t* list = llm_message_list_new();
  llm_message_node_t* n1 = llm_message_list_create_node("user", "head");
  llm_message_node_t* n2 = llm_message_list_create_node("user", "mid");
  llm_message_node_t* n3 = llm_message_list_create_node("user", "tail");

  llm_message_list_push_back(list, n1);
  llm_message_list_push_back(list, n2);
  llm_message_list_push_back(list, n3);

  llm_message_list_remove(n1, list);

  ASSERT_EQ(llm_message_list_get_count(list), 2);
  ASSERT_EQ(llm_message_list_get_head(list), n2);
  ASSERT_NULL(n2->prev);

  // 需要手动释放被移除的节点
  llm_message_node_free(n1);
  llm_message_list_free(list);
  PASS();
}

TEST(test_remove_tail) {
  llm_message_list_t* list = llm_message_list_new();
  llm_message_node_t* n1 = llm_message_list_create_node("user", "head");
  llm_message_node_t* n2 = llm_message_list_create_node("user", "tail");

  llm_message_list_push_back(list, n1);
  llm_message_list_push_back(list, n2);

  llm_message_list_remove(n2, list);

  ASSERT_EQ(llm_message_list_get_count(list), 1);
  ASSERT_EQ(llm_message_list_get_tail(list), n1);
  ASSERT_NULL(n1->next);

  llm_message_node_free(n2);
  llm_message_list_free(list);
  PASS();
}

TEST(test_remove_middle) {
  llm_message_list_t* list = llm_message_list_new();
  llm_message_node_t* n1 = llm_message_list_create_node("user", "A");
  llm_message_node_t* n2 = llm_message_list_create_node("user", "B");
  llm_message_node_t* n3 = llm_message_list_create_node("user", "C");

  llm_message_list_push_back(list, n1);
  llm_message_list_push_back(list, n2);
  llm_message_list_push_back(list, n3);

  llm_message_list_remove(n2, list);

  ASSERT_EQ(llm_message_list_get_count(list), 2);
  ASSERT_EQ(n1->next, n3);
  ASSERT_EQ(n3->prev, n1);

  llm_message_node_free(n2);
  llm_message_list_free(list);
  PASS();
}

TEST(test_remove_only_node) {
  llm_message_list_t* list = llm_message_list_new();
  llm_message_node_t* n1 = llm_message_list_create_node("user", "only");

  llm_message_list_push_back(list, n1);
  llm_message_list_remove(n1, list);

  ASSERT_EQ(llm_message_list_get_count(list), 0);
  ASSERT_NULL(llm_message_list_get_head(list));
  ASSERT_NULL(llm_message_list_get_tail(list));

  llm_message_node_free(n1);
  llm_message_list_free(list);
  PASS();
}

/* ============================================================
 * NULL 安全性
 * ============================================================ */

TEST(test_list_null_safety) {
  ASSERT_NULL(llm_message_list_get_head(NULL));
  ASSERT_NULL(llm_message_list_get_tail(NULL));
  ASSERT_EQ(llm_message_list_get_count(NULL), 0);

  // 所有操作函数在 NULL 参数时应安全无操作
  llm_message_list_push_back(NULL, NULL);
  llm_message_list_push_front(NULL, NULL);
  llm_message_list_insert_after(NULL, NULL, NULL);
  llm_message_list_insert_before(NULL, NULL, NULL);
  llm_message_list_remove(NULL, NULL);

  PASS();
}

/* ============================================================
 * Message free 安全性
 * ============================================================ */

TEST(test_message_free_null) {
  llm_message_free(NULL);
  llm_message_node_free(NULL);
  PASS();
}

/* ============================================================
 * 多次 free 测试（实际是验证无崩溃，但 double-free 是 UB，
 * 这里不测试 double-free，只测试 API 行为
 * ============================================================ */

TEST(test_list_operations_cleanup) {
  // 综合测试：多种操作后释放
  llm_message_list_t* list = llm_message_list_new();
  llm_message_node_t* nodes[5];

  for (int i = 0; i < 5; i++) {
    char buf[32];
    snprintf(buf, sizeof(buf), "msg_%d", i);
    nodes[i] = llm_message_list_create_node("user", buf);
    llm_message_list_push_back(list, nodes[i]);
  }

  ASSERT_EQ(llm_message_list_get_count(list), 5);
  ASSERT_STREQ(llm_message_list_get_head(list)->message->content, "msg_0");
  ASSERT_STREQ(llm_message_list_get_tail(list)->message->content, "msg_4");

  llm_message_list_free(list);
  PASS();
}
