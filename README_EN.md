<p align="right">
  <a href="README.md">简体中文</a>
</p>

# libllm

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![Build Status](https://img.shields.io/badge/build-passing-brightgreen)](https://github.com/your-repo/libllm)
[![C Standard](https://img.shields.io/badge/C-11-blue)](CMakeLists.txt)

**libllm** is a lightweight, cross-platform C library for interacting with mainstream Large Language Model (LLM) APIs. Currently supports the **OpenAI** backend with a unified API interface.

---

## Features

- ✅ **Unified Interface** — Get model lists and conversations via `get_models` and `generate` for any backend
- ✅ **Message Management** — Built-in doubly linked list message list with flexible CRUD operations
- ✅ **Tool Use** — Support for global tool registration/unregistration, automatically embedded in requests
- ✅ **Extensible** — Add a new backend by simply implementing `get_models_f` and `generate_f` function pointers
- ✅ **HTTP Wrapper** — Based on libcurl with built-in SSL/TLS verification and timeout control
- ✅ **JSON Parsing** — Based on cJSON, lightweight and efficient
- ✅ **C11 Standard** — Compatible with C11 and later compilers, with C++ friendly header

---

## Build

### Dependencies

| Dependency | Version | Purpose |
|------------|---------|---------|
| [CMake](https://cmake.org/) | >= 3.15 | Build system |
| [libcurl](https://curl.se/libcurl/) | >= 7.58 | HTTP communication |
| [cJSON](https://github.com/DaveGamble/cJSON) | >= 1.7 | JSON parsing |
| GCC / Clang | C11 support | Compiler |

### Build & Install

```bash
# 1. Clone the repository
git clone https://github.com/lljshh/libllm.git
cd libllm

# 2. Create build directory
mkdir -p build && cd build

# 3. Configure
cmake ..

# 4. Build
make -j$(nproc)

# 5. Install (optional)
sudo make install
```

### CMake Options

| Option | Default | Description |
|--------|---------|-------------|
| `BUILD_SHARED_LIBS` | `ON` | Build shared library (.so) |
| `BUILD_TESTS` | `OFF` | Build tests |
| `BUILD_EXAMPLES` | `OFF` | Build examples |

---

## Quick Start (see example.c)

### 1. Initialize LLM Instance

```c
#include "libllm.h"

// Get LLM instance
llm_t** models = NULL;
get_models(argv[1], argv[2], LLM_TYPE_OPENAI, &models_count, &models);
```

### 2. Build Messages and Request Generation

```c
// Create message list
llm_message_list_t *messages = llm_message_list_new();

// Add system message
llm_message_node_t *sys_msg = llm_message_list_create_node(
    "system", "You are a helpful assistant."
);
llm_message_list_push_front(messages, sys_msg);

// Add user message
llm_message_node_t *user_msg = llm_message_list_create_node(
    "user", "Tell me about the C language."
);
llm_message_list_push_back(messages, user_msg);

// Create request
llm_request_t *req = llm_request_new();
llm_request_set_thinking_budget(req, "high");
llm_request_set_tool_choice(req, "auto");
llm_request_set_top_p(req, 0.2);
llm_request_set_max_tokens(req, 10000);

// Initiate generation request
llm_tools_callback_t* tools_callback = NULL;
int tools_count = 0;
llm_token_usage_t usage;
llm_error_t err = generate(llm, req, NULL, &tools_callback,
                               &tools_count, &usage);
if (err == LLM_OK) {
    printf("Generation successful!\n");
}

// View response (messages are appended to the end of the message list)
llm_message_node_t *reply = llm_message_list_get_tail(llm_request_get_messages(req));
printf("Assistant: %s\n", reply->message->content);

// Cleanup
llm_request_free(req);
llm_message_list_free(messages);
llm_free(llm);
```

### 3. List Available Models

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
        printf("Model: %s\n", llm_get_model_name(models[i]));
    }
}
```

### 4. Register Tools (Tool Use)

```c
// Tool JSON Schema string
const char* weather_tool_json =
    "{"
    "  \"type\": \"function\","
    "  \"function\": {"
    "    \"name\": \"get_weather\","
    "    \"description\": \"Get the weather for a specified city\","
    "    \"parameters\": {"
    "      \"type\": \"object\","
    "      \"properties\": {"
    "        \"city\": { \"type\": \"string\", \"description\": \"City name\" }"
    "      },"
    "      \"required\": [\"city\"]"
    "    }"
    "  }"
    "}";

// Tool callback function
char* weather_callback(const llm_request_t* req, const char* args_json) {
  // Parse args_json and return result
  return strdup("{\"weather\":\"Sunny, 25°C\"}");
}

// Register tool
llm_tool_register("get_weather", weather_tool_json, weather_callback);

// Unregister after use
llm_tool_unregister("get_weather");
```

---

## API Reference

Full API documentation is available in [docs/api.md](docs/api.md).

### Core Functions Overview

| Function | Description |
|----------|-------------|
| `llm_new()` | Create an LLM instance |
| `llm_free()` | Destroy an LLM instance |
| `generate()` | Initiate a conversation generation request |
| `get_models()` | Retrieve the list of available models |
| `llm_request_new()` | Create a request object |
| `llm_request_free()` | Free a request object |
| `llm_tool_register()` | Register a tool |
| `llm_tool_unregister()` | Unregister a tool |

### Error Codes

| Error Code | Value | Description |
|------------|-------|-------------|
| `LLM_OK` | `0` | Success |
| `LLM_ERR_INVALID_PARAM` | `-1` | Invalid parameter |
| `LLM_ERR_MEMORY` | `-2` | Memory allocation failure |
| `LLM_ERR_BACKEND` | `-3` | Backend error |
| `LLM_ERR_NETWORK` | `-4` | Network error |
| `LLM_ERR_JSON` | `-5` | JSON parsing error |
| `LLM_ERR_AUTH` | `-6` | Authentication error |

---

## Error Handling

All functions return an `llm_error_t` enum value. Recommended usage:

```c
llm_error_t err = generate(llm, req, NULL);
if (err != LLM_OK) {
    fprintf(stderr, "Generation request failed, error code: %d\n", err);
    // Take appropriate action based on error code
}
```

---

## Thread Safety

- Global tool linked list (`s_tools`) is currently **not locked**. It is recommended to register tools during the initialization phase
- Other objects (`llm_t`, `llm_request_t`, `llm_message_list_t`) are not thread-safe; do not share them across threads

---

## Contributing

Contributions are welcome! Please follow these steps:

1. Fork this repository
2. Submit a Pull Request

### Development Conventions

- Public API uses `LIBLLM_EXPORT` for symbol visibility
- Internal functions are prefixed with `llm_internal_`
- New backends should be declared in `libllm_internal.h` and registered in `libllm.c`

---

## License

This project is open-sourced under the MIT License. See [LICENSE](LICENSE) for details.
