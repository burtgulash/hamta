#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include "hamta.h"

// detect 64 and 32 bit architectures
#if UINTPTR_MAX > 0xffffffff
    #define CHUNK_SIZE 6
    #define FNV_BASE 14695981039346656037
    #define FNV_PRIME 1099511628211
#else
    #define CHUNK_SIZE 5
    #define FNV_BASE 2166136261
    #define FNV_PRIME 16777619;
#endif


#define KEY_VALUE_T_FLAG 0 // leaf.values can be NULL, so keep order of constants this way
#define HAMT_NODE_T_FLAG 1



union hamt_node_;
typedef struct sub_node_ {
    unsigned int bitmap;
    union hamt_node_ *children;
} sub_node_t;

typedef union hamt_node_ {
    key_value_t leaf;
    sub_node_t sub;
} hamt_node_t;




bool hamt_int_equals(void *a, void *b) {
    if (a == NULL || b == NULL)
        return a == b;
    return memcmp(a, b, sizeof(int)) == 0;
}

bool hamt_str_equals(void *a, void *b) {
    if (a == NULL || b == NULL)
        return a == b;
    return strcmp(a, b) == 0;
}

// FNV-1 Hash function
unsigned int hamt_fnv1_hash(void *key, size_t len) {
    unsigned int hash = FNV_BASE;
    for (size_t i = 0; i < len; i++) {
        hash *= FNV_PRIME;
        hash ^= ((char*) key)[i];
    }
    return hash;
}

unsigned int hamt_int_hash(void *key) {
    return hamt_fnv1_hash(key, sizeof(int));
}

// FNV-1 Hash function
unsigned int hamt_str_hash(void *key) {
    unsigned int hash = FNV_BASE;
    for (; *(char*) key != 0; key ++) {
        hash *= FNV_PRIME;
        hash ^= *(char*) key;
    }
    return hash;
}


int hamt_get_symbol(unsigned int hash, int lvl) {
    int left = lvl * CHUNK_SIZE;
    int left_plus_chunk = left + CHUNK_SIZE;
    int right = 32 - left_plus_chunk;
    if (left_plus_chunk > 32)
        right = 0;

    unsigned int symbol = hash << left;
    // UINTPTR_MAX is 0xffffffff on 32bit and 0xffffffffffffffff
    assert((symbol & UINTPTR_MAX) == symbol);
    symbol >>= (right + left);

    assert(symbol < 32);
    return symbol;
}

hamt_node_t* get_children_pointer(hamt_node_t *node) {
    int children_ptr = (int) (node->sub.children);
    children_ptr &= ~HAMT_NODE_T_FLAG;
    return (hamt_node_t*) children_ptr;
}

bool is_leaf(hamt_node_t *node) {
    assert(node != NULL);
    int children_ptr = (int) (node->sub.children);
    return (children_ptr & 1) == KEY_VALUE_T_FLAG;
}

// TODO return const key_value_t*
key_value_t *hamt_node_search(hamt_node_t *node, unsigned int hash, int lvl, void *key, equals_fn_t equals_fn) {
    assert(node != NULL);

    if (is_leaf(node)) {
        if (equals_fn(node->leaf.key, key))
            return (key_value_t*) node;
    } else {
        hamt_node_t *children = get_children_pointer(node);
        int symbol = hamt_get_symbol(hash, lvl);
        unsigned int shifted = node->sub.bitmap >> symbol;
        bool child_exists = shifted & 1;

        if (child_exists) {
            // position of child is popcount of 1-bits to the left of bitmap at
            // keyed position
            int child_position = __builtin_popcount(shifted >> 1);
            return hamt_node_search(&children[child_position], hash, lvl + 1, key, equals_fn);
        }
    }

    return NULL;
}

