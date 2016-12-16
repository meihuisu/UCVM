Tutorial
========

This is a short walkthrough that demonstrates a typical "use-case" for UCVM.

Imagine that you are a graduate student that is interested in learning a bit more about Po and
En-Jui's Central California model. You have heard something about this model but you would like
to learn more about it and also explore how it interacts with the other full 3D tomographic models.

Installing UCVM
---------------

There are two ways to get started with UCVM. Either you can download this VirtualBox, or you can
install it directly on your Mac or Linux machine.

Please note that I am still trying to sort out some dependencies and how to install it on the
command line, so installing it direct in Linux or Mac may not work without additional packages
and requirements. The VirtualBox image has all these already installed for you.

Load up a terminal window on the VirtualBox or other machine on which you want to install UCVM.
Enter the following command:

.. code-block:: bash

   cd ~ && git clone -b dev https://github.com/SCECcode/UCVM.git && cd ./UCVM && python3 setup.py install --user

You should see something like this:

.. image:: /_static/ucvm_16.12a_sc2.png

In order to do this tutorial, please install just the CCA model. We will be adding other models
later.

Familiarizing yourself with UCVM
--------------------------------

After installing UCVM or downloading and loading up the VirtualBox, load up a terminal window.

Type in the command:

.. code-block:: bash

   ucvm_query -m cca06

This loads up the UCVM query utility. In it type in "-119.714 34.44 0", and then hit enter twice.
Unlike the previous versions of UCVM, this version searches for material properties after hitting
enter twice (providing it a blank line). You can also hit Control-D to search for the properties
the old way as well. This is the latitude and longitude of the Santa Barbara CyberShake station.
Note that you can query by elevation at this same point by changing "cca" to read "cca.elevation".
The ucvm_query utility assumes depth unless otherwise specified.

You should get back a line that looks like this:

.. image:: /_static/ucvm_16.12a_sc1.png

Now, let's suppose that you want to see a depth profile of the Santa Barbara station.
In your terminal window, type:

.. code-block:: bash

   ucvm_plot_depth_profile

You will be asked a sequence of questions. Answer them as follows:

.. code-block:: HTML

   Generating a depth profile requires various parameters to be defined (such
   as the profile point, the spacing, and so on). The following questions will guide
   you through the definition of those parameters.

   What is the X or longitudinal coordinate of the profile point? -119.714
   What is the Y or latitudinal coordinate of the profile point? 34.44
   What is the Z or depth/elevation coordinate of the profile point? 0

   Is your Z coordinate specified as depth (default) or elevation?
   Type 'd' or enter for depth, 'e' for elevation: d

   In which projection is your point specified?
   The default for UCVM is WGS84 latitude and longitude. To accept
   the default projection, simply hit enter: [hit enter]

   What should the spacing between each layer be? 100
   What should the last depth or elevation be? 10000

   Which property or properties (comma-separated) should be plotted?
   Acceptable answers include Vp, Vs, density, Qp, or Qs: vp, vs, density

   You must select the velocity model(s) from which you would like to retrieve this
   data. You can either enter in your model(s) as text (e.g. CVM-S4.usgs-noaa) or you
   can select from one of the predefined ones in the list below.
   1) CCA

   Which velocity model would you like? 1

You will get a depth profile that looks like the following! But the title isn't quite descriptive
enough... Let's change it to read "Depth Profile for Santa Barbara CyberShake Site".

.. image:: /_static/ucvm_16.12a_sc3.png

Do an ls of the folder that you are currently in. You will see an XML file. Open it with gedit,
like so:

.. code-block:: bash

   gedit plot_depth_profile.xml

Change, the text in the title tag to read "Depth Profile for Santa Barbara CyberShake Site"

You should see a title tag within the document. Change it from the default text to "Depth Profile
for Santa Barbara CyberShake Site", like so:

.. image:: /_static/ucvm_16.12a_sc4.png

Save the document and run the following command:

.. code-block:: bash

   ucvm_plot_depth_profile -f ./plot_depth_profile.xml

You will get a new plot with the revised title.

.. image:: /_static/ucvm_16.12a_sc5.png

We've explored one site for the CCA model, but let's now suppose that we want to see the tiling
interface between CCA and the CVM-S4.26 model. That is, how well do the two velocity models meet
at the intersection? Our instance of UCVM just has the CCA model installed. Let's add in CVM-S4.26.

In your terminal window, type:

