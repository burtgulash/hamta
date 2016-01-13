#include <stdio.h>
#include <string.h>
#include "include/minunit.h"
#include "hamta.h"


static char *test_create() {
    hamt_t *h = new_hamt(hamt_fnv1_hash);
    mu_assert("error, hamt not initialized", h != NULL);

    thing_t x = { .x="x", .len=1 };
    thing_t y = { .x="yy", .len=2 };

    hamt_insert(h, &x, &x);
    hamt_insert(h, &y, &y);
    hamt_insert(h, &x, &y);
    hamt_insert(h, &y, &x);

    hamt_destroy(h);

    return NULL;
}

static char *test_search_destroy() {
    hamt_t *h = new_hamt(hamt_fnv1_hash);

    thing_t aut = { .x = "aut", .len = 3 };
    thing_t bus = { .x = "bus", .len = 3 };
    thing_t vlak = { .x = "vlak", .len = 4 };
    thing_t kokos = { .x = "kokos", .len = 5 };
    thing_t banan = { .x = "banan", .len = 5 };
    thing_t losos = { .x = "losos", .len = 5 };
    thing_t bro = { .x = "bro", .len = 3 };
    thing_t b = { .x = "b", .len = 1 };
    thing_t bubakov = { .x = "bubakov", .len = 7 };

    hamt_insert(h, &aut, &aut);
    hamt_insert(h, &bus, &bus);
    hamt_insert(h, &vlak, &vlak);
    hamt_insert(h, &kokos, &kokos);
    hamt_insert(h, &banan, &banan);
    hamt_insert(h, &losos, &losos);
    hamt_insert(h, &bro, &bro);
    hamt_insert(h, &b, &b);
    hamt_insert(h, &bubakov, &bubakov);

    int num_elements = 9;
    mu_assert("error, hamt size doesn't match", hamt_size(h) == num_elements);

    thing_t *found;
    thing_t *searching_for, *removing;
    thing_t *s[] = {&losos, &bus, &aut, &vlak, &banan, &kokos, &bro, &b, &bubakov};
    int len = sizeof(s) / sizeof(thing_t*);

    DEBUG_PRINT("\nSEARCHING FOR %d ELEMENTS\n", len);
    #ifdef DEBUG
    hamt_print(h);
    #endif
    for (int i = 0; i < len; i++) {
        searching_for = s[i];
        DEBUG_PRINT("\nsearching for key %s\n", (char*) searching_for->x);
        found = hamt_search(h, searching_for);

        mu_assert("error, not found", found != NULL);
        mu_assert("error, didn't find the correct key", (strcmp(found->x, searching_for->x)) == 0);
    }

    // Now remove the elements
    mu_assert("error, hamt size doesn't match", hamt_size(h) == num_elements);

    DEBUG_PRINT("\nREMOVING %d ELEMENTS\n", len);
    #ifdef DEBUG
    hamt_print(h);
    #endif
    for (int i = 0; i < len; i++) {
        removing = s[i];
        DEBUG_PRINT("removing key %s\n", (char*) removing->x);

        found = hamt_remove(h, removing);
        mu_assert("error, returned element is NULL", found != NULL);

        #ifdef DEBUG
        hamt_print(h);
        #endif

        mu_assert("error, hamt size doesn't match after removal", hamt_size(h) == --num_elements);
        mu_assert("error, didn't delete the correct key", (strcmp(found->x, removing->x)) == 0);
    }

    hamt_destroy(h);

    return NULL;
}

static char *test_hamta2() {
    hamt_t *h = new_hamt(hamt_fnv1_hash);

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
        #ifdef DEBUG
        hamt_print(h);
        #endif
        hamt_insert(h, &z[i], &z[i]);
    }
    #ifdef DEBUG
    hamt_print(h);
    #endif

    mu_assert("error, hamt size doesn't match", hamt_size(h) == len);

    hamt_destroy(h);

    return NULL;
}



static char *all_tests() {
    mu_suite_start();

    mu_run_test(test_create);
    mu_run_test(test_hamta2);
    mu_run_test(test_search_destroy);

    return NULL;
}

RUN_TESTS(all_tests);
