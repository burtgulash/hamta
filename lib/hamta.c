#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include "hamta.h"

#define CHUNK_SIZE 5

// do not change macro of these values!  objects's last bit will be set to 1 if
// it has type hamt_node_t and 0 if key_value_t. This is possible since malloc
// will align to some multiple of even number.
#define KEY_VALUE_T_FLAG 0 // values can be NULL, so keep in this way
#define HAMT_NODE_T_FLAG 1


union hamt_node_;
typedef struct sub_node_ {
    uint32_t bitmap;
    union hamt_node_ *children;
} sub_node_t;

typedef union hamt_node_ {
    key_value_t leaf;
    sub_node_t sub;
} hamt_node_t;




bool thing_equals(thing_t *a, thing_t *b) {
    if (a == NULL || b == NULL)
        return a == b;
    if (a->len != b->len)
        return false;
    for (size_t i = 0; i < a->len; i++)
        if (((char*) a)[i] != ((char*) b)[i])
            return false;
    return true;
}

// FNV-1 Hash function
uint32_t hamt_fnv1_hash(void *key, size_t len) {
    uint32_t hash = 2166136261;
    for (size_t i = 0; i < len; i++) {
        hash *= 16777619;
        hash ^= ((char*) key)[i];
    }
    return hash;
}


