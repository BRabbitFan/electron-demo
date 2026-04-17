include(brabbit_cmake_utils)

set(BRABBIT_MODULE_NAME "native_apis")
brabbit_search_cxx_source_files(BRABBIT_CXX_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR})

project("${BRABBIT_VENDOR_NAME}_${BRABBIT_MODULE_NAME}")

find_package(Boost REQUIRED)
find_package(glm REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(SDL3 REQUIRED)

add_library(${PROJECT_NAME} SHARED)
add_library("${BRABBIT_VENDOR_NAME}::${BRABBIT_MODULE_NAME}" ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PRIVATE
  ${CMAKE_JS_INC}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_sources(${PROJECT_NAME} PRIVATE
  ${CMAKE_JS_SRC}
  ${BRABBIT_CXX_SOURCE_FILES}
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
  NAPI_DISABLE_CPP_EXCEPTIONS
)

target_link_libraries(${PROJECT_NAME} PRIVATE
  ${CMAKE_JS_LIB}
  # system dependencies
  OpenGL::GL
  # external dependencies
  boost::boost
  glm::glm
  nlohmann_json::nlohmann_json
  sdl::sdl
  # vendored dependencies
  glad::glad
)

set_target_properties(${PROJECT_NAME} PROPERTIES
  PREFIX ""
  SUFFIX ".node"
)
