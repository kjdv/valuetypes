add_subdirectory(templates)

file(GLOB sources *.h *.cpp templates/*.h templates/*.cpp)

add_library(valuetypes_common
    ${sources}
)

target_include_directories(valuetypes_common
    PUBLIC
      "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
)


target_link_libraries(valuetypes_common
    PUBLIC templates pantor::inja Kjson::kjson
)

