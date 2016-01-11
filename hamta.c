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

    hamt_node_t **children = node->children;
    int children_ptr_val = *((int*) &children);
    assert(children_ptr_val & 1 == HAMT_NODE_T_FLAG);
    children_ptr_val &= ~1;
    children = *((hamt_node_t***) &children_ptr_val);

    int hash_chunk = hash;
    hash_chunk <<= (32 - lvl * CHUNK_SIZE);
    hash_chunk >>= (lvl * CHUNK_SIZE + CHUNK_SIZE);

    int shifted = node->bitmap >> hash_chunk;
    bool child_exists = shifted & 1;
    if (child_exists) {
        // position of child is popcount of 1-bits to the left of bitmap at
        // keyed position
        int child_position = __builtin_popcount(shifted >> 1);
        hamt_node_t *subnode = children[child_position];

        hamt_node_t **subchildren = subnode->children;
        int subchildren_ptr_val = *((int*) &subchildren);

        if (subchildren_ptr_val & 1 == KEY_VALUE_T_FLAG) {
            // switch type of hamt_node_t to key_value_t
            key_value_t *leaf = (key_value_t*) subnode;
            if (thing_equals(leaf->key, key))
                return leaf->value;
        } else
            return hamt_node_search(subnode, hash, lvl + 1, key);
    }

    return NULL;
}

void hamt_node_insert(hamt_node_t *node, uint32_t hash, int lvl,
                                         thing_t *key, thing_t *value) {
    assert(node != NULL);

    hamt_node_t **children = node->children;
    int children_ptr_val = *((int*) &children);
    assert(children_ptr_val & 1 == HAMT_NODE_T_FLAG);
    children_ptr_val &= ~1;
    children = *((hamt_node_t***) &children_ptr_val);

    int hash_chunk = hash;
    hash_chunk <<= (32 - lvl * CHUNK_SIZE);
    hash_chunk >>= (lvl * CHUNK_SIZE + CHUNK_SIZE);

    int shifted = node->bitmap >> hash_chunk;
    bool child_exists = shifted & 1;
    if (child_exists) {
        // position of child is popcount of 1-bits to the left of bitmap at
        // keyed position
        int child_position = __builtin_popcount(shifted >> 1);
        hamt_node_t *subnode = children[child_position];

        hamt_node_t **subchildren = subnode->children;
        int subchildren_ptr_val = *((int*) &subchildren);

        if (subchildren_ptr_val & 1 == KEY_VALUE_T_FLAG) {
            // case: conflict with another key_value

            hamt_node_t *new_subnode = (hamt_node_t*) malloc(sizeof(hamt_node_t));
            // set first bit
            new_subnode->bitmap = 1 << 31;
            // make children array of size one for only the original node
            new_subnode->children = (hamt_node_t**) malloc(sizeof(hamt_node_t*) * 1);
            // copy over original conflicting key_value
            new_subnode->children[0] = subnode;

            // set parent's child to new_subnode
            children[child_position] = new_subnode;
            subnode = new_subnode;
        }

        hamt_node_insert(subnode, hash, lvl + 1, key, value);
    } else {
        // case: free node
        int children_size = __builtin_popcount(node->bitmap);
        int children_before = __builtin_popcount(shifted >> 1);

        // set new bit
        node->bitmap |= 1 << hash_chunk;
        hamt_node_t **new_children = (hamt_node_t**) malloc(sizeof(hamt_node_t*) * (children_size + 1));

        // copy over children until the spot for the new guy
        int i;
        for (i = 0; i < children_before; i++)
            new_children[i] = (node->children)[i];

        // create the new guy
        key_value_t *new_leaf = (key_value_t*) malloc(sizeof(key_value_t));
        new_leaf->key = key;
        new_leaf->value = value;

        // convert key_value_t* to hamt_node_t* for polymorphic type
        new_children[i] = (hamt_node_t*) new_leaf;

        // copy over remaining children
        for (; i < children_size; i++)
            new_children[i + 1] = (node->children)[i];

        // destroy the old children array
        free(node->children);
        node->children = new_children;
    }
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
    hamt_node_insert(trie->root, hash, 0, key, value);
}

void* hamt_search(hamt_t *trie, thing_t *key) {
    uint32_t hash = fnv1(key->x, key->len);
    return hamt_node_search(trie->root, hash, 0, key);
}
