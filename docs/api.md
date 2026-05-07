# libllm API 参考文档

> 版本: 0.2.0  
> 语言: C11  
> 许可证: Apache2.0  
> 本Docs由AI生成辅以人类修正，可能与实际存在出入。

---

## 目录

1. [数据类型](#1-数据类型)
   - [1.1 llm_type_t](#11-llm_type_t)
   - [1.2 llm_error_t](#12-llm_error_t)
   - [1.3 llm_t](#13-llm_t)
   - [1.4 llm_request_t](#14-llm_request_t)
   - [1.5 llm_tool_callback](#15-llm_tool_callback)
   - [1.6 llm_tools_callback_t](#16-llm_tools_callback_t)
   - [1.7 llm_token_usage_t](#17-llm_token_usage_t)
   - [1.8 llm_message_t](#18-llm_message_t)
   - [1.9 llm_message_node_t](#19-llm_message_node_t)
   - [1.10 llm_message_list_t](#110-llm_message_list_t)
   - [1.11 llm_tool_t](#111-llm_tool_t)
2. [全局变量](#2-全局变量)
   - [2.1 llm_last_request](#21-llm_last_request)
3. [LLM 实例管理](#3-llm-实例管理)
   - [3.1 llm_new](#31-llm_new)
   - [3.2 llm_free](#32-llm_free)
   - [3.3 llm_set_api_key / llm_get_api_key](#33-llm_set_api_key--llm_get_api_key)
   - [3.4 llm_set_model_name / llm_get_model_name](#34-llm_set_model_name--llm_get_model_name)
   - [3.5 llm_set_api_url / llm_get_api_url](#35-llm_set_api_url--llm_get_api_url)
   - [3.6 llm_set_type / llm_get_type](#36-llm_set_type--llm_get_type)
4. [LLM API 调用](#4-llm-api-调用)
   - [4.1 generate](#41-generate)
   - [4.2 get_models](#42-get_models)
5. [请求对象管理](#5-请求对象管理)
   - [5.1 llm_request_new](#51-llm_request_new)
   - [5.2 llm_request_free](#52-llm_request_free)
6. [请求参数设置](#6-请求参数设置)
   - [6.1 llm_request_set_messages](#61-llm_request_set_messages)
   - [6.2 llm_request_get_messages](#62-llm_request_get_messages)
   - [6.3 llm_request_set_temperature](#63-llm_request_set_temperature)
   - [6.4 llm_request_get_temperature](#64-llm_request_get_temperature)
   - [6.5 llm_request_set_top_p](#65-llm_request_set_top_p)
   - [6.6 llm_request_get_top_p](#66-llm_request_get_top_p)
   - [6.7 llm_request_set_max_tokens](#67-llm_request_set_max_tokens)
   - [6.8 llm_request_get_max_tokens](#68-llm_request_get_max_tokens)
   - [6.9 llm_request_set_thinking_budget](#69-llm_request_set_thinking_budget)
   - [6.10 llm_request_get_thinking_budget](#610-llm_request_get_thinking_budget)
   - [6.11 llm_request_set_tool_choice](#611-llm_request_set_tool_choice)
   - [6.12 llm_request_get_tool_choice](#612-llm_request_get_tool_choice)
7. [消息列表管理](#7-消息列表管理)
   - [7.1 llm_message_list_new](#71-llm_message_list_new)
   - [7.2 llm_message_list_free](#72-llm_message_list_free)
   - [7.3 llm_message_list_create_node](#73-llm_message_list_create_node)
   - [7.4 llm_message_list_create_node_tool_call_id](#74-llm_message_list_create_node_tool_call_id)
   - [7.5 llm_message_list_create_node_tool_calls](#75-llm_message_list_create_node_tool_calls)
   - [7.6 llm_message_node_free](#76-llm_message_node_free)
   - [7.7 llm_message_free](#77-llm_message_free)
   - [7.8 llm_message_list_push_front](#78-llm_message_list_push_front)
   - [7.9 llm_message_list_push_back](#79-llm_message_list_push_back)
   - [7.10 llm_message_list_insert_after](#710-llm_message_list_insert_after)
   - [7.11 llm_message_list_insert_before](#711-llm_message_list_insert_before)
   - [7.12 llm_message_list_remove](#712-llm_message_list_remove)
   - [7.13 llm_message_list_get_head](#713-llm_message_list_get_head)
   - [7.14 llm_message_list_get_tail](#714-llm_message_list_get_tail)
   - [7.15 llm_message_list_get_count](#715-llm_message_list_get_count)
8. [工具（Tool）管理](#8-工具tool管理)
   - [8.1 llm_tool_register](#81-llm_tool_register)
   - [8.2 llm_tool_unregister](#82-llm_tool_unregister)
9. [错误码](#9-错误码)
10. [使用示例](#10-使用示例)

---

## 1. 数据类型

### 1.1 llm_type_t

```c
typedef enum {
    LLM_TYPE_OPENAI   = 0,   // OpenAI 兼容 API
    LLM_TYPE_ANTHROPIC = 1,  // Anthropic API
} llm_type_t;
```

**说明**: 用于指定 LLM 后端的枚举类型。创建 `llm_t` 实例时需传入此值以选择对应的 provider。

**定义位置**: `include/libllm.h`

---

### 1.2 llm_error_t

```c
typedef enum {
    LLM_OK                 = 0,   // 成功
    LLM_ERR_INVALID_PARAM  = -1,  // 无效参数
    LLM_ERR_MEMORY         = -2,  // 内存分配失败
    LLM_ERR_BACKEND        = -3,  // 后端处理错误
    LLM_ERR_NETWORK        = -4,  // 网络通信错误
    LLM_ERR_JSON           = -5,  // JSON 解析错误
    LLM_ERR_AUTH           = -6,  // 认证错误
} llm_error_t;
```

**说明**: 所有公开 API 函数返回的错误码枚举。

**定义位置**: `include/libllm.h`

---

### 1.3 llm_t

```c
// 不完整类型（对外隐藏实现细节）
typedef struct llm_s llm_t;
```

**说明**: LLM 实例的不完整类型。表示一个已配置的 LLM 后端实例，包含 API Key、模型名称、API URL 和类型等信息。只能通过 `llm_new()` 创建，通过 `llm_free()` 销毁。

**内部定义**（src/libllm_internal.h）:
```c
struct llm_s {
    char *api_key;      // API 密钥
    char *model;        // 模型名称（如 "gpt-4o"）
    char *api_url;      // API 基础 URL（如 "https://api.openai.com/v1"）
    llm_type_t type;    // 后端类型
};
```

> **注意**: 内部结构对外部用户不可见，用户只能通过公开 API 操作实例。

**定义位置**: `include/libllm.h`

---

### 1.4 llm_request_t

```c
// 不完整类型（对外隐藏实现细节）
typedef struct llm_request_s llm_request_t;
```

**说明**: 请求参数的不完整类型。封装了生成请求所需的所有参数：消息列表、温度、top_p、max_tokens、reasoning_effort 和 tool_choice。

**内部定义**（src/libllm_internal.h）:
```c
struct llm_request_s {
    llm_message_list_t *messages;      // 消息列表
    float temperature;                  // 温度参数 (0.0 ~ 2.0)
    float top_p;                        // Top-p 采样参数 (0.0 ~ 1.0)
    int max_tokens;                     // 最大生成 Token 数
    char *reasoning_effort;            // 推理强度（如 "low"/"medium"/"high"），NULL 表示禁用
    char *tool_choice;                 // 工具选择策略
};
```

**说明**: 注意 `reasoning_effort` 字段替代了旧版的 `thinking_budget`。`response_format` 字段已被移除。

**定义位置**: `include/libllm.h`

---

### 1.5 llm_tool_callback

```c
typedef char* (*llm_tool_callback)(const llm_request_t *req, const char *args_json);
```

**说明**: 工具回调函数类型。当 LLM 请求调用某个工具时，此函数被触发。

| 参数 | 类型 | 说明 |
|------|------|------|
| `req` | `const llm_request_t *` | 当前请求上下文（只读） |
| `args_json` | `const char *` | 工具调用参数的 JSON 字符串 |

| 返回值 | 说明 |
|--------|------|
| `char *` | 工具执行结果字符串（由调用者 free） |

**定义位置**: `include/libllm.h`

---

### 1.6 llm_tools_callback_t

```c
typedef struct {
    llm_tool_callback callback;
    char* args_json;
    char* tool_call_id;
} llm_tools_callback_t;
```

**说明**: `generate()` 函数通过该结构体返回工具调用信息。当 LLM 返回工具调用时，`tools_callback` 数组中的每个元素包含：
- `callback`: 已注册工具的对应回调函数
- `args_json`: LLM 生成的工具调用参数 JSON 字符串
- `tool_call_id`: 工具调用唯一标识符（用于后续将 tool result 关联回原始调用）

**定义位置**: `include/libllm.h`

---

### 1.7 llm_token_usage_t

```c
typedef struct {
    int prompt_tokens;
    int completion_tokens;
    int total_tokens;
} llm_token_usage_t;
```

**说明**: Token 用量统计，通过 `generate()` 的 `out_usage` 参数返回。

| 字段 | 说明 |
|------|------|
| `prompt_tokens` | 提示（输入）消耗的 Token 数 |
| `completion_tokens` | 生成（输出）消耗的 Token 数 |
| `total_tokens` | 总消耗 Token 数 |

**定义位置**: `include/libllm.h`

---

### 1.8 llm_message_t

```c
typedef struct {
    char *role;             // 消息角色（如 "system"、"user"、"assistant"、"tool"）
    char *content;          // 消息内容
    char *tool_call_id;     // 工具调用 ID（用于 tool 角色消息）
    char *tool_calls;       // 工具调用信息 JSON（用于 assistant 角色消息）
    char *reasoning_content; // 推理/思考内容（某些模型输出）
} llm_message_t;
```

**说明**: 表示一条对话消息。新字段说明：
- `tool_call_id`: 当角色为 `"tool"` 时，此字段标识该结果对应的工具调用
- `tool_calls`: 当角色为 `"assistant"` 时，此字段包含 LLM 返回的工具调用 JSON
- `reasoning_content`: 部分模型（如 DeepSeek-R1）的推理过程内容

**定义位置**: `include/message_list.h`

---

### 1.9 llm_message_node_t

```c
typedef struct llm_message_node {
    llm_message_t *message;              // 消息数据
    llm_message_node_t *next;            // 后继节点指针
    llm_message_node_t *prev;            // 前驱节点指针
} llm_message_node_t;
```

**说明**: 双向链表中的节点，包装了一条 `llm_message_t` 消息。

**定义位置**: `include/message_list.h`

---

### 1.10 llm_message_list_t

```c
// 不完整类型
typedef struct llm_message_list_s llm_message_list_t;
```

**说明**: 消息双向链表的不完整类型。包含头指针、尾指针和节点计数。

**内部定义**（src/libllm_internal.h）:
```c
struct llm_message_list_s {
    llm_message_node_t *head;    // 链表头节点
    llm_message_node_t *tail;    // 链表尾节点
    int count;                   // 节点总数
};
```

**定义位置**: `include/message_list.h`

---

### 1.11 llm_tool_t

```c
// 不完整类型
typedef struct llm_tool_s llm_tool_t;
```

**说明**: 表示已注册工具的不完整类型。通过 `llm_tool_register()` 创建，通过 `llm_tool_unregister()` 销毁。内部实现为单向链表节点。

**内部定义**（src/libllm_internal.h）:
```c
struct llm_tool_s {
    char *name;              // 工具名称
    char *json;              // JSON Schema 描述
    llm_tool_callback callback;  // 工具回调
    struct llm_tool_s *next; // 链表下一节点
};
```

**定义位置**: `include/libllm.h`

---

## 2. 全局变量

### 2.1 llm_last_request

```c
extern _Atomic(char*) llm_last_request;
```

**说明**: 全局原子指针，保存最近一次 HTTP 请求的完整 JSON 请求体。用于调试和日志记录。

- 每次调用 `generate()` 时更新
- 使用 `_Atomic` 保证线程安全
- 返回的字符串由内部管理，**不要**手动 free

**示例**:
```c
llm_error_t err = generate(llm, req, NULL, &tc, &tc_count, &usage);
if (err != LLM_OK) {
    fprintf(stderr, "Last request: %s\n", llm_last_request);
}
```

---

## 3. LLM 实例管理

### 3.1 llm_new

```c
llm_t* llm_new(llm_type_t type, const char *api_key, const char *model, const char *api_url);
```

**说明**: 创建一个新的 LLM 实例。

| 参数 | 类型 | 说明 |
|------|------|------|
| `type` | `llm_type_t` | 后端类型（`LLM_TYPE_OPENAI` 或 `LLM_TYPE_ANTHROPIC`） |
| `api_key` | `const char *` | API 密钥（不能为 NULL） |
| `model` | `const char *` | 模型名称（不能为 NULL，如 `"gpt-4o"`） |
| `api_url` | `const char *` | API 基础 URL（可为 NULL） |

| 返回值 | 说明 |
|--------|------|
| `llm_t *` | 成功时返回 LLM 实例指针 |
| `NULL` | 失败（参数无效或内存分配失败） |

**示例**:
```c
// OpenAI
llm_t *openai = llm_new(LLM_TYPE_OPENAI, "sk-xxx", "gpt-4o", "https://api.openai.com/v1");

// Anthropic（api_url 为 NULL 时使用默认地址）
llm_t *claude = llm_new(LLM_TYPE_ANTHROPIC, "sk-ant-xxx", "claude-sonnet-4-20250514", NULL);
```

**注意**: 成功创建的实例必须通过 `llm_free()` 释放。

---

### 3.2 llm_free

```c
void llm_free(llm_t *llm);
```

**说明**: 释放 LLM 实例占用的内存。

| 参数 | 类型 | 说明 |
|------|------|------|
| `llm` | `llm_t *` | 要释放的 LLM 实例（可为 NULL） |

**示例**:
```c
llm_t *llm = llm_new(LLM_TYPE_OPENAI, "sk-xxx", "gpt-4o", "https://api.openai.com/v1");
// ... 使用 llm ...
llm_free(llm);  // 释放资源
```

---

### 3.3 llm_set_api_key / llm_get_api_key

```c
void llm_set_api_key(llm_t *llm, const char *api_key);
const char* llm_get_api_key(const llm_t *llm);
```

**说明**: 设置/获取 LLM 实例的 API 密钥。

| 参数 | 类型 | 说明 |
|------|------|------|
| `llm` | `llm_t *` | LLM 实例 |
| `api_key` | `const char *` | 新的 API 密钥（不能为 NULL） |

| 返回值 | 说明 |
|--------|------|
| `const char *` | API 密钥字符串，若 `llm` 为 NULL 则返回 NULL |

**注意**: `llm_get_api_key()` 返回的指针指向内部数据，**不要**手动释放。

---

### 3.4 llm_set_model_name / llm_get_model_name

```c
void llm_set_model_name(llm_t *llm, const char *model);
const char* llm_get_model_name(const llm_t *llm);
```

**说明**: 设置/获取 LLM 实例的模型名称。

| 参数 | 类型 | 说明 |
|------|------|------|
| `llm` | `llm_t *` | LLM 实例 |
| `model` | `const char *` | 新的模型名称（不能为 NULL） |

| 返回值 | 说明 |
|--------|------|
| `const char *` | 模型名称字符串，若 `llm` 为 NULL 则返回 NULL |

**示例**:
```c
llm_set_model_name(llm, "gpt-4o-mini");
printf("当前模型: %s\n", llm_get_model_name(llm));
```

---

### 3.5 llm_set_api_url / llm_get_api_url

```c
void llm_set_api_url(llm_t *llm, const char *api_url);
const char* llm_get_api_url(const llm_t *llm);
```

**说明**: 设置/获取 LLM 实例的 API 基础 URL。

| 参数 | 类型 | 说明 |
|------|------|------|
| `llm` | `llm_t *` | LLM 实例 |
| `api_url` | `const char *` | 新的 API URL（可为 NULL） |

| 返回值 | 说明 |
|--------|------|
| `const char *` | API URL 字符串，若 `llm` 为 NULL 则返回 NULL |

---

### 3.6 llm_set_type / llm_get_type

```c
void llm_set_type(llm_t *llm, llm_type_t type);
llm_type_t llm_get_type(const llm_t *llm);
```

**说明**: 设置/获取 LLM 实例的后端类型。

| 参数 | 类型 | 说明 |
|------|------|------|
| `llm` | `llm_t *` | LLM 实例 |
| `type` | `llm_type_t` | 新的后端类型 |

| 返回值 | 说明 |
|--------|------|
| `llm_type_t` | 后端类型值，若 `llm` 为 NULL 则返回 `LLM_TYPE_OPENAI`(0) |

---

## 4. LLM API 调用

### 4.1 generate

```c
llm_error_t generate(const llm_t *llm, const llm_request_t *req,
                     const char *extra_body,
                     llm_tools_callback_t **tools_callback,
                     int *tools_count,
                     llm_token_usage_t *out_usage);
```

**说明**: 向 LLM 发起对话生成请求。生成的结果（回复消息）会被自动追加到 `req` 的消息列表末尾。

| 参数 | 类型 | 说明 |
|------|------|------|
| `llm` | `const llm_t *` | 已配置的 LLM 实例 |
| `req` | `const llm_request_t *` | 请求参数（包含消息列表和各项参数） |
| `extra_body` | `const char *` | 附加 JSON 字段（可为 NULL），用于传入 API 特有参数 |
| `tools_callback` | `llm_tools_callback_t **` | 输出：工具回调数组（当 LLM 返回工具调用时填充），不需要时传 NULL |
| `tools_count` | `int *` | 输出：工具回调数量，不需要时传 NULL |
| `out_usage` | `llm_token_usage_t *` | 输出：Token 用量统计，不需要时传 NULL |

| 返回值 | 说明 |
|--------|------|
| `LLM_OK` | 成功 |
| `LLM_ERR_INVALID_PARAM` | 参数无效 |
| `LLM_ERR_MEMORY` | 内存分配失败 |
| `LLM_ERR_BACKEND` | 后端错误 |
| `LLM_ERR_NETWORK` | 网络错误 |
| `LLM_ERR_JSON` | JSON 解析错误 |

**extra_body 说明**:

`extra_body` 参数允许传入 JSON 对象字符串，其中的所有顶层键值对会被合并到最终请求体中。这对于传入 `frequency_penalty`、`logit_bias` 等 OpenAI 或 Anthropic 特有参数非常有用。

**tools_callback 说明**:

当 LLM 返回工具调用时，`generate()` 会分配 `tools_callback` 数组。数组中的每个元素包含已注册工具的回调函数指针、参数和调用 ID。调用者应：
1. 遍历数组，调用每个 `callback` 执行工具
2. 使用 `tool_call_id` 和工具结果创建 `tool` 角色的消息节点（通过 `llm_message_list_create_node_tool_call_id()`）
3. 将工具结果消息追加到消息列表
4. 再次调用 `generate()` 将工具结果发送给 LLM

**示例**:
```c
llm_tools_callback_t *tools_callback = NULL;
int tools_count = 0;
llm_token_usage_t usage;

llm_error_t err = generate(llm, req, NULL, &tools_callback, &tools_count, &usage);
if (err == LLM_OK) {
    printf("Token usage: prompt=%d, completion=%d, total=%d\n",
           usage.prompt_tokens, usage.completion_tokens, usage.total_tokens);

    // 处理工具调用
    for (int i = 0; i < tools_count; i++) {
        char *result = tools_callback[i].callback(req, tools_callback[i].args_json);
        llm_message_node_t *tool_msg =
            llm_message_list_create_node_tool_call_id(
                "tool", result, tools_callback[i].tool_call_id);
        llm_message_list_push_back(llm_request_get_messages(req), tool_msg);
        free(result);
    }
    free(tools_callback);
}
```

---

### 4.2 get_models

```c
llm_error_t get_models(const char* base_url, const char* api_key, llm_type_t type, int *out_len, llm_t ***out_models);
```

**说明**: 从指定的 LLM API 获取可用模型列表。

| 参数 | 类型 | 说明 |
|------|------|------|
| `base_url` | `const char *` | API 基础 URL |
| `api_key` | `const char *` | API 密钥 |
| `type` | `llm_type_t` | 后端类型 |
| `out_len` | `int *` | 输出：模型数量 |
| `out_models` | `llm_t ***` | 输出：模型指针数组（每个元素是 `llm_t *`） |

| 返回值 | 说明 |
|--------|------|
| `LLM_OK` | 成功 |
| `LLM_ERR_*` | 各类错误码 |

**注意**:
- 返回的 `out_models` 数组中的每个 `llm_t *` 需通过 `llm_free()` 释放
- `out_models` 数组本身需通过 `free()` 释放
- 当前仅 OpenAI 后端实现了此功能，Anthropic 后端返回 `LLM_ERR_BACKEND`

**示例**:
```c
int count;
llm_t **models;
llm_error_t err = get_models(
    "https://api.openai.com/v1",
    "sk-xxx",
    LLM_TYPE_OPENAI,
    &count,
    &models
);

if (err == LLM_OK) {
    for (int i = 0; i < count; i++) {
        printf("模型: %s\n", llm_get_model_name(models[i]));
        llm_free(models[i]);
    }
    free(models);
}
```

---

## 5. 请求对象管理

### 5.1 llm_request_new

```c
llm_request_t* llm_request_new(void);
```

**说明**: 创建并初始化一个新的请求对象。默认参数如下：

| 字段 | 默认值 |
|------|--------|
| `messages` | 新创建的空消息列表（非 NULL） |
| `temperature` | `0.0f` |
| `top_p` | `0.0f` |
| `max_tokens` | `0` |
| `reasoning_effort` | `NULL` |
| `tool_choice` | `NULL` |

| 返回值 | 说明 |
|--------|------|
| `llm_request_t *` | 成功时返回请求对象指针 |
| `NULL` | 内存分配失败 |

**注意**:
- 必须通过 `llm_request_free()` 释放
- 自 v0.2.0 起，`messages` 字段初始化为新创建的空消息列表，不再为 NULL

---

### 5.2 llm_request_free

```c
void llm_request_free(llm_request_t *req);
```

**说明**: 释放请求对象。自 v0.2.0 起，此函数**会同时释放**关联的 `messages` 消息列表及其所有节点。

| 参数 | 类型 | 说明 |
|------|------|------|
| `req` | `llm_request_t *` | 要释放的请求对象（可为 NULL） |

**示例**:
```c
llm_request_t *req = llm_request_new();
// ... 配置 req 并使用 ...
llm_request_free(req);  // 自动释放 messages 及其所有节点
```

---

## 6. 请求参数设置

### 6.1 llm_request_set_messages

```c
void llm_request_set_messages(llm_request_t *req, llm_message_list_t *messages);
```

**说明**: 设置请求的消息列表。

| 参数 | 类型 | 说明 |
|------|------|------|
| `req` | `llm_request_t *` | 请求对象 |
| `messages` | `llm_message_list_t *` | 消息列表指针 |

**注意**: 请求对象不拥有消息列表的所有权，释放请求前需确保消息列表仍有效。若调用此函数替换了默认的消息列表，请自行管理原列表的生命周期。

---

### 6.2 llm_request_get_messages

```c
llm_message_list_t* llm_request_get_messages(const llm_request_t *req);
```

**说明**: 获取请求的消息列表。

| 返回值 | 说明 |
|--------|------|
| `llm_message_list_t *` | 消息列表指针，若 `req` 为 NULL 则返回 NULL |

---

### 6.3 llm_request_set_temperature

```c
void llm_request_set_temperature(llm_request_t *req, float temperature);
```

**说明**: 设置生成温度参数。较高的值（如 0.8）会使输出更随机，较低的值（如 0.2）使输出更确定。

| 参数 | 类型 | 说明 |
|------|------|------|
| `req` | `llm_request_t *` | 请求对象 |
| `temperature` | `float` | 温度值（通常 0.0 ~ 2.0） |

---

### 6.4 llm_request_get_temperature

```c
float llm_request_get_temperature(const llm_request_t *req);
```

**说明**: 获取当前的温度参数。

| 返回值 | 说明 |
|--------|------|
| `float` | 温度值，若 `req` 为 NULL 则返回 0.0f |

---

### 6.5 llm_request_set_top_p

```c
void llm_request_set_top_p(llm_request_t *req, float top_p);
```

**说明**: 设置 Top-p（核采样）参数。模型只考虑累积概率达到 top_p 的 token。

| 参数 | 类型 | 说明 |
|------|------|------|
| `req` | `llm_request_t *` | 请求对象 |
| `top_p` | `float` | 核采样参数（通常 0.0 ~ 1.0） |

---

### 6.6 llm_request_get_top_p

```c
float llm_request_get_top_p(const llm_request_t *req);
```

**说明**: 获取当前的 Top-p 参数。

| 返回值 | 说明 |
|--------|------|
| `float` | Top-p 值，若 `req` 为 NULL 则返回 0.0f |

---

### 6.7 llm_request_set_max_tokens

```c
void llm_request_set_max_tokens(llm_request_t *req, int max_tokens);
```

**说明**: 设置最大生成 Token 数。

| 参数 | 类型 | 说明 |
|------|------|------|
| `req` | `llm_request_t *` | 请求对象 |
| `max_tokens` | `int` | 最大 Token 数量 |

---

### 6.8 llm_request_get_max_tokens

```c
int llm_request_get_max_tokens(const llm_request_t *req);
```

**说明**: 获取当前的最大 Token 数。

| 返回值 | 说明 |
|--------|------|
| `int` | 最大 Token 数，若 `req` 为 NULL 则返回 0 |

---

### 6.9 llm_request_set_thinking_budget

```c
void llm_request_set_thinking_budget(llm_request_t *req, const char *reasoning_effort);
```

**说明**: 设置模型推理强度（原 thinking_budget）。此参数用于控制模型在给出回答前的"思考"深度。注意内部字段名已变更为 `reasoning_effort`。

| 参数 | 类型 | 说明 |
|------|------|------|
| `req` | `llm_request_t *` | 请求对象 |
| `reasoning_effort` | `const char *` | 推理强度字符串（如 "low"、"medium"、"high"），传递 NULL 表示不设置 |

**注意**: 函数内部会复制字符串，传入的字符串可在调用后被释放。

---

### 6.10 llm_request_get_thinking_budget

```c
const char* llm_request_get_thinking_budget(const llm_request_t *req);
```

**说明**: 获取当前设置的推理强度。

| 返回值 | 说明 |
|--------|------|
| `const char *` | 推理强度字符串，若未设置或 `req` 为 NULL 则返回 NULL |

**注意**: 返回的指针指向内部数据，**不要**手动释放。

---

### 6.11 llm_request_set_tool_choice

```c
void llm_request_set_tool_choice(llm_request_t *req, const char *tool_choice);
```

**说明**: 设置工具选择策略。控制模型是否及如何调用已注册的工具。

| 参数 | 类型 | 说明 |
|------|------|------|
| `req` | `llm_request_t *` | 请求对象 |
| `tool_choice` | `const char *` | 工具选择策略：`"auto"`、`"none"`、`"required"`，或工具名称的 JSON 字符串 |

**示例**:
```c
// 让模型自动选择是否调用工具
llm_request_set_tool_choice(req, "auto");

// 禁止调用工具
llm_request_set_tool_choice(req, "none");

// 强制调用特定工具
llm_request_set_tool_choice(req, "{\"type\":\"function\",\"function\":{\"name\":\"get_weather\"}}");
```

---

### 6.12 llm_request_get_tool_choice

```c
const char* llm_request_get_tool_choice(const llm_request_t *req);
```

**说明**: 获取当前的工具选择策略。

| 返回值 | 说明 |
|--------|------|
| `const char *` | 工具选择策略字符串，若未设置或 `req` 为 NULL 则返回 NULL |

**注意**: 返回的指针指向内部数据，**不要**手动释放。

---

## 7. 消息列表管理

### 7.1 llm_message_list_new

```c
llm_message_list_t* llm_message_list_new();
```

**说明**: 创建一个新的空消息双向链表。

| 返回值 | 说明 |
|--------|------|
| `llm_message_list_t *` | 成功时返回消息列表指针 |
| `NULL` | 内存分配失败 |

---

### 7.2 llm_message_list_free

```c
void llm_message_list_free(llm_message_list_t *list);
```

**说明**: 释放整个消息列表，包括所有节点及其包含的消息数据。

| 参数 | 类型 | 说明 |
|------|------|------|
| `list` | `llm_message_list_t *` | 要释放的消息列表（可为 NULL） |

---

### 7.3 llm_message_list_create_node

```c
llm_message_node_t* llm_message_list_create_node(const char *role, const char *content);
```

**说明**: 创建一个新的消息节点。内部会自动复制字符串。

| 参数 | 类型 | 说明 |
|------|------|------|
| `role` | `const char *` | 消息角色（如 "system"、"user"、"assistant"） |
| `content` | `const char *` | 消息内容 |

| 返回值 | 说明 |
|--------|------|
| `llm_message_node_t *` | 成功时返回消息节点指针 |
| `NULL` | 参数为 NULL 或内存分配失败 |

---

### 7.4 llm_message_list_create_node_tool_call_id

```c
llm_message_node_t* llm_message_list_create_node_tool_call_id(
    const char *role, const char *content, const char *tool_call_id);
```

**说明**: 创建一个带有 `tool_call_id` 的消息节点。用于创建 `tool` 角色的消息，将工具执行结果关联回原始调用。

| 参数 | 类型 | 说明 |
|------|------|------|
| `role` | `const char *` | 消息角色（通常为 "tool"） |
| `content` | `const char *` | 工具执行结果内容 |
| `tool_call_id` | `const char *` | 工具调用 ID（从 `llm_tools_callback_t` 获取） |

| 返回值 | 说明 |
|--------|------|
| `llm_message_node_t *` | 成功时返回消息节点指针 |
| `NULL` | 参数为 NULL 或内存分配失败 |

**示例**:
```c
// 从 generate() 获取工具调用结果后，创建 tool 消息
llm_message_node_t *tool_msg =
    llm_message_list_create_node_tool_call_id(
        "tool", tool_result, tools_callback[i].tool_call_id);
llm_message_list_push_back(llm_request_get_messages(req), tool_msg);
```

---

### 7.5 llm_message_list_create_node_tool_calls

```c
llm_message_node_t* llm_message_list_create_node_tool_calls(
    const char *role, const char *content, const char *tool_calls);
```

**说明**: 创建一个带有 `tool_calls` 信息的消息节点。用于手动构建包含工具调用的 assistant 消息。

| 参数 | 类型 | 说明 |
|------|------|------|
| `role` | `const char *` | 消息角色（通常为 "assistant"） |
| `content` | `const char *` | 消息内容 |
| `tool_calls` | `const char *` | 工具调用信息 JSON 字符串 |

| 返回值 | 说明 |
|--------|------|
| `llm_message_node_t *` | 成功时返回消息节点指针 |
| `NULL` | 参数为 NULL 或内存分配失败 |

---

### 7.6 llm_message_node_free

```c
void llm_message_node_free(llm_message_node_t *node);
```

**说明**: 释放单个消息节点及其包含的消息数据。

| 参数 | 类型 | 说明 |
|------|------|------|
| `node` | `llm_message_node_t *` | 要释放的节点（可为 NULL） |

**注意**: 此函数仅释放节点自身，不会从链表中移除节点。请先使用 `llm_message_list_remove()` 将节点从链表中移除。

---

### 7.7 llm_message_free

```c
void llm_message_free(llm_message_t *msg);
```

**说明**: 释放一条消息数据。

| 参数 | 类型 | 说明 |
|------|------|------|
| `msg` | `llm_message_t *` | 要释放的消息（可为 NULL） |

---

### 7.8 llm_message_list_push_front

```c
void llm_message_list_push_front(llm_message_list_t *list, llm_message_node_t *node);
```

**说明**: 将节点插入链表头部。

| 参数 | 类型 | 说明 |
|------|------|------|
| `list` | `llm_message_list_t *` | 目标链表 |
| `node` | `llm_message_node_t *` | 要插入的节点 |

**示例**:
```c
llm_message_list_t *list = llm_message_list_new();
llm_message_node_t *node = llm_message_list_create_node("user", "你好");
llm_message_list_push_front(list, node);
```

---

### 7.9 llm_message_list_push_back

```c
void llm_message_list_push_back(llm_message_list_t *list, llm_message_node_t *node);
```

**说明**: 将节点插入链表尾部。

| 参数 | 类型 | 说明 |
|------|------|------|
| `list` | `llm_message_list_t *` | 目标链表 |
| `node` | `llm_message_node_t *` | 要插入的节点 |

---

### 7.10 llm_message_list_insert_after

```c
void llm_message_list_insert_after(llm_message_node_t *after, llm_message_node_t *node, llm_message_list_t *list);
```

**说明**: 在指定节点之后插入新节点。

| 参数 | 类型 | 说明 |
|------|------|------|
| `after` | `llm_message_node_t *` | 参考节点（新节点将插入到此节点之后） |
| `node` | `llm_message_node_t *` | 要插入的节点 |
| `list` | `llm_message_list_t *` | 目标链表 |

---

### 7.11 llm_message_list_insert_before

```c
void llm_message_list_insert_before(llm_message_node_t *before, llm_message_node_t *node, llm_message_list_t *list);
```

**说明**: 在指定节点之前插入新节点。

| 参数 | 类型 | 说明 |
|------|------|------|
| `before` | `llm_message_node_t *` | 参考节点（新节点将插入到此节点之前） |
| `node` | `llm_message_node_t *` | 要插入的节点 |
| `list` | `llm_message_list_t *` | 目标链表 |

---

### 7.12 llm_message_list_remove

```c
void llm_message_list_remove(llm_message_node_t *node, llm_message_list_t *list);
```

**说明**: 从链表中移除指定节点。**注意**：此函数仅移除节点，不释放节点内存。需要调用 `llm_message_node_free()` 释放。

| 参数 | 类型 | 说明 |
|------|------|------|
| `node` | `llm_message_node_t *` | 要移除的节点 |
| `list` | `llm_message_list_t *` | 目标链表 |

**示例**:
```c
// 移除并释放指定消息
llm_message_list_remove(node, list);
llm_message_node_free(node);
```

---

### 7.13 llm_message_list_get_head

```c
llm_message_node_t* llm_message_list_get_head(const llm_message_list_t *list);
```

**说明**: 获取消息链表的头节点。

| 返回值 | 说明 |
|--------|------|
| `llm_message_node_t *` | 头节点指针，若链表为空或 `list` 为 NULL 则返回 NULL |

---

### 7.14 llm_message_list_get_tail

```c
llm_message_node_t* llm_message_list_get_tail(const llm_message_list_t *list);
```

**说明**: 获取消息链表的尾节点。

| 返回值 | 说明 |
|--------|------|
| `llm_message_node_t *` | 尾节点指针，若链表为空或 `list` 为 NULL 则返回 NULL |

---

### 7.15 llm_message_list_get_count

```c
int llm_message_list_get_count(const llm_message_list_t *list);
```

**说明**: 获取消息链表中的节点数量。

| 返回值 | 说明 |
|--------|------|
| `int` | 节点数量，若 `list` 为 NULL 则返回 0 |

---

## 8. 工具（Tool）管理

### 8.1 llm_tool_register

```c
int llm_tool_register(const char *name, const char *json, llm_tool_callback callback);
```

**说明**: 注册一个全局工具。注册后，`generate()` 请求会自动将工具信息嵌入请求体。

| 参数 | 类型 | 说明 |
|------|------|------|
| `name` | `const char *` | 工具名称（必须唯一） |
| `json` | `const char *` | 工具定义的 JSON Schema 字符串 |
| `callback` | `llm_tool_callback` | 工具回调函数 |

| 返回值 | 说明 |
|--------|------|
| `0` | 注册成功 |
| `-1` | 注册失败（参数无效、名称重复或内存不足） |

**JSON Schema 格式**:

`json` 参数应为符合 OpenAI function calling 格式的 JSON 字符串，例如：

```json
{
    "type": "function",
    "function": {
        "name": "get_weather",
        "description": "获取指定城市的天气信息",
        "parameters": {
            "type": "object",
            "properties": {
                "city": {
                    "type": "string",
                    "description": "城市名称"
                }
            },
            "required": ["city"]
        }
    }
}
```

**示例**:
```c
char *weather_callback(const llm_request_t *req, const char *args_json) {
    // 解析 args_json 获取参数
    // 执行工具逻辑
    return strdup("{\"weather\": \"晴天\", \"temperature\": 25}");
}

llm_tool_register("get_weather", weather_tool_json, weather_callback);
```

---

### 8.2 llm_tool_unregister

```c
int llm_tool_unregister(const char *name);
```

**说明**: 注销一个已注册的全局工具。释放工具占用的内存。

| 参数 | 类型 | 说明 |
|------|------|------|
| `name` | `const char *` | 要注销的工具名称 |

| 返回值 | 说明 |
|--------|------|
| `0` | 注销成功 |
| `-1` | 未找到指定名称的工具或参数为 NULL |

**示例**:
```c
llm_tool_unregister("get_weather");
```

---

## 9. 错误码

| 错误码 | 值 | 说明 | 常见原因 |
|--------|----|------|----------|
| `LLM_OK` | `0` | 操作成功 | - |
| `LLM_ERR_INVALID_PARAM` | `-1` | 参数无效 | 传入 NULL 或非法参数 |
| `LLM_ERR_MEMORY` | `-2` | 内存分配失败 | 系统内存不足 |
| `LLM_ERR_BACKEND` | `-3` | 后端处理错误 | API 返回错误、未实现的后端功能 |
| `LLM_ERR_NETWORK` | `-4` | 网络通信错误 | DNS 解析失败、连接超时、SSL 错误 |
| `LLM_ERR_JSON` | `-5` | JSON 解析错误 | 无效的 JSON 字符串 |
| `LLM_ERR_AUTH` | `-6` | 认证错误 | API Key 无效或过期 |

---

> 文档版本: 0.2.0 | 最后更新: 2026-05-07