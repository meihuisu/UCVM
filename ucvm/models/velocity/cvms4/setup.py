from distutils.core import setup
from distutils.extension import Extension
from Cython.Distutils import build_ext

ext_modules = [
    Extension("CVMS4VelocityModel", ["cvms4.pyx"], extra_compile_args=["-Wunused-function"],
              libraries=["cvms"], library_dirs=["./fortran_src/"])
]

setup(
    name="CVMS4",
    cmdclass={"build_ext": build_ext},
    ext_modules=ext_modules
)
