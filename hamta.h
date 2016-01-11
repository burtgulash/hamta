#ifndef HAMTA_H
#define HAMTA_H
#include <stdint.h>

typedef struct key_value {
    void *key;
    void *value;
} key_value_t;

typedef struct hamt_node {
    uint32_t bitmap;
    struct hamt_node **children;
} hamt_node_t;

typedef struct hamt {
    hamt_node_t *root;
} hamt_t;

#endif
