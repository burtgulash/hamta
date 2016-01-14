cimport c_hamta

cdef class Hamt:
    cdef c_hamta.hamt_t* _c_hamt

    def __cinit__(self):
        self._c_hamt = c_hamta.new_hamt(c_hamta.hamt_fnv1_hash)
        if self._c_hamt is NULL:
            raise MemoryError()

    def __dealloc__(self):
        if self._c_hamt is not NULL:
            c_hamta.hamt_destroy(self._c_hamt)

