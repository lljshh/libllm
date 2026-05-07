// SPDX-FileCopyrightText: 2026 Kamov
//
// SPDX-License-Identifier: Apache-2.0

#include <stdatomic.h>
#include <stdlib.h>
#include <string.h>

#include "libllm.h"
#include "libllm_internal.h"

// ============================================================
// OpenAI provider
// ============================================================

llm_error_t llm_internal_openai_get_models(const char* base_url,
                                           const char* api_key, int* out_len,
                                           llm_t*** out_models) {
  if (base_url == NULL || api_key == NULL || out_len == NULL) {
    return LLM_ERR_INVALID_PARAM;
  }
  // 发送请求
  *out_len = 0;
  *out_models = NULL;
  llm_t** arr = NULL;

  char* url = build_url(base_url, "models");
  if (url == NULL) return LLM_ERR_MEMORY;

  int status_code = 0;
  size_t web_output_len = 0;
  char* content = NULL;

  char* header[2];
  char* auth = llm_internal_build_auth_header(api_key);
  if (auth == NULL) {
    free(url);
    return LLM_ERR_MEMORY;
  }
  header[0] = "Accept: application/json";
  header[1] = auth;
  CURLcode rc = llm_internal_http_get(url, header, 2, &status_code, &content,
                                      &web_output_len);
  // 解析JSON
  llm_error_t err = LLM_ERR_BACKEND;
  cJSON* root = NULL;
  if (rc == CURLE_OK && content != NULL) {
    root = cJSON_Parse(content);
    if (root != NULL) {
      cJSON* data_array = cJSON_GetObjectItemCaseSensitive(root, "data");
      if (cJSON_IsArray(data_array)) {
        int count = cJSON_GetArraySize(data_array);
        if (count == 0) {
          err = LLM_OK;  // 没有模型，但请求成功
          goto cleanup;
        }
        // 分配 llm_t* 指针数组
        arr = calloc((size_t)count, sizeof(llm_t*));
        if (arr == NULL) {
          err = LLM_ERR_MEMORY;
          goto cleanup;
        }

        int valid_count = 0;
        for (int i = 0; i < count; i++) {
          cJSON* item = cJSON_GetArrayItem(data_array, i);
          if (item == NULL) continue;
          // 获取模型id
          cJSON* id = cJSON_GetObjectItemCaseSensitive(item, "id");
          cJSON* obj_type = cJSON_GetObjectItemCaseSensitive(item, "object");
          if (id == NULL || !cJSON_IsString(id) || id->valuestring == NULL)
            continue;
          if (obj_type == NULL || !cJSON_IsString(obj_type) ||
              obj_type->valuestring == NULL)
            continue;
          if (strcmp(obj_type->valuestring, "model") != 0) continue;
          llm_t* model =
              llm_new(LLM_TYPE_OPENAI, api_key, id->valuestring, base_url);
          if (model != NULL) {
            arr[valid_count++] = model;
          }
        }

        *out_models = arr;
        *out_len = valid_count;
        err = LLM_OK;
      }
    }
  }
  if (err != LLM_OK) free(arr);
cleanup:
  free(url);
  free(auth);
  free(content);
  cJSON_Delete(root);
  return err;
}

