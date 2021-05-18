cmake_minimum_required(VERSION 3.10)

project(valuetypes)
set(CMAKE_CXX_STANDARD 17)

include(GoogleTest)
find_package(GTest MODULE REQUIRED)
find_library(GMOCK_LIBRARIES gmock)
find_package(rapidcheck CONFIG REQUIRED)
enable_testing()


# dependencies:

find_package(inja CONFIG REQUIRED)
find_package(cxxopts CONFIG REQUIRED)
find_package(benchmark CONFIG REQUIRED)

add_subdirectory(lib)
add_subdirectory(app)
add_subdirectory(test)
add_subdirectory(integration_test)

# install rules
install(TARGETS valuetypes)
