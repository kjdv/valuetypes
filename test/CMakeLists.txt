file(READ ${CMAKE_SOURCE_DIR}/lib/templates/minijson_declarations.cpp.inja minijson_declarations)
file(READ ${CMAKE_SOURCE_DIR}/lib/templates/minijson_definitions.cpp.inja minijson_definitions)
configure_file(minijson.cpp.in minijson.cpp)

add_executable(valuetypes_test ${CMAKE_CURRENT_BINARY_DIR}/minijson.cpp)
target_link_libraries(valuetypes_test valuetypes_common ${GMOCK_LIBRARIES} GTest::GTest GTest::Main)

gtest_discover_tests(valuetypes_test)
