.. _CommandReference:

Command Reference
=================

UCVM allows users to interact with multiple 3D seismic velocity models. Broadly, UCVM lets users query models, generate
material models for use with 3D wave propagation simulation codes, and visualize those models. Each command comes
with help documentation which can be accessed by passing the "-h" parameter to any command line tool.

Query
~~~~~

**ucvm_query**: This utility queries a given velocity model (in the UCVM model syntax format) and returns its material
properties on the command line. Returned material properties are configured by the velocity model, unless specifically
overridden by the user. All points provided to this utility are expected to be in WGS84 latitude, longitude format,
unless stated otherwise through the '-p' option.

If you type in your points in the command line instead of providing a file, you must hit enter twice after the last
point or type Ctrl-D to retrieve the properties.

Parameters:
::

    -m, --model m:         The unique model identifier, m. For example, "cvms4" or "cvms426". If the
                           model is not registered, an error will be thrown.
    -i, --input f:         Optional. An input file, i, from which all the points should be read. If
                           no file provided, the points will be read from the command line.
    -o, --output o:        Optional. The output file, o, to which all the material properties should
                           be written.
    -p, --projection p:    Optional. Specifies a projection, p, in Proj.4 format, in which all
                           points inputted to this utility are specified.
    -a, --all-meta:        Optional. Specifies that ucvm_query should output all the metadata
                           associated with the query. That is, it should output the references that
                           support the query and so forth.

Example usage:
::

    ucvm_query -m cvms426               -- Queries CVM-S4.26.
    ucvm_query -m cvms426.elevation     -- Queries CVM-S4.26 by elevation.
    ucvm_query -m 1d[SCEC]              -- Queries the SCEC 1D model.

Meshing
~~~~~~~

**ucvm_etree_create**: This is the single-core command to create an e-tree (unstructured octree) using UCVM. This
command accepts a configuration file or, if one is not provided, it will ask a series of questions before generating
the e-tree. This unstructured octree format is used primarily with the Hercules forward wave propagation simulation
code.

Parameters
::

    -c, --config-only c:   Generates the XML-style configuration file only. No e-tree will be made at
                           the end of the questions.
    -f, --file f:          Specifies the configuration file from which this utility should read.
    -r, --rows r:          Extracts one or more rows of the etree. If r is a number like "1"
                           just that row will be extracted. If r is an interval like "1-5"
                           those rows (1, 2, 3, 4, 5 e.g.) inclusive will be extracted.
    -i, --interval i:      Extracts exactly the interval desired. The parameter must be
                           specified as row,column-row,column. E.g. the parameters
                           1,5-10,12 would extract row 1, column 5 through row 10,
                           column 12 inclusive.

Example usage:
::

    ucvm_etree_create -c                -- Generates the e-tree configuration file.
    ucvm_etree_create -f myfile.xml     -- Reads the config file and generates the e-tree.
    ucvm_etree_create                   -- Asks a series of questions and then generates the e-tree.

**ucvm_etree_create_mpi**: This is the MPI version of the above utility. Please note that this must be executed
using a "mpirun"-like command. It cannot be launched directly from the command-line. Also, please note that due to
the way MPI programs operate, all library paths must be explicitly added to your DYLD_LIBRARY_PATH or LD_LIBRARY_PATH
variables. If they are not, this command may fail. One process acts as the writer, so if this command is run on eight
cores then seven cores will do the extraction and one will be responsible for writing to the data file.

Parameters:
::

    -c, --config-only c:   Generates the XML-style configuration file only. No e-tree will be made at
                           the end of the questions.
    -f, --file f:          Specifies the configuration file from which this utility should read.
    -r, --rows r:          Extracts one or more rows of the etree. If r is a number like "1"
                           just that row will be extracted. If r is an interval like "1-5"
                           those rows (1, 2, 3, 4, 5 e.g.) inclusive will be extracted.
    -i, --interval i:      Extracts exactly the interval desired. The parameter must be
                           specified as row,column-row,column. E.g. the parameters
                           1,5-10,12 would extract row 1, column 5 through row 10,
                           column 12 inclusive.

Example usage:
::

    mpirun -n 8 ucvm_etree_create_mpi -f myfile.xml     -- Generates the myfile.xml etree with 8 cores.
    mpirun -n 8 ucvm_etree_create_mpi                   -- Asks questions then generates using 8 cores.

**ucvm_mesh_create**: This is the single-core command to create a binary float mesh using UCVM. This command accepts
a configuration file or, if one is not provided, it will ask a series of questions before generating the mesh. This
command produces a mesh in either AWP format for use with the AWP-ODC wave propagation simulation code or RWG format
which is for use with Rob Graves' forward wave propagation simulation code.

Parameters:
::

    -c, --config-only c:   Generates the XML-style configuration file only. No mesh will be
                           made at the end of the questions.
    -f, --file f:          Specifies the configuration file from which this utility should read.
                           style configuration file vs. the new XML format.
    -s, --slices s:        Extracts one or more slices. If slice is a number, like '1' or '5',
                           that slice is extracted. If slice is a range, like 1-5, then slices
                           1, 2, 3, 4, and 5, will be extracted. Slice 1 is the surface.
    -i, --interval i:      Extracts a percentage of the mesh. If i is '0-10', for example, then
                           the first 10% of the mesh will be extracted. If i is '50-75' then
                           the third quarter of the mesh will be extracted.

