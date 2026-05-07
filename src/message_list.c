// SPDX-FileCopyrightText: 2026 Kamov
//
// SPDX-License-Identifier: Apache-2.0

#include <stdlib.h>
#include <string.h>

#include "libllm.h"
#include "libllm_internal.h"

llm_message_t* llm_internal_message_new(const char* role, const char* content) {
  if (role == NULL || content == NULL) {
    return NULL;
  }

  llm_message_t* msg = malloc(sizeof(llm_message_t));
  if (msg == NULL) return NULL;

  msg->role = strdup(role);
  msg->content = strdup(content);
  msg->tool_call_id = NULL;
  msg->tool_calls = NULL;
  msg->reasoning_content = NULL;
  if (msg->role == NULL || msg->content == NULL) {
    free(msg->role);
    free(msg->content);
    free(msg);
    return NULL;
  }

  return msg;
}

void llm_message_free(llm_message_t* msg) {
  if (msg == NULL) return;
  free(msg->role);
  free(msg->content);
  free(msg->tool_call_id);
  free(msg->tool_calls);
  free(msg->reasoning_content);
  free(msg);
}

llm_message_list_t* llm_message_list_new(void) {
  llm_message_list_t* list = malloc(sizeof(llm_message_list_t));
  if (list == NULL) return NULL;

  list->head = NULL;
  list->tail = NULL;
  list->count = 0;
  return list;
}

void llm_message_list_free(llm_message_list_t* list) {
  if (list == NULL) return;

  llm_message_node_t* cur = list->head;
  while (cur != NULL) {
    llm_message_node_t* next = cur->next;
    llm_message_node_free(cur);
    cur = next;
  }
  free(list);
}

llm_message_node_t* llm_message_list_create_node(const char* role,
                                                 const char* content) {
  if (role == NULL || content == NULL) return NULL;

  llm_message_node_t* node = malloc(sizeof(llm_message_node_t));
  if (node == NULL) return NULL;

  node->message = llm_internal_message_new(role, content);
  if (node->message == NULL) {
    free(node);
    return NULL;
  }

  node->next = NULL;
  node->prev = NULL;
  return node;
}
llm_message_node_t* llm_message_list_create_node_tool_call_id(
    const char* role, const char* content, const char* tool_call_id) {
  if (role == NULL || content == NULL || tool_call_id == NULL) return NULL;

  llm_message_node_t* node = malloc(sizeof(llm_message_node_t));
  if (node == NULL) return NULL;

  node->message = llm_internal_message_new(role, content);
  node->message->tool_call_id = strdup(tool_call_id);
  if (node->message == NULL || node->message->tool_call_id == NULL) {
    free(node->message->tool_call_id);
    free(node->message);
    free(node);
    return NULL;
  }

  node->next = NULL;
  node->prev = NULL;
  return node;
}
llm_message_node_t* llm_message_list_create_node_tool_calls(
    const char* role, const char* content, const char* tool_calls) {
  if (role == NULL || content == NULL || tool_calls == NULL) return NULL;

  llm_message_node_t* node = malloc(sizeof(llm_message_node_t));
  if (node == NULL) return NULL;

  node->message = llm_internal_message_new(role, content);
  node->message->tool_calls = strdup(tool_calls);
  if (node->message == NULL) {
    free(node);
    return NULL;
  }

  node->next = NULL;
  node->prev = NULL;
  return node;
}
void llm_message_node_free(llm_message_node_t* node) {
  if (node == NULL) return;
  llm_message_free(node->message);
  free(node);
}

void llm_message_list_push_front(llm_message_list_t* list,
                                 llm_message_node_t* node) {
  if (list == NULL || node == NULL) return;

  node->prev = NULL;
  node->next = list->head;

  if (list->head != NULL) {
    list->head->prev = node;
  } else {
    list->tail = node;
  }

  list->head = node;
  list->count++;
}

void llm_message_list_push_back(llm_message_list_t* list,
                                llm_message_node_t* node) {
  if (list == NULL || node == NULL) return;

  node->next = NULL;
  node->prev = list->tail;

  if (list->tail != NULL) {
    list->tail->next = node;
  } else {
    list->head = node;
  }

  list->tail = node;
  list->count++;
}

void llm_message_list_insert_after(llm_message_node_t* after,
                                   llm_message_node_t* node,
                                   llm_message_list_t* list) {
  if (after == NULL || node == NULL || list == NULL) return;

  node->prev = after;
  node->next = after->next;

  if (after->next != NULL) {
    after->next->prev = node;
  } else {
    list->tail = node;
  }
  after->next = node;
  list->count++;
}

void llm_message_list_insert_before(llm_message_node_t* before,
                                    llm_message_node_t* node,
                                    llm_message_list_t* list) {
  if (before == NULL || node == NULL || list == NULL) return;

  node->next = before;
  node->prev = before->prev;

  if (before->prev != NULL) {
    before->prev->next = node;
  } else {
    list->head = node;
  }
  before->prev = node;
  list->count++;
}

void llm_message_list_remove(llm_message_node_t* node,
                             llm_message_list_t* list) {
  if (node == NULL || list == NULL) return;

  if (node->prev != NULL) {
    node->prev->next = node->next;
  } else {
    list->head = node->next;
  }

  if (node->next != NULL) {
    node->next->prev = node->prev;
  } else {
    list->tail = node->prev;
  }

  node->next = NULL;
  node->prev = NULL;
  list->count--;
}

llm_message_node_t* llm_message_list_get_head(const llm_message_list_t* list) {
  if (list == NULL) return NULL;
  return list->head;
}

llm_message_node_t* llm_message_list_get_tail(const llm_message_list_t* list) {
  if (list == NULL) return NULL;
  return list->tail;
}

int llm_message_list_get_count(const llm_message_list_t* list) {
  if (list == NULL) return 0;
  return list->count;
}
