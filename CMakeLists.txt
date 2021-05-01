cmake_minimum_required(VERSION 3.10)

project(valuetypes)
set(CMAKE_CXX_STANDARD 17)

include(GoogleTest)
find_package(GTest MODULE REQUIRED)
find_library(GMOCK_LIBRARIES gmock)
enable_testing()


# dependencies:

find_package(inja CONFIG REQUIRED)
find_package(cxxopts CONFIG REQUIRED)
find_package(Kyaml CONFIG REQUIRED)
find_package(Results CONFIG REQUIRED)
find_package(Composite CONFIG REQUIRED)

add_subdirectory(lib)
add_subdirectory(app)
add_subdirectory(test)

# install rules
install(TARGETS valuetypes
        EXPORT ValuetypesTargets
        ARCHIVE DESTINATION lib COMPONENT lib
        PUBLIC_HEADER DESTINATION include/valuetypes COMPONENT dev
)
install(FILES ValuetypesConfig.cmake DESTINATION share/valuetypes)

install(EXPORT ValuetypesTargets DESTINATION share/valuetypes NAMESPACE Valuetypes::)
export(TARGETS valuetypes NAMESPACE Valuetypes:: FILE ${CMAKE_CURRENT_BINARY_DIR}/ValuetypesTargets.cmake)
