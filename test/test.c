#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "minunit.h"
#include "hamta.h"


static char *test_create() {
    hamt_t *h = new_hamt(hamt_fnv1_hash);
    mu_assert("error, hamt not initialized", h != NULL);

    thing_t x = { .x="x", .len=1 };
    thing_t y = { .x="yy", .len=2 };
    key_value_t original;

    hamt_set(h, &x, &x, &original);
    hamt_set(h, &y, &y, &original);
    hamt_set(h, &x, &y, &original);
    hamt_set(h, &y, &x, &original);

    hamt_remove(h, &x);

    hamt_destroy(h);

    return NULL;
}

static char *test_big() {
    hamt_t *h = new_hamt(hamt_fnv1_hash);

    for (int i = 0; i < 10000; i++) {
        thing_t *key = (thing_t*) malloc(sizeof(thing_t));
        thing_t *value = (thing_t*) malloc(sizeof(thing_t));

        key->x = (void*) malloc(sizeof(int));
        *((int*) key->x) = i;
        key->len = sizeof(int);

        value->x = (void*) malloc(sizeof(int));
        *((int*) value->x) = i * i;
        value->len = sizeof(int);

        key_value_t original;
        bool size_increased = hamt_set(h, key, value, &original);
        if (!size_increased) {
            //free(original.key->x);
            //free(original.key);
            //free(original.value->x);
            //free(original.value);
        }

        #ifdef DEBUG
        hamt_print(h);
        printf("size: %d\n", hamt_size(h));
        #endif

        mu_assert("error, size does not match in big test", hamt_size(h) == i + 1);
    }

    mu_assert("error, size does not match in big test", hamt_size(h) == 10000);

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

    key_value_t original;
    hamt_set(h, &aut, &aut, &original);
    hamt_set(h, &bus, &bus, &original);
    hamt_set(h, &vlak, &vlak, &original);
    hamt_set(h, &kokos, &kokos, &original);
    hamt_set(h, &banan, &banan, &original);
    hamt_set(h, &losos, &losos, &original);
    hamt_set(h, &bro, &bro, &original);
    hamt_set(h, &b, &b, &original);
    hamt_set(h, &bubakov, &bubakov, &original);

    int num_elements = 9;
    mu_assert("error, hamt size doesn't match", hamt_size(h) == num_elements);

    thing_t *found;
    thing_t *searching_for, *removing;
    thing_t *s[] = {&losos, &bus, &aut, &vlak, &banan, &kokos, &bro, &b, &bubakov};
    bool removed = false;
    int len = sizeof(s) / sizeof(thing_t*);

    DEBUG_PRINT("\nSEARCHING FOR %d ELEMENTS\n", len);
    #ifdef DEBUG
    hamt_print(h);
    #endif
    for (int i = 0; i < len; i++) {
        searching_for = s[i];
        DEBUG_PRINT("searching for key %s\n", (char*) searching_for->x);
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

        removed = hamt_remove(h, removing);
        mu_assert("error, returned element is NULL", removed == true);

        #ifdef DEBUG
        hamt_print(h);
        #endif

        mu_assert("error, hamt size doesn't match after removal", hamt_size(h) == --num_elements);
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

    key_value_t original;
    int len = sizeof(z) / sizeof(thing_t);
    for (int i = 0; i < len; i++) {
        #ifdef DEBUG
        hamt_print(h);
        #endif
        hamt_set(h, &z[i], &z[i], &original);
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
    mu_run_test(test_search_destroy);
    mu_run_test(test_hamta2);
    mu_run_test(test_big);

    return NULL;
}

RUN_TESTS(all_tests);
