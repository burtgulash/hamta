from cpython cimport Py_INCREF, Py_DECREF
from libc cimport stdlib
cimport c_hamta

cdef class Hamt:
    cdef c_hamta.hamt_t* _c_hamt

    def __cinit__(self):
        self._c_hamt = c_hamta.new_hamt(c_hamta.hamt_fnv1_hash)
        if self._c_hamt is NULL:
            raise MemoryError()

    def __setitem__(self, int key, object value):
        cdef c_hamta.thing_t *k = <c_hamta.thing_t*> stdlib.malloc(sizeof(c_hamta.thing_t))
        cdef c_hamta.thing_t *v = <c_hamta.thing_t*> stdlib.malloc(sizeof(c_hamta.thing_t))
        if not k or not v:
            raise MemoryError()

        k[0] = c_hamta.thing_t(key, sizeof(int))
        v[0] = c_hamta.thing_t(<void*> value, sizeof(void*))

        Py_INCREF(value)

        cdef bint inserted = c_hamta.hamt_insert(self._c_hamt, k, v)
        if not inserted:
            stdlib.free(v);
            Py_DECREF(value)


    def __dealloc__(self):
        if self._c_hamt is not NULL:
            c_hamta.hamt_destroy(self._c_hamt)

