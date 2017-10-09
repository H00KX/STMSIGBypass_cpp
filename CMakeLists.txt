cmake_minimum_required(VERSION 3.1.0)
project(STMSIGBypass_cpp)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_BUILD_TYPE Debug)
set(LIBRARY_OUTPUT_PATH "Bin")

file(GLOB_RECURSE SOURCES "Source/*.cpp")

add_library(STMSIGBypass32 SHARED ${SOURCES})
add_library(STMSIGBypass64 SHARED ${SOURCES})

set_target_properties(STMSIGBypass32 PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
set_target_properties(STMSIGBypass64 PROPERTIES COMPILE_FLAGS "-m64" LINK_FLAGS "-m64")
set_target_properties(STMSIGBypass32 STMSIGBypass64 PROPERTIES PREFIX "")
set_target_properties(STMSIGBypass32 PROPERTIES SUFFIX ".ayria32")
set_target_properties(STMSIGBypass64 PROPERTIES SUFFIX ".ayria64")
