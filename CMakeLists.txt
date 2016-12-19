cmake_minimum_required(VERSION 2.8)

project(NativeSample1)

set(CMAKE_CXX_FLAGS "--std=gnu++11 -O2 ${CMAKE_CXX_FLAGS}")
set(PROJECT http-sample1)

add_executable(${PROJECT} http1-init.cc)
target_link_libraries(${PROJECT} cocaine-framework-native cocaine-io-util)

add_executable(cli cli.cc)
target_link_libraries(cli cocaine-framework-native cocaine-io-util blackhole)

find_package(Boost REQUIRED COMPONENTS system)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
  target_link_libraries(cli ${Boost_LIBRARIES})
endif()

#include(CPack)
set(CPACK_GENERATOR "TGZ")
