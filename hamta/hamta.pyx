from cpython cimport Py_INCREF, Py_DECREF, PyObject
from cpython.mem cimport PyMem_Malloc, PyMem_Free
from libc.string cimport memcpy
cimport c_hamta

cdef class Hamt:
    cdef c_hamta.hamt_t* _c_hamt

    def __cinit__(self):
        self._c_hamt = c_hamta.new_hamt(c_hamta.hamt_fnv1_hash)
        if self._c_hamt is NULL:
            raise MemoryError()

    def __len__(self):
        return c_hamta.hamt_size(self._c_hamt)

    def __setitem__(self, int key, int value):
        cdef c_hamta.thing_t* k = <c_hamta.thing_t*> PyMem_Malloc(sizeof(c_hamta.thing_t))
        cdef c_hamta.thing_t* v = <c_hamta.thing_t*> PyMem_Malloc(sizeof(c_hamta.thing_t))
        if not k or not v:
            raise MemoryError()

        cdef void* k_mem = <void*> PyMem_Malloc(sizeof(int))
        cdef void* v_mem = <void*> PyMem_Malloc(sizeof(int))
        if not k_mem or not v_mem:
            raise MemoryError()

        memcpy(k_mem, &key, sizeof(int))
        memcpy(v_mem, &value, sizeof(int))

        k.x = k_mem
        k.len = sizeof(int)

        v.x = v_mem
        v.len = sizeof(int)

        cdef c_hamta.key_value_t original_kv
        cdef bint delete_ld = c_hamta.hamt_set(self._c_hamt, k, v, &original_kv)

        if delete_old:
            PyMem_Free(original_kv.key.x)
            PyMem_Free(original_kv.key)
            PyMem_Free(original_kv.value.x)
            PyMem_Free(original_kv.value)

    def __getitem__(self, int key):
        cdef void* mem = <void*> PyMem_Malloc(sizeof(int))
        cdef c_hamta.thing_t key_thing
        key_thing.x = mem
        key_thing.len = sizeof(int)

        cdef c_hamta.thing_t* value = c_hamta.hamt_search(self._c_hamt, &key_thing)
        PyMem_Free(mem)

        if value is NULL:
            raise KeyError("Item with key %s not found!" % key)
        return <int> value.x


    def __dealloc__(self):
        if self._c_hamt is not NULL:
            c_hamta.hamt_destroy(self._c_hamt)

