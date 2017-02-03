"""
Defines the Python build file for the projection library, proj4.

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
import shutil
from subprocess import call

from ucvm.src.shared import UCVM_LIBRARIES_DIRECTORY

temp_path = os.path.join(UCVM_LIBRARIES_DIRECTORY, "proj4", "temp")

# Builds the Proj.4 library.
try:
    os.mkdir(temp_path)
except OSError:
    print("Error: Temporary library directory " + temp_path + " could not be created.")

os.chdir(temp_path)

call(["tar", "-zxvf", os.path.join("..", "proj-4.9.2.tar.gz"), "-C", ".", "--strip", "1"])

call([os.path.join(".", "configure"),
      "--prefix=" + os.path.join(UCVM_LIBRARIES_DIRECTORY, "proj4")])

call(["make"])

call(["make", "install"])

shutil.rmtree(temp_path)
