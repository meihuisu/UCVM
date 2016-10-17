import os

from distutils.core import setup
from distutils.extension import Extension
from Cython.Distutils import build_ext

from ucvm.src.shared.constants import UCVM_MODELS_DIRECTORY

ext_modules = [
    Extension("BayAreaVelocityModel", ["bayarea.pyx"], libraries=["cencalvm"],
              library_dirs=[os.path.join(UCVM_MODELS_DIRECTORY, "bayarea", "lib")])
]

setup(
    name="BayArea",
    cmdclass={"build_ext": build_ext},
    ext_modules=ext_modules
)
