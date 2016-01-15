from distutils.core import setup
from distutils.extension import Extension
from Cython.Build import cythonize

extensions = [
    Extension("hamta", ["hamta.pyx", "../lib/hamta.c"],
              extra_compile_args=["-std=c99", "-O3"],
              include_dirs=["../include"],
        )
]

setup(
    ext_modules = cythonize(extensions)
)
