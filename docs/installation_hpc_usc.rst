USC (HPC)
==========

UCVM is mostly compatible with the HPC cluster at USC. This document describes how to install UCVM on the USC cluster
and also how to find and submit the example jobs.

Supported Capabilities
----------------------

+-----------------------------+-----------------------------+
| Query Velocity Models       | ✓ Yes                       |
+-----------------------------+-----------------------------+
| Generate Material Models    | ✓ Yes                       |
+-----------------------------+-----------------------------+
| Visualization               | ✕ No                        |
+-----------------------------+-----------------------------+

Setup
-----

Installing UCVM on HPC requires a few additional steps in addition to the standard installation script.

1. Configuration
~~~~~~~~~~~~~~~~

Login to USC HPC as follows:

.. code-block:: bash

   ssh -Y <user name>@hpc.usc.edu

2. Clone the UCVM Repository
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

We want to check out the master branch from GitHub. This contains all the code necessary to build and run UCVM.

.. code-block:: bash

   cd <ucvm install dir>
   git clone https://github.com/SCECcode/UCVM.git

3. Install Software
~~~~~~~~~~~~~~~~~~~

Now that our repository is checked out, we need to install UCVM. It is strongly recommended that you install UCVM
to your **projects** directory.

.. code-block:: bash

   cd UCVM
   ./ucvm_setup

UCVM will detect that you are running on USC HPC.

.. code-block:: text

   UCVM requires the 1D velocity model, the DataProductReader model, the USGS/NOAA digital
   elevation model, and the Wills-Wald Vs30 model to operate. Additional velocity, elevation,
   and Vs30 models are available for download. These models cover various regions within the
   world, although most are located within California.

   **Setup has detected that you are installing on USC HPC.**

Set the location to which UCVM should be installed. Due to disk quotas, UCVM cannot be installed on rcf filesystems.
You will need to install to either a SCEC disk (if you have a SCEC account) or your staging filesystem
(/staging/<pi>/<your username>).

Also select the models you want to install. Hit return.

.. toctree::
   :maxdepth: 2
