#ifndef HAMTA_H
#define HAMTA_H
#include <stdint.h>
#include <stdbool.h>

uint32_t hamt_fnv1_hash(void *key, size_t len);


typedef struct {
    void *x;
    size_t len;
} thing_t;

bool thing_equals(thing_t *a, thing_t *b);


typedef struct {
    thing_t *key;
    thing_t *value;
} key_value_t;

union hamt_node_;
typedef struct {
    union hamt_node_ *root;
    int size;
} hamt_t;


hamt_t* new_hamt();
int hamt_size(hamt_t *trie);
void hamt_insert(hamt_t *trie, thing_t *key, thing_t *value);
key_value_t* hamt_search(hamt_t *trie, thing_t *key);
key_value_t* hamt_remove(hamt_t *trie, thing_t *key);
void hamt_print(hamt_t *trie);

#endif
