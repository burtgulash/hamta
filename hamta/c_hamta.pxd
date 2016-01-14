cdef extern from "hamta.h":
    ctypedef struct hamt_t:
        pass
    ctypedef struct thing_t:
        pass

    int hamt_fnv1_hash(void *key, size_t length)

    hamt_t *new_hamt(int(*hash_fn)(void *key, size_t length))
    void hamt_destroy(hamt_t *h)

    int hamt_size(hamt_t *h)
    thing_t *hamt_search(hamt_t *h, thing_t *key)
    thing_t *hamt_remove(hamt_t *h, thing_t *key)
    void hamt_insert(hamt_t *h, thing_t *key, thing_t *value)
