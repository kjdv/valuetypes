add_subdirectory(templates)
add_subdirectory(definitions)

file(GLOB sources *.h *.cpp)

add_library(valuetypes_common
    ${sources}
)

target_include_directories(valuetypes_common
    PUBLIC
      "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
)

target_link_libraries(valuetypes_common
    PUBLIC templates definitions
)
