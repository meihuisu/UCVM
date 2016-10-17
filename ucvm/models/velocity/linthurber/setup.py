from distutils.core import setup
from distutils.extension import Extension
from Cython.Distutils import build_ext

ext_modules = [
    Extension("LinThurberVelocityModel", ["linthurber.pyx"],
              extra_compile_args=["-Wunused-function"],
              libraries=["cvmlt"], library_dirs=["./src/src"])
]

setup(
    name="LinThurber",
    cmdclass={"build_ext": build_ext},
    ext_modules=ext_modules
)