// return true if size of the tree increases after inserting
bool hamt_node_insert(hamt_node_t *node, unsigned int hash, int lvl, void *key, void *value,
                      hash_fn_t hash_fn, equals_fn_t equals_fn, key_value_t *conflict_kv) {
    if (lvl * CHUNK_SIZE > 32) {
        assert(false); // TODO make conflict arrays at the floor of the tree
        return false;
    }

    if (is_leaf(node)) {
        if (equals_fn(node->leaf.key, key)) {
            conflict_kv->key = node->leaf.key;
            conflict_kv->value = node->leaf.value;

            node->leaf.key = key;
            node->leaf.value = value;

            assert(is_leaf(node));

            return false;
        }

        unsigned int original_hash = hash_fn(node->leaf.key);
        int original_next_symbol = hamt_get_symbol(original_hash, lvl);

        hamt_node_t *new_children = (hamt_node_t*) malloc(sizeof(hamt_node_t) * 1);
        new_children[0].leaf.key = node->leaf.key;
        new_children[0].leaf.value = node->leaf.value;
        assert(is_leaf(&new_children[0]));

        node->sub.bitmap = 1 << original_next_symbol;
        node->sub.children = (hamt_node_t*) ((int) new_children | HAMT_NODE_T_FLAG);

        return hamt_node_insert(node, hash, lvl, key, value, hash_fn, equals_fn, conflict_kv);
    }

    hamt_node_t *children = get_children_pointer(node);
    int symbol = hamt_get_symbol(hash, lvl);
    unsigned int shifted = (node->sub.bitmap) >> symbol;
    bool child_exists = shifted & 1;

    if (child_exists) {
        int child_position = __builtin_popcount(shifted >> 1);
        return hamt_node_insert(&children[child_position], hash, lvl + 1, key, value, hash_fn, equals_fn, conflict_kv);
    } else {
        // case: free node

        int children_size = __builtin_popcount(node->sub.bitmap);
        int children_before = __builtin_popcount(shifted >> 1);

        assert(children_size > 0);

        // set new bit
        node->sub.bitmap |= 1 << symbol;
        hamt_node_t *new_children = (hamt_node_t*) malloc(sizeof(hamt_node_t) * (children_size + 1));

        // copy over children until the spot for the new guy
        int i;
        for (i = 0; i < children_before; i++)
            new_children[i] = children[i];

        new_children[i].leaf.key = key;
        new_children[i].leaf.value = value;
        assert(is_leaf(&new_children[i]));

        for (; i < children_size; i++)
            new_children[i + 1] = children[i];

        // destroy the old children array
        free(children);
        node->sub.children = (hamt_node_t*) ((int) new_children | HAMT_NODE_T_FLAG);

        return true;
    }
}

bool hamt_node_remove(hamt_node_t *node, unsigned int hash, int lvl, void *key, equals_fn_t equals_fn, key_value_t *removed_kv) {
    hamt_node_t *children = get_children_pointer(node);
    int symbol = hamt_get_symbol(hash, lvl);
    unsigned int shifted = (node->sub.bitmap) >> symbol;
    bool child_exists = shifted & 1;

    bool removed = false;
    if (child_exists) {
        int child_position = __builtin_popcount(shifted >> 1);
        hamt_node_t *subnode = &children[child_position];

        if (is_leaf(subnode)) {
            if (equals_fn(subnode->leaf.key, key)) {
                int children_size = __builtin_popcount(node->sub.bitmap);
                assert(children_size);

                // clear the leaf's bit
                node->sub.bitmap &= ~(1 << symbol);
                children_size--;

                removed = true;

                hamt_node_t *new_children = NULL;
                if (children_size > 0) {
                    new_children = (hamt_node_t*) malloc(sizeof(hamt_node_t) * children_size);

                    int i;
                    for (i = 0; i < child_position; i++)
                        new_children[i] = children[i];

                    removed_kv->key = subnode->leaf.key;
                    removed_kv->value = subnode->leaf.value;

                    for (; i < children_size; i++)
                        new_children[i] = children[i + 1];
                }

                // free the old array and set HAMT_NODE_T_FLAG in the new pointer
                free(children);
                node->sub.children = (hamt_node_t*) ((int) new_children | HAMT_NODE_T_FLAG);
                children = new_children;
            }
        } else
            removed = hamt_node_remove(subnode, hash, lvl + 1, key, equals_fn, removed_kv);
    }

    // if some element below was removed, check if the children array has only
    // one element and should collapse
    int children_size = __builtin_popcount(node->sub.bitmap);
    if (children_size < 2) {
        // If children array would contain only one element after node removal,
        // then collapse this single element array one level up to a single
        // key_value node
        assert(children_size == 1);
        hamt_node_t *only_remaining_child = &children[0];
        if (is_leaf(only_remaining_child)) {
            *node = *only_remaining_child;
            free(children);
        }
    }

    return removed;
}

