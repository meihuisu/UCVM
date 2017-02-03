Linux
=====

CentOS 6.8+
-----------

Prerequisites
~~~~~~~~~~~~~

UCVM has been tested and confirmed to work with CentOS. The VirtualBox version of UCVM uses CentOS 6.8. In order to
install UCVM on this distribution of Linux, a few required packages must be installed first.
::

    sudo yum install epel-release
    sudo yum install git
    sudo yum install freetype-devel make automake gcc gcc-c++ gcc-gfortran
        redhat-rpm-config subverison hdf5 hdf5-devel openssl-devel libpng-devel

To also add MPI support, the following packages need to be installed.
::

    sudo yum install openmpi openmpi-devel

If you are on a CentOS 6+ or on another distribution that has Python 3.5 installed, then you need
the following package:
::

    sudo yum install python3    <-- if you don't have Python 3.5 installed
    sudo yum install python3-devel

Please note that on some old versions of CentOS, Python 3 is not installable via yum. In order to build Python 3.5 from
scratch, do the following:
::

    wget https://www.python.org/ftp/python/3.5.2/Python-3.5.2.tgz
    tar zxvf Python-3.5.2.tgz
    cd Python-3.5.2
    ./configure
    make altinstall

Verify that you do indeed have a working Python 3.5 installation by running:
::

    python3.5 -v

Installing UCVM
~~~~~~~~~~~~~~~

It is strongly recommended that users of UCVM use Python virtual environments to install UCVM. Python virtual
environments allow all the UCVM components to be installed in one folder and independently of other Python packages.
This guide will detail how to install UCVM using a virtual environment first. The Advanced section at the end of this
document explains how to install UCVM without a virtual environment.

To create and activate your virtual environment, do the following:
::

    python3.5 -m venv ~/ucvm-17.2.0
    source ~/ucvm-17.2.0/bin/activate

You should notice that your command line prompt has changed include "ucvm-|version|" in brackets. If you don't see this,
then the virtual environment has not been activated correctly.
::

    (ucvm-17.2.0) sceccme@bash-3.2$

Now we can clone the UCVM software.
::

    git clone https://github.com/SCECcode/UCVM

Run the ucvm_setup script. This script does some basic sanity checks of the installation environment and makes sure
that the installation looks like it can proceed successfully.
::

    ./ucvm_setup

The UCVM setup script will ask a series of questions about which models you would like to install. Enter "y" to install
a model or "n" to not install it.

At the end of the setup script, you should see a series of tests being run. When these tests are completed, UCVM will
notify you that the installation has completed successfully. That that point, you can begin to use UCVM!

After installation, we highly recommend that you check out our tutorial and the command line reference.
