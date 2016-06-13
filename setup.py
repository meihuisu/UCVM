#!/usr/bin/env python

##
#   Defines the list of strings and IDs that are used within UCVM. This prevents
#   problems with something being changed in one place but not the other.

from setuptools import setup

setup(name="UCVM",
      version="16.9.0",
      description="Unified Community Velocity Model Framework",
      author="Southern California Earthquake Center",
      author_email="software@scec.org",
      url="http://www.scec.org/ucvm",
      packages=["ucvm", "ucvm.server", "ucvm.api", "ucvm.common", "ucvm.visualization", "ucvm.commands", 
                "ucvm.model"],
      package_dir={'ucvm.server': 'ucvm/server', 'ucvm.api': 'ucvm/api', 'ucvm.common': 'ucvm/common',
                   'ucvm.visualization': 'ucvm/visualization', 'ucvm.commands': 'ucvm/commands',
                   'ucvm.model': 'ucvm/model'},
      package_data={'ucvm.server': ['html/*.html', 'html/css/*.css']},
      scripts=['ucvm/bin/ucvm']
     )

from ucvm.api.strings import get_string
from ucvm.model.install import install_model

print(get_string(3))
print(get_string(1))
print(get_string(3))

print(get_string(4))

# Ask about CVM-S4.
if input(get_string(7)).strip() == "y":
    install_model("cvms4")

print(get_string(3))

print(get_string(5))