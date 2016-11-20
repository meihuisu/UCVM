"""
Setup.py Script

Copyright:
    Southern California Earthquake Center

Developer:
    David Gill <davidgil@usc.edu>
"""
# Python Imports
from distutils.core import setup
from distutils.extension import Extension
from Cython.Distutils import build_ext

ext_modules = [
    Extension("ucvm_c_common", ["common.pyx"])
]

setup(
    name="ucvm_c_common",
    cmdclass={"build_ext": build_ext},
    ext_modules=ext_modules
)