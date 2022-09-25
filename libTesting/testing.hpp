// Copyright (c) 2022 Tom Hancocks
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include <string>
#include <type_traits>

typedef void(*test_function_t)();

/**
 * Register a new unit test.
 * Must be called prior to main() being called. Typically this isn't done directly, but through the use of
 * the TEST() macro.
 * @param test_name     The name of the test.
 * @param fn            The test code to be executed.
 */
auto register_unit_test(const std::string& test_name, test_function_t fn) -> void;

#define XSTR(_s)    STR(_s)
#define STR(_s)     #_s

/*
 * Unit Tests need to be setup and registered when the binary undergoes its initial
 * static construction as it launches. Unit tests should be declared using the TEST()
 * macro and specifying a plain English name to describe the purpose of the test.
 */
#define TEST(_name)     auto _name () -> void;                             \
                        __attribute__((constructor)) auto _name##_trampoline() -> void {    \
                            register_unit_test(XSTR(_name), _name);                         \
                        }                                                                   \
                        auto _name () -> void

// MARK: - Test Assertions

namespace test
{
    /**
     * Indicate that the current test has failed.
     * @param reason        The message to be shown in the test log describing the failure.
     */
    auto fail(const std::string &reason, const char *file = __builtin_FILE(), int line = __builtin_LINE()) -> void;

    template<typename T, typename U, typename std::enable_if<std::is_convertible<T, U>::value>::type* = nullptr>
    auto equal(const T& lhs, const U& rhs, const std::string& reason, const char *file = __builtin_FILE(), int line = __builtin_LINE()) -> void
    {
        /* in order to make sure the implementation of a custom '==' operator is valid, we need to explicitly call
         * it, and not rely on shorthand. */
        bool result = (lhs == rhs);
        if (!result) {
            fail(reason, file, line);
        }
    }

    template<typename T, typename U, typename std::enable_if<std::is_convertible<T, U>::value>::type* = nullptr>
    auto not_equal(const T& lhs, const U& rhs, const std::string& reason, const char *file = __builtin_FILE(), int line = __builtin_LINE()) -> void
    {
        /* in order to make sure the implementation of a custom '!=' operator is valid, we need to explicitly call
         * it, and not rely on shorthand. */
        bool result = (lhs != rhs);
        if (!result) {
            fail(reason, file, line);
        }
    }

    template<typename T, typename U, typename std::enable_if<std::is_convertible<T, U>::value>::type* = nullptr>
    auto less_than(const T& lhs, const U& rhs, const std::string& reason, const char *file = __builtin_FILE(), int line = __builtin_LINE()) -> void
    {
        /* in order to make sure the implementation of a custom '<' operator is valid, we need to explicitly call
         * it, and not rely on shorthand. */
        bool result = (lhs < rhs);
        if (!result) {
            fail(reason, file, line);
        }
    }

    template<typename T, typename U, typename std::enable_if<std::is_convertible<T, U>::value>::type* = nullptr>
    auto less_than_equal(const T& lhs, const U& rhs, const std::string& reason, const char *file = __builtin_FILE(), int line = __builtin_LINE()) -> void
    {
        /* in order to make sure the implementation of a custom '<=' operator is valid, we need to explicitly call
         * it, and not rely on shorthand. */
        bool result = (lhs <= rhs);
        if (!result) {
            fail(reason, file, line);
        }
    }

    template<typename T, typename U, typename std::enable_if<std::is_convertible<T, U>::value>::type* = nullptr>
    auto greater_than(const T& lhs, const U& rhs, const std::string& reason, const char *file = __builtin_FILE(), int line = __builtin_LINE()) -> void
    {
        /* in order to make sure the implementation of a custom '>' operator is valid, we need to explicitly call
         * it, and not rely on shorthand. */
        bool result = (lhs > rhs);
        if (!result) {
            fail(reason, file, line);
        }
    }

    template<typename T, typename U, typename std::enable_if<std::is_convertible<T, U>::value>::type* = nullptr>
    auto greater_than_equal(const T& lhs, const U& rhs, const std::string& reason, const char *file = __builtin_FILE(), int line = __builtin_LINE()) -> void
    {
        /* in order to make sure the implementation of a custom '>=' operator is valid, we need to explicitly call
         * it, and not rely on shorthand. */
        bool result = (lhs >= rhs);
        if (!result) {
            fail(reason, file, line);
        }
    }

    template<typename T, typename U, typename std::enable_if<std::is_convertible<T, U>::value>::type* = nullptr>
    auto is_true(bool condition, const std::string& reason, const char *file = __builtin_FILE(), int line = __builtin_LINE()) -> void
    {
        if (!condition) {
            fail(reason, file, line);
        }
    }

    template<typename T, typename U, typename std::enable_if<std::is_convertible<T, U>::value>::type* = nullptr>
    auto is_false(bool condition, const std::string& reason, const char *file = __builtin_FILE(), int line = __builtin_LINE()) -> void
    {
        if (condition) {
            fail(reason, file, line);
        }
    }
}