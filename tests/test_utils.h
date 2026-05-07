// SPDX-FileCopyrightText: 2026 Kamov
//
// SPDX-License-Identifier: Apache-2.0

/**
 * @file test_utils.h
 * @brief 极简 C 测试框架头文件（零依赖，纯宏实现）
 *
 * 用法示例：
 *
 *   #include "test_utils.h"
 *
 *   TEST("test_addition") {
 *     ASSERT_EQ(2 + 2, 4);
 *     ASSERT_NE(2 + 2, 5);
 *     ASSERT_TRUE(1);
 *     ASSERT_FALSE(0);
 *     ASSERT_NULL(NULL);
 *     ASSERT_NOT_NULL("hello");
 *     ASSERT_STREQ("hello", "hello");
 *     ASSERT_STRNE("hello", "world");
 *     ASSERT_GT(5, 3);
 *     ASSERT_LT(3, 5);
 *     ASSERT_GE(5, 5);
 *     ASSERT_LE(3, 3);
 *     PASS();
 *   }
 *
 *   int main(void) {
 *     RUN_TEST(test_addition);
 *     PRINT_TEST_RESULTS();
 *     return TEST_FAILED ? 1 : 0;
 *   }
 */

#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---- 全局计数器 ---- */
static int g_test_passed = 0;
static int g_test_failed = 0;
static int g_assert_passed = 0;
static int g_assert_failed = 0;
static int g_current_test_failed = 0;

/* 用于跟踪当前测试名称的缓冲区 */
static char g_current_test_name[256] = "";

/* ---- 颜色输出 ---- */
#define TEST_COLOR_RESET "\033[0m"
#define TEST_COLOR_RED "\033[31m"
#define TEST_COLOR_GREEN "\033[32m"
#define TEST_COLOR_YELLOW "\033[33m"
#define TEST_COLOR_CYAN "\033[36m"

/* ---- 宏定义 ---- */

/** 定义测试函数 */
#define TEST(name)                                                \
  void name(void);                                                \
  void __test_register_##name(void) __attribute__((constructor)); \
  void __test_register_##name(void) {                             \
    /* 函数指针可以在 RUN_TEST 中直接调用 */                      \
  }                                                               \
  void name(void)

