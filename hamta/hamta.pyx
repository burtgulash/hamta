from cpython cimport Py_INCREF, Py_DECREF, PyObject
from cpython.mem cimport PyMem_Malloc, PyMem_Free
from libc.string cimport memcpy
cimport c_hamta

cdef class Hamt:
    cdef c_hamta.hamt_t* _c_hamt

    def __cinit__(self):
        self._c_hamt = c_hamta.new_hamt(c_hamta.hamt_int_hash, c_hamta.hamt_int_equals)
        if self._c_hamt is NULL:
            raise MemoryError()

    def __len__(self):
        return c_hamta.hamt_size(self._c_hamt)

    def __setitem__(self, int key, int value):
        cdef void* k_mem = <void*> PyMem_Malloc(sizeof(int))
        cdef void* v_mem = <void*> PyMem_Malloc(sizeof(int))
        if not k_mem or not v_mem:
            raise MemoryError()

        memcpy(k_mem, &key, sizeof(int))
        memcpy(v_mem, &value, sizeof(int))

        cdef c_hamta.key_value_t conflict_kv
        cdef bint conflicted = c_hamta.hamt_set(self._c_hamt, k_mem, v_mem, &conflict_kv)

        if conflicted:
            PyMem_Free(conflict_kv.key)
            PyMem_Free(conflict_kv.value)


    def __getitem__(self, int key):
        cdef c_hamta.key_value_t* found = c_hamta.hamt_search(self._c_hamt, <void*> &key)
        if found is NULL:
            raise KeyError("Item with key %s not found!" % key)
        return (<int*> found.value)[0]


    def __delitem__(self, int key):
        cdef c_hamta.key_value_t removed_kv
        cdef bint removed = c_hamta.hamt_remove(self._c_hamt, <void*> &key, &removed_kv)

        if removed:
            PyMem_Free(removed_kv.key)
            PyMem_Free(removed_kv.value)


    def __dealloc__(self):
        if self._c_hamt is not NULL:
            c_hamta.hamt_destroy(self._c_hamt, PyMem_Free)

