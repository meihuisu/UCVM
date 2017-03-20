"""
Setup.py Script

The CVM-H model setup.py script. This is fairly basic and generic, specifying how the
velocity model is supposed to be installed.

Copyright 2017 Southern California Earthquake Center

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
"""
# Python Imports
import os
from distutils.core import setup
from distutils.extension import Extension

# UCVM Imports
from ucvm.src.shared.constants import UCVM_MODELS_DIRECTORY

ext_modules = [
    Extension("CVMH1510VelocityModel", ["cvmh1510.c"], libraries=["vxapi", "geo"],
              library_dirs=[os.path.join(UCVM_MODELS_DIRECTORY, "cvmh1510", "lib")])
]

setup(
    name="CVMH1510",
    ext_modules=ext_modules
)
