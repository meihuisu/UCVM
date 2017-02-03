"""
Setup.py Script

The Lin-Thurber model setup.py script. This is fairly basic and generic, specifying how the
velocity model is supposed to be installed.

Copyright:
    Southern California Earthquake Center

Developer:
    David Gill <davidgil@usc.edu>
"""
from distutils.core import setup
from distutils.extension import Extension

ext_modules = [
    Extension("LinThurberVelocityModel", ["linthurber.c"],
              extra_compile_args=["-Wunused-function"],
              libraries=["cvmlt"], library_dirs=["./src/src"])
]

setup(
    name="LinThurber",
    ext_modules=ext_modules
)
