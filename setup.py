#!/usr/bin/env python
"""
Defines the setup script. This installs UCVM and downloads all the requested models. Models
can be added or deleted after the fact with the ucvm_model_manager utility. This auto-downloads new
models and can remove existing ones.

Copyright:
    Southern California Earthquake Center

Author:
    David Gill <davidgil@usc.edu>
"""
# Python Imports
from setuptools import setup
from distutils.command.install import install
import urllib.request
import xml.dom.minidom
import os
from io import StringIO
from math import log
from subprocess import Popen, PIPE, CalledProcessError
import shutil
import sys
from collections import OrderedDict

UCVM_INFORMATION = {
    "short_name": "ucvm",
    "version": "17.1.0",
    "long_name": "Unified Community Velocity Model Framework",
    "author": "Southern California Earthquake Center",
    "email": "software@scec.org",
    "url": "http://www.scec.org/projects/ucvm"
}


class OnlyGetScriptPath(install):
    def run(self):
        self.distribution.install_scripts = self.install_scripts
        self.distribution.package_dir = self.install_purelib


def get_setuptools_script_dir():
    s_out = sys.stdout
    sys.stdout = StringIO()
    dist = setup(name=UCVM_INFORMATION["short_name"],
                 version=UCVM_INFORMATION["version"],
                 cmdclass={'install': OnlyGetScriptPath})
    dist.dry_run = True  # not sure if necessary
    command = dist.get_command_obj('install')
    command.ensure_finalized()
    command.run()
    sys.stdout = s_out
    return dist.package_dir, dist.install_scripts

try:
    os.makedirs(os.path.abspath(get_setuptools_script_dir()[0]), exist_ok=True)
    os.makedirs(os.path.abspath(get_setuptools_script_dir()[1]), exist_ok=True)
except OSError as e:
    print("Could not create the directory " + os.path.abspath(get_setuptools_script_dir()[0]) + ".\n" +
          "Please try again with a different prefix or make that directory yourself.")
    sys.exit(-1)

if "ucvm_setup_bootstrapped" not in os.environ:
    os.environ["ucvm_setup_bootstrapped"] = "YES"
    if "PYTHONPATH" in os.environ:
        os.environ["PYTHONPATH"] = os.path.abspath(get_setuptools_script_dir()[0]) + ":" + os.environ["PYTHONPATH"]
    else:
        os.environ["PYTHONPATH"] = os.path.abspath(get_setuptools_script_dir()[0])
    os.execv(sys.executable, [sys.executable] + sys.argv)

_HYPOCENTER_BASE = "http://hypocenter.usc.edu/research/ucvm/" + UCVM_INFORMATION["version"]
_HYPOCENTER_MODEL_LIST = _HYPOCENTER_BASE + "/model_list.xml"

INSTALL_REQUIRES = ["h5py", "xmltodict", "humanize", "pyproj", "psutil", "matplotlib"]


def execute(cmd, **kwargs):
    env = os.environ
    if "env" in kwargs:
        env = kwargs["env"]
    popen = Popen(cmd, universal_newlines=True, env=env)
    return_code = popen.wait()
    if return_code != 0:
        raise CalledProcessError(return_code, cmd)


def sizeof_fmt(num):
    """
    Taken from http://stackoverflow.com/questions/1094841/reusable-library-to-get-human-readable \
               -version-of-file-size
    :param num: The number to make human readable.
    :return: The number.
    """
    unit_list = [
        ('bytes', 0),
        ('kB', 0),
        ('MB', 1),
        ('GB', 2),
        ('TB', 2),
        ('PB', 2)
    ]

    if num > 1:
        exponent = min(int(log(num, 1024)), len(unit_list) - 1)
        quotient = float(num) / 1024**exponent
        unit, num_decimals = unit_list[exponent]
        format_string = '{:.%sf} {}' % num_decimals
        return format_string.format(quotient, unit)
    if num == 0:
        return '0 bytes'
    if num == 1:
        return '1 byte'


def get_list_of_installable_internet_models() -> dict:
    model_list_xml = xml.dom.minidom.parseString(
        urllib.request.urlopen(_HYPOCENTER_MODEL_LIST).read()
    )

    installable_models = {
        "velocity": [],
        "elevation": [],
        "vs30": [],
        "operator": []
    }

    all_models = model_list_xml.getElementsByTagName("model")

    for model_item in all_models:
        inner_item = {
            "id": str(model_item.getElementsByTagName("file")[0].firstChild.data).split(".")[0],
            "description": "\n".join([x.strip() for x in str(
                model_item.getElementsByTagName("description")[0].firstChild.data
            ).split("\n")]),
            "name": str(model_item.getElementsByTagName("name")[0].firstChild.data),
            "coverage": str(model_item.getElementsByTagName("coverage")[0].
                            getElementsByTagName("description")[0].firstChild.data),
            "type": str(model_item.getElementsByTagName("type")[0].firstChild.data)
        }

        installable_models[inner_item["type"]].append(inner_item)

    return installable_models

download_everything = False
if "--everything" in sys.argv:
    download_everything = True
    sys.argv.remove("--everything")

