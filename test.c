#include <stdio.h>
#include <string.h>
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

static char *test_hamta() {
    hamt_t *h = new_hamt();
    thing_t a = { .x = "auto", .len = 4 };
    thing_t b = { .x = "bus", .len = 3 };
    thing_t c = { .x = "vlak", .len = 4 };

    hamt_insert(h, &a, &b);
    hamt_insert(h, &b, &a);
    hamt_insert(h, &c, &a);
    hamt_insert(h, &c, &a);
    mu_assert("error, hamt size doesn't match", hamt_size(h) == 3);

    key_value_t *found;
    thing_t *searching_for;
    thing_t *s[] = {&a, &b, &c, &c};
    for (int i = 0; i < 4; i++) {
        searching_for = s[i];
        found = hamt_search(h, searching_for);
        mu_assert("error, didn't find the correct key", (strcmp(found->key->x, searching_for->x)) == 0);
    }

    return NULL;
};

static char *all_tests() {
    mu_suite_start();
    mu_run_test(test_foo);
    mu_run_test(test_hamta);

    return NULL;
}

RUN_TESTS(all_tests);
