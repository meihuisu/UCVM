"""
Setup.py Script

The CVM-S4 model setup.py script. This is fairly basic and generic, specifying how the
velocity model is supposed to be installed.

Copyright:
    Southern California Earthquake Center

Developer:
    David Gill <davidgil@usc.edu>
"""
# Python Imports
from distutils.core import setup
from distutils.extension import Extension

ext_modules = [
    Extension("CVMS4VelocityModel", ["cvms4.c"], extra_compile_args=["-Wunused-function"],
              include_dirs=["src"], libraries=["cvms"], library_dirs=["./src/"])
]

setup(
    name="CVMS4",
    ext_modules=ext_modules
)
