Mac OS X Sierra
===============

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

    SCECs-MacBook-Pro:~ scec$ xcode-select --install

You will need Python 3.5 to install UCVM.
::

    SCECs-MacBook-Pro:~ scec$ curl -o python.pkg https://www.python.org/ftp/python/3.5.0/python-3.5.0-macosx10.6.pkg
    SCECs-MacBook-Pro:~ scec$ open python.pkg

The installer will guide you through the installation process.

Finally, you will need the X11 server as UCVM depends on Matplotlib which requires X11 to be installed. Please visit the
following link to download and install the latest X11 version.

http://www.xquartz.org

Once that is installed, you will need to create a couple of symlinks to ensure that Matplotlib can compile and install
correctly.
::

    SCECs-MacBook-Pro:~ scec$ sudo mkdir -p /usr/local/include
    SCECs-MacBook-Pro:~ scec$ sudo ln -s /usr/X11/include/freetype2/freetype /usr/local/include/freetype
    SCECs-MacBook-Pro:~ scec$ sudo ln -s /usr/X11/include/freetype2/ft2build.h /usr/local/include/ft2build.h
    SCECs-MacBook-Pro:~ scec$ sudo mkdir -p /usr/local/lib
    SCECs-MacBook-Pro:~ scec$ sudo ln -s /usr/X11/lib/libfreetype.dylib /usr/local/lib/libfreetype.dylib

Finally, if you are planning on installing the Bay Area, CVM-S4, or CVM-S4.26.M01 velocity models, you will need
GFortran installed. Download and install the following DMG to satisfy this requirement.

http://coudert.name/software/gfortran-6.3-Sierra.dmg

Now you are ready to download and install UCVM!

Installing UCVM
~~~~~~~~~~~~~~~

It is strongly recommended that users of UCVM use Python virtual environments to install UCVM. Python virtual
environments allow all the UCVM components to be installed in one folder and independently of other Python packages.
This guide will detail how to install UCVM using a virtual environment first. The Advanced section explains how to
install UCVM without a virtual environment.

To create and activate your virtual environment, do the following:
::

    SCECs-MacBook-Pro:~ scec$ pyvenv-3.5 ~/ucvm-17.2.0
    SCECs-MacBook-Pro:~ scec$ source ~/ucvm-17.2.0/bin/activate

You should notice that your command line prompt has changed include "ucvm-|version|" in brackets. If you don't see this,
then the virtual environment has not been activated correctly.
::

    (ucvm-17.2.0) SCECs-MacBook-Pro:~ scec$

Now we can clone the UCVM software.
::

    (ucvm-17.2.0) SCECs-MacBook-Pro:~ scec$ git clone https://github.com/SCECcode/UCVM

Run the ucvm_setup script. This script does some basic sanity checks of the installation environment and makes sure
that the installation looks like it can proceed successfully.
::
::

    (ucvm-17.2.0) SCECs-MacBook-Pro:~ scec$ cd UCVM
    (ucvm-17.2.0) SCECs-MacBook-Pro:UCVM scec$ ./ucvm_setup

The UCVM setup script will ask a series of questions about which models you would like to install. Enter "y" to install
a model or "n" to not install it.

At the end of the setup script, you should see a series of tests being run. When these tests are completed, UCVM will
notify you that the installation has completed successfully.

