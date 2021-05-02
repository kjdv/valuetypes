function(generate_value_type name)
    add_custom_command(
            OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${name}/valuetypes.h ${CMAKE_CURRENT_BINARY_DIR}/${name}/valuetypes.cpp
            MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/${name}.yaml
            COMMAND valuetypes --output ${CMAKE_CURRENT_BINARY_DIR}/${name} ${CMAKE_CURRENT_SOURCE_DIR}/${name}.yaml
            DEPENDS valuetypes
    )
   
    add_library(${name} ${CMAKE_CURRENT_BINARY_DIR}/${name}/valuetypes.h ${CMAKE_CURRENT_BINARY_DIR}/${name}/valuetypes.cpp)
    target_include_directories(${name}
        PUBLIC ${CMAKE_CURRENT_BINARY_DIR}
    )
endfunction()

generate_value_type(point)

file(GLOB sources *.h *.cpp)

add_executable(integration_test ${sources})
target_link_libraries(integration_test point ${GMOCK_LIBRARIES} GTest::GTest GTest::Main rapidcheck)

gtest_discover_tests(integration_test)
