set(CMAKE_C_STANDARD 23)

add_executable(
    ll_test
    linked_list_test.c
    ${INCLUDE}/linked_list.h
    ${INCLUDE}/linked_list.c
)
