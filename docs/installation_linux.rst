Linux
=====

UCVM works with many distributions of Linux. It has been tested and confirmed to work on Ubuntu 16.04.1 LTS. This is the
distribution that the VirtualBox uses.

Supported Capabilities
----------------------

+-----------------------------+-----------------------------+
| Query Velocity Models       | ✓ Yes                       |
+-----------------------------+-----------------------------+
| Generate Material Models    | ✓ Yes                       |
+-----------------------------+-----------------------------+
| Visualization               | ✓ Yes                       |
+-----------------------------+-----------------------------+

Ubuntu 16.04.1 LTS
------------------

Prerequisites
~~~~~~~~~~~~~

In order to install UCVM on this distribution of Linux, a few required packages must be installed first.
::

    sudo apt install git python3-venv libfreetype6 libfreetype6-dev python3-dev libhdf5-serial-dev gfortran

To also add MPI support, the following packages need to be installed.
::

    sudo apt install openmpi-bin libopenmpi-dev

Anaconda (Easy Method)
~~~~~~~~~~~~~~~~~~~~~~

Using Anaconda for your UCVM installation makes the process easier and is supported on Linux. We have tested UCVM
against Anaconda version 4.3.0.

If you have :ref:`Anaconda` installed, please visit our :ref:`Anaconda` guide now.

Installing UCVM (Advanced)
~~~~~~~~~~~~~~~~~~~~~~~~~~

It is strongly recommended that users of UCVM use Python virtual environments to install UCVM. Python virtual
environments allow all the UCVM components to be installed in one folder and independently of other Python packages.
This guide will detail how to install UCVM using a virtual environment first. The Advanced section explains how to
install UCVM without a virtual environment.

To create and activate your virtual environment, do the following:
::

    pyvenv-3.5 ~/ucvm-17.3.0
    source ~/ucvm-17.3.0/bin/activate

You should notice that your command line prompt has changed include "ucvm-|version|" in brackets. If you don't see this,
then the virtual environment has not been activated correctly.
::
    Your command prompt should look something like this:
        (ucvm-17.3.0) scec@scec-VirtualBox:~$

Now we can clone the UCVM software.
::

    git clone https://github.com/SCECcode/UCVM

Run the ucvm_setup script. This script does some basic sanity checks of the installation environment and makes sure
that the installation looks like it can proceed successfully.
::

    cd UCVM
    ./ucvm_setup

The UCVM setup script will ask a series of questions about which models you would like to install. Enter "y" to install
a model or "n" to not install it.

At the end of the setup script, you should see a series of tests being run. When these tests are completed, UCVM will
notify you that the installation has completed successfully.
::

    test_ucvm_load_models... ok
    test_ucvm_parse_model_string... ok
    and so on

At this point, you can begin to use any text-based component of UCVM.

If you are looking to *use UCVM for visualization*, you will need to install basemap. Basemap requires geos to
be installed.
::

    sudo apt install libgeos-3.5.0 libgeos-dev
    sudo ln -s /usr/lib/x86_64-linux-gnu/libgeos_c.so /usr/lib/x86_64-linux-gnu/libgeos.so

Now we can download basemap. The source code to basemap is available from
https://downloads.sourceforge.net/project/matplotlib/matplotlib-toolkits/basemap-1.0.7/basemap-1.0.7.tar.gz. Download
this file and then execute the following:
::

    tar zxvf basemap-1.0.7.tar.gz
    cd basemap-1.0.7
    python3 setup.py install

After installation, we highly recommend that you check out our :ref:`Tutorial` and
the :ref:`CommandReference` pages. These will enable you to become more familiar with the UCVM platform.

Quick Test
~~~~~~~~~~

To quickly test if UCVM is installed correctly run the following command, enter the given input of "-118 34 0" and
ensure that the output you see matches the provided output below.
::
    Command:
        ucvm_query -m 1d[SCEC]

    Output:
        Enter points to query. The X, Y, and Z components should be separated by spaces. When you have entered
        all of your points, hit enter twice or press Ctrl-D to retrieve the material properties.

    Input:
        -118 34 0

    Response:
        Retrieving material properties...
        X           Y           Z           Vp (m/s)    Vs (m/s)    Dn (kg/m^3) Qp          Qs          Source              Elev. (m)   Source      Vs30 (m/s)  Source
        -118.0000   34.0000     0.0000      5000.0000   2886.7513   2654.5000   N/A         N/A         scec 1d (interpolat 287.9969    usgs-noaa   2886.7513   vs30-calc

The above command queries the 1D SCEC model at point (-118, 34, 0) for material properties. If you do not see the above,
please email software@scec.org.