llm_error_t llm_internal_openai_generate(const llm_t* llm,
                                         const llm_request_t* req,
                                         const char* extra_body,
                                         llm_tools_callback_t** tools_callback,
                                         int* tools_count,
                                         llm_token_usage_t* out_usage) {
  llm_error_t err = LLM_ERR_BACKEND;
  if (llm == NULL || req == NULL) {
    return LLM_ERR_INVALID_PARAM;
  }
  if (llm->type != LLM_TYPE_OPENAI) {
    return LLM_ERR_INVALID_PARAM;
  }
  if (llm->api_url == NULL || llm->model == NULL || llm->api_key == NULL ||
      req->messages == NULL) {
    return LLM_ERR_INVALID_PARAM;
  }
  char* url = build_url(llm->api_url, "chat/completions");

  int status_code = 0;
  size_t web_output_len = 0;
  char* content = NULL;
  cJSON* root = cJSON_CreateObject();
  if (root == NULL) {
    err = LLM_ERR_MEMORY;
    goto cleanup;
  }
  // 构造请求内容
  cJSON* messages_array = cJSON_CreateArray();
  cJSON_AddItemToObject(root, "messages", messages_array);
  llm_message_node_t* node = req->messages->head;
  while (node != NULL) {
    cJSON* message_obj = cJSON_CreateObject();
    cJSON_AddItemToArray(messages_array, message_obj);
    cJSON_AddStringToObject(message_obj, "role", node->message->role);
    cJSON_AddStringToObject(message_obj, "content", node->message->content);
    if (node->message->tool_call_id != NULL) {
      cJSON_AddStringToObject(message_obj, "tool_call_id",
                              node->message->tool_call_id);
    }
    if (node->message->tool_calls != NULL) {
      cJSON* tc = cJSON_Parse(node->message->tool_calls);
      if (tc) {
        cJSON_AddItemToObject(message_obj, "tool_calls", tc);
      } else {
        cJSON_AddStringToObject(message_obj, "tool_calls",
                                node->message->tool_calls);
      }
    }
    if (node->message->reasoning_content != NULL) {
      cJSON_AddStringToObject(message_obj, "reasoning_content",
                              node->message->reasoning_content);
    }
    node = node->next;
  }
  cJSON_AddStringToObject(root, "model", llm->model);
  if (req->reasoning_effort != NULL) {
    cJSON_AddStringToObject(root, "reasoning_effort", req->reasoning_effort);
  } else {
    cJSON_AddNullToObject(root, "reasoning_effort");
  }
  cJSON_AddNumberToObject(root, "max_tokens", req->max_tokens);
  if (req->response_format != NULL) {
    cJSON* response_format = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "response_format", response_format);
    cJSON_AddStringToObject(response_format, "type", req->response_format);
  } else {
    cJSON_AddNullToObject(root, "response_format");
  }
  cJSON_AddNullToObject(root, "stop");
  cJSON_AddBoolToObject(root, "stream", 0);       // false
  cJSON_AddNullToObject(root, "stream_options");  // null
  cJSON_AddNumberToObject(root, "temperature", req->temperature);
  cJSON_AddNumberToObject(root, "top_p", req->top_p);
  if (s_tools == NULL || strcmp(req->tool_choice, "none") == 0) {
    cJSON_AddNullToObject(root, "tools");
  } else {
    cJSON* tools_array = cJSON_CreateArray();
    cJSON_AddItemToObject(root, "tools", tools_array);
    for (llm_tool_t* t = s_tools; t != NULL; t = t->next) {
      cJSON* item = cJSON_Parse(t->json);
      if (!item) {
        // 解析失败处理
        const char* error = cJSON_GetErrorPtr();
        err = LLM_ERR_JSON;
        goto cleanup;
      }

      cJSON_AddItemToArray(tools_array, item);
    }
  }
  if (req->tool_choice != NULL) {
    cJSON* tc = cJSON_Parse(req->tool_choice);
    if (tc != NULL) {
      cJSON_AddItemToObject(root, "tool_choice", tc);
    } else {
      cJSON_AddStringToObject(root, "tool_choice", req->tool_choice);
    }
  }
  cJSON_AddBoolToObject(root, "logprobs", 0);
  cJSON_AddNullToObject(root, "top_logprobs");
  // 增加extra_body中的字段（如果有的话），同名字段会被覆盖
  if (extra_body && extra_body[0] != '\0') {
    cJSON* extra = cJSON_Parse(extra_body);
    if (extra) {
      cJSON* child = extra->child;
      while (child) {
        cJSON* next = child->next;
        cJSON_DetachItemViaPointer(extra, child);
        cJSON_DeleteItemFromObject(root, child->string);
        cJSON_AddItemToObject(root, child->string, child);
        child = next;
      }
      cJSON_Delete(extra);
    }
  }
  char* json_str = cJSON_Print(root);
  // 发送请求
  char* auth = llm_internal_build_auth_header(llm->api_key);
  if (auth == NULL) {
    err = LLM_ERR_MEMORY;
    goto cleanup;
  }
  char* header[3];
  header[0] = "Content-Type: application/json";
  header[1] = "Accept: application/json";
  header[2] = auth;
  CURLcode rc = llm_internal_http_post(url, header, 3, json_str, &status_code,
                                       &content, &web_output_len);
  char* new_val = content ? strdup(content) : NULL;
  char* old_val =
      __atomic_exchange_n(&llm_last_request, new_val, __ATOMIC_SEQ_CST);
  free(old_val);
  // 解析返回内容
  cJSON* content_root = NULL;
  llm_message_node_t* msg = NULL;
  if (rc == CURLE_OK && content != NULL) {
    content_root = cJSON_Parse(content);
    if (content_root != NULL) {
      cJSON* choice_array =
          cJSON_GetObjectItemCaseSensitive(content_root, "choices");
      if (cJSON_IsArray(choice_array) && cJSON_GetArraySize(choice_array) > 0) {
        cJSON* first_choice = cJSON_GetArrayItem(choice_array, 0);
        cJSON* message =
            cJSON_GetObjectItemCaseSensitive(first_choice, "message");
        if (message != NULL) {
          cJSON* content = cJSON_GetObjectItemCaseSensitive(message, "content");
          cJSON* role = cJSON_GetObjectItemCaseSensitive(message, "role");
          if (content != NULL && cJSON_IsString(content) &&
              content->valuestring != NULL && role != NULL &&
              cJSON_IsString(role) && role->valuestring != NULL) {
            msg = llm_message_list_create_node(role->valuestring,
                                               content->valuestring);
            if (msg != NULL) {
              llm_message_list_push_back(req->messages, msg);
              err = LLM_OK;
            }
          }
          cJSON* tool_calls_array =
              cJSON_GetObjectItemCaseSensitive(message, "tool_calls");
          if (tool_calls_array != NULL && cJSON_IsArray(tool_calls_array)) {
            int tool_calls_count = cJSON_GetArraySize(tool_calls_array);
            *tools_count = tool_calls_count;
            if (tool_calls_count > 0) {
              *tools_callback = calloc((size_t)tool_calls_count,
                                       sizeof(llm_tools_callback_t));
              if (*tools_callback == NULL) {
                err = LLM_ERR_MEMORY;
                goto cleanup;
              }
              for (int i = 0; i < tool_calls_count; i++) {
                cJSON* tool_call = cJSON_GetArrayItem(tool_calls_array, i);
                cJSON* tool =
                    cJSON_GetObjectItemCaseSensitive(tool_call, "function");
                cJSON* tool_name =
                    cJSON_GetObjectItemCaseSensitive(tool, "name");
                cJSON* tool_args =
                    cJSON_GetObjectItemCaseSensitive(tool, "arguments");

                cJSON* id = cJSON_GetObjectItemCaseSensitive(tool_call, "id");
                if (tool_name != NULL && cJSON_IsString(tool_name) &&
                    tool_name->valuestring != NULL && tool_args != NULL) {
                  // 查找工具，记录回调
                  llm_tool_t* t =
                      llm_internal_tool_find(tool_name->valuestring);
                  if (t != NULL) {
                    tools_callback[i]->callback = t->callback;
                    tools_callback[i]->args_json =
                        strdup(tool_args->valuestring);
                    tools_callback[i]->tool_call_id = strdup(id->valuestring);
                  }
                }
              }
              msg->message->tool_calls = cJSON_Print(tool_calls_array);
              if (cJSON_GetObjectItemCaseSensitive(
                      message, "reasoning_content") != NULL) {
                msg->message->reasoning_content =
                    strdup(cJSON_GetObjectItemCaseSensitive(message,
                                                            "reasoning_content")
                               ->valuestring);
              } else {
                msg->message->reasoning_content = "";
              }
            }
          }
        }
      }
      // 记录token使用量
      cJSON* usage = cJSON_GetObjectItemCaseSensitive(content_root, "usage");
      if (usage != NULL) {
        cJSON* prompt_tokens =
            cJSON_GetObjectItemCaseSensitive(usage, "prompt_tokens");
        cJSON* completion_tokens =
            cJSON_GetObjectItemCaseSensitive(usage, "completion_tokens");
        cJSON* total_tokens =
            cJSON_GetObjectItemCaseSensitive(usage, "total_tokens");
        if (cJSON_IsNumber(prompt_tokens) &&
            cJSON_IsNumber(completion_tokens) && cJSON_IsNumber(total_tokens)) {
          out_usage->prompt_tokens = prompt_tokens->valueint;
          out_usage->completion_tokens = completion_tokens->valueint;
          out_usage->total_tokens = total_tokens->valueint;
        }
      }
    }
  cleanup:
    free(url);
    free(auth);
    free(content);
    cJSON_Delete(root);
    cJSON_Delete(content_root);
    free(json_str);
    return err;
  }
}