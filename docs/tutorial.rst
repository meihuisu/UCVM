.. _Tutorial:

Tutorial
========

This tutorial works through three typical use-cases for the new UCVM. This tutorial assumes that you have the VirtualBox
distribution of the software which has CVM-S4.26 installed. Three hypothetical scenarios are explored and these
represent the most common use cases for the UCVM platform.

Activate Python Environment
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Every time you want to run UCVM, you must ensure that all the Python paths are configured properly.

If you installed via Anaconda and you named your environment "ucvm-17.3.0", run:

* source activate ucvm-17.3.0

If you installed via virtual environment and you put your installation in a folder like "/your/path/to/ucvm-17.3.0", run:

* source /your/path/to/ucvm-17.3.0/bin/activate

Otherwise, if you installed UCVM to a custom location, please configure your PYTHONPATH and PATH variables correctly.
To do this, assuming you installed to /your/path/to/ucvm-17.3.0, the commands would be:

* export PATH="/your/path/to/ucvm-17.3.0/bin"
* export PYTHONPATH="/your/path/to/ucvm-17.3.0/lib/python3.5/site-packages:$PYTHONPATH"

It may be helpful to add those to your ~/.bashrc or ~/.bash_profile to avoid having to run them each time.

Query
~~~~~

Suppose you are a seismologist looking to better understand some of the material properties within the Los Angeles
Basin. You have seismic data readings from four SCSN stations:

* CE 13884, Garden Grove Hwy 22 & Harbor, -117.91780, 33.76670
* CE 24048, Santa Monica 19th & Wilshire, -118.48240, 34.02890
* CE 14001, Long Beach Santa Fe & Willow, -118.21500, 33.80200
* CI USC, University of Southern California, -118.28631, 34.01919

In order to better understand the characteristics of the seismograms you are looking at, you want to learn about the
material properties at each station. You want to learn the velocity information at 0m, 500m, and 5000m to gain better
insight into the ground motion observed on the seismograms.

The utility we will want to use is **ucvm_query** which makes answering this question very easy. We need to specify
the model we want to use to UCVM using the -m parameters (for a list of available models that can be installed, please
visit the :ref:`AvailableModels` page). This tutorial assumes that you want these material properties from **cvms426**
which is the latest Southern California full 3D tomographic improvement model. The command to run ucvm_query is:
::

    ucvm_query -m cvms426

UCVM will then ask you to input your desired query points.
::

    Enter points to query. The X, Y, and Z components should be separated by spaces. When you have
    entered all of your points, hit enter twice or press Ctrl-D to retrieve the material properties.

We wish to type in our longitudes, latitudes, and desired depths. Type (or copy and paste) the following lines:
::

    -117.91780  33.76670  0
    -117.91780  33.76670  500
    -117.91780  33.76670  5000
    -118.48240  34.02890  0
    -118.48240  34.02890  500
    -118.48240  34.02890  5000
    -118.21500  33.80200  0
    -118.21500  33.80200  500
    -118.21500  33.80200  5000
    -118.28631  34.01919  0
    -118.28631  34.01919  500
    -118.28631  34.01919  5000