void hamt_node_destroy(hamt_node_t *node, deallocate_fn_t deallocate_fn) {
    if (!is_leaf(node)) {
        hamt_node_t *children = get_children_pointer(node);

        int children_size = __builtin_popcount(node->sub.bitmap);
        for (int i = 0; i < children_size; i++)
            hamt_node_destroy(&children[i], deallocate_fn);

        free(children);
    } else if (deallocate_fn != NULL) {
        deallocate_fn(node->leaf.key);
        deallocate_fn(node->leaf.value);
    }
}

void hamt_node_print(hamt_node_t *node, int lvl, str_fn_t str_fn) {
    for (int i = 0; i < lvl * 2; i++)
        putchar(' ');

    if (is_leaf(node)) {
        key_value_t *leaf = (key_value_t*) node;
        printf("{%s -> %s}\n", str_fn(leaf->key), str_fn(leaf->value));
    } else {
        int children_size = __builtin_popcount(node->sub.bitmap);
        hamt_node_t *children = get_children_pointer(node);

        printf("bitmap: %08x\n", node->sub.bitmap);
        for (int i = 0; i < children_size; i++)
            hamt_node_print(&children[i], lvl + 1, str_fn);
    }
}


// HAMTa constructor
hamt_t *new_hamt(hash_fn_t hash_fn, equals_fn_t equals_fn) {
    hamt_t *h = (hamt_t*) malloc(sizeof(hamt_t));
    assert(h != NULL);

    h->root = (hamt_node_t*) malloc(sizeof(hamt_node_t) * 1);
    assert(h->root != NULL);

    h->root->leaf.key = NULL;
    h->root->leaf.value = NULL;
    h->size = 0;
    h->hash_fn = hash_fn;
    h->equals_fn = equals_fn;

    return h;
}

int hamt_size(hamt_t *trie) {
    return trie->size;
}


// return true if original_kv should be freed
bool hamt_set(hamt_t *trie, void *key, void *value, key_value_t *conflict_kv) {
    unsigned int hash = trie->hash_fn(key);
    bool inserted = false;

    if (trie->size == 0) {
        trie->root->leaf.key = key;
        trie->root->leaf.value = value;
        trie->size = 1;
        return false;
    }

    inserted = hamt_node_insert(trie->root, hash, 0, key, value, trie->hash_fn, trie->equals_fn, conflict_kv);

    if (inserted)
        trie->size ++;

    return !inserted;
}


key_value_t *hamt_search(hamt_t *trie, void *key) {
    unsigned int hash = trie->hash_fn(key);
    return hamt_node_search(trie->root, hash, 0, key, trie->equals_fn);
}

bool hamt_remove(hamt_t *trie, void *key, key_value_t *removed_kv) {
    unsigned int hash = trie->hash_fn(key);

    if (trie->size == 0)
        return false;

    bool removed = false;
    if (trie->size == 1) {
        assert(is_leaf(trie->root));
        removed_kv->key = trie->root->leaf.key;
        removed_kv->value = trie->root->leaf.value;
        removed = true;
    } else
        removed = hamt_node_remove(trie->root, hash, 0, key, trie->equals_fn, removed_kv);

    if (removed)
        trie->size--;

    return removed;
}

void hamt_destroy(hamt_t *trie, deallocate_fn_t deallocate_fn) {
    if (trie->size > 0)
        hamt_node_destroy(trie->root, deallocate_fn);
    free(trie->root);
    free(trie);
}

void hamt_print(hamt_t *trie, str_fn_t str_fn) {
    if (trie->size > 0)
        hamt_node_print(trie->root, 0, str_fn);
    else
        printf("{}\n");
    printf("---\n\n");
}
