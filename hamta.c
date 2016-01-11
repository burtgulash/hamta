#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "hamta.h"

hamt_t* new_hamt() {
    hamt_t *h = (hamt_t*) malloc(sizeof(hamt_t));
    assert(h);
    h->root = (hamt_node_t*) malloc(sizeof(hamt_node_t));
    return h;
}

uint32_t fnv1(void *key, size_t len) {
    uint32_t hash = 2166136261;
    for (size_t i = 0; i < len; i++) {
        hash *= 16777619;
        hash ^= ((char*) key)[i];
    }
    return hash;
}

void insert(hamt_t *trie, void *key, size_t key_len,
            void *value, size_t value_len) {
    uint32_t hash = fnv1(key, key_len);
}

void main() {
    char *a = "autobus";
    printf("%d\n", fnv1(a, 7));
}
