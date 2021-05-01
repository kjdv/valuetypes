function(generate_value_type name)
    add_custom_command(
            OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${name}/valuetypes.hh ${CMAKE_CURRENT_BINARY_DIR}/${name}/valuetypes.cc
            COMMAND valuetypes --output ${CMAKE_CURRENT_BINARY_DIR}/${name} ${CMAME_CURRENT_SOURCE_DIR}/${name}.yaml
            DEPENDS valuetypes
    )

    add_library(${name} ${CMAKE_CURRENT_BINARY_DIR}/${name}/valuetypes.hh ${CMAKE_CURRENT_BINARY_DIR}/${name}/valuetypes.cc)
    target_include_directories(${name}
        PUBLIC ${CMAKE_CURRENT_BINARY_DIR}
    )
endfunction()

generate_value_type(point)

file(GLOB sources *.c *.cc *.cpp *.h *.hh)

add_executable(integration_test ${sources})
target_link_libraries(integration_test point ${GMOCK_LIBRARIES} GTest::GTest GTest::Main)

gtest_discover_tests(integration_test)