Example usage:
::

    ucvm_mesh_create -c                -- Generates the mesh configuration file.
    ucvm_mesh_create -f myfile.xml     -- Reads the config file and generates the mesh.
    ucvm_mesh_create                   -- Asks a series of questions and then generates the mesh.

**ucvm_mesh_create_mpi**: This is the MPI version of the above utility. Please note that this must be executed
using a "mpirun"-like command. It cannot be launched directly from the command-line. Also, please note that due to
the way MPI programs operate, all library paths must be explicitly added to your DYLD_LIBRARY_PATH or LD_LIBRARY_PATH
variables. If they are not, this command may fail.

Parameters:
::

    -c, --config-only c:   Generates the XML-style configuration file only. No mesh will be
                           made at the end of the questions.
    -f, --file f:          Specifies the configuration file from which this utility should read.
                           style configuration file vs. the new XML format.
    -s, --slices s:        Extracts one or more slices. If slice is a number, like '1' or '5',
                           that slice is extracted. If slice is a range, like 1-5, then slices
                           1, 2, 3, 4, and 5, will be extracted. Slice 1 is the surface.
    -i, --interval i:      Extracts a percentage of the mesh. If i is '0-10', for example, then
                           the first 10% of the mesh will be extracted. If i is '50-75' then
                           the third quarter of the mesh will be extracted.

Example usage:
::

    mpirun -n 8 ucvm_mesh_create_mpi -f myfile.xml     -- Generates the myfile.xml mesh with 8 cores.
    mpirun -n 8 ucvm_mesh_create_mpi                   -- Asks questions then generates using 8 cores.

Visualization
~~~~~~~~~~~~~

**ucvm_plot_comparison**: Compares two or more slices from meshes, models, and/or e-trees to check if they are
equivalent or not. This utility generates horizontal slices and calculates various statistics to determine equivalency.

Parameters:
::

    -f,  --file path:           Specifies a previously generated configuration file.

Example usage:
::

    ucvm_plot_comparison                 -- Asks a series of questions and generates the plot.
    ucvm_plot_comparison -f myplot.xml   -- Generates the plot described with myplot.xml.

**ucvm_plot_cross_section**: Generates a cross-section through the earth given one or more models. This utility can
either ask a series of questions and generate the plot or it can read in a saved configuration file.

Parameters:
::

    -f, --file f:          The configuration file from which to read.

Example usage:
::

    ucvm_plot_cross_section                 -- Asks a series of questions and generates the plot.
    ucvm_plot_cross_section -f myplot.xml   -- Generates the plot described with myplot.xml.

**ucvm_plot_depth_profile**: Generates a depth profile through the earth given one or more models. This utility can
either ask a series of questions and generate the plot or it can read in a saved configuration file.

Parameters:
::

    -f, --file f:          The configuration file from which to read.

Example usage:
::

    ucvm_plot_depth_profile                 -- Asks a series of questions and generates the plot.
    ucvm_plot_depth_profile -f myplot.xml   -- Generates the plot described with myplot.xml.

**ucvm_plot_horizontal_slice**: Generates a horizontal slice through the earth given one or more models. This utility
can either ask a series of questions and generate the plot or it can read in a saved configuration file.

Parameters:
::

    -f, --file f:          The configuration file from which to read.
    -a, --advanced:        If this flag is set, advanced questions will be asked.

Example usage:
::

    ucvm_plot_horiztonal_slice                 -- Asks a series of questions and generates the plot.
    ucvm_plot_horizontal_slice -f myplot.xml   -- Generates the plot described with myplot.xml.

Miscellaneous
~~~~~~~~~~~~~

**ucvm_model_manager**: Lists, adds, or removes models within UCVM. Models can be downloaded and added from the main
SCEC web servers or they can be added locally (e.g. for custom models). Removing a model will also delete the installed
files from your computer.

Parameters:
::

    -l, --list:            Lists all models available and states which ones are installed.
    -a, --add model:       Downloads and installs "model" and adds it to UCVM.
    -d, --directory dir:   Installs a model from a local directory. The directory must have
                           the proper ucvm_model.xml descriptor. If that is not found, UCVM
                           cannot install the model.
    -r, --remove model:    Removes "model" from the UCVM list of models. For models
                           downloaded and installed from the web, or copied locally, this
                           will also delete the model code and data itself.

Example usage:
::

    ucvm_model_manager -a cvms426       -- Adds CVM-S4.26 to your UCVM installation.
    ucvm_model_manager -r cvms426       -- Removes CVM-S4.26 from your UCVM installation.
    ucvm_model_manager -l               -- Lists all models installed within your copy of UCVM.

**ucvm_help**: Launches a web browser with the address of the help documentation for UCVM. There are no parameters
passable to this utility.

Example usage:
::

    ucvm_help