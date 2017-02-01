Configuration Files
===================

The plotting and mesh generation utilities of UCVM use XML configuration files. Each utility can generate its own
configuration files through a series of questions. Configuration files can be edited by hand as well. Below are
reference configuration files with a description of what each value represents.

Meshing
-------

AWP or RWG Binary Float
~~~~~~~~~~~~~~~~~~~~~~~

::

    <?xml version="1.0" encoding="utf-8"?>
    <root>
	    <initial_point>
		    <y>34.120549</y>        <-- the Y (latitude) for the bottom-left starting point
		    <x>-119.288842</x>      <-- the X (longitude) for the bottom-left point
		    <z>0.0</z>              <-- the depth/elevation at which the surface slice is
		    <depth_elev>depth</depth_elev>  <-- specifies query by depth or elevation
		    <projection>+proj=latlong +datum=WGS84</projection>     <-- point's projection
	    </initial_point>
	    <minimums>
		    <vp>1700</vp>   <-- minimum Vp
		    <vs>500</vs>    <-- minimum Vs
	    </minimums>
	    <spacing>20</spacing>                   <-- grid spacing in the projection's size
	    <cvm_list>cvms426;1d[SCEC]</cvm_list>   <-- list of CVMs to query
	    <rotation>39.9</rotation>               <-- rotation angle of the box
	    <out_dir>./</out_dir>                   <-- output directory for the file
	    <dimensions>
		    <y>6750</y>     <-- number of Y grid points
		    <x>9000</x>     <-- number of X grid points
		    <z>3100</z>     <-- number of Z grid points
	    </dimensions>
	    <format>awp</format>        <-- format of the mesh - either awp or rwg
	    <mesh_name>high_f_mesh_awp</mesh_name>      <-- file name of the mesh
	    <grid_type>center</grid_type>       <-- grid point is centered or vertex
	    <projection>+proj=utm +datum=WGS84 +zone=11</projection>    <-- mesh projection
    </root>

E-tree
~~~~~~

::

    <?xml version="1.0" encoding="utf-8"?>
    <root>
	    <projection>geo-bilinear</projection>   <-- for e-trees this is always geo-bilinear
	    <minimums>
		    <vp>1700</vp>   <-- minimum Vp
		    <vs>500</vs>    <-- minimum Vs
	    </minimums>
	    <author>
		    <title>Example E-tree</title>               <-- title for your e-tree
		    <person>John Doe</person>                   <-- the person who generated it
		    <date>2016-12-01 00:07:42.010714</date>     <-- the date/time this file was made
	    </author>
	    <format>etree</format>                          <-- format is always etree
	    <etree_name>high_f_etree</etree_name>
	    <dimensions>
		    <x>180000.0</x>         <-- the e-tree X length in meters
		    <y>135000.0</y>         <-- the e-tree Y length in meters
		    <z>61875.0</z>          <-- the e-tree Z length in meters
	    </dimensions>
	    <properties>
		    <parts_per_wavelength>10.0</parts_per_wavelength>   <-- parts per wavelength
		    <max_frequency>4.0</max_frequency>                  <-- max frequency in hertz
		    <max_octant_size>1000.0</max_octant_size>           <-- max octant size in meters
		    <columns>64</columns>                               <-- number of columns
		    <rows>48</rows>                                     <-- number of rows
	    </properties>
	    <cvm_list>cvms426;1d[SCEC]</cvm_list>   <-- the CVM list
	    <out_dir>./</out_dir>                   <-- the output directory
	    <corners>
		    <ul>
			    <y>35.061096</y>        <-- the upper-left latitude coordinate
			    <x>-118.354016</x>      <-- the upper-left longitude coordinate
		    </ul>
		    <bl>
			    <y>34.120549</y>        <-- the bottom-left latitude coordinate
			    <x>-119.288842</x>      <-- the bottom-left longitude coordinate
		    </bl>
		    <ur>
			    <y>34.025873</y>        <-- the upper-right latitude coordinate
			    <x>-116.846030</x>      <-- the upper-right longitude coordinate
		    </ur>
		    <br>
			    <y>33.096503</y>        <-- the bottom-right latitude coordinate
			    <x>-117.780976</x>      <-- the bottom-right longitude coordinate
		    </br>
	    </corners>
    </root>

Visualization
-------------

Comparison
~~~~~~~~~~

Cross-Section
~~~~~~~~~~~~~

Depth Profile
~~~~~~~~~~~~~

Horizontal Slice
~~~~~~~~~~~~~~~~
