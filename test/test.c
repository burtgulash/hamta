#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "minunit.h"
#include "hamta.h"


static char *test_empty() {
    hamt_t *h = new_hamt(hamt_int_hash, hamt_int_equals);
    mu_assert("error, hamt not initialized", h != NULL);
    hamt_destroy(h, free);

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

        key_value_t *found = hamt_search(h, kmem);
        mu_assert("value inserted and retrieved don't match!", *(int*) found->value == value);
    }

    hamt_destroy(h, free);

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

        key_value_t *found = hamt_search(h, kmem);
        mu_assert("value inserted and retrieved don't match!", *(int*) found->value == value);
    }

    for (int i = 0; i < n; i++) {
        key_value_t removed_kv;
        bool removed = hamt_remove(h, (void*) &i, &removed_kv);
        if (removed) {
            free(removed_kv.key);
            free(removed_kv.value);
        }
    }

    hamt_destroy(h, free);

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
        // don't deallocate, it was allocated on stack
    }

    hamt_destroy(h, NULL);

    return NULL;
}

#define MK_STRING(text) char *(text) = #text; num_elements++

static char *test_search_destroy() {
    hamt_t *h = new_hamt(hamt_str_hash, hamt_str_equals);

    int num_elements;

    MK_STRING(aut);
    MK_STRING(bus);
    MK_STRING(vlak);
    MK_STRING(kokos);
    MK_STRING(banan);
    MK_STRING(losos);
    MK_STRING(bro);
    MK_STRING(b);
    MK_STRING(bubakov);

    key_value_t conflict_kv;
    hamt_set(h, aut, aut, &conflict_kv);
    hamt_set(h, bus, bus, &conflict_kv);
    hamt_set(h, vlak, vlak, &conflict_kv);
    hamt_set(h, kokos, kokos, &conflict_kv);
    hamt_set(h, banan, banan, &conflict_kv);
    hamt_set(h, losos, losos, &conflict_kv);
    hamt_set(h, bro, bro, &conflict_kv);
    hamt_set(h, b, b, &conflict_kv);
    hamt_set(h, bubakov, bubakov, &conflict_kv);

    mu_assert("error, hamt size doesn't match 1", hamt_size(h) == num_elements);

    key_value_t *found;
    char *searching_for, *removing;
    char *s[] = {losos, bus, aut, vlak, banan, kokos, bro, b, bubakov};
    bool removed = false;
    int len = sizeof(s) / sizeof(char*);

    DEBUG_PRINT("\nSEARCHING FOR %d ELEMENTS\n", len);
    for (int i = 0; i < len; i++) {
        searching_for = s[i];
        DEBUG_PRINT("searching for key %s\n", (char*) searching_for);
        found = hamt_search(h, searching_for);

        mu_assert("error, not found", found != NULL);
        mu_assert("error, didn't find the correct key", (strcmp(found->value, searching_for)) == 0);
    }

    // Now remove the elements
    mu_assert("error, hamt size doesn't match", hamt_size(h) == num_elements);

    DEBUG_PRINT("\nREMOVING %d ELEMENTS\n", len);
    for (int i = 0; i < len; i++) {
        removing = s[i];
        DEBUG_PRINT("removing key %s\n", (char*) removing);

        key_value_t removed_kv;
        removed = hamt_remove(h, removing, &removed_kv);

        mu_assert("error, returned element is NULL", removed == true);
        mu_assert("error, hamt size doesn't match after removal", hamt_size(h) == --num_elements);
    }

    hamt_destroy(h, NULL);

    return NULL;
}

char *to_str(void *x) {
    return (char*) x;
}

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
                    "__x__",
                    "y" };

    key_value_t conflict_kv;
    int len = sizeof(s) / sizeof(char*);
    for (int i = 0; i < len; i++) {
        #ifdef DEBUG
        hamt_print(h, to_str);
        #endif
        hamt_set(h, s[i], s[i], &conflict_kv);
    }
    #ifdef DEBUG
    hamt_print(h, to_str);
    #endif

    mu_assert("error, hamt size doesn't match", hamt_size(h) == len);

    hamt_destroy(h, NULL);

    return NULL;
}


static char *all_tests() {
    mu_suite_start();

    mu_run_test(test_empty);
    mu_run_test(test_big);
    mu_run_test(test_big2);
    mu_run_test(test_create);
    mu_run_test(test_big);
    mu_run_test(test_search_destroy);
    mu_run_test(test_hamta2);

    return NULL;
}

RUN_TESTS(all_tests);
