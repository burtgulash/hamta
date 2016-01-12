#ifndef HAMTA_H
#define HAMTA_H
#include <stdint.h>
#include <stdbool.h>

// do not change macro of these values!  objects's last bit will be set to 1 if
// it has type hamt_node_t and 0 if key_value_t. This is possible since malloc
// will align to some multiple of even number.
#define KEY_VALUE_T_FLAG 0 // values can be NULL, so keep in this way
#define HAMT_NODE_T_FLAG 1

typedef struct thing {
    void *x;
    size_t len;
} thing_t;

typedef struct key_value {
    thing_t *key;
    thing_t *value;
} key_value_t;

typedef struct hamt_node {
    uint32_t bitmap;
    struct hamt_node **children;
} hamt_node_t;

typedef struct hamt {
    hamt_node_t *root;
    int size;
} hamt_t;

hamt_t* new_hamt();
void hamt_insert(hamt_t *trie, thing_t *key, thing_t *value);
key_value_t* hamt_search(hamt_t *trie, thing_t *key);
int hamt_size(hamt_t *trie);
void hamt_print(hamt_t *trie);

#endif
