.. _Mac OS X:

Mac OS X
========

UCVM works with Mac OS X. It has been tested and confirmed to work on Mac OS X Sierra.

Supported Capabilities
----------------------

+-----------------------------+-----------------------------+
| Query Velocity Models       | ✓ Yes                       |
+-----------------------------+-----------------------------+
| Generate Material Models    | ✓ Yes                       |
+-----------------------------+-----------------------------+
| Visualization               | ✓ Yes                       |
+-----------------------------+-----------------------------+

Mac OS X Sierra
---------------

Prerequisites
~~~~~~~~~~~~~

In order to install UCVM on Mac OS X Sierra, the Xcode tools must be installed. To do so, go to the Mac App Store and
download Xcode using the link below.

https://itunes.apple.com/us/app/xcode/id497799835?mt=12

You will also need to install the Xcode command line tools.
::

    xcode-select --install
    sudo xcodebuild -license accept

The installer will guide you through the installation process.

You will need the X11 server as UCVM depends on Matplotlib which requires X11 to be installed. Please visit the
following link to download and install the latest X11 version.

http://www.xquartz.org

Once that is installed, you will need to create a couple of symlinks to ensure that Matplotlib can compile and install
correctly.
::

    sudo mkdir -p /usr/local/include
    sudo ln -s /usr/X11/include/freetype2/freetype /usr/local/include/freetype
    sudo ln -s /usr/X11/include/freetype2/ft2build.h /usr/local/include/ft2build.h
    sudo mkdir -p /usr/local/lib
    sudo ln -s /usr/X11/lib/libfreetype.dylib /usr/local/lib/libfreetype.dylib

If you are planning on installing the Bay Area, CVM-S4, or CVM-S4.26.M01 velocity models, you will need
GFortran installed. Download and install the following DMG to satisfy this requirement.

http://coudert.name/software/gfortran-6.3-Sierra.dmg

Now you are ready to download and install UCVM.

Anaconda (Easy Method)
~~~~~~~~~~~~~~~~~~~~~~

Using Anaconda for your UCVM installation makes the process easier and is supported on Mac OS X. We have tested UCVM
against Anaconda version 4.3.0.

If you have :ref:`Anaconda` installed, please visit our :ref:`Anaconda` guide now.

Installing UCVM (Advanced Method)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

It is strongly recommended that users of UCVM use Python virtual environments to install UCVM. Python virtual
environments allow all the UCVM components to be installed in one folder and independently of other Python packages.
This guide will detail how to install UCVM using a virtual environment first. The Advanced section explains how to
install UCVM without a virtual environment.

First, you will need Python 3.5:
::

    curl -o python.pkg https://www.python.org/ftp/python/3.5.0/python-3.5.0-macosx10.6.pkg
    open python.pkg

To create and activate your virtual environment, do the following:
::

    pyvenv-3.5 ~/ucvm-17.3.0
    source ~/ucvm-17.3.0/bin/activate

You should notice that your command line prompt has changed include "ucvm-|version|" in brackets. If you don't see this,
then the virtual environment has not been activated correctly.
::
    Your command prompt should look something like this:
        (ucvm-17.3.0) SCECs-MacBook-Pro:~ scec$

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

If you are looking to use UCVM for visualization, you will need to download Basemap and install it on your
computer.  The source code to basemap is available from
https://downloads.sourceforge.net/project/matplotlib/matplotlib-toolkits/basemap-1.0.7/basemap-1.0.7.tar.gz. Download
this file and then execute the following:
::

    tar zxvf basemap-1.0.7.tar.gz
    cd basemap-1.0.7
    cd geos-3.3.3
    export GEOS_DIR=/usr/local/geos
    ./configure --prefix=$GEOS_DIR
    make
    sudo make install
    cd ..
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