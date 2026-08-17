project(cpplox LANGUAGES CXX VERSION 0.1)

set(INCLUDE ${CMAKE_SOURCE_DIR}/src/cpplox/include)

add_executable(
    cpplox
    main.cpp
    ${INCLUDE}/lox.hpp
    ${INCLUDE}/lox.cpp
    ${INCLUDE}/token.hpp
    ${INCLUDE}/lexer.hpp
    ${INCLUDE}/lexer.cpp
    ${INCLUDE}/ast.hpp
    ${INCLUDE}/ast.cpp
    ${INCLUDE}/parser.hpp
    ${INCLUDE}/parser.cpp
    ${INCLUDE}/interpreter.hpp
    ${INCLUDE}/interpreter.cpp
)
target_link_libraries(cpplox Boost::program_options replxx)
