#ifndef HAMTA_H
#define HAMTA_H
#include <stdlib.h>
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

typedef uint32_t (*hash_fn_t)(void *key, size_t len);
union hamt_node_;
typedef struct {
    union hamt_node_ *root;
    int size;
    hash_fn_t hash_fn;
} hamt_t;


hamt_t *new_hamt(hash_fn_t hash_fn);
int hamt_size(hamt_t *trie);
bool hamt_set(hamt_t *trie, thing_t *key, thing_t *value, key_value_t *original_kv);
thing_t *hamt_search(hamt_t *trie, thing_t *key);
bool hamt_remove(hamt_t *trie, thing_t *key);
void hamt_destroy(hamt_t *trie, bool free_things, bool free_values);
void hamt_print(hamt_t *trie);

#endif