models_to_download = [
    ("onedimensional", "1D"),
    ("usgs-noaa", "USGS/NOAA Digital Elevation Model"),
    ("wills-wald-2006", "Wills-Wald Vs30"),
    ("vs30-calc", "Calculated Vs30 from the model (top 30 meters slowness)"),
    ("dataproductreader", "Data Product Reader")
]

if shutil.which("mpicc") is not None:
    print("Setup has detected that you have MPI installed on your computer. UCVM will be installed\n"
          "with MPI support.\n")
    INSTALL_REQUIRES += ["mpi4py", "psutil"]

print("Specify which of the following models you wish to download and install with UCVM:\n")

model_list_u = get_list_of_installable_internet_models()
model_list = OrderedDict()
model_list["velocity"] = model_list_u["velocity"]
model_list["elevation"] = model_list_u["elevation"]
model_list["vs30"] = model_list_u["vs30"]
model_list["operator"] = model_list_u["operator"]

for key, models in model_list.items():
    # If all the models are already set to be downloaded by default, remove them.
    if len([x for x in models if (x["id"], x["name"]) not in models_to_download]) == 0:
        continue

    # Print the header.
    print(str(key).capitalize() + " Models:\n")
    for item in models:

        # If this model is already set to be downloaded, skip it.
        if (item["id"], item["name"]) in models_to_download:
            continue

        # Ask the user if they would like to download it
        print("Name:   " + item["name"])
        print("Covers: " + item["coverage"])
        print(item["description"])
        if download_everything:
            models_to_download.append((item["id"], item["name"]))
        else:
            if str(input("Would you like to install " +
                         item["name"] + "?\nType yes or y to install, no or n to pass: ")).\
                       strip().lower()[:1] == "y":
                models_to_download.append((item["id"], item["name"]))
        print("")
    print("")

total_model_size = 0

print("You have selected to install the following models: ")
for item in models_to_download:
    model_file = urllib.request.urlopen(_HYPOCENTER_BASE + "/models/" + item[0] + ".ucv")
    model_size = int(model_file.headers["Content-Length"])
    total_model_size += model_size
    print("   - " + (item[1][:42] + "..." if len(item[1]) > 45 else item[1]).ljust(55) +
          (" (" + item[0] + ")").ljust(20) + sizeof_fmt(model_size).rjust(10))

print("\nYour total download size will be: " + sizeof_fmt(total_model_size))

if not download_everything:
    if str(input("Would you like to install UCVM with these models? "
                 "Type yes or y to start the installation: ")).strip().lower()[:1] != "y":
        print("Aborting installation...")
        exit(1)

print("\nInstalling UCVM...")

s_out = sys.stdout
sys.stdout = StringIO()
setup(name=UCVM_INFORMATION["short_name"],
      version=UCVM_INFORMATION["version"],
      description=UCVM_INFORMATION["long_name"],
      author=UCVM_INFORMATION["author"],
      author_email=UCVM_INFORMATION["email"],
      url=UCVM_INFORMATION["url"],
      packages=["ucvm", "ucvm.src", "ucvm.src.framework", "ucvm.src.model",
                "ucvm.src.model.velocity", "ucvm.src.model.elevation", "ucvm.src.model.vs30",
                "ucvm.src.model.fault", "ucvm.src.model.operator", "ucvm.src.shared",
                "ucvm.src.visualization", "ucvm.models",
                "ucvm.tests", "ucvm.tests.data", "ucvm.tests.scratch", "ucvm.libraries"],
      package_dir={'ucvm': 'ucvm',
                   'ucvm.src': 'ucvm/src',
                   'ucvm.src.framework': 'ucvm/src/framework',
                   'ucvm.src.model': 'ucvm/src/model',
                   'ucvm.src.model.velocity': 'ucvm/src/model/velocity',
                   'ucvm.src.model.elevation': 'ucvm/src/model/elevation',
                   'ucvm.src.model.vs30': 'ucvm/src/model/vs30',
                   'ucvm.src.model.fault': 'ucvm/src/model/fault',
                   'ucvm.src.model.operator': 'ucvm/src/model/operator',
                   'ucvm.src.shared': 'ucvm/src/shared',
                   'ucvm.src.visualization': 'ucvm/src/visualization',
                   'ucvm.models': 'ucvm/models',
                   'ucvm.tests': 'ucvm/tests',
                   'ucvm.tests.data': 'ucvm/tests/data',
                   'ucvm.tests.scratch': 'ucvm/tests/scratch',
                   'ucvm.libraries': 'ucvm/libraries'},
      package_data={'ucvm.models': ['ucvm/models/installed.xml'],
                    'ucvm.libraries': ['ucvm/libraries/installed.xml'],
                    'ucvm.tests.data': ['ucvm/tests/data/simple_mesh_ijk12_rotated.xml',
                                        'ucvm/tests/data/simple_mesh_ijk12_unrotated.xml',
                                        'ucvm/tests/data/simple_mesh_rwg_unrotated.xml',
                                        'ucvm/tests/data/simple_utm_mesh_ijk12_rotated.xml']},
      data_files=[("ucvm/models", ["ucvm/models/installed.xml"]),
                  ("ucvm/libraries", ["ucvm/libraries/installed.xml"]),
                  ('ucvm/tests/data', ['ucvm/tests/data/simple_utm_mesh_ijk12_rotated.xml',
                                       'ucvm/tests/data/simple_mesh_ijk12_unrotated.xml',
                                       'ucvm/tests/data/simple_mesh_rwg_unrotated.xml',
                                       'ucvm/tests/data/simple_mesh_ijk12_rotated.xml'])],
      install_requires=INSTALL_REQUIRES,
      scripts=['ucvm/bin/ucvm_etree_create', 'ucvm/bin/ucvm_etree_create_mpi', 'ucvm/bin/ucvm_help',
               'ucvm/bin/ucvm_mesh_create', 'ucvm/bin/ucvm_mesh_create_mpi', 'ucvm/bin/ucvm_model_manager',
               'ucvm/bin/ucvm_plot_comparison', 'ucvm/bin/ucvm_plot_cross_section',
               'ucvm/bin/ucvm_plot_depth_profile', 'ucvm/bin/ucvm_plot_horizontal_slice', 'ucvm/bin/ucvm_query',
               'ucvm/bin/ucvm_run_tests', 'ucvm/bin/ucvm_start_web'],
      zip_safe=False
      )
