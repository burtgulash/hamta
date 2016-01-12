#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>
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

// FNV-1 Hash function
uint32_t fnv1(void *key, size_t len) {
    uint32_t hash = 2166136261;
    for (size_t i = 0; i < len; i++) {
        hash *= 16777619;
        hash ^= ((char*) key)[i];
    }
    return hash;
}


int _hamt_get_symbol(uint32_t hash, int lvl) {
    int left = lvl * CHUNK_SIZE;
    int left_plus_chunk = left + CHUNK_SIZE;
    int right = 32 - left_plus_chunk;
    if (left_plus_chunk > 32)
        right = 0;

    int symbol = (hash << left) >> (right + left);

    assert(0 <= symbol && symbol < 32);
    return symbol;
}

key_value_t* new_kv(thing_t *key, thing_t *value) {
    key_value_t *kv = (key_value_t*) malloc(sizeof(key_value_t));
    kv->key = key;
    kv->value = value;
    return kv;
}

hamt_node_t** get_children_pointer(hamt_node_t *node) {
    int children_ptr_val = (int) (node->children);
    children_ptr_val &= ~HAMT_NODE_T_FLAG;
    return (hamt_node_t**) children_ptr_val;
}


key_value_t* hamt_node_search(hamt_node_t *node, uint32_t hash, int lvl, thing_t *key) {
    assert(node != NULL);

    hamt_node_t **children = get_children_pointer(node);
    int symbol = _hamt_get_symbol(hash, lvl);
    int shifted = node->bitmap >> symbol;
    bool child_exists = shifted & 1;

    if (child_exists) {
        // position of child is popcount of 1-bits to the left of bitmap at
        // keyed position
        int child_position = __builtin_popcount(shifted >> 1);
        hamt_node_t *subnode = children[child_position];

        int subchildren_ptr_val = (int) (subnode->children);
        if ((subchildren_ptr_val & 1) == KEY_VALUE_T_FLAG) {
            key_value_t *leaf = (key_value_t*) subnode;
            if (thing_equals(leaf->key, key))
                return leaf;
        } else
            return hamt_node_search(subnode, hash, lvl + 1, key);
    }

    return NULL;
}

hamt_node_t* _hamt_make_subnode(key_value_t *first_child, int symbol) {
    hamt_node_t *new_subnode = (hamt_node_t*) malloc(sizeof(hamt_node_t));
    // set first bit
    new_subnode->bitmap = 1 << symbol;
    // make children array of size one for only the original node
    new_subnode->children = (hamt_node_t**) malloc(sizeof(hamt_node_t*) * 1);
    // copy over original conflicting key_value
    new_subnode->children[0] = (hamt_node_t*) first_child;
    // set the type flag
    new_subnode->children = (hamt_node_t**) ((int) (new_subnode->children) | HAMT_NODE_T_FLAG);
    return new_subnode;
}

bool hamt_node_insert(hamt_node_t *node, uint32_t hash, int lvl,
                                         thing_t *key, thing_t *value) {
    assert(node != NULL);
    if (lvl * CHUNK_SIZE > 32) {
        assert(false); // TODO make conflict arrays at the floor of the tree
        return false;
    }

    hamt_node_t **children = get_children_pointer(node);
    int symbol = _hamt_get_symbol(hash, lvl);
    int shifted = (node->bitmap) >> symbol;
    bool child_exists = shifted & 1;

    if (child_exists) {
        int child_position = __builtin_popcount(shifted >> 1);
        hamt_node_t *subnode = children[child_position];

        int subchildren_ptr_val = (int) (subnode->children);
        if ((subchildren_ptr_val & 1) == KEY_VALUE_T_FLAG) {
            // case: conflict with another key_value

            key_value_t *leaf = (key_value_t*) subnode;
            // key already inside, exit
            if (thing_equals(leaf->key, key))
                return false;

            uint32_t original_hash = fnv1(leaf->key->x, leaf->key->len);
            int subnode_symbol = _hamt_get_symbol(original_hash, lvl + 1);
            hamt_node_t *new_subnode = _hamt_make_subnode(leaf, subnode_symbol);

            // set parent's child to new_subnode
            children[child_position] = new_subnode;
            subnode = new_subnode;
        }

        return hamt_node_insert(subnode, hash, lvl + 1, key, value);
    } else {
        // case: free node
        int children_size = __builtin_popcount(node->bitmap);
        int children_before = __builtin_popcount(shifted >> 1);

        // set new bit
        node->bitmap |= 1 << symbol;
        hamt_node_t **new_children = (hamt_node_t**) malloc(sizeof(hamt_node_t*) * (children_size + 1));

        // copy over children until the spot for the new guy
        int i;
        for (i = 0; i < children_before; i++)
            new_children[i] = children[i];

        key_value_t *new_leaf = new_kv(key, value);

        // convert key_value_t* to hamt_node_t* for polymorphic type
        new_children[i] = (hamt_node_t*) new_leaf;

        // copy over remaining children
        for (; i < children_size; i++)
            new_children[i + 1] = children[i];

        // destroy the old children array
        free(children);
        node->children = (hamt_node_t**) ((int) new_children | HAMT_NODE_T_FLAG);

        return true;
    }
}

