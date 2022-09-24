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
#include <iostream>

typedef void(*test_function_t)(const std::string&);

/*
 * Register a new unit test.
 */
auto register_unit_test(const char* name, test_function_t test) -> void;

auto test_fail(const char *reason) -> void;

/*
 * Unit Tests need to be setup and registered when the binary undergoes its initial
 * static construction as it launches. Unit tests should be declared using the TEST()
 * macro and specifying a plain English name to describe the purpose of the test.
 */
#define TEST(_name)     auto test_##__FILE__##_##__LINE__(const std::string&) -> void;                  \
                        __attribute__((constructor)) auto test_stub_##__FILE__##_##__LINE__() -> void { \
                            register_unit_test((_name), test_##__FILE__##_##__LINE__);       \
                        }                                                                               \
                        auto test_##__FILE__##_##__LINE__(const std::string& test_name) -> void

// MARK: - Test Helpers / Assertions

#define assert_base(_cond, _note)  if (!(_cond)) { \
    test_fail((_note));                            \
    return;                                        \
}