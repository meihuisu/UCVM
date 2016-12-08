Mira (HPC)
==========

UCVM is compatible with Mira. It can be used to query material properties and extract material models (e-trees
and meshes).

Supported Capabilities
----------------------

+-----------------------------+-----------------------------+
| Query Velocity Models       | ✓ Yes                       |
+-----------------------------+-----------------------------+
| Generate Material Models    | ✓ Yes                       |
+-----------------------------+-----------------------------+
| Visualization               | ✓ Yes                       |
+-----------------------------+-----------------------------+

Getting Started
---------------

Installing UCVM on Mira requires a few steps in addition to the standard installation guide.

1. Login and Checkout
~~~~~~~~~~~~~~~~~~~~~

Login to Mira as follows:

.. code-block:: bash

   ssh -Y <user name>@mira.alcf.anl.gov

Once you are logged in, you will need to edit your ~/.soft file to add in support for Python 3.5. If you have not
already modified this file, run the following command:

.. code-block:: bash

   echo "+python-3.5.1" >> ~/.soft

We want to check out the master branch from GitHub. This contains all the code necessary to build and run UCVM.

.. code-block:: bash

   cd ~ && git checkout https://github.com/SCECcode/UCVM.git

.. toctree::
   :maxdepth: 2
