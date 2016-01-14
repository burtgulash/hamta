
cdef extern from "hamta.h":
    ctypedef struct thing_t:
        void* x
        size_t len
    ctypedef struct key_value_t:
        thing_t* key
        thing_t* value
    ctypedef struct hamt_t:
        pass

    int hamt_fnv1_hash(void* key, size_t length)

    hamt_t* new_hamt(int(*hash_fn)(void* key, size_t length))
    void hamt_destroy(hamt_t* h, bint free_things, bint free_values)

    int hamt_size(hamt_t* h)
    thing_t* hamt_search(hamt_t* h, thing_t* key)
    thing_t* hamt_remove(hamt_t* h, thing_t* key)
    bint hamt_set(hamt_t* h, thing_t* key, thing_t* value, key_value_t* original_kv)