Hit enter twice or press Ctrl-D to ask UCVM to retrieve the material properties. You will see the following appear:
::

    Retrieving material properties...
    X           Y           Z           Vp (m/s)    Vs (m/s)    Dn (kg/m^3) Qp          Qs          Source              Elev. (m)   Source      Vs30 (m/s)  Source
    -117.9178   33.7667     0.0000      1595.4200   841.5213    2015.2928   N/A         N/A         cvms426             30.8035     usgs-noaa   280.0000    wills-wald-2006
    -117.9178   33.7667     500.0000    2003.3247   1051.8660   2213.5022   N/A         N/A         cvms426             30.8035     usgs-noaa   280.0000    wills-wald-2006
    -117.9178   33.7667     5000.0000   4720.6367   2613.8403   2572.4458   N/A         N/A         cvms426             30.8035     usgs-noaa   280.0000    wills-wald-2006
    -118.4824   34.0289     0.0000      1710.9546   887.1201    2050.1179   N/A         N/A         cvms426             57.1107     usgs-noaa   387.0000    wills-wald-2006
    -118.4824   34.0289     500.0000    1962.2249   957.5009    2205.3362   N/A         N/A         cvms426             57.1107     usgs-noaa   387.0000    wills-wald-2006
    -118.4824   34.0289     5000.0000   4726.5142   2563.1416   2599.4243   N/A         N/A         cvms426             57.1107     usgs-noaa   387.0000    wills-wald-2006
    -118.2150   33.8020     0.0000      1624.0164   832.7258    2029.0493   N/A         N/A         cvms426             8.2870      usgs-noaa   263.8778    wills-wald-2006
    -118.2150   33.8020     500.0000    1857.1193   925.0756    2231.2244   N/A         N/A         cvms426             8.2870      usgs-noaa   263.8778    wills-wald-2006
    -118.2150   33.8020     5000.0000   5462.9761   3055.6775   2624.2107   N/A         N/A         cvms426             8.2870      usgs-noaa   263.8778    wills-wald-2006
    -118.2863   34.0192     0.0000      1766.6285   895.5740    2014.7020   N/A         N/A         cvms426             60.1029     usgs-noaa   280.0000    wills-wald-2006
    -118.2863   34.0192     500.0000    2217.9875   1083.3892   2191.3220   N/A         N/A         cvms426             60.1029     usgs-noaa   280.0000    wills-wald-2006
    -118.2863   34.0192     5000.0000   4575.7495   2598.0950   2593.0027   N/A         N/A         cvms426             60.1029     usgs-noaa   280.0000    wills-wald-2006

The first three columns represent the X, Y, and Z coordinates to query (in this case the longitude, latitude of each
station and the desired depth). The next five columns represent the material properties (P-wave velocity, S-wave
velocity, density, P-wave attenuation, and S-wave attenuation). The ninth column is the source from which this data
came. The next two columns state the elevation for this point and the source it came from (the USGS National Map/ETOPO1
data). Finally, the last two columns indicate the Vs30 data as represented by the Wills-Wald map. This map represents
the Wills & Clahan 2006 dataset within the state of California, falling back to the Wald 2007 set outside of the state.

Let's now suppose that you decide that you don't want to get the Vs30 information from the Wills-Wald map for the USC
site, but rather from the model itself. That is, you want to calculate Vs30 direct from CVM-S4.26. To do this, we can
combine multiple data sources together using dots. To accomplish this, do the following:
::

    Command:
        ucvm_query -m cvms426.vs30-calc

    Output:
        Enter points to query. The X, Y, and Z components should be separated by spaces. When you have
        entered all of your points, hit enter twice or press Ctrl-D to retrieve the material properties.

    Input:
        -118.28631  34.01919  0

    Response:
        Retrieving material properties...
        X           Y           Z           Vp (m/s)    Vs (m/s)    Dn (kg/m^3) Qp          Qs          Source              Elev. (m)   Source      Vs30 (m/s)  Source
        -118.2863   34.0192     0.0000      1766.6285   895.5740    2014.7020   N/A         N/A         cvms426             60.1029     usgs-noaa   901.0089    vs30-calc

Taking Vs30 direct from the CVM-S4.26 model results in a 901m/s value instead of a 280m/s one. For GMPEs and other
equations that require Vs30, you may need to use the Wills Vs30 or the model Vs30 or both and compare.

We mentioned that we can tile models together. This means that you can string multiple models together and if material
properties do not exist in one model, we can try querying the next model, and so on until all models are exhausted.

