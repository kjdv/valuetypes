file(GLOB sources *.cc *.cpp *.c *.hh *.h templates/*.hh templates/*.cc)

add_library(valuetypes_common
    ${sources}
)

target_include_directories(valuetypes_common
    PUBLIC
      "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
)


target_link_libraries(valuetypes_common
    PUBLIC pantor::inja Kyaml::kyaml
)

