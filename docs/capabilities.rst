Capabilities
============

UCVM includes some key capabilities that are essential to supporting velocity model query, visualization, and
extraction.

**Support For Many Models**

UCVM supports 7 3D California velocity models. It also supports 1D models, the Ely GTL scheme, trilinear interpolation
and a DEM that has both USGS National Map data and falls back to the ETOPO1 data outside of California. All in all, UCVM
can interact with 14 total sources of data for query, visualization, and meshing.

**Querying Velocity Models**

UCVM presently supports the ability to query (retrieve material properties) from multiple 3D community velocity models
in the southern California region.

Example:

.. code-block:: text

    ucvm_query -m cvms426
    Queries the CVM-S4.26 velocity model.

**Tiling Multiple Velocity Models**

When querying, visualizing, or extracting material models, UCVM allows the user to tile models in three dimensions. You
can specify a model order using a semi-colon. In the case of a model like "cvms426;1d[SCEC]", for any longitude,
latitude, UCVM will first query CVM-S4.26. If no material properties exist, it will then query the 1d model. There is
no limitation on how many models can be tiled together.

.. code-block:: text

    ucvm_query -m "cvms426;1d[SCEC]"
    Queries the CVM-S4.26 velocity model, defaulting back to the 1D model if material
    properties at the given longitudes and latitudes do not exist.

**Combining Multiple Models and/or Operators Together**

UCVM can combine multiple models together and use material properties from one model, Vs30 data from another, and so
on. Operators included are the Ely GTL (elygtl), trilinear interpolator (trilinear). Vs30 can also be calculated
instead of being taken from the Wills-Wald 2006.

.. code-block:: text

    ucvm_query -m "(cvms426;1d[SCEC]).vs30-calc"
    Queries the CVM-S4.26 model, defaulting back to the 1D model if material properties
    do not exist. The returned Vs30 data is computed using model's Vs values at the top
    30m and does not come from the Wills-Wald map.

**Models That Have Attenuation Now Display It**

The USGS Bay Area velocity model has a built-in attenuation model. Previous versions of UCVM would discard this
information, but the new UCVM will display it to the user.

.. code-block:: text

    ucvm_query -m bayarea
    Queries that fall within the domain of the USGS Bay Area velocity model will return
    attenuation.

**Query By Depth or Elevation**

UCVM can query any velocity model by depth or elevation (with the model being flattened or raised by the amount
that is provided by either the USGS National Map data or the ETOPO1 background model). If the model includes its
own DEM, then that is used instead. This .elevation notation can be used throughout the entire UCVM package.

.. code-block:: text

    ucvm_query -m bayarea.elevation
    Queries the Bay Area velocity model by elevation, not by depth.

**Add Models On The Fly**

A re-installation of UCVM is not required to add a velocity model. The ucvm_model_manager utility can download
and add velocity models to a pre-existing installation.

.. code-block:: text

    ucvm_model_manager -a bayarea
    Adds the USGS Bay Area velocity model to your pre-existing UCVM installation.

**List Models Available For Download**

The same ucvm_model_manager utility can list available velocity models that can be downloaded to your UCVM installation.

.. code-block:: text

    ucvm_model_manager -l
    Lists all the velocity models that can be downloaded from the internet.

**Plot Horizontal Slices**

UCVM can plot horizontal slices of material models. UCVM can also plot Vs30 and elevation data. Furthermore, extracted
data is saved by default so that re-extraction doesn't need to happen every time a plot is generated.

.. code-block:: text

    ucvm_plot_horizontal_slice
    Asks a series of questions to generate a horizontal slice.

**Horizontal Slices With San Andreas Fault And Topography**

UCVM can plot horizontal slices which include overlays of the San Adreas Fault and topography to help guide the user's
eye when discerning the plots. This can be added into a generated XML file by modifying root/plot/features/faults to yes
or no and root/plot/features/topography to yes or no, respectively. Alternatively, the user can choose this during
the plot generation questions if the "-a" or "advanced questions" flag is set.

.. code-block:: text

    ucvm_plot_horizontal_slice -a
    Asks a series of questions to generate a horizontal slice and also asks some
    advanced questions (such as topography).

**Plot Cross-Sections**

Cross-sections or vertical slices of material models can be generated with UCVM. Similar to the horizontal slice
plotting scripts, UCVM can plot cross-sections of any model or tiled combination of models.

.. code-block:: text

    ucvm_plot_cross_section
    Asks a series of questions to generate a cross-section.

**Plot Cross-Sections With Topography**

Cross-sections can include topography. There is a question within the plotting script if you want the cross-section
to be by elevation or depth. Select elevation. Then when it says "which velocity model would you like", type in the
model code and appened .elevation (e.g. bayarea.elevation).

