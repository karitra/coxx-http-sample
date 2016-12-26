cmake_minimum_required(VERSION 2.8)

project(NativeSample1)

set(CMAKE_CXX_FLAGS "--std=gnu++11 -O2 ${CMAKE_CXX_FLAGS}")
set(PROJECT http-native1)
set(PROJECT_LIBS cocaine-framework-native cocaine-io-util blackhole msgpack)

add_executable(${PROJECT} http1-init.cc)
target_link_libraries(${PROJECT} ${PROJECT_LIBS})

add_executable(cli cli.cc)
target_link_libraries(cli ${PROJECT_LIBS})

find_package(Boost REQUIRED COMPONENTS system)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
  target_link_libraries(${PROJECT} ${Boost_LIBRARIES})
	target_link_libraries(cli ${Boost_LIBRARIES})
endif()

include(CPack)
set(CPACK_GENERATOR "ZIP")
set(CPACK_PACKEGE_NAME "coxx-native-test")
