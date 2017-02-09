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
		    <y>34.120549</y>        <!-- the Y (latitude) for the bottom-left starting point -->
		    <x>-119.288842</x>      <!-- the X (longitude) for the bottom-left point -->
		    <z>0.0</z>              <!-- the depth/elevation at which the surface slice is -->
		    <depth_elev>depth</depth_elev>  <!-- specifies query by depth or elevation -->
		    <projection>+proj=latlong +datum=WGS84</projection>     <!-- point's projection -->
	    </initial_point>
	    <minimums>
		    <vp>1700</vp>   <!-- minimum Vp -->
		    <vs>500</vs>    <!-- minimum Vs -->
	    </minimums>
	    <spacing>20</spacing>                   <!-- grid spacing in the projection's size -->
	    <cvm_list>cvms426;1d[SCEC]</cvm_list>   <!-- list of CVMs to query -->
	    <rotation>39.9</rotation>               <!-- rotation angle of the box -->
	    <out_dir>./</out_dir>                   <!-- output directory for the file -->
	    <dimensions>
		    <y>6750</y>     <!-- number of Y grid points -->
		    <x>9000</x>     <!-- number of X grid points -->
		    <z>3100</z>     <!-- number of Z grid points -->
	    </dimensions>
	    <format>awp</format>        <!-- format of the mesh - either awp or rwg -->
	    <mesh_name>high_f_mesh_awp</mesh_name>      <!-- file name of the mesh -->
	    <grid_type>center</grid_type>       <!-- grid point is centered or vertex -->
	    <projection>+proj=utm +datum=WGS84 +zone=11</projection>    <!-- mesh projection -->
    </root>

E-tree
~~~~~~

::

    <?xml version="1.0" encoding="utf-8"?>
    <root>
	    <projection>geo-bilinear</projection>   <!-- for e-trees this is always geo-bilinear -->
	    <minimums>
		    <vp>1700</vp>   <!-- minimum Vp -->
		    <vs>500</vs>    <!-- minimum Vs -->
	    </minimums>
	    <author>
		    <title>Example E-tree</title>               <!-- title for your e-tree -->
		    <person>John Doe</person>                   <!-- the person who generated it -->
		    <date>2016-12-01 00:07:42.010714</date>     <!-- the date/time this file was made -->
	    </author>
	    <format>etree</format>                          <!-- format is always etree -->
	    <etree_name>high_f_etree</etree_name>
	    <dimensions>
		    <x>180000.0</x>         <!-- the e-tree X length in meters -->
		    <y>135000.0</y>         <!-- the e-tree Y length in meters -->
		    <z>61875.0</z>          <!-- the e-tree Z length in meters -->
	    </dimensions>
	    <properties>
		    <parts_per_wavelength>10.0</parts_per_wavelength>   <!-- parts per wavelength -->
		    <max_frequency>4.0</max_frequency>                  <!-- max frequency in hertz -->
		    <max_octant_size>1000.0</max_octant_size>           <!-- max octant size in meters -->
		    <columns>64</columns>                               <!-- number of columns -->
		    <rows>48</rows>                                     <!-- number of rows -->
	    </properties>
	    <cvm_list>cvms426;1d[SCEC]</cvm_list>   <!-- the CVM list -->
	    <out_dir>./</out_dir>                   <!-- the output directory -->
	    <corners>
		    <ul>
			    <y>35.061096</y>        <!-- the upper-left latitude coordinate -->
			    <x>-118.354016</x>      <!-- the upper-left longitude coordinate -->
		    </ul>
		    <bl>
			    <y>34.120549</y>        <!-- the bottom-left latitude coordinate -->
			    <x>-119.288842</x>      <!-- the bottom-left longitude coordinate -->
		    </bl>
		    <ur>
			    <y>34.025873</y>        <!-- the upper-right latitude coordinate -->
			    <x>-116.846030</x>      <!-- the upper-right longitude coordinate -->
		    </ur>
		    <br>
			    <y>33.096503</y>        <!-- the bottom-right latitude coordinate -->
			    <x>-117.780976</x>      <!-- the bottom-right longitude coordinate -->
		    </br>
	    </corners>
    </root>

Visualization
-------------

Comparison
~~~~~~~~~~

::

    <?xml version="1.0" encoding="utf-8"?>
    <root>
	    <properties>
		    <spacing>
			    <width>0.01</width>     <!-- x grid spacing in start point projection -->
			    <height>0.01</height>   <!-- y grid spacing in start point projection -->
		    </spacing>
		    <property>vs</property>     <!-- material property to plot difference for -->
	    </properties>
	    <cvms>
		    <cvm1>1d[SCEC]</cvm1>                       <!-- first CVM for comparison -->
		    <cvm2>1d[CyberShake_BBP_LA_Basin]</cvm2>    <!-- second CVM for comparison -->
	    </cvms>
	    <type>horizontal</type>         <!-- this is always horizontal for now -->
	    <end_point>
		    <depth_elev>0</depth_elev>  <!-- 0 means point is depth, 1 for elev -->
		    <y>35.0</y>                 <!-- y or latitude coordinate -->
		    <projection>+proj=latlong +datum=WGS84</projection>  <!-- Proj.4 projection -->
		    <x>-117.0</x>               <!-- x or longitude coordinate -->
		    <z>0.0</z>                  <!-- z or depth/elevation coordinate -->
	    </end_point>
	    <start_point>
		    <depth_elev>0</depth_elev>  <!-- 0 means point is depth, 1 for elev -->
		    <y>34.0</y>                 <!-- y or latitude coordinate -->
		    <projection>+proj=latlong +datum=WGS84</projection>  <!-- Proj.4 projection -->
		    <x>-118.0</x>               <!-- x or longitude coordiante -->
		    <z>0.0</z>                  <!-- z or depth/elevation coordinate -->
	    </start_point>
	    <save>n</save>                  <!-- save extracted data, y for yes, n for no -->
    </root>

