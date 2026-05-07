# SPDX-FileCopyrightText: 2026 Kamov
#
# SPDX-License-Identifier: Apache-2.0

#!/usr/bin/env python3
"""
libllm Mock HTTP Server

提供模拟的 OpenAI-compatible API，用于集成测试。
支持：
  - GET /v1/models          → 返回模型列表
  - POST /v1/chat/completions → 返回模拟的聊天补全响应

用法：
  python3 mock_server.py [--port PORT]

默认端口 8089。
"""

import argparse
import json
import sys
from http.server import HTTPServer, BaseHTTPRequestHandler


class MockLLMHandler(BaseHTTPRequestHandler):
    """模拟 LLM API 请求处理器"""

    # 模拟的模型列表
    MODELS = [
        {"id": "gpt-4", "object": "model", "created": 1677610602, "owned_by": "openai"},
        {"id": "gpt-4-turbo", "object": "model", "created": 1677610602, "owned_by": "openai"},
        {"id": "gpt-3.5-turbo", "object": "model", "created": 1677610602, "owned_by": "openai"},
    ]

    def _send_json(self, status_code: int, data: dict):
        """发送 JSON 响应"""
        body = json.dumps(data).encode("utf-8")
        self.send_response(status_code)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    def _read_body(self) -> dict:
        """读取并解析请求体 JSON"""
        content_length = int(self.headers.get("Content-Length", 0))
        if content_length == 0:
            return {}
        raw = self.rfile.read(content_length)
        return json.loads(raw.decode("utf-8"))

    def do_GET(self):
        """处理 GET 请求"""
        if self.path == "/v1/models":
            self._handle_get_models()
        else:
            self._send_json(404, {"error": "Not found"})

    def do_POST(self):
        """处理 POST 请求"""
        if self.path == "/v1/chat/completions":
            self._handle_chat_completions()
        else:
            self._send_json(404, {"error": "Not found"})

    def _handle_get_models(self):
        """处理 GET /v1/models"""
        self._send_json(200, {"data": self.MODELS, "object": "list"})

    def _handle_chat_completions(self):
        """处理 POST /v1/chat/completions"""
        body = self._read_body()

        # 验证必要字段
        if "model" not in body:
            self._send_json(400, {"error": "model is required"})
            return

        if "messages" not in body or not isinstance(body["messages"], list):
            self._send_json(400, {"error": "messages is required"})
            return

        # 构建模拟响应
        response = {
            "id": "chatcmpl-mock-123456",
            "object": "chat.completion",
            "created": 1677610602,
            "model": body["model"],
            "choices": [
                {
                    "index": 0,
                    "message": {
                        "role": "assistant",
                        "content": "This is a mock response from the test server.",
                    },
                    "finish_reason": "stop",
                    "logprobs": None,
                }
            ],
            "usage": {
                "prompt_tokens": 10,
                "completion_tokens": 10,
                "total_tokens": 20,
            },
        }

        # 检查是否有工具调用请求
        last_msg = body["messages"][-1] if body["messages"] else {}
        if "tool_calls" in last_msg:
            # 如果是工具调用结果，返回一个更丰富的响应
            response["choices"][0]["message"]["content"] = (
                "The weather tool returned data. Let me summarize it for you."
            )

        self._send_json(200, response)

    def log_message(self, format, *args):
        """添加前缀的日志输出"""
        sys.stderr.write(f"[MockServer] {args[0]} {args[1]} {args[2]}\n")


def parse_args():
    parser = argparse.ArgumentParser(description="libllm Mock HTTP Server")
    parser.add_argument("--port", type=int, default=8089, help="Port to listen on")
    return parser.parse_args()


def main():
    args = parse_args()
    server = HTTPServer(("127.0.0.1", args.port), MockLLMHandler)
    print(f"[MockServer] Starting on http://127.0.0.1:{args.port}")
    print(f"[MockServer] Endpoints:")
    print(f"  GET  http://127.0.0.1:{args.port}/v1/models")
    print(f"  POST http://127.0.0.1:{args.port}/v1/chat/completions")
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\n[MockServer] Shutting down...")
        server.server_close()


if __name__ == "__main__":
    main()
