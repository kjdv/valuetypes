function(generate_value_type name)
    add_custom_command(
            OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${name}/valuetypes.h ${CMAKE_CURRENT_BINARY_DIR}/${name}/valuetypes.cpp
            MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/${name}.json
            COMMAND valuetypes --permissive-json --output ${CMAKE_CURRENT_BINARY_DIR}/${name} ${CMAKE_CURRENT_SOURCE_DIR}/${name}.json
            DEPENDS valuetypes
    )
   
    add_library(${name} ${CMAKE_CURRENT_BINARY_DIR}/${name}/valuetypes.h ${CMAKE_CURRENT_BINARY_DIR}/${name}/valuetypes.cpp)
    target_include_directories(${name}
        PUBLIC ${CMAKE_CURRENT_BINARY_DIR}
    )
endfunction()

generate_value_type(point)
generate_value_type(basic_types)
generate_value_type(optionals)
generate_value_type(structs)
generate_value_type(vectors)
generate_value_type(variants)

file(GLOB sources *.h *.cpp)

add_executable(integration_test ${sources})
target_link_libraries(integration_test point basic_types optionals structs vectors variants ${GMOCK_LIBRARIES} GTest::GTest GTest::Main rapidcheck)

gtest_discover_tests(integration_test)
