# Copyright (c) 2020 Tom Hancocks
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

cmake_minimum_required(VERSION 3.0)

project(Graphite CXX)

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(CMAKE_CXX_STANDARD 20)
if (APPLE)
	set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
endif()

# libGraphite
include_directories("${PROJECT_SOURCE_DIR}")
file(GLOB_RECURSE graphite_sources
	libGraphite/*.cpp
) 
add_library(Graphite ${graphite_sources})

# libTesting
file(GLOB_RECURSE testing_sources
	libTesting/*.cpp
)
add_library(Testing ${testing_sources})

# libGraphite Tests
file(GLOB_RECURSE graphiteTest_sources
	Tests/*.cpp
)
add_executable(GraphiteTestRunner ${graphiteTest_sources})
target_link_libraries(GraphiteTestRunner Graphite Testing)
target_include_directories(GraphiteTestRunner PUBLIC "${PROJECT_SOURCE_DIR}/Tests")

enable_testing()
include("CTestTestfile.cmake")