USC (HPC)
=========

UCVM is fully compatible with the HPC cluster at USC. This document describes how to install UCVM on the USC cluster
and also how to find and submit the example jobs.

Supported Capabilities
----------------------

+-----------------------------+-----------------------------+
| Query Velocity Models       | ✓ Yes                       |
+-----------------------------+-----------------------------+
| Generate Material Models    | ✓ Yes                       |
+-----------------------------+-----------------------------+
| Visualization               | × No                        |
+-----------------------------+-----------------------------+

Setup
-----

Installing UCVM on HPC is a straightforward process requiring .

1. Configuration
~~~~~~~~~~~~~~~~

Login to USC HPC as follows:

.. code-block:: bash

   ssh -Y <user name>@hpc.usc.edu

2. Clone the UCVM Repository
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

We want to check out the master branch from GitHub. This contains all the code necessary to build and run UCVM.

.. code-block:: bash

   cd <base path for UCVM>
   git clone https://github.com/SCECcode/UCVM.git

3. Install Software
~~~~~~~~~~~~~~~~~~~

Now that our repository is checked out, we need to source two files in order to install UCVM. Type in the commands
below:

.. code-block:: bash

   source /usr/usc/python/3.5.2/setup.csh
   source /usr/usc/openmpi/default/setup.csh

We want to create a Python virtual environment for UCVM.

.. code-block:: bash

   pyvenv-3.5 <install ucvm path>/ucvm-17.3.0

We can now proceed with the installation. It is strongly recommended that you install UCVM to your **staging**
directory.

.. code-block:: bash

   cd UCVM
   source <install ucvm path>/ucvm-17.3.0/bin/activate.csh
   ./ucvm_setup

UCVM will detect that you are running on USC HPC (look for the last line below in your terminal output).

.. code-block:: text

   UCVM requires the 1D velocity model, the DataProductReader model, the USGS/NOAA digital
   elevation model, and the Wills-Wald Vs30 model to operate. Additional velocity, elevation,
   and Vs30 models are available for download. These models cover various regions within the
   world, although most are located within California.

   Setup has detected that you are installing on USC HPC.

Set the location to which UCVM should be installed. Due to disk quotas, UCVM cannot be installed on rcf filesystems.
You will need to install to either a SCEC disk (if you have a SCEC account), your own disk space with a quota that is
big enough for the UCVM distribution, or your staging filesystem (/staging/<pi>/<your username>).

Also select the models you want to install. Hit return.

You should see some warnings as the installation progresses. This is normal. They do not affect the UCVM distribution
and they are a result of the included libraries and dependencies.

Eventually UCVM will begin installing the models and running the tests, like so:

.. code-block:: text

   Downloading 1D...
   Extracting 1D...
   Installing 1D...
       Moving model data to directory...
   Running tests for 1D...
   Testing 1D...
       [2 tests]
       [001] Running 1D BBP format...
             PASSED

Please be patient as this process can take a substantial amount of time (on the order of 20 minutes or longer).

UCVM will then proceed to conduct tests to ensure the installation is working right. All these tests should pass. If
one of them does not, then the installation process has failed. For example, you should see something like this towards
the end of the installation:

.. code-block:: text

   test_generate_simple_mesh_ijk12_rotated (ucvm.tests.mesh.UCVMMeshTest) ... ok
   test_generate_simple_mesh_ijk12_unrotated (ucvm.tests.mesh.UCVMMeshTest) ... ok
   test_generate_simple_utm_mesh_ijk12_rotated (ucvm.tests.mesh.UCVMMeshTest) ... ok

After the tests are run, UCVM is installed. Please note that you will have to source UCVM or add the source UCVM
virtual environment command to your ~/.cshrc or .bashrc file.

.. code-block:: text

   For bash the command to add to your .bashrc or .bash_profile is:
   source <install ucvm path>/ucvm-17.3.0/bin/activate

   For csh the command to add to your .cshrc

That's it! UCVM should now be installed and operational on your USC HPC account!

4. Running Your Installation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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

Examples
--------

Please note that the following examples assume that **CVM-S4.26 is installed**. If it is not then you will need to modify
the xml files in your GitHub UCVM/examples directory. Change the <cvm_list>cvms426</cvm_list> tag to read <cvm_list>
your desired models</cvm_list>.

UCVM includes multiple execution examples for a variety of platforms. Two examples are included in your GitHub
UCVM/examples directory:

* extract_test_mesh_mpi_awp.usc
* extract_test_tree_mpi_her.usc

These extract a test AWP cartesian mesh and a test E-tree for Hercules. To use this examples you will need to open
the extract_test files and change the following line:

.. code-block:: text

   PATH_TO_UCVM=/staging/path/to/your/ucvm-venv

To reflect your UCVM virtual environment path.

Then you can run the examples using the following commands.

.. code-block:: text

   cd <github UCVM directory>/examples
   qsub extract_test_mesh_mpi_awp.usc
   qsub extract_test_etree_mpi_her.usc

.. toctree::
   :maxdepth: 2
