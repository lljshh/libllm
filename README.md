<p align="right">
  <a href="README_EN.md">English</a>
</p>

# libllm

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![Build Status](https://img.shields.io/badge/build-passing-brightgreen)](https://github.com/your-repo/libllm)
[![C Standard](https://img.shields.io/badge/C-11-blue)](CMakeLists.txt)

**libllm** 是一个轻量级、跨平台的 C 语言库，用于与主流大语言模型（LLM）API 进行交互。目前支持 **OpenAI** 后端，提供统一的 API 接口。

---

## 特性

- ✅ **统一接口** — 任何后端均可通过 `get_models` 和 `generate` 获取模型列表和对话
- ✅ **消息管理** — 内置双向链表实现的消息列表，支持灵活的增删改查
- ✅ **工具调用（Tool Use）** — 支持全局注册/注销工具，自动嵌入请求
- ✅ **可扩展** — 新增后端只需实现 `get_models_f` 和 `generate_f` 函数指针
- ✅ **HTTP 封装** — 基于 libcurl，内置 SSL/TLS 验证、超时控制
- ✅ **JSON 解析** — 基于 cJSON，轻量高效
- ✅ **C11 标准** — 兼容 C11 及以上编译器，同时提供 C++ 友好头文件

---

## 构建

### 依赖

| 依赖 | 版本要求 | 用途 |
|------|---------|------|
| [CMake](https://cmake.org/) | >= 3.15 | 构建系统 |
| [libcurl](https://curl.se/libcurl/) | >= 7.58 | HTTP 通信 |
| [cJSON](https://github.com/DaveGamble/cJSON) | >= 1.7 | JSON 解析 |
| GCC / Clang | 支持 C11 | 编译器 |

### 编译安装

```bash
# 1. 克隆仓库
git clone https://github.com/lljshh/libllm.git
cd libllm

# 2. 创建构建目录
mkdir -p build && cd build

# 3. 配置
cmake ..

# 4. 编译
make -j$(nproc)

# 5. 安装（可选）
sudo make install
```

### CMake 选项

| 选项 | 默认值 | 说明 |
|------|--------|------|
| `BUILD_SHARED_LIBS` | `ON` | 构建共享库（.so） |
| `BUILD_TESTS` | `OFF` | 构建测试 |
| `BUILD_EXAMPLES` | `OFF` | 构建示例 |

---

## 快速上手（见example.c）

### 1. 初始化 LLM 实例

```c
#include "libllm.h"

// 获取LLM实例
llm_t** models = NULL;
get_models(argv[1], argv[2], LLM_TYPE_OPENAI, &models_count, &models);
```

### 2. 构建消息并请求生成

```c
// 创建消息列表
llm_message_list_t *messages = llm_message_list_new();

// 添加系统消息
llm_message_node_t *sys_msg = llm_message_list_create_node(
    "system", "你是一个有用的助手。"
);
llm_message_list_push_front(messages, sys_msg);

// 添加用户消息
llm_message_node_t *user_msg = llm_message_list_create_node(
    "user", "介绍一下 C 语言。"
);
llm_message_list_push_back(messages, user_msg);

// 创建请求
llm_request_t *req = llm_request_new();
llm_request_set_thinking_budget(req, "high");
llm_request_set_tool_choice(req, "auto");
llm_request_set_top_p(req, 0.2);
llm_request_set_max_tokens(req, 10000);

// 发起生成请求
llm_tools_callback_t* tools_callback = NULL;
int tools_count = 0;
llm_token_usage_t usage;
llm_error_t err = generate(llm, req, NULL, &tools_callback,
                               &tools_count, &usage);
if (err == LLM_OK) {
    printf("生成成功！\n");
}

// 查看回复（消息已被追加到 messages 末尾）
llm_message_node_t *reply = llm_message_list_get_tail(llm_request_get_messages(req));
printf("Assistant: %s\n", reply->message->content);

// 清理
llm_request_free(req);
llm_message_list_free(messages);
llm_free(llm);
```

### 3. 获取可用模型列表

```c
int model_count = 0;
llm_t **models = NULL;

llm_error_t err = get_models(
    "<base-url>",
    "<your-api-key>",
    LLM_TYPE_OPENAI,
    &model_count,
    &models
);

if (err == LLM_OK) {
    for (int i = 0; i < model_count; i++) {
        printf("模型: %s\n", llm_get_model_name(models[i]));
    }
}
```

### 4. 注册工具（Tool Use）

```c
// 工具 JSON Schema 字符串
const char* weather_tool_json =
    "{"
    "  \"type\": \"function\","
    "  \"function\": {"
    "    \"name\": \"get_weather\","
    "    \"description\": \"获取指定城市的天气\","
    "    \"parameters\": {"
    "      \"type\": \"object\","
    "      \"properties\": {"
    "        \"city\": { \"type\": \"string\", \"description\": \"城市名称\" }"
    "      },"
    "      \"required\": [\"city\"]"
    "    }"
    "  }"
    "}";

// 工具回调函数
char* weather_callback(const llm_request_t* req, const char* args_json) {
  // 解析 args_json 并返回结果
  return strdup("{\"weather\":\"晴天, 25°C\"}");
}

// 注册工具
llm_tool_register("get_weather", weather_tool_json, weather_callback);

// 使用后注销
llm_tool_unregister("get_weather");
```

---

## API 参考

完整的 API 文档请参阅 [docs/api.md](docs/api.md)。

### 核心函数速览

| 函数 | 说明 |
|------|------|
| `llm_new()` | 创建 LLM 实例 |
| `llm_free()` | 释放 LLM 实例 |
| `generate()` | 发起对话生成请求 |
| `get_models()` | 获取可用模型列表 |
| `llm_request_new()` | 创建请求对象 |
| `llm_request_free()` | 释放请求对象 |
| `llm_tool_register()` | 注册工具 |
| `llm_tool_unregister()` | 注销工具 |

### 错误码

| 错误码 | 值 | 说明 |
|--------|----|------|
| `LLM_OK` | `0` | 成功 |
| `LLM_ERR_INVALID_PARAM` | `-1` | 无效参数 |
| `LLM_ERR_MEMORY` | `-2` | 内存分配失败 |
| `LLM_ERR_BACKEND` | `-3` | 后端错误 |
| `LLM_ERR_NETWORK` | `-4` | 网络错误 |
| `LLM_ERR_JSON` | `-5` | JSON 解析错误 |
| `LLM_ERR_AUTH` | `-6` | 认证错误 |

---

## 错误处理

所有函数均返回 `llm_error_t` 枚举值。建议的检查方式：

```c
llm_error_t err = generate(llm, req, NULL);
if (err != LLM_OK) {
    fprintf(stderr, "生成请求失败，错误码: %d\n", err);
    // 根据错误码采取相应处理
}
```

---

## 线程安全

- 全局工具链表 (`s_tools`) 当前**未加锁**，建议在初始化阶段集中注册工具
- 其他对象（`llm_t`, `llm_request_t`, `llm_message_list_t`）非线程安全，请勿跨线程共享

---

## 贡献

欢迎贡献代码！请遵循以下步骤：

1. Fork 本仓库
2. 开启 Pull Request

### 开发约定

- 公开 API 使用 `LIBLLM_EXPORT` 导出
- 内部函数以 `llm_internal_` 为前缀
- 新增后端需在 `libllm_internal.h` 声明，并在 `libllm.c` 注册

---

## 许可

本项目基于 MIT 许可证开源。详见 [LICENSE](LICENSE) 文件。
