file(READ ${CMAKE_CURRENT_LIST_DIR}/${name}.cpp.inja content)
configure_file(${CMAKE_CURRENT_LIST_DIR}/template.cpp.in ${name}.cpp)