Cross-Section
~~~~~~~~~~~~~

::

    <?xml version="1.0" encoding="utf-8"?>
    <root>
	    <cvm_list>1d[SCEC]</cvm_list>   <!-- list of CVMs to query -->
	    <end_point>
		    <x>-117.0</x>               <!-- x or longitude coordinate -->
		    <depth_elev>0</depth_elev>  <!-- 0 means point is depth, 1 for elev -->
		    <z>10000.0</z>              <!-- ending depth for cross-section -->
		    <projection>+proj=latlong +datum=WGS84</projection> <!-- Proj.4 projection -->
		    <y>35.0</y>                 <!-- y or latitude coordinate -->
	    </end_point>
	    <cross_section_properties>
	    	<height_spacing>100</height_spacing>    <!-- the cell spacing height in m -->
	    	<width_spacing>1000</width_spacing>     <!-- the cell spacing width in m -->
	    	<property>vs</property>                 <!-- cross-section material property -->
	    </cross_section_properties>
	    <start_point>
	    	<x>-118.0</x>               <!-- x or longitude coordinate -->
	    	<depth_elev>0</depth_elev>  <!-- 0 means point is depth, 1 for elev -->
	    	<z>0.0</z>                  <!-- starting depth for cross-section -->
	    	<projection>+proj=latlong +datum=WGS84</projection> <!-- Proj.4 projection -->
	    	<y>34.0</y>                 <!-- y or latitude coordinate -->
	    </start_point>
    </root>

Depth Profile
~~~~~~~~~~~~~

::

    <?xml version="1.0" encoding="utf-8"?>
    <root>
	    <cvm_list>1d[SCEC]</cvm_list>   <!-- list of CVMs to query -->
	    <plot>
		    <title>Depth Profile at (-118.00, 34.00)</title>    <!-- title for the plot -->
	    </plot>
	    <profile_properties>
	    	<spacing>20.0</spacing>     <!-- vertical spacing at which to query -->
	    	<depth>50000.0</depth>      <!-- final depth for plot -->
	    	<properties>vs</properties> <!-- material properties or list of properties to query -->
	    </profile_properties>
	    <profile_point>
	    	<projection>+proj=latlong +datum=WGS84</projection> <!-- Proj. 4 projection -->
	    	<y>34.0</y>                     <-- y or latitude coordinate -->
	    	<x>-118.0</x>                   <-- x or longitude coordinate -->
	    	<z>0.0</z>                      <-- starting depth or elevation for profile -->
	    	<depth_elev>0</depth_elev>      <-- 0 means point is depth, 1 for elev -->
	    </profile_point>
    </root>

Horizontal Slice
~~~~~~~~~~~~~~~~

::

    <?xml version="1.0" encoding="utf-8"?>
    <root>
    	<data>
    		<save>extraction_b3239.data</save>  <!-- save the extracted data to file -->
    	</data>
    	<cvm_list>1d[SCEC]</cvm_list>           <!-- the list of CVMs to query -->
    	<bottom_left_point>
    		<x>-119.0</x>                       <!-- x or longitude coordinate -->
    		<depth_elev>0</depth_elev>          <!-- 0 means point is depth, 1 for elev -->
    		<y>33.0</y>                         <!-- y or latitude coordinate -->
    		<projection>+proj=latlong +datum=WGS84</projection>     <!-- Proj. 4 projection -->
    		<z>0.0</z>                          <!-- horizontal slice depth or elev in m -->
    	</bottom_left_point>
    	<plot>
    		<property>vs</property>             <!-- material property to plot -->
    		<generate>y</generate>              <!-- show the plot -->
    		<title>cvms426 Slice From (-119.00, 33.00)</title>  <!-- plot title -->
    		<features>
    			<scale>discrete</scale>         <!-- use the discrete scale color bar -->
    			<colormap>RdBu</colormap>       <!-- matplotlib color scale (RdBu = Red-Blue) -->
    			<faults>Yes</faults>            <!-- show the San Andreas fault -->
    			<topography>No</topography>     <!-- show topography on the plot -->
    		</features>
    	</plot>
    	<properties>
    		<rotation>0.0</rotation>            <!-- rotation for the box to query -->
    		<spacing>0.1</spacing>              <!-- grid spacing -->
    		<num_y>10</num_y>                   <!-- number of x points to query -->
    		<num_x>10</num_x>                   <!-- number of y points to query -->
    	</properties>
    </root>