"""
Defines the Python build file for the projection library, proj4.

:copyright: Southern California Earthquake Center
:author:    David Gill <davidgil@usc.edu>
:created:   October 4, 2016
:modified:  October 5, 2016
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