.. code-block:: text

    ucvm_plot_cross_section
    Asks a series of questions. To plot elevation, select elevation and make sure to type
    in the model code and append .elevation (like bayarea.elevation).

**Plot Depth Profile**

Depth profiles can be generated in UCVM in a very similar manager to horizontal slices and cross-sections. Run the
utility and answer a series of questions.

.. code-block:: text

    ucvm_plot_depth_profile
    Asks a series of questions and plots a depth profile.

**Plot Comparisons of Data Products and Models Within UCVM**

Through the ucvm_plot_comparison utility, UCVM can plot and display differences and similarities between two models.
For example, suppose you have just extracted a mesh of CVM-S4.26 and want to see how it compares with the real S4.26
material properties. You can use the ucvm_plot_comparison script to ensure that the extraction happened successfully.

.. code-block:: text

    ucvm_plot_comparison
    Asks a series of questions and plots the comparison between two models.

**E-tree Extraction**

UCVM can extract e-tree material models. To do so using the single-core program, do ucvm_etree_create. The MPI version
is ucvm_etree_create_mpi.

.. code-block:: text

    ucvm_etree_create or ucvm_etree_create_mpi
    Asks a series of questions and creates the e-tree. Alternatively, the -f flag
    can be used to provide an already-generated XML file.

**Partial E-tree Extraction**

A new capability in this version of UCVM is the ability to extract parts of an e-tree at a time. E-trees are extracted
by rows and columns. You can extract from one column to another column (inclusive) or a row or rows at a time. So if
your e-tree has 40 rows, you can extract the first 10 rows, then the next 10, and so on until the full material model
is extracted. Rows and columns **are 1-based**. You cannot add the same column twice to an e-tree, but you can stagger
out the jobs with consecutive columns and rows until the full model is extracted.

.. code-block:: text

    ucvm_etree_create(_mpi) -f extract.xml -r 1
    Extracts the first row of the e-tree.
    ucvm_etree_create(_mpi) -f extract.xml -r 1-5
    Extracts the first five rows of the e-tree.
    ucvm_etree_create(_mpi) -f extract.xml -i 1,1
    Extracts the first column of the first row only.
    ucvm_etree_create(_mpi) -f extract.xml -i 1,5-2,10
    Extracts the fifth column of the first row through the 10th column of the
    second row.

**Mesh Extraction**

UCVM can extract a standard Cartesian mesh either in a format that works with the AWP-ODC code or the RWG wave
propagation code. Like the e-tree, the single-core version is ucvm_mesh_create. The MPI version is
ucvm_mesh_create_mpi.

.. code-block:: text

    ucvm_mesh_create or ucvm_mesh_create_mpi
    Asks a series of questions and then creates the mesh. If the -f flag is provided
    then it can use a pre-existing XML file.

**Partial Mesh Extraction**

As with e-trees, UCVM has hte ability to extract partial meshes. For example, you can extract the first 10% of a mesh,
then the second 10%, the third, and so on. This helps break up big jobs into smaller, more manageable ones. UCVM
can also generate slices, so the first (surface slice) for example or a range of slices. Slices are **1-based** but
intervals (percentages) are **0-based**. So saying extract 0%-10% means extract from the first point to 10% in.

.. code-block:: text

    ucvm_mesh_create(_mpi) -f extract.xml -s 1
    Extracts the first (surface) slice of the mesh.
    ucvm_mesh_create(_mpi) -f extract.xml -s 10-20
    Extracts the 10th slice through the 20th slice.
    ucvm_mesh_create(_mpi) -f extract.xml -i 0-10
    Extracts the first 10% of the mesh.

**Re-use Data Products Within UCVM**

UCVM includes a model called "dataproductreader" which takes as input the XML and file that were generated as part of
either the meshing or e-tree procedures listed above and uses that new product as a model within UCVM. Cartesian meshes
are trilinearly interpolated.

.. code-block:: text

    ucvm_query -m dataproductreader[my_new_mesh_or_etree]
    Queries the mesh or e-tree as defined by my_new_mesh_or_etree.xml.

**Display More Extensive Help Documentation**

More extensive documentation can be displayed in the web browser.

.. code-block:: text

    ucvm_help
    Pops up a web browser with the help documentation.

**Improved Tests**

Most models within UCVM include their own tests. UCVM also includes framework and meshing tests to ensure that those
capabilities work on users' computers.

.. code-block:: text

    ucvm_run_tests -t
    Runs the framework tests for UCVM.
    ucvm_run_tests -m cvms426
    Runs the tests for CVM-S4.26 (usually this includes an acceptance test).
