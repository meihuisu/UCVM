"""
Setup.py Script

The CVM-S4.26.M01 model setup.py script. This is fairly basic and generic, specifying how the
velocity model is supposed to be installed.

Copyright:
    Southern California Earthquake Center

Developer:
    David Gill <davidgil@usc.edu>
"""
# Python Imports
import os
from distutils.core import setup
from distutils.extension import Extension
from Cython.Distutils import build_ext

# UCVM Imports
from ucvm.src.shared.constants import UCVM_MODELS_DIRECTORY

ext_modules = [
    Extension("CVMS426M01VelocityModel", ["cvms426m01.pyx"], libraries=["cvmsi"],
              library_dirs=[os.path.join(UCVM_MODELS_DIRECTORY, "cvms426m01", "lib")])
]

setup(
    name="CVMS426M01",
    cmdclass={"build_ext": build_ext},
    ext_modules=ext_modules
)