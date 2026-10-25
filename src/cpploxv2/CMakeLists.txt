project(cpploxv2 LANGUAGES CXX VERSION 0.1)

set(INCLUDE ${CMAKE_SOURCE_DIR}/src/cpploxv2/include)

add_executable(
    cpploxv2
    include/chunk.hpp
    include/chunk.cpp
    include/debug.hpp
    include/debug.cpp
    include/value.hpp
    include/value.cpp
    include/vm.hpp
    include/vm.cpp
    include/utils.hpp
    main.cpp
)
target_link_libraries(cpploxv2 Boost::program_options replxx)
