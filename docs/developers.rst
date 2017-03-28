Developers
==========

UCVM is, at its core, a Python library that happens to also provide some command-line tools to interact with it. It can
be extended to include your own models and it can also be used in your own software. For this guide, we will assume
that you wish to interact with it through the Python interface.

.. toctree::

    developers_api

Using UCVM In Your Software
~~~~~~~~~~~~~~~~~~~~~~~~~~~

UCVM can be embedded within your own Python scripts. There are three key classes to begin using UCVM in your own
applications:

- SeismicData: This holds material properties for a given point.
- Point: Defines the coordinates in X, Y, and Z as well as the projection.
- UCVM: This class encompasses most of the UCVM framework methods (including the main query method).

A very simple script to query "cvms426" at point -118, 34, 0 and print out the material properties is as follows:
::

    #!/usr/bin/env python3
    from ucvm.src.framework.ucvm import UCVM   # Import the UCVM framework first.
    from ucvm.src.shared.properties import Point, SeismicData   # Classes needed to query.

    # Create a point at -118, 34 at 0 depth with the default projection.
    point = Point(-118, 34, 0)
    # Create a new SeismicData object to hold the material properties at that point.
    data = SeismicData(point)
    # Query the point for cvms426. Note, query expects a list of SeismicData objects.
    UCVM.query([data], "cvms426")
    # Print the result.
    print(data)

Running this little script will give you:
::

    (1860.9648, 909.3662, 2050.1179)

The first number is Vp, the second is Vs, and the third is density.

Adding A Model To UCVM
~~~~~~~~~~~~~~~~~~~~~~

It is fairly easy to add a velocity model to UCVM although you do need to place your model in the right directory. Let's
suppose you want to create a velocity model entitled "OneFiveVM" which sets Vp, Vs, and density equal to 1.5 times
the query depth in meters with a maximum of 5000. Of course, this model is absurd from a scientific perspective, but for
the purposes of illustrating how to add a model to UCVM, it will do nicely.

All models within UCVM must have two files minimum: a ucvm_model.xml file that describes the velocity model to UCVM and
a Python script which is executed by UCVM to get the material properties. In this example we will first create the
XML file and then the Python script.

First, we must create the ucvm_model.xml file which describes the model to UCVM.
::

    vi ucvm_model.xml

::

    <?xml version="1.0" encoding="ISO-8859-1"?>

    <!-- Model root tag -->
    <root>

        <!-- Publicly available information. -->
        <information>

            <!-- A human-friendly name identifier -->
            <identifier>OneFiveVM</identifier>

            <!-- A machine-friendly identifier (lowercase alpha-numeric only) -->
            <id>onefivevm</id>

            <!-- Describes the type of model (velocity, elevation, or vs30) -->
            <type>velocity</type>

            <!-- Public model description -->
            <description>
                OneFiveVM is my very own custom velocity model!
            </description>

            <!-- The license for this velocity model (all SCEC software is Apache version 2.0) -->
            <license>
                You can copy OneFiveVM as much as you want!
            </license>

            <!-- A broad description and range of the coverage region -->
            <coverage>
                <description>Global</description>
            </coverage>

        </information>

        <!-- Private information used by UCVM to understand how to query the model -->
        <internal>

            <!-- The model class that inherits from Model -->
            <class>OneFiveVM</class>

            <!-- The main model file which contains the class, if none then it's Cython -->
            <file>onefivevm.py</file>

            <!-- Projection that this model uses (DEFAULT means the default UCVM WGS84 projection -->
            <projection>DEFAULT</projection>

            <!-- Specifies if this model is publicly available (most models should be) -->
            <public>Yes</public>

            <!-- Defines the default elevation and Vs30 models that go with this model -->
            <defaults>
                <elevation>usgs-noaa</elevation>
                <vs30>wills-wald-2006</vs30>
            </defaults>

            <!-- Defines if the default query mode is by DEPTH or ELEVATION -->
            <query_by>DEPTH</query_by>

        </internal>

    </root>

The model code is:
::

    vi onefivevm.py

::

    # Python Imports
    from typing import List

    # UCVM Imports
    from ucvm.src.model.velocity.velocity_model import VelocityModel
    from ucvm.src.shared.properties import SeismicData
    from ucvm.src.shared import VelocityProperties, UCVM_DEPTH, UCVM_DEFAULT_PROJECTION


    class OneFiveVM(VelocityModel):

        # This method actually does the query.
        def _query(self, data: List[SeismicData], **kwargs) -> bool:

            # Loop through each SeismicData object and multiply the depth Z value by 1.5.
            for datum in data:

                # Set the value to 1.5 times the depth.
                value = 1.5 * datum.converted_point.z_value

                # If greater than 5000, set at 5000.
                if value > 5000:
                    value = 5000

                # Set the velocity data. First param is Vp, next is Vs, then density. Fourth and fifth
                # params are Qp and Qs. Then the next five params are the source where the data came from.
                # In this case, it's all OneFiveVM.
                datum.set_velocity_data(
                    VelocityProperties(
                    value, value, value, value, value,  # Set Vp, Vs, Dn, Qp, Qs all to value.
                    "OneFiveVM", "OneFiveVM", "OneFiveVM", "OneFiveVM", "OneFiveVM"
                )
            )

            # Signal all done without any errors!
            return True

We now need to create the model directory. The below commands assume that UCVM has been installed in ~/ucvm-|version|.
::

    mkdir ~/ucvm-17.3.0/lib/python3.5/site-packages/ucvm-17.3.0-py3.5.egg/ucvm/models/onefivevm
    mv ucvm_model.xml ~/ucvm-17.3.0/lib/python3.5/site-packages/ucvm-17.3.0-py3.5.egg/ucvm/models/onefivevm
    mv onefivevm.py ~/ucvm-17.3.0/lib/python3.5/site-packages/ucvm-17.3.0-py3.5.egg/ucvm/models/onefivevm

Finally, we need to let UCVM know what the class, file name, etc. for this model are. Edit
~/ucvm-|version|/lib/python3.5/site-packages/ucvm-17.3.0-py3.5.egg/ucvm/models/installed.xml and add the following in
between the <root> and </root> tag.
::

    <velocity file="onefivevm.py" name="OneFiveVM" id="onefivevm" class="OneFiveVM"></velocity>

We can now test to make sure that our model works! You should be able to do the following and get back our example
hypothetical material properties:
::

    (ucvm-17.3.0) $ ucvm_query -m onefivevm
    Enter points to query. The X, Y, and Z components should be separated by spaces. When you have entered
    all of your points, hit enter twice or press Ctrl-D to retrieve the material properties.
    -118 34 0
    -118 34 100
    -118 34 5000

    Retrieving material properties...
    X           Y           Z           Vp (m/s)    Vs (m/s)    Dn (kg/m^3) Qp          Qs          Source              Elev. (m)   Source      Vs30 (m/s)  Source
    -118.0000   34.0000     0.0000      0.0000      0.0000      0.0000      0.0000      0.0000      OneFiveVM           287.9969    usgs-noaa   390.0000    wills-wald-2006
    -118.0000   34.0000     100.0000    150.0000    150.0000    150.0000    150.0000    150.0000    OneFiveVM           287.9969    usgs-noaa   390.0000    wills-wald-2006
    -118.0000   34.0000     5000.0000   5000.0000   5000.0000   5000.0000   5000.0000   5000.0000   OneFiveVM           287.9969    usgs-noaa   390.0000    wills-wald-2006
