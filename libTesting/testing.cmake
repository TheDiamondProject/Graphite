enable_testing()

set(LIB_TESTING_PATH ${CMAKE_CURRENT_LIST_DIR})

if(NOT DEFINED CMAKE_OUTPUT_PATH)
    set(CMAKE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
endif()

# libTesting
function(build_testing_library)
    message(${LIB_TESTING_PATH})
    file(GLOB_RECURSE testing_sources
        ${LIB_TESTING_PATH}/../libTesting/*.cpp
    )
    add_library(Testing ${testing_sources})
    target_include_directories(Testing PUBLIC ${LIB_TESTING_PATH}/..)
endfunction()

function(add_testing_target name dir)
    file(GLOB_RECURSE test_sources ${dir}/*.cpp)
    add_executable(${name}_TestRunner ${test_sources})
    target_link_libraries(${name}_TestRunner ${name} Testing)
    target_include_directories(${name}_TestRunner PUBLIC ${dir})
endfunction()

function(test target name)
    add_test(NAME ${name} COMMAND ${CMAKE_OUTPUT_PATH}/${target}_TestRunner -s ${name})
endfunction()

function(test_suite target list)
    math(EXPR last "${ARGC} - 1")
    foreach(n RANGE 1 ${last})
        test(${target} ${ARGV${n}})
    endforeach()
endfunction()
