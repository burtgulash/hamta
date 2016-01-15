#ifndef HAMTA_H
#define HAMTA_H
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

typedef uint32_t (*hash_fn_t)(void *key);
typedef bool (*equals_fn_t)(void *a, void *b);
typedef char* (*str_fn_t)(void *a);
typedef void (*deallocate_fn_t)(void *ptr);

uint32_t hamt_int_hash(void *key);
uint32_t hamt_str_hash(void *key);

bool hamt_int_equals(void *a, void *b);
bool hamt_str_equals(void *a, void *b);


// TODO make const void* ?
typedef struct {
    void *key;
    void *value;
} key_value_t;

union hamt_node_;
typedef struct {
    union hamt_node_ *root;
    int size;
    hash_fn_t hash_fn;
    equals_fn_t equals_fn;
} hamt_t;


hamt_t *new_hamt(hash_fn_t hash_fn, equals_fn_t equals_fn);
int hamt_size(hamt_t *trie);
bool hamt_set(hamt_t *trie, void *key, void *value, key_value_t *conflict_kv);
key_value_t *hamt_search(hamt_t *trie, void *key);
bool hamt_remove(hamt_t *trie, void *key, key_value_t *removed_kv);
void hamt_destroy(hamt_t *trie, deallocate_fn_t deallocate_fn);
void hamt_print(hamt_t *trie, str_fn_t str_fn);

#endif
