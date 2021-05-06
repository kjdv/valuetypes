set(template_sources 
    templates.h 
    templates.cpp 
    ${CMAKE_CURRENT_BINARY_DIR}/header.cpp 
    ${CMAKE_CURRENT_BINARY_DIR}/source.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/hash_declarations.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/hash_definitions.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/iostream_declarations.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/iostream_definitions.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/minijson_declarations.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/minijson_definitions.cpp
)

function(generate_template name)
    file(READ ${name}.cpp.inja content)
    configure_file(template.cpp.in ${name}.cpp)
endfunction()

generate_template(header)
generate_template(source)
generate_template(hash_declarations)
generate_template(hash_definitions)
generate_template(iostream_declarations)
generate_template(iostream_definitions)
generate_template(minijson_declarations)
generate_template(minijson_definitions)

add_library(templates ${template_sources})
target_include_directories(templates
    PUBLIC
      "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
)
target_link_libraries(templates pantor::inja)

