add_library(definitions valuetypes.h valuetypes.cpp)
target_link_libraries(definitions PRIVATE Kjson::kjson)
