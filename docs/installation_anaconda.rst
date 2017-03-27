.. _Anaconda:

Anaconda (Linux & Mac)
======================

Using Anaconda can make installation of UCVM simpler on both Linux and OS X platforms. This guide assumes you have
Anaconda 4.3.0 (or a later version) installed. If you do not have at least 4.3 installed, please upgrade to at least
this version. UCVM may not work with versions below 4.3.0.

Pre-requisites
~~~~~~~~~~~~~~

**Linux (Ubuntu)**

In order to install UCVM on this distribution of Linux, a couple required packages must be installed first.
::

    sudo apt install git gfortran

**Mac OS X Sierra**

In order to install UCVM using Anaconda on Mac OS X Sierra, the Xcode tools must be installed. To do so, go to the Mac
App Store and download Xcode using the link below.

https://itunes.apple.com/us/app/xcode/id497799835?mt=12

You will also need to install the Xcode command line tools.
::

    xcode-select --install
    sudo xcodebuild -license accept

Finally, you will need the X11 server as UCVM depends on Matplotlib which requires X11 to be installed. Please visit the
following link to download and install the latest X11 version.

http://www.xquartz.org

If you are planning on installing the Bay Area, CVM-S4, or CVM-S4.26.M01 velocity models, you will need
GFortran installed. Download and install the following DMG to satisfy this requirement.

http://coudert.name/software/gfortran-6.3-Sierra.dmg

Now you are ready to download and install UCVM.

Installation
~~~~~~~~~~~~

First of all, if you have not downloaded Anaconda yet, please do so from https://www.continuum.io/downloads. You only
need the free version of the software to begin using UCVM.

We need to first create and activate our environment, and add a dependency.
::

    conda create --name ucvm-17.3.0 python=3.5
    source activate ucvm-17.3.0
    conda config --add channels conda-forge

Now we can install UCVM with one command:
::

    conda install -c http://hypocenter.usc.edu/research/ucvm/17.3.0 ucvm

UCVM will download and install. Please note that the **Anaconda version does not come with any models**. To install
models, wait for the Anaconda installation to finish and then type:
::

    ucvm_model_manager -l

This will list all available models as well as the command needed to install each one.

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

Full Test
~~~~~~~~~

To run the test suite and ensure that UCVM was installed properly do:
::

    ucvm_run_tests -t

