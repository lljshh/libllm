// SPDX-FileCopyrightText: 2026 Kamov
//
// SPDX-License-Identifier: Apache-2.0

#include "libllm.h"

#include <stdlib.h>
#include <string.h>

#include "libllm_internal.h"

_Atomic(char*) llm_last_request = NULL;
// 注册表
const llm_provider_t llm_providers[] = {
    {LLM_TYPE_OPENAI, llm_internal_openai_get_models,
     llm_internal_openai_generate},
    {LLM_TYPE_ANTHROPIC, llm_internal_anthropic_get_models,
     llm_internal_anthropic_generate},
    {(llm_type_t)2, NULL, NULL} /* sentinel */
};
int llm_internal_provider_count(void) {
  int count = 0;
  while (llm_providers[count].generate != NULL) {
    count++;
  }
  return count;
}

static const llm_provider_t* find_provider(llm_type_t type) {
  if (llm_providers[type].generate == NULL) {
    return NULL;
  }
  return llm_providers + type;
}

llm_error_t get_models(const char* base_url, const char* api_key,
                       llm_type_t type, int* out_len, llm_t*** out_models) {
  if (base_url == NULL || api_key == NULL) {
    return LLM_ERR_INVALID_PARAM;
  }

  const llm_provider_t* provider = find_provider(type);
  if (provider == NULL) {
    return LLM_ERR_INVALID_PARAM;
  }

  if (provider->get_models == NULL) {
    return LLM_ERR_BACKEND;
  }

  return provider->get_models(base_url, api_key, out_len, out_models);
}

llm_error_t generate(const llm_t* llm, const llm_request_t* req,
                     const char* extra_body,
                     llm_tools_callback_t** tools_callback, int* tools_count,
                     llm_token_usage_t* out_usage) {
  if (llm == NULL || req == NULL) {
    return LLM_ERR_INVALID_PARAM;
  }

  const llm_provider_t* provider = find_provider(llm->type);
  if (provider == NULL) {
    return LLM_ERR_INVALID_PARAM;
  }

  return provider->generate(llm, req, extra_body, tools_callback, tools_count,
                            out_usage);
}

llm_t* llm_new(llm_type_t type, const char* api_key, const char* model,
               const char* api_url) {
  if (api_key == NULL || model == NULL) {
    return NULL;
  }

  if (find_provider(type) == NULL) {
    return NULL;
  }

  llm_t* llm = (llm_t*)malloc(sizeof(llm_t));
  if (llm == NULL) {
    return NULL;
  }

  llm->api_key = strdup(api_key);
  if (llm->api_key == NULL) {
    free(llm);
    return NULL;
  }

  llm->model = strdup(model);
  if (llm->model == NULL) {
    free(llm->api_key);
    free(llm);
    return NULL;
  }

  llm->api_url = api_url ? strdup(api_url) : NULL;
  llm->type = type;

  return llm;
}

void llm_free(llm_t* llm) {
  if (llm == NULL) return;
  free(llm->api_key);
  free(llm->model);
  free(llm->api_url);
  free(llm);
}

void llm_set_api_key(llm_t* llm, const char* api_key) {
  if (llm == NULL || api_key == NULL) return;
  free(llm->api_key);
  llm->api_key = strdup(api_key);
}

const char* llm_get_api_key(const llm_t* llm) {
  if (llm == NULL) return NULL;
  return llm->api_key;
}

void llm_set_model_name(llm_t* llm, const char* model) {
  if (llm == NULL || model == NULL) return;
  free(llm->model);
  llm->model = strdup(model);
}

const char* llm_get_model_name(const llm_t* llm) {
  if (llm == NULL) return NULL;
  return llm->model;
}

void llm_set_api_url(llm_t* llm, const char* api_url) {
  if (llm == NULL) return;
  free(llm->api_url);
  llm->api_url = api_url ? strdup(api_url) : NULL;
}

const char* llm_get_api_url(const llm_t* llm) {
  if (llm == NULL) return NULL;
  return llm->api_url;
}

void llm_set_type(llm_t* llm, llm_type_t type) {
  if (llm == NULL) return;
  llm->type = type;
}

