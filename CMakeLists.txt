if(WIN32)
  set_property(GLOBAL PROPERTY USE_FOLDERS On)
  set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "_Predefined")
endif(WIN32)

cmake_minimum_required(VERSION 2.6)
project(XScript) 

if(CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "-std=c++11 -pthread ${CMAKE_CXX_FLAGS}")
endif(CMAKE_COMPILER_IS_GNUCXX)

add_subdirectory(src/common) 
add_subdirectory(src/core) 
add_subdirectory(src/luabinder) 
add_subdirectory(src/v8binder) 
add_subdirectory(samplers) 