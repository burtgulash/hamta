#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "minunit.h"
#include "hamta.h"


static char *test_empty() {
    hamt_t *h = new_hamt(hamt_int_hash, hamt_int_equals);
    mu_assert("error, hamt not initialized", h != NULL);
    hamt_destroy(h, true);

    return NULL;
}

static char *test_big() {
    hamt_t *h = new_hamt(hamt_int_hash, hamt_int_equals);

    int n = 10000;
    for (int i = 0; i < n; i++) {
        int key = i % (n / 1337) + 1;
        int value = i * i + 10;

        void *kmem = malloc(sizeof(int));
        void *vmem = malloc(sizeof(int));

        memcpy(kmem, (void*) &key, sizeof(int));
        memcpy(vmem, (void*) &value, sizeof(int));

        key_value_t conflict_kv;
        bool conflict = hamt_set(h, kmem, vmem, &conflict_kv);
        if (conflict) {
            free(conflict_kv.key);
            free(conflict_kv.value);
        }

        int *found = (int*) hamt_search(h, kmem);
        mu_assert("value inserted and retrieved don't match!", *found == value);
    }

    hamt_destroy(h, true);

    return NULL;
}

static char *test_big2() {
    hamt_t *h = new_hamt(hamt_int_hash, hamt_int_equals);

    int n = 10000;
    for (int i = 0; i < n; i++) {
        int key = i % (n / 1531);
        int value = i * i * i;

        void *kmem = malloc(sizeof(int));
        void *vmem = malloc(sizeof(int));

        memcpy(kmem, (void*) &key, sizeof(int));
        memcpy(vmem, (void*) &value, sizeof(int));

        key_value_t conflict_kv;
        bool conflict = hamt_set(h, kmem, vmem, &conflict_kv);
        if (conflict) {
            free(conflict_kv.key);
            free(conflict_kv.value);
        }

        int *found = (int*) hamt_search(h, kmem);
        mu_assert("value inserted and retrieved don't match!", *found == value);
    }

    for (int i = 0; i < n; i++) {
        key_value_t removed_kv;
        bool removed = hamt_remove(h, (void*) &i, &removed_kv);
        if (removed) {
            free(removed_kv.key);
            free(removed_kv.value);
        }
    }

    hamt_destroy(h, true);

    return NULL;
}

static char *test_create() {
    hamt_t *h = new_hamt(hamt_str_hash, hamt_str_equals);

    char *x = "xx";
    char *y = "yy";

    key_value_t conflict_kv;
    hamt_set(h, x, x, &conflict_kv);
    hamt_set(h, y, y, &conflict_kv);
    hamt_set(h, x, y, &conflict_kv);
    hamt_set(h, y, x, &conflict_kv);

    bool removed = hamt_remove(h, x, &conflict_kv);
    if (removed) {
        // pass
    }

    hamt_destroy(h, false);

    return NULL;
}

/*

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

    hamt_destroy(h, false, false);

    return NULL;
}
*/

static char *test_hamta2() {
    hamt_t *h = new_hamt(hamt_str_hash, hamt_str_equals);

    char *s[] = {   "a",
                    "bb",
                    "auto",
                    "bus",
                    "vlak",
                    "kokos",
                    "banan",
                    "losos",
                    "bubakov",
                    "korkodyl",
                    "x",
                    "__x__" };

    key_value_t conflict_kv;
    int len = sizeof(s) / sizeof(char*);
    for (int i = 0; i < len; i++) {
        #ifdef DEBUG
        //hamt_print(h);
        #endif
        hamt_set(h, s[i], s[i], &conflict_kv);
    }
    #ifdef DEBUG
    //hamt_print(h);
    #endif

    mu_assert("error, hamt size doesn't match", hamt_size(h) == len);

    hamt_destroy(h, false);

    return NULL;
}


static char *all_tests() {
    mu_suite_start();

    mu_run_test(test_big2);
    mu_run_test(test_big);
    mu_run_test(test_empty);
    mu_run_test(test_create);
    mu_run_test(test_big);
    //mu_run_test(test_search_destroy);
    mu_run_test(test_hamta2);

    return NULL;
}

RUN_TESTS(all_tests);
