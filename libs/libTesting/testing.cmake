enable_testing()
set(LIB_TESTING_PATH ${CMAKE_CURRENT_LIST_DIR})

if(NOT DEFINED CMAKE_OUTPUT_PATH)
    set(CMAKE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
endif()

# libTesting
function(build_testing_library)
    message("libTesting @ ${LIB_TESTING_PATH}")
    file(GLOB_RECURSE testing_sources
        ${LIB_TESTING_PATH}/*.cpp
    )
    add_library(Testing ${testing_sources})
    target_include_directories(Testing PUBLIC ${LIB_TESTING_PATH}/..)
endfunction()

set(_TESTING_CURRENT_TEST_TARGET "" CACHE INTERNAL "")
set(_TESTING_CURRENT_TEST_SUITE "" CACHE INTERNAL "")
set(_TESTING_CURRENT_TEST_CASE "" CACHE INTERNAL "")

function(add_test_target name dir )
    message("Adding test target: ${name} @ ${dir}")
    set(_TESTING_CURRENT_TEST_TARGET ${name} CACHE INTERNAL "")

    file(GLOB_RECURSE ${name}_test_sources ${dir}/*.cpp)
    add_executable(${name}_TestRunner ${${name}_test_sources})
    target_link_libraries(${name}_TestRunner ${name} Testing)
    target_include_directories(${name}_TestRunner PUBLIC ${dir})

    include(${dir}/CMakeLists.txt)
endfunction()

function(test_include_directory dir)
    target_include_directories(${_TESTING_CURRENT_TEST_TARGET}_TestRunner PUBLIC ${dir})
endfunction()

function(test_suite name)
    set(_TESTING_CURRENT_TEST_SUITE ${name} CACHE INTERNAL "")
    message("  Test Suite '${name}'")
endfunction()

function(end_test_suite)
    set(_TESTING_CURRENT_TEST_SUITE "" CACHE INTERNAL "")
    set(_TESTING_CURRENT_TEST_CASE "" CACHE INTERNAL "")
endfunction()

function(test_case name)
    set(_TESTING_CURRENT_TEST_CASE ${name} CACHE INTERNAL "")
    message("    Test case '${name}'")
endfunction()

function(end_test_case)
    set(_TESTING_CURRENT_TEST_CASE "" CACHE INTERNAL "")
endfunction()

function(test name)
    message("      - ${name}")
    add_test(NAME ${_TESTING_CURRENT_TEST_TARGET}_${name} COMMAND ${CMAKE_OUTPUT_PATH}/${_TESTING_CURRENT_TEST_TARGET}_TestRunner ${name})
endfunction()
