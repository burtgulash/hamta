#ifndef HAMTA_H
#define HAMTA_H
#include <stdint.h>

// do not change macro of these values!  objects's last bit will be set to 1 if
// it has type hamt_node_t and 0 if key_value_t. This is possible since malloc
// will align to some multiple of even number.
#define KEY_VALUE_T_FLAG 0
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
} hamt_t;

#endif