/** 运行一个测试函数 */
#define RUN_TEST(func)                                                       \
  do {                                                                       \
    g_current_test_failed = 0;                                               \
    snprintf(g_current_test_name, sizeof(g_current_test_name), "%s", #func); \
    printf(TEST_COLOR_CYAN "[ RUN     ] " TEST_COLOR_RESET "%s\n",           \
           g_current_test_name);                                             \
    func();                                                                  \
    if (g_current_test_failed) {                                             \
      g_test_failed++;                                                       \
      printf(TEST_COLOR_RED "[  FAILED ] " TEST_COLOR_RESET "%s\n",          \
             g_current_test_name);                                           \
    } else {                                                                 \
      g_test_passed++;                                                       \
      printf(TEST_COLOR_GREEN "[  PASSED ] " TEST_COLOR_RESET "%s\n",        \
             g_current_test_name);                                           \
    }                                                                        \
  } while (0)

/** 打印测试结果摘要 */
#define PRINT_TEST_RESULTS()                                                \
  do {                                                                      \
    int total = g_test_passed + g_test_failed;                              \
    printf("\n" TEST_COLOR_CYAN                                             \
           "======================================" TEST_COLOR_RESET "\n"); \
    printf(TEST_COLOR_CYAN "  Test Results: " TEST_COLOR_RESET              \
                           "%d total, " TEST_COLOR_GREEN                    \
                           "%d passed" TEST_COLOR_RESET ", " TEST_COLOR_RED \
                           "%d failed" TEST_COLOR_RESET "\n",               \
           total, g_test_passed, g_test_failed);                            \
    printf(TEST_COLOR_CYAN "  Assertions: " TEST_COLOR_RESET                \
                           "%d total, " TEST_COLOR_GREEN                    \
                           "%d passed" TEST_COLOR_RESET ", " TEST_COLOR_RED \
                           "%d failed" TEST_COLOR_RESET "\n",               \
           g_assert_passed + g_assert_failed, g_assert_passed,              \
           g_assert_failed);                                                \
    printf(TEST_COLOR_CYAN                                                  \
           "======================================" TEST_COLOR_RESET "\n"); \
  } while (0)

#define TEST_FAILED (g_test_failed > 0)

/** 内部断言失败处理 */
#define __TEST_FAIL(file, line, expr_str)                  \
  do {                                                     \
    g_current_test_failed = 1;                             \
    g_assert_failed++;                                     \
    printf(TEST_COLOR_RED "  [ASSERT] " TEST_COLOR_RESET   \
                          "%s:%d: Assertion failed: %s\n", \
           file, line, expr_str);                          \
  } while (0)

#define __TEST_FAIL_FMT(file, line, expr_str, fmt, ...)              \
  do {                                                               \
    g_current_test_failed = 1;                                       \
    g_assert_failed++;                                               \
    printf(TEST_COLOR_RED "  [ASSERT] " TEST_COLOR_RESET             \
                          "%s:%d: Assertion failed: %s (" fmt ")\n", \
           file, line, expr_str, ##__VA_ARGS__);                     \
  } while (0)

/* ---- 断言宏 ---- */

#define ASSERT_EQ(a, b)                                               \
  do {                                                                \
    if ((a) != (b)) {                                                 \
      __TEST_FAIL_FMT(__FILE__, __LINE__, #a " == " #b,               \
                      "actual: %lld, expected: %lld", (long long)(a), \
                      (long long)(b));                                \
    } else {                                                          \
      g_assert_passed++;                                              \
    }                                                                 \
  } while (0)

#define ASSERT_NE(a, b)                                                  \
  do {                                                                   \
    if ((a) == (b)) {                                                    \
      __TEST_FAIL_FMT(__FILE__, __LINE__, #a " != " #b, "both are %lld", \
                      (long long)(a));                                   \
    } else {                                                             \
      g_assert_passed++;                                                 \
    }                                                                    \
  } while (0)

#define ASSERT_TRUE(expr)                                \
  do {                                                   \
    if (!(expr)) {                                       \
      __TEST_FAIL(__FILE__, __LINE__, #expr " is true"); \
    } else {                                             \
      g_assert_passed++;                                 \
    }                                                    \
  } while (0)

#define ASSERT_FALSE(expr)                                \
  do {                                                    \
    if ((expr)) {                                         \
      __TEST_FAIL(__FILE__, __LINE__, #expr " is false"); \
    } else {                                              \
      g_assert_passed++;                                  \
    }                                                     \
  } while (0)

#define ASSERT_NULL(ptr)                                \
  do {                                                  \
    if ((ptr) != NULL) {                                \
      __TEST_FAIL(__FILE__, __LINE__, #ptr " is NULL"); \
    } else {                                            \
      g_assert_passed++;                                \
    }                                                   \
  } while (0)

#define ASSERT_NOT_NULL(ptr)                                \
  do {                                                      \
    if ((ptr) == NULL) {                                    \
      __TEST_FAIL(__FILE__, __LINE__, #ptr " is not NULL"); \
    } else {                                                \
      g_assert_passed++;                                    \
    }                                                       \
  } while (0)

#define ASSERT_STREQ(a, b)                                           \
  do {                                                               \
    if (strcmp((a), (b)) != 0) {                                     \
      __TEST_FAIL_FMT(__FILE__, __LINE__, #a " == " #b,              \
                      "actual: \"%s\", expected: \"%s\"", (a), (b)); \
    } else {                                                         \
      g_assert_passed++;                                             \
    }                                                                \
  } while (0)

#define ASSERT_STRNE(a, b)                                                 \
  do {                                                                     \
    if (strcmp((a), (b)) == 0) {                                           \
      __TEST_FAIL_FMT(__FILE__, __LINE__, #a " != " #b, "both are \"%s\"", \
                      (a));                                                \
    } else {                                                               \
      g_assert_passed++;                                                   \
    }                                                                      \
  } while (0)

#define ASSERT_GT(a, b)                                                \
  do {                                                                 \
    if (!((a) > (b))) {                                                \
      __TEST_FAIL_FMT(__FILE__, __LINE__, #a " > " #b,                 \
                      "%lld is not greater than %lld", (long long)(a), \
                      (long long)(b));                                 \
    } else {                                                           \
      g_assert_passed++;                                               \
    }                                                                  \
  } while (0)

#define ASSERT_LT(a, b)                                             \
  do {                                                              \
    if (!((a) < (b))) {                                             \
      __TEST_FAIL_FMT(__FILE__, __LINE__, #a " < " #b,              \
                      "%lld is not less than %lld", (long long)(a), \
                      (long long)(b));                              \
    } else {                                                        \
      g_assert_passed++;                                            \
    }                                                               \
  } while (0)

#define ASSERT_GE(a, b)                                                        \
  do {                                                                         \
    if (!((a) >= (b))) {                                                       \
      __TEST_FAIL_FMT(__FILE__, __LINE__, #a " >= " #b, "%lld is not >= %lld", \
                      (long long)(a), (long long)(b));                         \
    } else {                                                                   \
      g_assert_passed++;                                                       \
    }                                                                          \
  } while (0)

#define ASSERT_LE(a, b)                                                        \
  do {                                                                         \
    if (!((a) <= (b))) {                                                       \
      __TEST_FAIL_FMT(__FILE__, __LINE__, #a " <= " #b, "%lld is not <= %lld", \
                      (long long)(a), (long long)(b));                         \
    } else {                                                                   \
      g_assert_passed++;                                                       \
    }                                                                          \
  } while (0)

/** 主动标记测试通过 */
#define PASS()         \
  do {                 \
    g_assert_passed++; \
  } while (0)

/** 主动标记测试失败 */
#define FAIL(msg)                         \
  do {                                    \
    __TEST_FAIL(__FILE__, __LINE__, msg); \
  } while (0)

#endif /* TEST_UTILS_H */
