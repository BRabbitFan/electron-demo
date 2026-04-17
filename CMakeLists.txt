cmake_minimum_required(VERSION 3.28)

set(BRABBIT_VENDOR_NAME "brabbit")
set(BRABBIT_PROJECT_NAME "nodejs_native_addons")

project("${BRABBIT_VENDOR_NAME}_${BRABBIT_PROJECT_NAME}")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/bin)
set(CMAKE_PDB_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/lib)

# The cmake-js set the /DELAYLOAD:node.exe linker flag into CMAKE_SHARED_LINKER_FLAGS on Windows.
# But clang-cl's toolchain can't handle it, so we need to remove it and add delayimp.lib to the link libraries.
if(WIN32)
  link_libraries(delayimp.lib)
endif()

# system dependencies

find_package(OpenGL REQUIRED)

# package dependencies

list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/conan_packages")
include(${CMAKE_SOURCE_DIR}/conan_packages/conandeps_legacy.cmake)

# build-in dependencies

add_subdirectory(native/glad EXCLUDE_FROM_ALL)

# source code of project

add_subdirectory(native/brabbit_native_apis)
