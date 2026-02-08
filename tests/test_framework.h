/**
 * @file test_framework.h
 * @brief Minimal C test framework — zero external dependencies.
 *
 * Provides simple assertion macros for unit testing wireless comms functions.
 * Usage pattern:
 *
 *   TEST_SUITE("My Tests");
 *   TEST_CASE_BEGIN("case name") {
 *       ... assertions ...
 *       TEST_PASS_STMT;
 *   } TEST_CASE_END();
 *   TEST_SUMMARY();
 *
 * Macros:
 *   TEST_SUITE(name)          — Print suite header, reset counters.
 *   TEST_CASE_BEGIN(name)     — Start a named test case (opens do-block).
 *   TEST_CASE_END()           — Close test case block.
 *   TEST_PASS_STMT            — Mark current test as passed.
 *   TEST_FAIL_STMT(msg)       — Mark current test as failed.
 *   TEST_ASSERT(cond)         — Fail + skip rest if cond is false.
 *   TEST_ASSERT_NEAR(a,b,eps) — Fail + skip rest if |a-b| > epsilon.
 *   TEST_ASSERT_INT_EQ(a,b)   — Fail + skip rest if integers differ.
 *   TEST_SUMMARY()            — Print pass/fail totals + return code.
 */

#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

static int test_count   __attribute__((unused)) = 0;
static int test_passed  __attribute__((unused)) = 0;
static int test_failed  __attribute__((unused)) = 0;

#define TEST_SUITE(name) \
    printf("\n=== Test Suite: %s ===\n", name)

#define TEST_CASE_BEGIN(name) \
    printf("  [TEST] %s ... ", name); \
    fflush(stdout); \
    test_count++; \
    do

#define TEST_CASE_END() \
    while (0)

#define TEST_PASS_STMT \
    test_passed++; \
    printf("PASS\n")

#define TEST_FAIL_STMT(msg) \
    test_failed++; \
    printf("FAIL: %s\n", msg)

#define TEST_ASSERT(cond) \
    if (!(cond)) { \
        TEST_FAIL_STMT(#cond); \
        break; \
    }

#define TEST_ASSERT_NEAR(a, b, eps) \
    if (fabs((double)(a) - (double)(b)) > (eps)) { \
        char _msg[128]; \
        snprintf(_msg, sizeof(_msg), "%s ~ %s (got %.6g vs %.6g, eps=%.2e)", \
                 #a, #b, (double)(a), (double)(b), (double)(eps)); \
        TEST_FAIL_STMT(_msg); \
        break; \
    }

#define TEST_ASSERT_INT_EQ(a, b) \
    if ((a) != (b)) { \
        char _msg[128]; \
        snprintf(_msg, sizeof(_msg), "%s == %s (got %d vs %d)", #a, #b, (int)(a), (int)(b)); \
        TEST_FAIL_STMT(_msg); \
        break; \
    }

#define TEST_SUMMARY() \
    printf("\n=== Test Summary ===\n"); \
    printf("Total: %d, Passed: %d, Failed: %d\n", \
           test_count, test_passed, test_failed); \
    printf("Pass Rate: %.1f%%\n", \
           test_count > 0 ? (100.0 * test_passed / test_count) : 0.0); \
    return (test_failed == 0) ? 0 : 1

#endif /* TEST_FRAMEWORK_H */