key_value_t* hamt_node_remove(hamt_node_t *node, uint32_t hash, int lvl, thing_t *key) {
    assert(node != NULL);

    hamt_node_t **children = get_children_pointer(node);
    int symbol = _hamt_get_symbol(hash, lvl);
    int shifted = (node->bitmap) >> symbol;
    bool child_exists = shifted & 1;

    if (child_exists) {
        int child_position = __builtin_popcount(shifted >> 1);
        hamt_node_t *subnode = children[child_position];

        int subchildren_ptr_val = (int) (subnode->children);
        if ((subchildren_ptr_val & 1) == KEY_VALUE_T_FLAG) {
            key_value_t *leaf = (key_value_t*) subnode;
            if (thing_equals(leaf->key, key)) {
                int children_size = __builtin_popcount(node->bitmap);

                // TODO remove the node
                return leaf;
            }
        } else
            return hamt_node_remove(subnode, hash, lvl + 1, key);
    }

    return NULL;
}

void hamt_node_print(hamt_node_t *node, int lvl) {
    for (int i = 0; i < lvl * 2; i++)
        putchar(' ');

    int children_ptr_val = (int) (node->children);
    if ((children_ptr_val & 1) == HAMT_NODE_T_FLAG) {
        int children_size = __builtin_popcount(node->bitmap);
        hamt_node_t **children = get_children_pointer(node);

        printf("bitmap: %08x\n", node->bitmap);
        for (int i = 0; i < children_size; i++)
            hamt_node_print(children[i], lvl + 1);
    } else {
        key_value_t *leaf = (key_value_t*) node;
        printf("{%0*x -> %0*x}\n", leaf->key->len, (int) leaf->key->x, leaf->value->len, (int) leaf->value->x);
    }
}


// HAMTa constructor
hamt_t* new_hamt() {
    hamt_t *h = (hamt_t*) malloc(sizeof(hamt_t));
    assert(h);
    h->root = (hamt_node_t*) malloc(sizeof(hamt_node_t));
    h->size = 0;
    return h;
}

int hamt_size(hamt_t *trie) {
    return trie->size;
}

void hamt_insert(hamt_t *trie, thing_t *key, thing_t *value) {
    uint32_t hash = fnv1(key->x, key->len);
    bool inserted = false;

    if (trie->size == 0) {
        key_value_t *new_leaf = new_kv(key, value);

        int symbol = hash >> (32 - CHUNK_SIZE);
        trie->root = _hamt_make_subnode(new_leaf, symbol);

        inserted = true;
    } else
        inserted = hamt_node_insert(trie->root, hash, 0, key, value);

    if (inserted)
        trie->size++;
}

key_value_t* hamt_search(hamt_t *trie, thing_t *key) {
    uint32_t hash = fnv1(key->x, key->len);
    return hamt_node_search(trie->root, hash, 0, key);
}

void hamt_print(hamt_t *trie) {
    if (trie->size > 0)
        hamt_node_print(trie->root, 0);
    else
        printf("{}\n");
    printf("---\n\n");
}
