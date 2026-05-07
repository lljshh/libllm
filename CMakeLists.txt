cmake_minimum_required(VERSION 3.15)
project(libllm VERSION 0.1.0 LANGUAGES C)

# 选项
option(BUILD_SHARED_LIBS "Build shared library" ON)
option(BUILD_TESTS "Build tests" OFF)
option(BUILD_EXAMPLES "Build examples" OFF)

# 设置 C 标准
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# 导出 compile_commands.json（供 VS Code / clangd 使用）
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 符号可见性 — 默认隐藏，仅导出带 __attribute__((visibility("default"))) 的符号
set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)


# 查找依赖
find_package(CURL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(CJSON REQUIRED libcjson IMPORTED_TARGET)


# 设置版本号和 soversion
set(LIBLLM_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(LIBLLM_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(LIBLLM_VERSION_PATCH ${PROJECT_VERSION_PATCH})

# 收集源文件
file(GLOB_RECURSE LIBLLM_SOURCES "src/*.c")
# 排除测试文件（如果 src 下有测试辅助文件）
list(REMOVE_ITEM LIBLLM_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/test_helpers.c")

# 创建库
add_library(${PROJECT_NAME} ${LIBLLM_SOURCES})

target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# libcurl + cJSON（使用现代 CMake 的 IMPORTED 目标方式）
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        CURL::libcurl
        PkgConfig::CJSON
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${LIBLLM_VERSION_MAJOR}
)

# 安装规则（只保留一份，避免重复）
include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}Targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(FILES include/libllm.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT ${PROJECT_NAME}Targets
    NAMESPACE ${PROJECT_NAME}::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)


# 测试
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# 示例
if(BUILD_EXAMPLES)
    add_executable(libllm_example "${CMAKE_CURRENT_SOURCE_DIR}/example.c")
    target_link_libraries(libllm_example PRIVATE ${PROJECT_NAME})
    target_include_directories(libllm_example PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
endif()

# ===== CPack 配置 =====
set(CPACK_PACKAGE_NAME "libllm")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_VENDOR "Kamov")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A C library for LLM API communication")
set(CPACK_PACKAGE_CONTACT "your-email@example.com")

# 架构由 CPack 自动检测，不写死
include(CPack)