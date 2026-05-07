#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libllm.h"
void sanitize_utf8(char* s) {
  // 简单版：去除控制字符（除了 \t \n）
  char *p = s, *q = s;
  while (*p) {
    if ((unsigned char)*p >= 32 || *p == '\t' || *p == '\n') *q++ = *p;
    p++;
  }
  *q = '\0';
}
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
int main(int argc, char* argv[]) {
  if (argc == 1) {
    printf("Usage: %s <base_url> <api_key>\n", argv[0]);
    return 1;
  }

  int models_count = 0;
  llm_t** models = NULL;
  get_models(argv[1], argv[2], LLM_TYPE_OPENAI, &models_count, &models);
  // return 0;
  /* 使用 models[i] 访问模型，使用完后清理 */
  for (int i = 0; i < models_count; i++) {
    printf("%d. %s %s %d\n", i + 1, llm_get_model_name(models[i]),
           llm_get_api_url(models[i]), llm_get_type(models[i]));
  }
choose:
  printf("choose model:");
  int choice = 0;
  if (scanf("%d", &choice) != 1) {
    fprintf(stderr, "Invalid input\n");
    goto choose;
  }
  choice--;
  if (choice < 0 || choice >= models_count) {
    fprintf(stderr, "Invalid choice\n");
    goto choose;
  }
  printf("You chose: %s\n", llm_get_model_name(models[choice]));
  llm_request_t* req = llm_request_new();
  llm_request_set_reasoning_effort(req, "high");
  llm_request_set_tool_choice(req, "auto");
  llm_request_set_top_p(req, 0.2);
  llm_request_set_max_tokens(req, 10000);
  llm_message_node_t* system_msg =
      llm_message_list_create_node("system", "You are a helpful assistant.");
  llm_message_list_push_back(llm_request_get_messages(req), system_msg);
  int c;
  while ((c = getchar()) != '\n' && c != EOF);  // 清除缓冲区中残留的换行符
  llm_tool_register("get_weather", weather_tool_json, weather_callback);
  for (;;) {
    printf("Input> ");
    fflush(stdout);

    char buf[512];
    // 使用 fgets 读取一行（包括换行符）
    if (fgets(buf, sizeof(buf), stdin) == NULL) {
      // EOF 或错误，退出循环
      break;
    }

    // 去掉末尾的换行符（如果有）
    size_t len = strlen(buf);
    if (len > 0 && buf[len - 1] == '\n') {
      buf[len - 1] = '\0';
    }

    // 如果用户只按了回车（输入为空），跳过本次循环（不发送空消息）
    if (buf[0] == '\0') {
      continue;
    }
    if (strcmp(buf, "exit") == 0) {
      break;
    }
    // 可选：清理控制字符（保留 UTF-8 多字节字符）
    sanitize_utf8(buf);
    llm_message_node_t* msg = llm_message_list_create_node("user", buf);
    llm_message_list_push_back(llm_request_get_messages(req), msg);
  send:
    llm_tools_callback_t* tools_callback = NULL;
    int tools_count = 0;
    llm_token_usage_t usage;
    llm_error_t err = generate(models[choice], req, NULL, &tools_callback,
                               &tools_count, &usage);
    if (err != LLM_OK) {
      fprintf(stderr, "Generation error: %d\n", err);
      fprintf(stderr, "Last request: %s\n", llm_last_request);
    } else {
      printf(
          "Generation successful! Token usage: prompt=%d, completion=%d, "
          "total=%d\n",
          usage.prompt_tokens, usage.completion_tokens, usage.total_tokens);
    }
    printf("LLM Response:\n%s\n----\n", llm_last_request);
    printf("Assistant>%s",
           llm_message_list_get_tail(llm_request_get_messages(req))
               ->message->content);
    if (tools_count > 0) {
      printf("\n (工具调用 %d 个)，分别为\n", tools_count);
      for (int i = 0; i < tools_count; i++) {
        printf("%d. %s \n", i + 1, tools_callback[i].args_json);
        char* tool_result =
            tools_callback[i].callback(req, tools_callback[i].args_json);
        printf("=> %s\n", tool_result);
        llm_message_node_t* tool_msg =
            llm_message_list_create_node_tool_call_id(
                "tool", tool_result, tools_callback[i].tool_call_id);
        llm_message_list_push_back(llm_request_get_messages(req), tool_msg);
        free(tool_result);
      }
      llm_tools_callback_free(tools_callback, tools_count);
      goto send;
    }
    puts("");
  }
  llm_tool_unregister("get_weather");
  for (int i = 0; i < models_count; i++) {
    printf("  free model %d\n", i);
    llm_free(models[i]);
  }
  printf("Freeing request...\n");
  llm_request_free(req);
  printf("Freeing models array...\n");
  free(models);
  printf("All freed.\n");
  return 0;
}
