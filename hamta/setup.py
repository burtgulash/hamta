from distutils.core import setup
from distutils.extension import Extension
from Cython.Build import cythonize

extensions = [
    Extension("hamta", ["hamta.pyx"],
        include_dirs=["../include"],
        libraries=["hamta"],
        library_dirs=["../bin"],
        runtime_library_dirs=["../bin"])
]

setup(
    ext_modules = cythonize(extensions)
)
