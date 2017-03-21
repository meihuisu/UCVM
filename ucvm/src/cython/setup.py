"""
UCVM C Routines Setup Script

Installs the UCVM C routines as part of the installation package.

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
