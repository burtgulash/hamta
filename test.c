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

    thing_t aut = { .x = "aut", .len = 3 };
    thing_t bus = { .x = "bus", .len = 3 };
    thing_t vlak = { .x = "vlak", .len = 4 };
    thing_t kokos = { .x = "kokos", .len = 5 };
    thing_t banan = { .x = "banan", .len = 5 };
    thing_t losos = { .x = "losos", .len = 5 };
    thing_t bro = { .x = "bro", .len = 3 };
    thing_t b = { .x = "b", .len = 1 };
    thing_t bubakov = { .x = "bubakov", .len = 7 };

    hamt_insert(h, &aut, &bus);
    hamt_insert(h, &bus, &vlak);
    hamt_insert(h, &vlak, &kokos);
    hamt_insert(h, &kokos, &banan);
    hamt_insert(h, &losos, &bro);
    hamt_insert(h, &bro, &b);
    hamt_insert(h, &bubakov, &aut);

    mu_assert("error, hamt size doesn't match", hamt_size(h) == 7);

    key_value_t *found;
    thing_t *searching_for;
    thing_t *s[] = {&aut, &bus, &vlak, &kokos, &banan, &losos, &bro, &b, &bubakov};
    int len = sizeof(s) / sizeof(thing_t*);

    for (int i = 0; i < len; i++) {
        searching_for = s[i];
        found = hamt_search(h, searching_for);
        mu_assert("error, not found", found != NULL);
        mu_assert("error, didn't find the correct key", (strcmp(found->key->x, searching_for->x)) == 0);
    }

    return NULL;
}

static char *test_hamta2() {
    hamt_t *h = new_hamt();

    thing_t z[] = { {.x="a",        .len=1},
                    {.x="bb",       .len=2},
                    {.x="auto",     .len=4},
                    {.x="bus",      .len=3},
                    {.x="vlak",     .len=4},
                    {.x="kokos",    .len=5},
                    {.x="banan",    .len=5},
                    {.x="losos",    .len=5},
                    {.x="bubakov",  .len=7},
                    {.x="korkodyl", .len=8},
                    {.x="x",        .len=1},
                    {.x="__x__",    .len=5} };

    int len = sizeof(z) / sizeof(thing_t);
    for (int i = 0; i < len; i++) {
        hamt_print(h);
        hamt_insert(h, &z[i], &z[i]);
    }
    hamt_print(h);

    mu_assert("error, hamt size doesn't match", hamt_size(h) == len);

    return NULL;
}



static char *all_tests() {
    mu_suite_start();

    mu_run_test(test_foo);
    mu_run_test(test_hamta2);
    mu_run_test(test_hamta);

    return NULL;
}

RUN_TESTS(all_tests);
