project(glad LANGUAGES C)

add_library(${PROJECT_NAME} STATIC
  src/glad.c
)

target_include_directories(${PROJECT_NAME} PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(glad::glad ALIAS ${PROJECT_NAME})