sys.stdout = s_out

print("Installing C components of UCVM...")

_LOCAL_LIBRARY_PATH, _LOCAL_SCRIPT_PATH = get_setuptools_script_dir()
_LOCAL_PACKAGE_PATH = _LOCAL_LIBRARY_PATH
_LOCAL_LIBRARY_PATH = os.path.join(
    _LOCAL_LIBRARY_PATH, "-".join([UCVM_INFORMATION["short_name"], UCVM_INFORMATION["version"],
                                   "py" + str(sys.version_info.major) + "." +
                                   str(sys.version_info.minor)]) + ".egg",
    "ucvm", "libraries"
)
_LOCAL_LIBRARY_PATH = os.path.abspath(_LOCAL_LIBRARY_PATH)

os.mkdir(os.path.join(_LOCAL_LIBRARY_PATH, "temp"))
library_file = urllib.request.URLopener()
library_file.retrieve("http://hypocenter.usc.edu/research/ucvm/" + UCVM_INFORMATION["version"] +
                      "/libraries/euclid3.ucv", os.path.join(_LOCAL_LIBRARY_PATH, "temp", "euclid3.ucv"))
try:
    os.mkdir(os.path.join(_LOCAL_LIBRARY_PATH, "temp", "euclid3"))
except FileExistsError:
    shutil.rmtree(os.path.join(_LOCAL_LIBRARY_PATH, "temp", "euclid3"))

try:
    os.mkdir(os.path.join(_LOCAL_LIBRARY_PATH, "euclid3"))
except FileExistsError:
    pass

p = Popen(["tar", "-zxvf", os.path.join(_LOCAL_LIBRARY_PATH, "temp", "euclid3.ucv"),
           "-C", os.path.join(_LOCAL_LIBRARY_PATH, "euclid3")], stdout=PIPE, stderr=PIPE)
p.wait()

print("\tInstalling E-tree Library.")

cwd = os.getcwd()
os.chdir(os.path.join(_LOCAL_LIBRARY_PATH, "euclid3"))

p = Popen(["python3", "build.py"])
p.communicate()

os.chdir(cwd)

os.remove(os.path.join(_LOCAL_LIBRARY_PATH, "temp", "euclid3.ucv"))

# Install C framework.
print("\tInstalling UCVM C library.")
os.chdir(os.path.join(os.path.dirname(os.path.realpath(__file__)), "ucvm", "src", "cython"))
p = Popen(["python3", "setup.py", "install", "--prefix=" +
           os.path.abspath(os.path.join(_LOCAL_PACKAGE_PATH, "..", "..", "..")), "--library-path=" +
           _LOCAL_LIBRARY_PATH])
p.communicate()
print("\tDone!")
print("")

# Now that UCVM is installed, we can go through the requested models and install them.
for model in models_to_download:
    execute([os.path.join(_LOCAL_SCRIPT_PATH, "ucvm_model_manager"), "-a", model[0]])
    print("")

# Now run the tests, through the command line so that the library paths are correct!
environment_variable = "LD_LIBRARY_PATH"
if sys.platform == "darwin":
    environment_variable = "DYLD_LIBRARY_PATH"

paths = []
if environment_variable in os.environ:
    paths = str(os.environ[environment_variable]).split(":")

# This is Cythonized code.
paths.append(os.path.join(_LOCAL_LIBRARY_PATH, "euclid3", "lib"))
os.environ[environment_variable] = ":".join(paths)

# Run the tests.
execute([os.path.join(_LOCAL_SCRIPT_PATH, "ucvm_run_tests"), "-t"], env=os.environ)

print("Thank you for installing UCVM. The installation is now complete. To learn more about")
print("UCVM, please run the command ucvm_help.")
