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
#include <chrono>
#include <iostream>
#include <functional>

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
    auto fail(const std::string &reason = "", const char *file = __builtin_FILE(), int line = __builtin_LINE()) -> void;

    template<typename T, typename std::enable_if<std::is_pointer<T>::value>::type* = nullptr>
    static auto is_null(T ptr, const std::string& reason = "", const char *file = __builtin_FILE(), int line = __builtin_LINE()) -> void
    {
        if (ptr != nullptr) {
            fail(reason, file, line);
        }
    }

    template<typename T, typename std::enable_if<std::is_pointer<T>::value>::type* = nullptr>
    static auto not_null(T ptr, const std::string& reason = "", const char *file = __builtin_FILE(), int line = __builtin_LINE()) -> void
    {
        if (ptr == nullptr) {
            fail(reason, file, line);
        }
    }

    static auto equal(const std::string& lhs, const std::string& rhs, const std::string& reason = "", const char *file = __builtin_FILE(), int line = __builtin_LINE()) -> void
    {
        /* in order to make sure the implementation of a custom '==' operator is valid, we need to explicitly call
         * it, and not rely on shorthand. */
        bool result = (lhs == rhs);
        if (!result) {
            fail(reason, file, line);
        }
    }

    template<typename T, typename U, typename std::enable_if<std::is_convertible<T, U>::value>::type* = nullptr>
    static auto equal(const T& lhs, const U& rhs, const std::string& reason = "", const char *file = __builtin_FILE(), int line = __builtin_LINE()) -> void
    {
        /* in order to make sure the implementation of a custom '==' operator is valid, we need to explicitly call
         * it, and not rely on shorthand. */
        bool result = (lhs == rhs);
        if (!result) {
            fail(reason, file, line);
        }
    }

    template<
        typename T, typename U,
        typename std::enable_if<std::is_floating_point<T>::value>::type* = nullptr,
        typename std::enable_if<std::is_convertible<T, U>::value>::type* = nullptr
    >
    static auto equal(const T& lhs, const U& rhs, const T& tolerance, const std::string& reason = "", const char *file = __builtin_FILE(), int line = __builtin_LINE()) -> void
    {
        /* in order to make sure the implementation of a custom '==' operator is valid, we need to explicitly call
         * it, and not rely on shorthand. */
        bool result = (lhs - rhs) < tolerance;
        if (!result) {
            fail(reason, file, line);
        }
    }


    static auto not_equal(const std::string& lhs, const std::string& rhs, const std::string& reason = "", const char *file = __builtin_FILE(), int line = __builtin_LINE()) -> void
    {
        /* in order to make sure the implementation of a custom '==' operator is valid, we need to explicitly call
         * it, and not rely on shorthand. */
        bool result = (lhs != rhs);
        if (!result) {
            fail(reason, file, line);
        }
    }

    template<typename T, typename U, typename std::enable_if<std::is_convertible<T, U>::value>::type* = nullptr>
    static auto not_equal(const T& lhs, const U& rhs, const std::string& reason = "", const char *file = __builtin_FILE(), int line = __builtin_LINE()) -> void
    {
        /* in order to make sure the implementation of a custom '!=' operator is valid, we need to explicitly call
         * it, and not rely on shorthand. */
        bool result = (lhs != rhs);
        if (!result) {
            fail(reason, file, line);
        }
    }

    template<typename T, typename U, typename std::enable_if<std::is_convertible<T, U>::value>::type* = nullptr>
    static auto less_than(const T& lhs, const U& rhs, const std::string& reason = "", const char *file = __builtin_FILE(), int line = __builtin_LINE()) -> void
    {
        /* in order to make sure the implementation of a custom '<' operator is valid, we need to explicitly call
         * it, and not rely on shorthand. */
        bool result = (lhs < rhs);
        if (!result) {
            fail(reason, file, line);
        }
    }

    template<typename T, typename U, typename std::enable_if<std::is_convertible<T, U>::value>::type* = nullptr>
    static auto less_than_equal(const T& lhs, const U& rhs, const std::string& reason = "", const char *file = __builtin_FILE(), int line = __builtin_LINE()) -> void
    {
        /* in order to make sure the implementation of a custom '<=' operator is valid, we need to explicitly call
         * it, and not rely on shorthand. */
        bool result = (lhs <= rhs);
        if (!result) {
            fail(reason, file, line);
        }
    }

    template<typename T, typename U, typename std::enable_if<std::is_convertible<T, U>::value>::type* = nullptr>
    static auto greater_than(const T& lhs, const U& rhs, const std::string& reason = "", const char *file = __builtin_FILE(), int line = __builtin_LINE()) -> void
    {
        /* in order to make sure the implementation of a custom '>' operator is valid, we need to explicitly call
         * it, and not rely on shorthand. */
        bool result = (lhs > rhs);
        if (!result) {
            fail(reason, file, line);
        }
    }

    template<typename T, typename U, typename std::enable_if<std::is_convertible<T, U>::value>::type* = nullptr>
    static auto greater_than_equal(const T& lhs, const U& rhs, const std::string& reason = "", const char *file = __builtin_FILE(), int line = __builtin_LINE()) -> void
    {
        /* in order to make sure the implementation of a custom '>=' operator is valid, we need to explicitly call
         * it, and not rely on shorthand. */
        bool result = (lhs >= rhs);
        if (!result) {
            fail(reason, file, line);
        }
    }

    static auto is_true(bool condition, const std::string& reason = "", const char *file = __builtin_FILE(), int line = __builtin_LINE()) -> void
    {
        if (!condition) {
            fail(reason, file, line);
        }
    }

    static auto is_false(bool condition, const std::string& reason = "", const char *file = __builtin_FILE(), int line = __builtin_LINE()) -> void
    {
        if (condition) {
            fail(reason, file, line);
        }
    }

    static auto bytes_equal(const std::uint8_t *lhs, const std::uint8_t *rhs, std::size_t count,
                            const std::string& reason = "", const char *file = __builtin_FILE(), int line = __builtin_LINE()) -> void
    {
        for (auto i = 0; i < count; ++i) {
            if (lhs[i] != rhs[i]) {
                fail(reason, file, line);
            }
        }
    }

    template<typename T, typename std::enable_if<std::is_base_of<std::exception, T>::value>::type* = nullptr>
    static auto does_throw(const std::function<auto()->void>& fn, const std::string& reason = "", const char *file = __builtin_FILE(), int line = __builtin_LINE()) -> void
    {
        bool should_fail = true;

        try {
            fn();
        }
        catch (const T& e) {
            should_fail = false;
        }
        catch (...) {
            // Still fail but warn?
            fail("Unexpected exception was reached.", file, line);
        }

        if (should_fail) {
            fail(reason, file, line);
        }
    }

    static auto does_not_throw(const std::function<auto()->void>& fn, const std::string& reason = "", const char *file = __builtin_FILE(), int line = __builtin_LINE()) -> void
    {
        try {
            fn();
        }
        catch (...) {
            fail(reason, file, line);
        }
    }

    static auto measure(const std::function<auto()->void>& fn, const std::string& name = "", const char *file = __builtin_FILE(), int line = __builtin_LINE()) -> void
    {
        auto start = std::chrono::high_resolution_clock::now();
        fn();
        auto elapsed = std::chrono::high_resolution_clock::now() - start;

        // Report the time
        std::int64_t microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
        std::string report;

        if (!name.empty()) {
            report += name + ": ";
        }

        report += std::to_string(microseconds) + "µs";
        std::cout << report << std::endl;
    }
}