Let's now suppose we have data from a new station in San Jose. CVM-S4.26 doesn't extend that far up, but we want to
query the SCEC 1D model to have some material properties for our equations. Running UCVM with our old model string
will return no material properties.
::

    Command:
        ucvm_query -m cvms426.vs30-calc

    Output:
        Enter points to query. The X, Y, and Z components should be separated by spaces. When you have
        entered all of your points, hit enter twice or press Ctrl-D to retrieve the material properties.

    Input:
        -121.91088  37.38332  0

    Response:
        Retrieving material properties...
        X           Y           Z           Vp (m/s)    Vs (m/s)    Dn (kg/m^3) Qp          Qs          Source              Elev. (m)   Source      Vs30 (m/s)  Source
        -121.9109   37.3833     0.0000      N/A         N/A         N/A         N/A         N/A         N/A                 11.7362     usgs-noaa   N/A         N/A

Tiling is done by sequencing models using semi-colons. So if we want to query cvms426 and then the 1D SCEC model, we would
do the following:
::

    Command:
        ucvm_query -m cvms426.vs30-calc;1d[SCEC]

    Output:
        Enter points to query. The X, Y, and Z components should be separated by spaces. When you have entered
        all of your points, hit enter twice or press Ctrl-D to retrieve the material properties.

    Input:
        -118.28631  34.01919  0
        -121.91088  37.38332  0

    Response:
        Retrieving material properties...
        X           Y           Z           Vp (m/s)    Vs (m/s)    Dn (kg/m^3) Qp          Qs          Source              Elev. (m)   Source      Vs30 (m/s)  Source
        -118.2863   34.0192     0.0000      1766.6285   895.5740    2014.7020   N/A         N/A         cvms426             60.1029     usgs-noaa   901.0089    vs30-calc
        -121.9109   37.3833     0.0000      5000.0000   2886.7513   2654.5000   N/A         N/A         scec 1d (interpolat 11.7362     usgs-noaa   2886.7513   vs30-calc

Notice how material properties are now returned for both points but the one USC station that lies within the CVM-S4.26
model domain has a source of cvms426. The San Jose point, which falls outside of the domain, has the 1D SCEC background
model as its source. Tiling can be a very powerful way to combine models.

Note that you can also get references for each data source (if one is available) by passing the -a parameter. This can
be helpful when writing a paper or when attempting to better understand the science behind a specific model. These
citations are also given on the :ref:`AvailableModels` page.
::

    Command:
        ucvm_query -m cvms426.vs30-calc;1d[SCEC] -a

    Output:
        Enter points to query. The X, Y, and Z components should be separated by spaces. When you have entered
        all of your points, hit enter twice or press Ctrl-D to retrieve the material properties.

    Input:
        -118.28631  34.01919  0
        -121.91088  37.38332  0

    Response:
        Retrieving material properties...
        X           Y           Z           Vp (m/s)    Vs (m/s)    Dn (kg/m^3) Qp          Qs          Source              Elev. (m)   Source      Vs30 (m/s)  Source
        -118.2863   34.0192     0.0000      1766.6285   895.5740    2014.7020   N/A         N/A         cvms426             60.1029     usgs-noaa   901.0089    vs30-calc
        -121.9109   37.3833     0.0000      5000.0000   2886.7513   2654.5000   N/A         N/A         scec 1d (interpolat 11.7362     usgs-noaa   2886.7513   vs30-calc

        References:

        CVM-S4.26 has 1 reference:
	        - Lee, E.-J., P. Chen, T. H. Jordan, P. J. Maechling, M. A. M. Denolle, and G. C.Beroza (2014), Full 3-D tomography for crustal structure in Southern California based on the scattering-integral and the adjoint-wave?eld methods, J. Geophys. Res. Solid Earth, 119, doi:10.1002/2014JB011346.

        USGS/NOAA Digital Elevation Model has 2 references:
	        - The National Map 1/3rd arc-second DEM (http://www.nationalmap.gov)
	        - Amante, C. and B.W. Eakins, 2009. ETOPO1 1 Arc-Minute Global Relief Model: Procedures, Data Sources and Analysis. NOAA Technical Memorandum NESDIS NGDC-24. National Geophysical Data Center, NOAA. doi:10.7289/V5C8276M



Visualization
~~~~~~~~~~~~~

The previous example assumed that you are a seismologist looking to better understand some of the material properties
within the Los Angeles Basin. Continuing with these four points, suppose you have seismic data readings from the
same four SCSN stations:

* CE 13884, Garden Grove Hwy 22 & Harbor, -117.91780, 33.76670
* CE 24048, Santa Monica 19th & Wilshire, -118.48240, 34.02890
* CE 14001, Long Beach Santa Fe & Willow, -118.21500, 33.80200
* CI USC, University of Southern California, -118.28631, 34.01919

In order to better understand the data, sometimes it is important to plot maps and depth profiles of sites. First, let's
analyze what the surface velocities look like in the LA basin around these points. More specifically, we're interested
in the Vs velocities at 0m depth.

To accomplish this task, we need the horizontal slice plotting utility.
::

    ucvm_plot_horizontal_slice

This utility will ask a series of questions. Please answer the questions as follows.
::

    Generating a horizontal slice requires various parameters to be defined (such
    as the origin of the slice, the length of the slice, and so on). The following
    questions will guide you through the definition of those parameters.

    What is the X or longitudinal coordinate of your bottom-left starting point? -119
    What is the Y or latitudinal coordinate of your bottom-left starting point? 33
    What is the Z or depth/elevation coordinate of your bottom-left starting point? 0

    Is your Z coordinate specified as depth (default) or elevation?
    Type 'd' or enter for depth, 'e' for elevation: d

    How many longitudinal (or X-axis) grid points should there be? 201
    How many latitudinal (or Y-axis) grid points should there be? 201
    What should the spacing between each grid point be? 0.01

    What is the rotation angle, in degrees, of this box (relative to the bottom-left corner)? 0

    You must select the velocity model(s) from which you would like to retrieve this
    data. You can either enter in your model(s) as text (e.g. cvms4 or dataproductreader[file]) or you
    can select from one of the predefined ones in the list below.
    1) USGS Bay Area
    2) CCA
    3) CVM-H 15.1.0
    4) CVM-S4
    5) CVM-S4.26
    6) CVM-S4.26.M01
    7) Lin-Thurber

    Which velocity model would you like? 5

    Which property should be plotted?
    Acceptable answers include Vp, Vs, density, Qp, or Qs: vs

