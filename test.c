#include <stdio.h>
#include "include/minunit.h"
#include "hamta.h"

int foo = 7;

static char *test_foo() {
    mu_assert("error, foo != 7", foo == 7);
    return NULL;
}

static char *test_create() {
    hamt_t *h = new_hamt();
    mu_assert("error, hamt not initialized", h != NULL);
    return NULL;
}

static char *test_insert() {
    hamt_t *h = new_hamt();
    thing_t key = { .x = "auto", .len = 4 };
    thing_t value = { .x = "bus", .len = 3 };
    hamt_insert(h, &key, &value);

    mu_assert("error, hamt fucked up", h != NULL);
    return NULL;
};

static char *all_tests() {
    mu_suite_start();
    mu_run_test(test_foo);

    return NULL;
}

RUN_TESTS(all_tests);
