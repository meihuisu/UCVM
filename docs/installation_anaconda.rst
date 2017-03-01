.. _Anaconda:

Anaconda
========

Using Anaconda can make installation of UCVM simpler on both Linux and OS X platforms. This guide assumes you have
Anaconda 4.3.0 (or a later version) installed. If you do not have at least 4.3 installed, please upgrade to at least
this version. UCVM may not work with versions below 4.3.0.

Installation
~~~~~~~~~~~~

First of all, if you have not downloaded Anaconda yet, please do so from https://www.continuum.io/downloads.

On the Mac, you will need the Xcode tools installed, GFortran, and Xquartz installed. If you do not have all three of
those installed, please refer to the :ref:`Mac OS X` installation guide.

We need to first create and activate our environments.

::

    conda create --name ucvm-17.3.0 python=3.6
    source activate ucvm-17.3.0

This activates our 17.3.0 environment.
