#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include "hamta.h"

#define CHUNK_SIZE 5

bool thing_equals(thing_t *a, thing_t *b) {
    if (a->len != b->len)
        return false;
    for (size_t i = 0; i < a->len; i++)
        if (((char*) a)[i] != ((char*) b)[i])
            return false;
    return true;
}

hamt_node_t* new_hamt_node() {
    hamt_node_t *node = (hamt_node_t*) malloc(sizeof(hamt_node_t));
    return node;
}

thing_t* hamt_node_search(hamt_node_t *node, uint32_t hash, int lvl,
                                                            thing_t *key) {
    assert(node != NULL);

    int hash_chunk = hash;
    hash_chunk <<= (32 - lvl * CHUNK_SIZE);
    hash_chunk >>= (lvl * CHUNK_SIZE + CHUNK_SIZE);

    int shifted = node->bitmap >> hash_chunk;
    bool child_exists = shifted & 1;
    if (child_exists) {
        int child_position = __builtin_popcount(shifted >> 1);
        hamt_node_t *subnode = (node->children)[child_position];
        hamt_node_t **children = subnode->children;
        int children_ptr_val = *((int*) &children);
        if (children_ptr_val & 1 == KEY_VALUE_T_FLAG) {
            // clear flag bit before dereferencing
            children_ptr_val &= ~1;
            // convert back to original type without the flag bit
            children = *((hamt_node_t***) &children_ptr_val);
        } else {
            key_value_t *leaf = (key_value_t*) subnode;
            if (thing_equals(leaf->key, key))
                return leaf->value;
            else {
            }
        }


    } else
        return NULL;

    return NULL;
}

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

void hamt_insert(hamt_t *trie, thing_t *key, thing_t *value) {
    uint32_t hash = fnv1(key->x, key->len);
}

void* hamt_find(hamt_t *trie, thing_t *key) {
    uint32_t hash = fnv1(key->x, key->len);
    return hamt_node_search(trie->root, hash, 0, key);
}

void main() {
    char *a = "autobus";
    printf("%d\n", fnv1(a, 7));
}
