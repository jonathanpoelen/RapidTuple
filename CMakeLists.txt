project(RapidTuple)

cmake_minimum_required(VERSION 2.8)

#if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
#  include(CMakeForceCompiler)
#  CMAKE_FORCE_CXX_COMPILER(g++-4.9 GNU-4.9)
#endif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")

include(CMakeDefinitions.txt)
include_directories(include)
include_directories(modules/falcon.cxx/include)
#add_definitions(-g)

add_executable(tuple_test test/tuple.cpp)

# add_subdirectory(tests)

# enable_testing()
