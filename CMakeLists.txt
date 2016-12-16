cmake_minimum_required(VERSION 2.8)

project(NativeSample1)

set(CMAKE_CXX_FLAGS "--std=gnu++11 -O2 ${CMAKE_CXX_FLAGS}")
set(PROJECT http-sample1)

add_executable(${PROJECT} http1-init.cc)
target_link_libraries(${PROJECT} cocaine-framework-native)

