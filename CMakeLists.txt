if(WIN32)
  set_property(GLOBAL PROPERTY USE_FOLDERS On)
  set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "_Predefined")
endif(WIN32)

if(CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")
  message(STATUS "optional:-std=c++11")
endif(CMAKE_COMPILER_IS_GNUCXX)

cmake_minimum_required(VERSION 2.6)

project(XScript) 
add_subdirectory(src/common) 
add_subdirectory(src/core) 
add_subdirectory(src/luabinder) 
add_subdirectory(src/v8binder) 
add_subdirectory(sampler) 