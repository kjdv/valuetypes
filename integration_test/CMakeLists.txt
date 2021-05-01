file(GLOB sources *.c *.cc *.cpp *.h *.hh)

add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/point/point.hh ${CMAKE_CURRENT_BINARY_DIR}/point/point.cc
        COMMAND valuetypes --output ${CMAKE_CURRENT_BINARY_DIR}/point point.yaml
        DEPENDS valuetypes
)
add_library(point ${CMAKE_CURRENT_BINARY_DIR}/point/point.hh ${CMAKE_CURRENT_BINARY_DIR}/point/point.cc)
target_include_directories(point
    PUBLIC ${CMAKE_CURRENT_BINARY_DIR}
)


add_executable(integration_test ${sources})
target_link_libraries(integration_test point ${GMOCK_LIBRARIES} GTest::GTest GTest::Main)

gtest_discover_tests(integration_test)
