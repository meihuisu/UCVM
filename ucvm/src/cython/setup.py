import os

from distutils.core import setup
from distutils.extension import Extension
from Cython.Distutils import build_ext

from ucvm.src.shared.constants import UCVM_LIBRARIES_DIRECTORY
_LOCAL_LIBRARY_PATH = UCVM_LIBRARIES_DIRECTORY

print("Installing C components of UCVM...")

ext_modules = [
    Extension("ucvm_c_common", ["ucvm_c_common.pyx"], libraries=["etree"],
              library_dirs=[os.path.join(_LOCAL_LIBRARY_PATH, "euclid3", "lib")],
              include_dirs=[os.path.join(_LOCAL_LIBRARY_PATH, "euclid3", "include")])
]

setup(
    name="ucvm_c_common",
    cmdclass={"build_ext": build_ext},
    ext_modules=ext_modules
)
print("\tDone!")
print("")
