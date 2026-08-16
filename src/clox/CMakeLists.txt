project(clox LANGUAGES C VERSION 0.1)

set(CMAKE_C_STANDARD 23)

include_directories(include)
set(INCLUDE ${CMAKE_SOURCE_DIR}/src/clox/include)

add_executable(clox main.c)
add_subdirectory(tests)
