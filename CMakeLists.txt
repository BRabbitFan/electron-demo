cmake_minimum_required(VERSION 3.28)

project(native_apis)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/bin)
set(CMAKE_PDB_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/lib)

# sdl

add_subdirectory(native/third_party/sdl EXCLUDE_FROM_ALL)

# node_bridge

execute_process(
  COMMAND node -p "require('node-addon-api').include"
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE NODE_ADDON_API_DIR
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

string(REPLACE "\"" "" NODE_ADDON_API_DIR ${NODE_ADDON_API_DIR})

set(CPP_SOURCE_DIR_PATH ${CMAKE_SOURCE_DIR}/native/node_bridge)

file(GLOB_RECURSE CPP_SOURCE_FILE_PATH_LIST
  ${CPP_SOURCE_DIR_PATH}/*.c
  ${CPP_SOURCE_DIR_PATH}/*.cc
  ${CPP_SOURCE_DIR_PATH}/*.cpp
  ${CPP_SOURCE_DIR_PATH}/*.cxx
)

add_library(${PROJECT_NAME} SHARED
  ${CMAKE_JS_SRC}
  ${CPP_SOURCE_FILE_PATH_LIST}
)

target_include_directories(${PROJECT_NAME} PRIVATE
  ${NODE_ADDON_API_DIR}
  ${CMAKE_JS_INC}
  ${CPP_SOURCE_DIR_PATH}
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
  NAPI_DISABLE_CPP_EXCEPTIONS
)

target_link_libraries(${PROJECT_NAME} PRIVATE
  ${CMAKE_JS_LIB}
  SDL3::SDL3
)

if(WIN32)
  target_link_libraries(${PROJECT_NAME} PRIVATE
    delayimp.lib
  )
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
  PREFIX ""
  SUFFIX ".node"
)
