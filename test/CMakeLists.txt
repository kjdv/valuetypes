find_package(Kjson CONFIG REQUIRED)

function(generate_value_type name)
    add_custom_command(
            OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${name}/valuetypes.h ${CMAKE_CURRENT_BINARY_DIR}/${name}/valuetypes.cpp
            MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/${name}.json
            COMMAND valuetypes --json --output ${CMAKE_CURRENT_BINARY_DIR}/${name} ${CMAKE_CURRENT_SOURCE_DIR}/${name}.json
            DEPENDS valuetypes
    )
   
    add_library(${name} ${CMAKE_CURRENT_BINARY_DIR}/${name}/valuetypes.h ${CMAKE_CURRENT_BINARY_DIR}/${name}/valuetypes.cpp)
    target_include_directories(${name}
        PUBLIC ${CMAKE_CURRENT_BINARY_DIR}
    )
    target_link_libraries(${name} PRIVATE Kjson::kjson)
endfunction()

generate_value_type(point)
generate_value_type(basic_types)
generate_value_type(optionals)
generate_value_type(structs)
generate_value_type(vectors)
generate_value_type(variants)

set(sources
    point.cpp
    basic_types.cpp
    optionals.cpp
    structs.cpp
    vectors.cpp
    variants.cpp
    # scratchpad is a pseudo-test, meant to manually develop code before
    # writing a template
    scratchpad.cpp
    scratchpad/valuetypes.h
    scratchpad/valuetypes.cpp
)

add_executable(valuetypes_test ${sources})
target_link_libraries(valuetypes_test
    point
    basic_types
    optionals
    structs
    vectors
    variants
    ${GMOCK_LIBRARIES}
    GTest::GTest
    GTest::Main
    rapidcheck
)

gtest_discover_tests(valuetypes_test)

add_executable(benchmarks benchmarks.cpp)
target_link_libraries(benchmarks PUBLIC basic_types structs variants benchmark::benchmark benchmark::benchmark_main)
add_test(benchmarks.test benchmarks)