After a minute or so, you will see a plot appear on your screen that looks like this:

.. image:: http://hypocenter.usc.edu/research/ucvm/17.3.0/docs/_static/tutorial1.png
    :width: 500px

We can also analyze cross-sections through the earth as well. Suppose we wanted to get a better idea of the material
properties from depth 0m to depth 10km between the Garden Grove CE13884 station and the Santa Monica CE24048 station.
We can visualize this by using the cross-section plotting utility.
::

    ucvm_plot_cross_section

Like the previous utility, this will ask a series of questions. Please answer the questions as follows.
::

    Generating a cross-section requires various parameters to be defined (such
    as the start point, ending point, and so on). The following questions will guide
    you through the definition of those parameters.

    What is the X or longitudinal coordinate of the start point? -117.91780
    What is the Y or latitudinal coordinate of the start point? 33.76670

    What is the X or longitudinal coordinate of the end point? -118.48240
    What is the Y or latitudinal coordinate of the end point? 34.02890

    What is the top depth or elevation for your cross-section? 0
    What is the bottom depth or elevation for your cross-section? 10000

    Are your top and bottom numbers depth (by default) or elevation?
    Type 'd' or enter for depth, 'e' for elevation: d

    In which projection are your points specified?
    The default for UCVM is WGS84 latitude and longitude. To accept
    the default projection, simply hit enter:

    What horizontal spacing be, in meters, for each extracted point? 1000
    What vertical spacing be, in meters, for each extracted point? 500

    Which property or properties (comma-separated) should be plotted?
    Acceptable answers include Vp, Vs, density, Qp, or Qs: vs

    You must select the velocity model(s) from which you would like to retrieve this
    data. You can either enter in your model(s) as text (e.g. cvms4.usgs-noaa) or you
    can select from one of the predefined ones in the list below.
    1) USGS Bay Area
    2) CCA
    3) CVM-H 15.1.0
    4) CVM-S4
    5) CVM-S4.26
    6) CVM-S4.26.M01
    7) Lin-Thurber

    Which velocity model would you like? 5

