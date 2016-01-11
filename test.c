#include <stdio.h>
#include "include/minunit.h"
#include "hamta.h"

int foo = 7;

static char *test_foo() {
    mu_assert("error, foo != 7", foo == 7);
    return NULL;
}

static char *all_tests() {
    mu_suite_start();
    mu_run_test(test_foo);

    return NULL;
}

RUN_TESTS(all_tests);
