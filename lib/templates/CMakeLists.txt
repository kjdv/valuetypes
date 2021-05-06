set(template_sources templates.h templates.cpp header.cpp source.cpp)

function(generate_template name)
    file(READ ${name}.cpp.inja content)
    configure_file(template.cpp.in ${name}.cpp)
endfunction()

generate_template(header)
generate_template(source)

message(FATAL ${template_sources})

add_library(templates ${template_sources})
target_include_directories(templates
    PUBLIC
      "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
)
target_link_libraries(templates pantor::inja)

