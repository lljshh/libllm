# libllm 测试套件

# 收集所有测试源文件
file(GLOB TEST_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/test_llm_lifecycle.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/test_message_list.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/test_tools.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/test_internal.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/test_openai_backend.c"
)

# 由于库编译时符号可见性为 hidden（CMAKE_C_VISIBILITY_PRESET），
# 内部函数（build_url, llm_internal_* 等）无法从共享库链接。
# 因此测试直接编译所有 src 源文件，不链接 libllm 共享库。
file(GLOB ALL_SRC_SOURCES "${CMAKE_SOURCE_DIR}/src/*.c")

# 创建测试可执行文件，包含所有库源文件 + 测试源文件
add_executable(test_libllm
    "${CMAKE_CURRENT_SOURCE_DIR}/main.c"
    ${TEST_SOURCES}
    ${ALL_SRC_SOURCES}
)

# 设置 include 路径（用于访问 libllm_internal.h 等内部头文件）
target_include_directories(test_libllm PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/include
    ${CJSON_INCLUDE_DIRS}
)

# 链接 cURL 和 cJSON
target_link_libraries(test_libllm PRIVATE
    CURL::libcurl
    PkgConfig::CJSON
)

# 注册 CTest 测试
add_test(NAME test_libllm COMMAND test_libllm)

# 可选：添加一个集成测试目标，可同时启动 Mock Server
# 使用方法: cmake --build . --target test_with_mock
add_custom_target(test_with_mock
    COMMAND ${CMAKE_COMMAND} -E echo "Starting Mock Server..."
    COMMAND python3 "${CMAKE_CURRENT_SOURCE_DIR}/mock_server.py" --port 8089
    COMMENT "Run Mock Server for integration tests (press Ctrl+C to stop)"
    USES_TERMINAL
)
