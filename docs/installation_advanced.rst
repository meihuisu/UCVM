Advanced and FAQ
================

UCVM is a complex software stack that has multiple dependencies. This page lists any installation issues or advanced topics that may apply to your particular system or configuration.

Installation Without Virtual Environment or Anaconda
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

It is possible to install UCVM without a virtual environment or Anaconda. In terms of complexity, Anaconda < Virtual Environment < Custom Install, which is why we recommend the first two options first. If you cannot use a virtual environment, the installation instructions are mostly the same except that when you run:
::

    ./ucvm_setup

It will ask you where you want to install UCVM. Type the folder that you would like to use as the installation location.

Let the process run to completion. UCVM will be installed but you will need to add the resulting location to both your PYTHONPATH and PATH locations.

Edit your ~/.bashrc file to add the following:
::

    export PATH="/your/folder/with/ucvm-17.3.0/bin:$PATH"
    export PYTHONPATH="/your/folder/with/ucvm-17.3.0/lib/python3.5/site-packages/:$PYTHONPATH"

Save the file and restart your shell. The UCVM commands such as ucvm_query and ucvm_help should now be available and work.

Cannot Create Virtual Environment With Some Versions of Python 3.5
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

When you try and create a Python virtual environment with pyvenv3.5, sometimes you may get this error:
::

    Error: Command '['/path/to/ucvm-17.3.0/bin/python', '-Im', 'ensurepip', '--upgrade', '--default-pip']' returned non-zero exit status 1

If this happens, then the version of Python that you have was a version that was shipped with a broken pip command. This affects some versions of Ubuntu, especially. To fix this, enter:
::

    pyvenv3.5 --without-pip /path/to/ucvm-17.3.0
    source /path/to/ucvm-17.3.0/bin/activate
    curl https://bootstrap.pypa.io/get-pip.py | python
    deactivate
    source /path/to/ucvm-17.3.0/bin/activate

This will get the Python virtual environment set up with a working version of pip!

fPIC Command Not Found
~~~~~~~~~~~~~~~~~~~~~~

When installing a model such as CVM-S4.26.M01 you may find that you get an error like this:
::

   /bin/bash: fPIC: command not found

This error occurs because you do not have GFortran installed. To install GFortran on Ubuntu do sudo apt install
gfortran. For Mac OS X users, please get the version that
`corresponds to your OS here <http://coudert.name/software.html>`_.
