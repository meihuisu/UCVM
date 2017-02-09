.. _Tutorial:

Tutorial
========

This tutorial works through three typical use-cases for the new UCVM. This tutorial assumes that you have the VirtualBox
distribution of the software which has CVM-S4.26 installed. Three hypothetical scenarios are explored and these
represent the most common use cases for the UCVM platform.

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

    (ucvm-17.2.0) sceccme@bash-3.2$ ucvm_query -m cvms426

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

    (ucvm-17.2.0) sceccme@bash-3.2$ ucvm_query -m cvms426.vs30-calc
    Enter points to query. The X, Y, and Z components should be separated by spaces. When you have
    entered all of your points, hit enter twice or press Ctrl-D to retrieve the material properties.
    -118.28631  34.01919  0

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

    (ucvm-17.2.0) sceccme@bash-3.2$ ucvm_query -m cvms426.vs30-calc
    Enter points to query. The X, Y, and Z components should be separated by spaces. When you have
    entered all of your points, hit enter twice or press Ctrl-D to retrieve the material properties.
    -121.91088  37.38332  0

    Retrieving material properties...
    X           Y           Z           Vp (m/s)    Vs (m/s)    Dn (kg/m^3) Qp          Qs          Source              Elev. (m)   Source      Vs30 (m/s)  Source
    -121.9109   37.3833     0.0000      N/A         N/A         N/A         N/A         N/A         N/A                 11.7362     usgs-noaa   N/A         N/A

Tiling is done by sequencing models using semi-colons. So if we want to query cvms426 and then the 1D SCEC model, we would
do the following:
::

    (ucvm-17.2.0) sceccme@bash-3.2$ ucvm_query -m cvms426.vs30-calc;1d[SCEC]
    Enter points to query. The X, Y, and Z components should be separated by spaces. When you have entered
    all of your points, hit enter twice or press Ctrl-D to retrieve the material properties.
    -118.28631  34.01919  0
    -121.91088  37.38332  0

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

    (ucvm-17.2.0) sceccme@bash-3.2$ ucvm_query -m cvms426.vs30-calc;1d[SCEC] -a
    Enter points to query. The X, Y, and Z components should be separated by spaces. When you have entered
    all of your points, hit enter twice or press Ctrl-D to retrieve the material properties.
    -118.28631  34.01919  0
    -121.91088  37.38332  0

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

    (ucvm-17.2.0) sceccme@bash-3.2$ ucvm_plot_horizontal_slice -m cvms426

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
    1) CVM-S4.26

    Which velocity model would you like? 1

    Which property should be plotted?
    Acceptable answers include Vp, Vs, density, Qp, or Qs: vs

You will see a plot appear on your screen that looks like this:



Meshing
~~~~~~~

.. toctree::
    :maxdepth: 2

