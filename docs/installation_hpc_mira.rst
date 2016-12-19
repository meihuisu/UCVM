Mira (HPC)
==========

UCVM is fully compatible with Mira. This document describes how to install UCVM on Mira and also how to find and
submit the example jobs.

Supported Capabilities
----------------------

+-----------------------------+-----------------------------+
| Query Velocity Models       | ✓ Yes                       |
+-----------------------------+-----------------------------+
| Generate Material Models    | ✓ Yes                       |
+-----------------------------+-----------------------------+
| Visualization               | ✓ Yes                       |
+-----------------------------+-----------------------------+

Setup
-----

Installing UCVM on Mira requires a few additional steps in addition to the standard installation script.

1. Configuration
~~~~~~~~~~~~~~~~

Login to Mira as follows:

.. code-block:: bash

   ssh -Y <user name>@mira.alcf.anl.gov

Once you are logged in, you will need to edit your ~/.soft file to add in support for Python 3.5. If you have not
already modified this file, run the following command:

.. code-block:: bash

   echo "+python-3.5.1" >> ~/.soft

2. Clone the UCVM Repository
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

We want to check out the master branch from GitHub. This contains all the code necessary to build and run UCVM.

.. code-block:: bash

   cd ~
   git clone https://github.com/SCECcode/UCVM.git

3. Start Setup Script
~~~~~~~~~~~~~~~~~~~~~

Now that our repository is checked out, we need to install UCVM. It is strongly recommended that you install UCVM
to your **projects** directory.

.. code-block:: bash

   cd UCVM
   ./ucvm_setup

UCVM will detect that you are running on Mira.

.. code-block:: text

Edit your ~/.bashrc file:

.. code-block:: text

   PYTHONPATH="/path/to/ucvm-16.12.0/lib/directory:$PYTHONPATH"
   PATH="/path/to/ucvm-16.12.0/bin:$PATH"
   export PYTHONPATH
   export PATH

.. toctree::
   :maxdepth: 2
