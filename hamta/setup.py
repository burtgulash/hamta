from distutils.core import setup
from distutils.extension import Extension
from Cython.Build import cythonize

extensions = [
    Extension("hamta", ["hamta.pyx", "../lib/hamta.c"],
              extra_compile_args=["-O3", "-std=c99"],
              include_dirs=["../include"],
        )
        #library_dirs=["../bin"],
        #runtime_library_dirs=["../bin"])
]

setup(
    ext_modules = cythonize(extensions)
)
