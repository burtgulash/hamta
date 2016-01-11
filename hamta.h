#ifndef HAMTA_H
#define HAMTA_H
#include <stdint.h>

typedef struct {
    hamt_node_t *root;
} hamt;

typedef struct {
    void *key;
    void *value;
} key_value_t;

typedef struct {
    uint32_t bitmap;
    hamt_node_t **children;
} hamt_node_t;

#endif
