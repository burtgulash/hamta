
cdef extern from "hamta.h":
    ctypedef struct key_value_t:
        void* key
        void* value
    ctypedef struct hamt_t:
        pass

    unsigned int hamt_int_hash(void* key)
    bint hamt_int_equals(void* a, void* b)

    hamt_t* new_hamt(unsigned int(*hash_fn)(void* key), bint(*equals_fn)(void* a, void* b))
    void hamt_destroy(hamt_t* h, void(*deallocate_fn)(void* ptr))

    int hamt_size(hamt_t* h)
    key_value_t* hamt_search(hamt_t* h, void* key)
    bint hamt_remove(hamt_t* h, void* key, key_value_t* removed_kv)
    bint hamt_set(hamt_t* h, void* key, void* value, key_value_t* conflict_kv)
