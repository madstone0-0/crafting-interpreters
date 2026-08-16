#ifndef __TESTS__
#define __TESTS__

#include <stdbool.h>

struct test {
    bool (*test_f)(void);
    char* name;
};

void run_all(void);

#endif  // !__TESTS__
