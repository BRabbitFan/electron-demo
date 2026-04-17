set(BRABBIT_MODULE_NAME "native_apis")

project("${BRABBIT_VENDOR_NAME}_${BRABBIT_MODULE_NAME}")

add_library(${PROJECT_NAME} SHARED)
add_library("${BRABBIT_VENDOR_NAME}::${BRABBIT_MODULE_NAME}" ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PRIVATE
  # electron environment
  ${CMAKE_JS_INC}

  # library source code
  ${CMAKE_CURRENT_SOURCE_DIR}
)

brabbit_search_cxx_source_files(BRABBIT_CXX_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR})
target_sources(${PROJECT_NAME} PRIVATE
  # electron environment
  ${CMAKE_JS_SRC}

  # library source code
  ${BRABBIT_CXX_SOURCE_FILES}
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
  NAPI_DISABLE_CPP_EXCEPTIONS
)

target_link_libraries(${PROJECT_NAME} PRIVATE
  # electron environment
  ${CMAKE_JS_LIB}

  # system dependencies
  OpenGL::GL

  # conan dependencies
  boost::boost
  glm::glm
  nlohmann_json::nlohmann_json
  sdl::sdl

  # source code dependencies
  glad::glad
)

set_target_properties(${PROJECT_NAME} PROPERTIES
  PREFIX ""
  SUFFIX ".node"
)
