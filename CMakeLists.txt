cmake_minimum_required(VERSION 3.28)

cmake_policy(SET CMP0167 NEW)  # use XxxConfig.cmake insead of FindXxx.cmake when find_package is called

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

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_SOURCE_DIR}/conan_packages)

include(brabbit_cmake_utils)
brabbit_setup_electron_environment()
brabbit_copy_conan_packages_to_output_directory()

# system dependencies

find_package(OpenGL REQUIRED)  # libraries: OpenGL::GL

# conan dependencies

include(${CMAKE_SOURCE_DIR}/conan_packages/conandeps_legacy.cmake)
find_package(Boost REQUIRED)  # libraries: boost::boost
find_package(glm REQUIRED)  # libraries: glm::glm
find_package(nlohmann_json REQUIRED)  # libraries: nlohmann_json::nlohmann_json
find_package(SDL3 REQUIRED)  # libraries: sdl::sdl

# source code dependencies

add_subdirectory(native/glad EXCLUDE_FROM_ALL)  # libraries: glad::glad

# project targets

add_subdirectory(native/brabbit_native_apis)  # libraries: brabbit::native_apis