int hamt_get_symbol(uint32_t hash, int lvl) {
    int left = lvl * CHUNK_SIZE;
    int left_plus_chunk = left + CHUNK_SIZE;
    int right = 32 - left_plus_chunk;
    if (left_plus_chunk > 32)
        right = 0;

    int symbol = (hash << left) >> (right + left);

    assert(0 <= symbol && symbol < 32);
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

key_value_t* hamt_node_search(hamt_node_t *node, uint32_t hash, int lvl, thing_t *key) {
    assert(node != NULL);

    if (is_leaf(node)) {
        if (thing_equals(node->leaf.key, key))
            return (key_value_t*) node;
    } else {
        hamt_node_t *children = get_children_pointer(node);
        int symbol = hamt_get_symbol(hash, lvl);
        int shifted = node->sub.bitmap >> symbol;
        bool child_exists = shifted & 1;

        if (child_exists) {
            // position of child is popcount of 1-bits to the left of bitmap at
            // keyed position
            int child_position = __builtin_popcount(shifted >> 1);
            return hamt_node_search(&children[child_position], hash, lvl + 1, key);
        }
    }

    return NULL;
}

bool hamt_node_insert(hamt_node_t *node, uint32_t hash, int lvl, thing_t *key, thing_t *value, hash_fn_t hash_fn) {
    if (lvl * CHUNK_SIZE > 32) {
        assert(false); // TODO make conflict arrays at the floor of the tree
        return false;
    }

    if (is_leaf(node)) {
        if (thing_equals(node->leaf.key, key)) {
            node->leaf.value = value;
            return false;
            // TODO add update flag as a function argument
        }

        uint32_t original_hash = hash_fn(node->leaf.key->x, node->leaf.key->len);
        int original_next_symbol = hamt_get_symbol(original_hash, lvl);

        hamt_node_t *new_children = (hamt_node_t*) malloc(sizeof(hamt_node_t) * 1);
        new_children[0].leaf.key = node->leaf.key;
        new_children[0].leaf.value = node->leaf.value;
        assert(is_leaf(&new_children[0]));

        node->sub.bitmap = 1 << original_next_symbol;
        node->sub.children = (hamt_node_t*) ((int) new_children | HAMT_NODE_T_FLAG);

        return hamt_node_insert(node, hash, lvl, key, value, hash_fn);
    }

    hamt_node_t *children = get_children_pointer(node);
    int symbol = hamt_get_symbol(hash, lvl);
    int shifted = (node->sub.bitmap) >> symbol;
    bool child_exists = shifted & 1;

    if (child_exists) {
        int child_position = __builtin_popcount(shifted >> 1);
        return hamt_node_insert(&children[child_position], hash, lvl + 1, key, value, hash_fn);
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

bool hamt_node_remove(hamt_node_t *node, uint32_t hash, int lvl, thing_t *key) {
    hamt_node_t *children = get_children_pointer(node);
    int symbol = hamt_get_symbol(hash, lvl);
    int shifted = (node->sub.bitmap) >> symbol;
    bool child_exists = shifted & 1;

    bool removed = false;
    if (child_exists) {
        int child_position = __builtin_popcount(shifted >> 1);
        hamt_node_t *subnode = &children[child_position];

        if (is_leaf(subnode)) {
            if (thing_equals(subnode->leaf.key, key)) {
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
                    for (; i < children_size; i++)
                        new_children[i] = children[i + 1];
                }

                // free the old array and set HAMT_NODE_T_FLAG in the new pointer
                free(children);
                node->sub.children = (hamt_node_t*) ((int) new_children | HAMT_NODE_T_FLAG);
                children = new_children;
            }
        } else
            removed = hamt_node_remove(subnode, hash, lvl + 1, key);
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

void hamt_node_destroy(hamt_node_t *node) {
    if (!is_leaf(node)) {
        hamt_node_t *children = get_children_pointer(node);

        int children_size = __builtin_popcount(node->sub.bitmap);
        for (int i = 0; i < children_size; i++)
            hamt_node_destroy(&children[i]);

        free(children);
    }
}

void hamt_node_print(hamt_node_t *node, int lvl) {
    for (int i = 0; i < lvl * 2; i++)
        putchar(' ');

    if (is_leaf(node)) {
        key_value_t *leaf = (key_value_t*) node;

        // %.*s means print non-null terminated string if the length is known
        printf("{%.*s -> %.*s}\n", leaf->key->len, (char*) leaf->key->x, leaf->value->len, (char*) leaf->value->x);
    } else {
        int children_size = __builtin_popcount(node->sub.bitmap);
        hamt_node_t *children = get_children_pointer(node);

        printf("bitmap: %08x\n", node->sub.bitmap);
        for (int i = 0; i < children_size; i++)
            hamt_node_print(&children[i], lvl + 1);
    }
}


// HAMTa constructor
hamt_t *new_hamt(hash_fn_t hash_fn) {
    hamt_t *h = (hamt_t*) malloc(sizeof(hamt_t));
    assert(h != NULL);

    h->root = (hamt_node_t*) malloc(sizeof(hamt_node_t) * 1);
    assert(h->root != NULL);

    h->root->leaf.key = NULL;
    h->root->leaf.value = NULL;
    h->size = 0;
    h->hash_fn = hash_fn;

    return h;
}

int hamt_size(hamt_t *trie) {
    return trie->size;
}

void hamt_insert(hamt_t *trie, thing_t *key, thing_t *value) {
    uint32_t hash = trie->hash_fn(key->x, key->len);
    bool inserted = false;

    // Turn root into sentinel on first insert
    if (trie->size == 0)
        trie->root->leaf.key = key;

    inserted = hamt_node_insert(trie->root, hash, 0, key, value, trie->hash_fn);
    if (trie->size == 0)
        inserted = true;

    if (inserted)
        trie->size++;
}

thing_t *hamt_search(hamt_t *trie, thing_t *key) {
    uint32_t hash = trie->hash_fn(key->x, key->len);
    key_value_t *found = hamt_node_search(trie->root, hash, 0, key);
    if (found != NULL)
        return found->value;

    return NULL;
}

bool hamt_remove(hamt_t *trie, thing_t *key) {
    uint32_t hash = trie->hash_fn(key->x, key->len);

    bool removed = false;
    if (trie->size == 0)
        return removed;
    else if (is_leaf(trie->root)) {
        assert(trie->size == 1);
        removed = trie->root[0].leaf.value;
    } else
        removed = hamt_node_remove(trie->root, hash, 0, key);

    if (removed)
        trie->size--;


    return removed;
}

void hamt_destroy(hamt_t *trie) {
    hamt_node_destroy(trie->root);
    free(trie->root);
    free(trie);
}

void hamt_print(hamt_t *trie) {
    if (trie->size > 0)
        hamt_node_print(trie->root, 0);
    else
        printf("{}\n");
    printf("---\n\n");
}
