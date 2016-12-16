import os
import sys

from distutils.core import setup
from distutils.extension import Extension
import getopt

optlist, args = getopt.getopt(sys.argv[3:], "l:", ["library-path="])
_LOCAL_LIBRARY_PATH = optlist[0][1]

sys.argv = sys.argv[:-1]

print("Installing C components of UCVM...")

ext_modules = [
    Extension("ucvm_c_common", ["common.c"], libraries=["etree"],
              library_dirs=[os.path.join(_LOCAL_LIBRARY_PATH, "euclid3", "lib")],
              include_dirs=[os.path.join(_LOCAL_LIBRARY_PATH, "euclid3", "include")])
]

setup(
    name="ucvm_c_common",
    ext_modules=ext_modules
)
print("\tDone!")
print("")