You will see a plot appear on your screen that looks like this:

.. image:: http://hypocenter.usc.edu/research/ucvm/17.3.0/docs/_static/tutorial2.png
    :width: 500px

Finally, we can also plot depth profiles to get a better sense of the material properties at each station. If we wished
to analyze the material properties below the Santa Monica station, we could do so using the depth profile plotting
utility.
::

    ucvm_plot_depth_profile

This utility also asks a series of questions. Please answer them as follows.
::

    Generating a depth profile requires various parameters to be defined (such
    as the profile point, the spacing, and so on). The following questions will guide
    you through the definition of those parameters.

    What is the X or longitudinal coordinate of the profile point? -118.48240
    What is the Y or latitudinal coordinate of the profile point? 34.02890
    What is the Z or depth/elevation coordinate of the profile point? 0

    Is your Z coordinate specified as depth (default) or elevation?
    Type 'd' or enter for depth, 'e' for elevation: d

    In which projection is your point specified?
    The default for UCVM is WGS84 latitude and longitude. To accept
    the default projection, simply hit enter:

    What should the spacing between each layer be? 20
    What should the last depth or elevation be? 40000

    Which property or properties (comma-separated) should be plotted?
    Acceptable answers include Vp, Vs, density, Qp, or Qs: vp,vs,density

    You must select the velocity model(s) from which you would like to retrieve this
    data. You can either enter in your model(s) as text (e.g. cvms4.usgs-noaa) or you
    can select from one of the predefined ones in the list below.
    1) USGS Bay Area
    2) CCA
    3) CVM-H 15.1.0
    4) CVM-S4
    5) CVM-S4.26
    6) CVM-S4.26.M01
    7) Lin-Thurber

    Which velocity model would you like? 5

You will see a plot appear on your screen that looks like this:

.. image:: http://hypocenter.usc.edu/research/ucvm/17.3.0/docs/_static/tutorial3.png
    :width: 500px

Meshing
~~~~~~~

Creating meshes for earthquake simulations is a key component of UCVM. Suppose we were performing a very small simulation
of the region in the LA Basin. We are going to use the AWP-ODC code, so we need a cartesian mesh, not an octree mesh.
Further suppose that we are going to start this mesh from -118.50, 33.5 and make it a 50km by 50km by 50km mesh. We
are going to choose 500m grid spacing for 1,000,000 total grid points. This will result in a 12MB mesh which is easy
for a single core to extract.

We need to use the cartesian mesh generation utility to extract this mesh.
::

    ucvm_mesh_create

