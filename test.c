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
    thing_t a = { .x = "auto", .len = 4 };
    thing_t b = { .x = "bus", .len = 3 };
    thing_t c = { .x = "vlak", .len = 4 };

    hamt_insert(h, &a, &b);
    hamt_insert(h, &b, &a);
    hamt_insert(h, &c, &a);

//    fprintf(stderr, "hamt_size: %d\n", hamt_size(h));
    mu_assert("error, hamt size doesn't match", hamt_size(h) == 2);
    return NULL;
};

static char *all_tests() {
    mu_suite_start();
    mu_run_test(test_foo);
    mu_run_test(test_insert);

    return NULL;
}

RUN_TESTS(all_tests);
