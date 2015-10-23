cmake_minimum_required(VERSION 2.8)

project(lua_extended_types)

include_directories(/usr/include/lua5.2)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

file(GLOB CPP_FILES "*.cpp")
add_library(xtypes MODULE ${CPP_FILES})
target_link_libraries(xtypes lua5.2 m)
set_target_properties(xtypes PROPERTIES PREFIX "" OUTPUT_NAME "xtypes")
