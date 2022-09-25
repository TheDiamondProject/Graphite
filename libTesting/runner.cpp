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

#include <string>
#include <vector>
#include <unordered_set>
#include <iostream>
#include <libTesting/testing.hpp>

// MARK: - Test Runner Support

struct test_failure
{
    std::string expression;
    std::string text;
    std::string file;
    std::uint32_t line;
};

struct test_case
{
    enum class result { not_run, passed, failed };
    std::string name;
    test_function_t impl;
    enum result result { result::not_run };
    std::vector<struct test_failure> reasons;
};

struct test_suite
{
    static auto instance() -> test_suite&
    {
        static struct test_suite _instance;
        return _instance;
    }
    std::uint32_t test_count { 0 };
    std::uint32_t tests_run { 0 };
    std::uint32_t tests_passed { 0 };
    std::uint32_t tests_failed { 0 };
    std::vector<struct test_case> tests;
    struct test_case *current_test { nullptr };
};

// MARK: - Test Entry Point

auto main(int argc, const char *argv[]) -> int
{
    // Build a list of the desired tests to be run.
    bool test_logs = true;
    std::unordered_set<std::string> enabled_tests;
    if (argc > 1) {
        for (auto i = 1; i < argc; ++i) {
            if (std::string(argv[i]) == "-s") {
                test_logs = false;
                continue;
            }
            enabled_tests.emplace(argv[i]);
        }
    }

    // Process each of the test cases.
    std::uint32_t test_number = 0;
    for (auto it : test_suite::instance().tests) {
        if (!enabled_tests.empty()) {
            const auto& flag = enabled_tests.find(it.name);
            if (flag == enabled_tests.end()) {
                continue;
            }
        }

        test_number++;
        test_suite::instance().current_test = &it;

        if (test_logs) {
            std::cout << "[" << test_number << "/" << test_suite::instance().test_count << "] "
                      << it.name << "... ";
        }

        // Execute the test...
        it.result = test_case::result::passed;
        it.impl(it.name);
        test_suite::instance().tests_run++;

        // Report the result to the host
        if (it.result == test_case::result::not_run) {
            if (test_logs) {
                std::cout << "Not Run" << std::endl;
            }
        }
        else if (it.result == test_case::result::failed) {
            if (test_logs) {
                std::cout << "Failed" << std::endl;
            }
            test_suite::instance().tests_failed++;
        }
        else if (it.result == test_case::result::passed) {
            if (test_logs) {
                std::cout << "Passed" << std::endl;
            }
            test_suite::instance().tests_passed++;
        }

        if (!it.reasons.empty() && test_logs) {
            for (const auto& reason : it.reasons) {
                std::cout << "\t" << reason.file << " - L" << reason.line << std::endl;
                if (!reason.text.empty()) {
                    std::cout << "\t\t" << reason.text << std::endl;
                }
                if (!reason.expression.empty()) {
                    std::cout << "\t\t" << reason.expression << std::endl;
                }
            }

        }

        test_suite::instance().current_test = nullptr;
    }

    // Construct a report about tests passed/failed
    if (enabled_tests.empty() && test_logs) {
        std::cout << std::endl;
        std::cout << test_suite::instance().tests_passed << " tests passed." << std::endl;
        std::cout << test_suite::instance().tests_failed << " tests failed." << std::endl;
    }

    // Return the result of the tests to the host.
    return (test_suite::instance().tests_failed > 0) ? 1 : 0;
}

// MARK: - Harness Setup Function

auto register_unit_test(const std::string& name, test_function_t test) -> void
{
    struct test_case test_case;
    test_case.name = name;
    test_case.impl = test;
    test_suite::instance().tests.emplace_back(test_case);
    test_suite::instance().test_count++;
}

// MARK: - Test Assertions

auto test::fail(const std::string &reason, const char *file, int line) -> void
{
    if (test_suite::instance().current_test) {
        struct test_failure failure;
        failure.file = file;
        failure.line = line;
        failure.text = reason;

        test_suite::instance().current_test->result = test_case::result::failed;
        test_suite::instance().current_test->reasons.emplace_back(failure);
    }
}