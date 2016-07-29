#!/usr/bin/env python
"""
Defines the setup script. This installs UCVM and downloads all the requested models. Models
can be added or deleted after the fact with the ucvm_model_manager utility. This auto-downloads new
models and can remove existing ones.

:copyright: Southern California Earthquake Center
:author:    David Gill <davidgil@usc.edu>
:created:   July 6, 2016
:modified:  July 21, 2016
"""

from setuptools import setup
from distutils.command.install import install
import urllib.request
import xml.dom.minidom
import os
from math import log
from subprocess import Popen, PIPE, CalledProcessError
import shutil


UCVM_INFORMATION = {
    "short_name": "ucvm",
    "version": "16.9.0",
    "long_name": "Unified Community Velocity Model Framework",
    "author": "Southern California Earthquake Center",
    "email": "software@scec.org",
    "url": "http://www.scec.org/projects/ucvm"
}

_HYPOCENTER_BASE = "http://hypocenter.usc.edu/research/ucvm/" + UCVM_INFORMATION["version"]
_HYPOCENTER_MODEL_LIST = _HYPOCENTER_BASE + "/model_list.xml"

INSTALL_REQUIRES = ["h5py", "xmltodict", "humanize", "pyproj"]


class OnlyGetScriptPath(install):
    def run(self):
        self.distribution.install_scripts = self.install_scripts


def get_setuptools_script_dir():
    dist = setup(cmdclass={'install': OnlyGetScriptPath})
    dist.dry_run = True  # not sure if necessary
    command = dist.get_command_obj('install')
    command.ensure_finalized()
    command.run()
    return dist.install_scripts


def execute(cmd):
    popen = Popen(cmd, stdout=PIPE, universal_newlines=True)
    stdout_lines = iter(popen.stdout.readline, "")
    for stdout_line in stdout_lines:
        yield stdout_line

    popen.stdout.close()
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
        "vs30": []
    }

    all_models = model_list_xml.getElementsByTagName("model")

    for model_item in all_models:
        inner_item = {
            "id": str(model_item.getElementsByTagName("file")[0].firstChild.data).split(".")[0],
            "description": "\n".join([x.strip() for x in str(
                model_item.getElementsByTagName("description")[0].firstChild.data
            ).split("\n")]),
            "name": str(model_item.getElementsByTagName("name")[0].firstChild.data),
            "coverage": str(model_item.getElementsByTagName("coverage")[0].firstChild.data),
            "type": str(model_item.getElementsByTagName("type")[0].firstChild.data)
        }

        installable_models[inner_item["type"]].append(inner_item)

    return installable_models

models_to_download = [
    ("usgs_noaa", "USGS/NOAA Digital Elevation Model"),
    ("vs30_calc", "Calculated Vs30 from the model (top 30 meters slowness)")
]

print(
    "\n"
    "Thank you for downloading and installing UCVM " + UCVM_INFORMATION["version"] + "!\n"
    "\n"
    "UCVM software framework is a collection of software tools designed to provide a standard\n"
    "interface to multiple, alternative, California 3D velocity models.\n"
    "\n"
    "UCVM requires at least the USGS/NOAA digital elevation model and the calculated Vs30 model\n"
    "to operate. Additional velocity, elevation, and Vs30 models are available for download.\n"
    "These models cover various regions within the world, "
    "\n"
)

if shutil.which("mpicc") is not None:
    print("Setup has detected that you have MPI installed on your computer. UCVM will be\n"
          "installed with MPI support.\n")
    INSTALL_REQUIRES.append("mpi4py")

print("Specify which of the following models you wish to download and install with UCVM:\n")

model_list = get_list_of_installable_internet_models()

for key, models in model_list.items():
    # If all the models are already set to be downloaded by default, remove them.
    if len([x for x in models if (x["id"], x["name"]) not in models_to_download]) == 0:
        continue

    # Print the header.
    print(str(key).capitalize() + " Models:")
    for item in models:

        # If this model is already set to be downloaded, skip it.
        if item["id"] in models_to_download:
            continue

        # Ask the user if they would like to download it
        print("Name:   " + item["name"])
        print("Covers: " + item["coverage"])
        print(item["description"])
        if str(input("Would you like to install " +
                     item["name"] + "? Type yes or y to install: ")).strip().lower()[:1] == "y":
            models_to_download.append((item["id"], item["name"]))
    print("")

total_model_size = 0

print("You have selected to install the following models: ")
for item in models_to_download:
    model_file = urllib.request.urlopen(_HYPOCENTER_BASE + "/models/" + item[0] + ".ucv")
    model_size = int(model_file.headers["Content-Length"])
    total_model_size += model_size
    print("   - " + (item[1][:42] + "..." if len(item[1]) > 45 else item[1]).ljust(55) +
          (" (" + item[0] + ")").ljust(15) + sizeof_fmt(model_size).rjust(10))

print("\nYour total download size will be: " + sizeof_fmt(total_model_size))
if str(input("Would you like to install UCVM? "
             "Type yes or y to start the installation: ")).strip().lower()[:1] != "y":
    print("Aborting installation...")
    exit(1)

print("\nInstalling UCVM...")

setup(name=UCVM_INFORMATION["short_name"],
      version=UCVM_INFORMATION["version"],
      description=UCVM_INFORMATION["long_name"],
      author=UCVM_INFORMATION["author"],
      author_email=UCVM_INFORMATION["email"],
      url=UCVM_INFORMATION["url"],
      packages=["ucvm", "ucvm.src", "ucvm.src.framework", "ucvm.src.model",
                "ucvm.src.model.velocity", "ucvm.src.model.elevation", "ucvm.src.model.vs30",
                "ucvm.src.shared", "ucvm.models"],
      package_dir={'ucvm': 'ucvm',
                   'ucvm.src': 'ucvm/src',
                   'ucvm.src.framework': 'ucvm/src/framework',
                   'ucvm.src.model': 'ucvm/src/model',
                   'ucvm.src.model.velocity': 'ucvm/src/model/velocity',
                   'ucvm.src.model.elevation': 'ucvm/src/model/elevation',
                   'ucvm.src.model.vs30': 'ucvm/src/model/vs30',
                   'ucvm.src.shared': 'ucvm/src/shared',
                   'ucvm.models': 'ucvm/models'},
      package_data={'ucvm.models': ['ucvm/models/installed.xml']},
      data_files=[("ucvm/models", ["ucvm/models/installed.xml"])],
      install_requires=INSTALL_REQUIRES,
      scripts=['ucvm/bin/ucvm_query', 'ucvm/bin/ucvm_model_manager'],
      zip_safe=False
      )

_LOCAL_SCRIPT_PATH = get_setuptools_script_dir()
print("")

# Now that UCVM is installed, we can go through the requested models and install them.
for model in models_to_download:
    for line in execute([os.path.join(_LOCAL_SCRIPT_PATH, "ucvm_model_manager"), "-a", model[0]]):
        print(line, end="")
    print("")

print("Thank you for installing UCVM. The installation is now complete.")
