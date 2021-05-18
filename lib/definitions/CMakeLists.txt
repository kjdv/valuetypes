add_library(definitions valuetypes.h valuetypes.cpp)

find_package(Kjson CONFIG REQUIRED)
target_link_libraries(definitions PRIVATE Kjson::kjson)
