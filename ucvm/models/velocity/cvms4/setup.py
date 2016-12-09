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
from Cython.Distutils import build_ext

ext_modules = [
    Extension("CVMS4VelocityModel", ["cvms4.pyx"], extra_compile_args=["-Wunused-function"],
              include_dirs=["src"], libraries=["cvms"], library_dirs=["./src/"])
]

setup(
    name="CVMS4",
    cmdclass={"build_ext": build_ext},
    ext_modules=ext_modules
)
