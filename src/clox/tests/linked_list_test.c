#include "../include/linked_list.h"

#include <stdio.h>
#include <stdlib.h>

#include "tests.h"

bool check_expected(bool* expected, bool* got, int n) {
    for (int i = 0; i < n; ++i) {
        if (expected[i] != got[i]) return false;
    }
    return true;
}

bool test_linked_list_new() {
    auto ll = list_new();
    bool expected[] = {true, true, true};
    bool got[] = {list_size(ll) == 0, ll->head == NULL, ll->tail == NULL};
    list_free(ll);
    return check_expected(expected, got, 3);
}

bool test_linked_list_size() {
    auto ll = list_new();
    list_add(ll, 1);
    list_add(ll, 2);
    list_add(ll, 3);
    bool expected[] = {true};
    bool got[] = {list_size(ll) == 3};
    list_free(ll);
    return check_expected(expected, got, 1);
}

bool test_linked_list_add() {
    auto ll = list_new();
    list_add(ll, 1);
    list_add(ll, 2);
    list_add(ll, 3);
    bool expected[] = {true, true, true};
    bool got[] = {list_size(ll) == 3, ll->head->val == 1, ll->tail->val == 3};
    list_free(ll);
    return check_expected(expected, got, 3);
}

bool test_linked_list_insert() {
    auto ll = list_new();
    list_add(ll, 1);
    list_add(ll, 3);
    list_insert(ll, 1, 2);
    bool expected[] = {true, true, true};
    bool got[] = {list_size(ll) == 3, ll->head->next->val == 2,
                  ll->tail->prev->val == 2};
    list_free(ll);
    return check_expected(expected, got, 3);
}

bool test_linked_list_remove() {
    auto ll = list_new();
    list_add(ll, 1);
    list_add(ll, 2);
    list_add(ll, 3);
    int removed_val = list_remove(ll, 1);
    bool expected[] = {true, true, true};
    bool got[] = {list_size(ll) == 2, removed_val == 2,
                  ll->head->next->val == 3};
    list_free(ll);
    return check_expected(expected, got, 3);
}

bool test_linked_list_at() {
    auto ll = list_new();
    list_add(ll, 1);
    list_add(ll, 2);
    list_add(ll, 3);
    bool expected[] = {true, true, true};
    bool got[] = {list_at(ll, 0) == 1, list_at(ll, 1) == 2,
                  list_at(ll, 2) == 3};
    list_free(ll);
    return check_expected(expected, got, 3);
}

bool test_linked_list_node_at() {
    auto ll = list_new();
    list_add(ll, 1);
    list_add(ll, 2);
    list_add(ll, 3);
    bool expected[] = {true, true, true};
    bool got[] = {list_node_at(ll, 0)->val == 1, list_node_at(ll, 1)->val == 2,
                  list_node_at(ll, 2)->val == 3};
    list_free(ll);
    return check_expected(expected, got, 3);
}

bool test_linked_list_node_find() {
    auto ll = list_new();
    list_add(ll, 1);
    list_add(ll, 2);
    list_add(ll, 3);
    bool expected[] = {true, true, true};
    bool got[] = {list_node_find(ll, 1)->val == 1,
                  list_node_find(ll, 2)->val == 2,
                  list_node_find(ll, 3)->val == 3};
    list_free(ll);
    return check_expected(expected, got, 3);
}

void _display_result(bool test_success, char* test_name) {
    if (test_success) {
        printf("*");
    } else {
        printf("\nTest %s failed.\n", test_name);
    }
}

void _display_final_result(bool all_success, int n_pass_tests,
                           int n_fail_tests) {
    printf("\n");
    if (all_success) {
        printf("All tests pass (%d passed, %d failed).\n", n_pass_tests,
               n_fail_tests);
    } else {
        printf("Test failure (%d passed, %d failed).\n", n_pass_tests,
               n_fail_tests);
    }
}

void run_tests(struct test tests[], int n_tests) {
    bool test_success;
    bool all_success = true;
    int n_pass_tests = 0;
    int n_fail_tests = 0;
    for (int i = 0; i < n_tests; i++) {
        test_success = (tests[i].test_f)();
        n_pass_tests += test_success;
        n_fail_tests += 1 - test_success;
        _display_result(test_success, tests[i].name);
        all_success = all_success && test_success;
    }
    _display_final_result(all_success, n_pass_tests, n_fail_tests);
}

#define N_LL_CONSTRUCTION_TESTS 2
struct test ll_construction_tests[] = {
    {test_linked_list_new, "test_linked_list_new"},
    {test_linked_list_size, "test_linked_list_size"},
};

#define N_LL_MANIPULATION_TESTS 6
struct test ll_manipulation_tests[] = {
    {test_linked_list_add, "test_linked_list_add"},
    {test_linked_list_insert, "test_linked_list_insert"},
    {test_linked_list_remove, "test_linked_list_remove"},
    {test_linked_list_at, "test_linked_list_at"},
    {test_linked_list_node_at, "test_linked_list_node_at"},
    {test_linked_list_node_find, "test_linked_list_node_find"},
};

void run_all() {
    run_tests(ll_construction_tests, N_LL_CONSTRUCTION_TESTS);
    run_tests(ll_manipulation_tests, N_LL_MANIPULATION_TESTS);
}

int main() {
    run_all();
    return EXIT_SUCCESS;
}