.. code-block:: bash

   ucvm_model_manager

This will show you all the available models that you can download. You should see an entry
like this:

.. image:: /_static/ucvm_16.12a_sc6.png

Type in:

.. code-block:: bash

   ucvm_model_manager -a cvms426

The model should download and install. You can now query the CVM-S4.26 model as follows:

.. code-block:: bash

   ucvm_query -m cvms426

If you enter in "-118 34 0" as your query point, you will get back:

.. image:: /_static/ucvm_16.12a_sc7.png

Delving More Into The CCA Model
-------------------------------

We want to generate both a horizontal slice and a cross-section to get an idea of how well the
models meet.

To generate a horizontal slice, type:

.. code-block:: bash

   ucvm_plot_horizontal_slice -a

The "-a" flag means advanced. We'll see all the parameters we can control that way.
Like all the UCVM utilities, you will be presented with a series of questions. Answer the
questions as follows:

.. code-block:: html

   Generating a horizontal slice requires various parameters to be defined (such
   as the origin of the slice, the length of the slice, and so on). The following
   questions will guide you through the definition of those parameters.

   To start, in which projection is your starting point specified?
   The default for UCVM is WGS84 latitude and longitude. To accept
   the default projection, simply hit enter: [press enter]

   What is the X or longitudinal coordinate of your bottom-left starting point? -123
   What is the Y or latitudinal coordinate of your bottom-left starting point? 30
   What is the Z or depth/elevation coordinate of your bottom-left starting point? 0

   Is your Z coordinate specified as depth (default) or elevation?
   Type 'd' or enter for depth, 'e' for elevation: d

   How many longitudinal (or X-axis) grid points should there be? 1101
   How many latitudinal (or Y-axis) grid points should there be? 1001
   What should the spacing between each grid point be? 0.01

   What is the rotation angle, in degrees, of this box (relative to the bottom-left corner)? 0

   You must select the velocity model(s) from which you would like to retrieve this
   data. You can either enter in your model(s) as text (e.g. CVM-S4.usgs-noaa) or you
   can select from one of the predefined ones in the list below.
   1) 1D
   2) CCA
   3) CVM-S4
   4) CVM-S4.26

   Which velocity model would you like? cca06;cvms426

   Would you like to plot the extracted data?
   Type 'y' or 'yes' to plot: y

   Which property should be plotted?
   Acceptable answers include Vp, Vs, density, Qp, or Qs: vs
   What should the title of the plot be?
   Hit enter to accept the default title: CCA, CVM-S4.26 Slice
   Would you like a discrete or smooth color scale?
   Type 'd' for discrete, 's' for smooth: d
   What should the type of scale be (in Matplotlib colors)?
   The default is RdBu, which means red-blue: [press enter]

   Would you like to include faults on the map?
   Type 'y' or 'yes' to plot, 'n' or 'no' to leave off: y

   Would you like to include topography on the map?
   Type 'y' or 'yes' to plot, 'n' or 'no' to leave off: n
   Would you like to save the extracted data for future use?
   Type 'y' for yes: y
   What file name would you like to give
   the data? new_slice</pre>

Please note that this may take awhile to generate... When it does come up, you should see something
like this:

.. image:: /_static/ucvm_16.12a_sc8.png

Since we set the file name to be "new_slice", UCVM saves the extracted data to new_slice.data
and also has a plot_new_slice.xml file. Using gedit, change the topography XML element to say
"yes" instead of "n". That is:

.. code-block:: bash

   <topography>n</topography> becomes <topography>yes</topography>

Re-run the plot using the command:

.. code-block:: bash

   ucvm_plot_horizontal_slice -f ./plot_new_slice.xml

This command will be faster as no extraction is needed (just reading it in the previously
extracted data from memory).

You will get a plot like this:

.. image:: /_static/ucvm_16.12a_sc9.png

Plotting a cross-section is similar. The command is:

.. code-block:: bash

   ucvm_plot_cross_section

As it follows a very similar pattern to the horizontal slice, I will omit that from this tutorial.

Read The Docs
-------------

If you are interested in learning more about UCVM and some of the additional capabilities,
run the command:

.. code-block:: bash

   ucvm_help

This will bring up a web browser with the latest documentation!

All the example scripts that were discussed in the presentation are located in
/home/ucvmalpha/presentation.

Thanks for trying UCVM!

.. toctree::
    :maxdepth: 2

