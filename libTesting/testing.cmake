enable_testing()

if(NOT DEFINED CMAKE_OUTPUT_PATH)
    set(CMAKE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
endif()

# libTesting
function(build_testing_library)
    file(GLOB_RECURSE testing_sources
        ${CMAKE_SOURCE_DIR}/libTesting/*.cpp
    )
    add_library(Testing ${testing_sources})
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
