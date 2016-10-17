"""
Defines the Python build file for the etree library, euclid3.

:copyright: Southern California Earthquake Center
:author:    David Gill <davidgil@usc.edu>
:created:   October 4, 2016
:modified:  October 5, 2016
"""
import os
import shutil
from subprocess import call

from ucvm.src.shared import UCVM_LIBRARIES_DIRECTORY

temp_path = os.path.join(UCVM_LIBRARIES_DIRECTORY, "euclid3", "temp")

lib_path = os.path.join(UCVM_LIBRARIES_DIRECTORY, "euclid3", "lib")
include_path = os.path.join(UCVM_LIBRARIES_DIRECTORY, "euclid3", "include")

# Builds the e-tree library.
try:
    os.mkdir(temp_path)
except OSError:
    print("Error: Temporary library directory " + temp_path + " could not be created.")

try:
    os.mkdir(lib_path)
except OSError:
    print("Error: Lib library directory " + temp_path + " could not be created.")

try:
    os.mkdir(include_path)
except OSError:
    print("Error: Include library directory " + temp_path + " could not be created.")

os.chdir(temp_path)

call(["tar", "-zxvf", os.path.join("..", "euclid3-latest.tar.gz"), "-C", ".", "--strip", "1"])

os.chdir(os.path.join(temp_path, "libsrc"))

os.environ["CFLAGS"] = "-w"
os.environ["CPPFLAGS"] = "-w"

call(["make"], env=os.environ)

call(["cp", os.path.join(".", "libetree.so"), lib_path])
call(["cp", os.path.join(".", "libetree.a"), lib_path])

call(["cp", os.path.join(".", "btree.h"), include_path])
call(["cp", os.path.join(".", "buffer.h"), include_path])
call(["cp", os.path.join(".", "code.h"), include_path])
call(["cp", os.path.join(".", "dlink.h"), include_path])
call(["cp", os.path.join(".", "etree.h"), include_path])
call(["cp", os.path.join(".", "etree_inttypes.h"), include_path])
call(["cp", os.path.join(".", "expandtable.h"), include_path])
call(["cp", os.path.join(".", "extracttable.h"), include_path])
call(["cp", os.path.join(".", "schema.h"), include_path])
call(["cp", os.path.join(".", "schemax.h"), include_path])
call(["cp", os.path.join(".", "wrapper.h"), include_path])
call(["cp", os.path.join(".", "xplatform.h"), include_path])

os.chdir(os.path.dirname(os.path.realpath(__file__)))

shutil.rmtree(temp_path)