llm_type_t llm_get_type(const llm_t* llm) {
  if (llm == NULL) return (llm_type_t)0;
  return llm->type;
}

llm_request_t* llm_request_new(void) {
  llm_request_t* req = malloc(sizeof(llm_request_t));
  if (req == NULL) return NULL;

  req->messages = llm_message_list_new();
  req->temperature = 0.0f;
  req->top_p = 0.0f;
  req->max_tokens = 0;
  req->reasoning_effort = NULL;
  req->tool_choice = NULL;
  req->response_format = NULL;

  return req;
}

void llm_request_free(llm_request_t* req) {
  if (req == NULL) return;
  llm_message_list_free(req->messages);
  free(req->reasoning_effort);
  free(req->tool_choice);
  free(req->response_format);
  free(req);
}

void llm_request_set_messages(llm_request_t* req,
                              llm_message_list_t* messages) {
  if (req == NULL) return;
  req->messages = messages;
}

void llm_request_set_temperature(llm_request_t* req, float temperature) {
  if (req == NULL) return;
  req->temperature = temperature;
}

void llm_request_set_top_p(llm_request_t* req, float top_p) {
  if (req == NULL) return;
  req->top_p = top_p;
}

void llm_request_set_max_tokens(llm_request_t* req, int max_tokens) {
  if (req == NULL) return;
  req->max_tokens = max_tokens;
}

void llm_request_set_reasoning_effort(llm_request_t* req,
                                      const char* reasoning_effort) {
  if (req == NULL) return;
  free(req->reasoning_effort);
  req->reasoning_effort = reasoning_effort ? strdup(reasoning_effort) : NULL;
}

void llm_request_set_tool_choice(llm_request_t* req, const char* tool_choice) {
  if (req == NULL) return;
  free(req->tool_choice);
  req->tool_choice = tool_choice ? strdup(tool_choice) : NULL;
}

llm_message_list_t* llm_request_get_messages(const llm_request_t* req) {
  if (req == NULL) return NULL;
  return req->messages;
}

float llm_request_get_temperature(const llm_request_t* req) {
  if (req == NULL) return 0.0f;
  return req->temperature;
}

float llm_request_get_top_p(const llm_request_t* req) {
  if (req == NULL) return 0.0f;
  return req->top_p;
}

int llm_request_get_max_tokens(const llm_request_t* req) {
  if (req == NULL) return 0;
  return req->max_tokens;
}

const char* llm_request_get_thinking_budget(const llm_request_t* req) {
  if (req == NULL) return NULL;
  return req->reasoning_effort;
}

const char* llm_request_get_tool_choice(const llm_request_t* req) {
  if (req == NULL) return NULL;
  return req->tool_choice;
}

llm_tool_t* s_tools = NULL;

llm_tool_t* llm_internal_tool_find(const char* name) {
  if (name == NULL) return NULL;
  for (llm_tool_t* t = s_tools; t != NULL; t = t->next) {
    if (strcmp(t->name, name) == 0) return t;
  }
  return NULL;
}

int llm_tool_register(const char* name, const char* json,
                      llm_tool_callback callback) {
  if (name == NULL || callback == NULL) return -1;

  // 重名检查
  if (llm_internal_tool_find(name) != NULL) return -1;

  llm_tool_t* tool = malloc(sizeof(llm_tool_t));
  if (tool == NULL) return -1;

  tool->name = strdup(name);
  tool->json = json ? strdup(json) : NULL;
  tool->callback = callback;
  tool->next = s_tools;
  s_tools = tool;

  return 0;
}

int llm_tool_unregister(const char* name) {
  if (name == NULL) return -1;

  llm_tool_t** pp = &s_tools;
  while (*pp != NULL) {
    if (strcmp((*pp)->name, name) == 0) {
      llm_tool_t* victim = *pp;
      *pp = victim->next;
      free(victim->name);
      free(victim->json);
      free(victim);
      return 0;
    }
    pp = &(*pp)->next;
  }
  return -1;
}

void llm_tools_callback_free(llm_tools_callback_t* callbacks, int count) {
  if (callbacks == NULL) return;
  for (int i = 0; i < count; i++) {
    free(callbacks[i].args_json);
    free(callbacks[i].tool_call_id);
  }
  free(callbacks);
}
