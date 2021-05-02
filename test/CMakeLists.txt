file(GLOB sources *.cpp)

add_executable(valuetypes_test ${sources})
target_link_libraries(valuetypes_test valuetypes_common ${GMOCK_LIBRARIES} GTest::GTest GTest::Main)

gtest_discover_tests(valuetypes_test)