This utility will ask a series of questions. Please answer them as follows.
::

    Generating a mesh requires the definition of various parameters to be defined (such
    as the origin of the mesh, the length of the mesh, and so on). The following questions
    will guide you through the definition of those parameters. At the end, you will be
    asked if you want to just generate the configuration file to make the mesh at a later
    time or if you want to generate the mesh immediately.

    From which velocity model(s) should this mesh be generated: cvms426

    Meshes are constructed by specifying a bottom-left origin point, a rotation for the
    rectangular region, and then a width, height, and depth for the box.

    To start, in which projection is your starting point specified? The default for UCVM
    is WGS84 latitude and longitude. To accept that projection, simply hit enter:

    What is the X or longitudinal coordinate of your bottom-left starting point? -118.5
    What is the Y or latitudinal coordinate of your bottom-left starting point? 33.5
    What is the Z or depth/elevation coordinate of your bottom-left starting point? 0

    Is your Z coordinate specified as depth (default) or elevation? Type 'd' or enter for depth,
    'e' for elevation: d

    By default, UCVM queries the center of each grid point to get the material properties
    (so it is an average of the cell). UCVM can query at each vertex instead. Type 'c' or
    hit enter to accept the center grid type, or type 'v' or 'vertex' for a point at each
    corner:

    What should your mesh projection be? The default is UTM WGS84 Zone 11.
    Hit enter to accept this projection or specify your own, as a Proj.4 string:

    What is the rotation angle, in degrees, of this box (relative to the bottom-left corner)? 0
    In your projection's co-ordinate system, what should the spacing between each grid
    point be? 500

    How many grid points should there be in the X or longitudinal direction? 100
    How many grid points should there be in the Y or latitudinal direction? 100
    How many grid points should there be in the Z or depth/elevation direction? 100

    What should the minimum Vs, in meters, be? The default is 0:  0
    What should the minimum Vp, in meters, be? The default is 0:  0

    In which format would you like this mesh? Type 'awp' for AWP-ODC, 'rwg' for a Graves'
    format mesh: awp
    To which directory should the mesh and metadata be saved? ./
    Please provide a name for this mesh: test_mesh_la_basin

In order to check if the mesh was extract successfully, we need to compare it against the original model to make sure
that the differences are small or virtually non-existant. To do this, we can use the DataProductReader "model". This
model takes as a parameter the XML file that was generated during the mesh extraction and the data, and makes it into
its own model within UCVM.

We can then use the ucvm_plot_comparison utility to visualize and see differences between the two meshes.
::

    ucvm_plot_comparison

This utility also asks a series of questions. Please answer them as follows.
::

    Generating a comparison between models requires various parameters to be defined (such as the start point,
    ending point, and so on). The following questions will guide you through the definition of those parameters.

    What is the X or longitudinal coordinate of the bottom-left corner? -118.5
    What is the Y or latitudinal coordinate of the bottom-left corner? 33.5

    What is the X or longitudinal coordinate of the top-right corner? -118
    What is the Y or latitudinal coordinate of the top-right corner? 33.9

    In which projection are your points specified? Hit enter to accept the default WGS84 latitude, longitude projection:

    Should this slice be generated at depth or elevation? Type 'd' for depth, 'e' for elevation: d
    At which depth should this horizontal slice be generated? 0

    What should the horizontal slice sampling spacing be (in the projection specified earlier)? 0.01

    Which material property would you like this comparison to be for?
    Possibilities include vp, vs, and density: vs

    Enter the name of the first velocity model for the comparison. Extracted data products can be used with
    the dataproductreader[xml location] syntax: dataproductreader[test_mesh_la_basin]
    Enter the name of the second velocity model for the comparison. Extracted data products can be used with
    the dataproductreader[xml location] syntax: cvms426

    Would you like the comparison saved to disk? Type 'y' for yes, 'n' for no: n

    Would you like to save this configuration file for future use? Type 'y' for yes, 'n' for no: n

    The configuration is now complete. The comparison will start now.

You will see a plot appear on your screen that looks like this:

.. image:: http://hypocenter.usc.edu/research/ucvm/17.3.0/docs/_static/tutorial4.png
    :width: 500px

Notice how the differences are very, very small. There will be some differences due to trilinear interpolation and
rotation angles, but the vast majority of differences are in the "white" part of the scale which is around +/- 1%.

Congratulations! You have now explored the material properties surrounding some stations within the LA basin and you
have also learned how to use some of the key capabilities within UCVM.

Please contact software@scec.org if you have any further questions or if anything in this tutorial did not work and
we will respond as soon as possible!

.. toctree::
    :maxdepth: 2

