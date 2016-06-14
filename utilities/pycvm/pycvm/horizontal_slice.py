##
#  @file horizontal_slice.py
#  @brief Plots a horizontal slice either for display or saving to a file.
#  @author David Gill - SCEC <davidgil@usc.edu>
#  @version 14.7.0
#
#  Allows for generation of a horizontal slice, either interactively, via
#  arguments, or through Python code in the class HorizontalSlice.

#  Imports
from common import Plot, Point, MaterialProperties, UCVM, UCVM_CVMS, \
                   math, pycvm_cmapDiscretize, cm, mcolors, basemap, np, plt

##
#  @class HorizontalSlice
#  @brief Plots a horizontal slice starting at a given @link common.Point Point @endlink
#         to another @link common.Point Point @endlink.
#
#  Generates a horizontal slice that can either be displayed to the user, saved to a file
#  or differenced with another plot.
class HorizontalSlice:
    
    ##
    #  Initializes the horizontal slice. The slice will go from the upper-left 
    #  @link common.Point Point @endlink parameter to the bottom-right 
    #  @link common.Point Point @endlink parameter, at the depth specified in the 
    #  upper-left point. 
    #
    #  @param upperleftpoint The @link common.Point starting point @endlink from which this plot should start.
    #  @param bottomrightpoint The @link common.Point ending point @endlink at which this plot should end.
    #  @param spacing The spacing, in degrees, for this plot.
    #  @param cvm The community velocity model from which this data should come.
    #  
    def __init__(self, upperleftpoint, bottomrightpoint, spacing, cvm):
        
        if not isinstance(upperleftpoint, Point):
            raise TypeError("Parameter upperleftpoint must be of type Point.")
        else:
            ## The upper-left point from which this plot should originate.
            self.upperleftpoint = upperleftpoint
            
        if not isinstance(bottomrightpoint, Point):
            raise TypeError("Parameter bottomrightpoint must be of type Point.")
        else:
            ## The bottom-right corner at which this plot should end.
            self.bottomrightpoint = bottomrightpoint
        
        #  Check that the bottom-right point is below and right of the upper-left point.
        if (self.upperleftpoint.longitude > self.bottomrightpoint.longitude) or \
           (self.upperleftpoint.latitude < self.bottomrightpoint.latitude):
            raise ValueError("The upper-left point must be higher than, and to the " + \
                             "left of, the bottom-right point.")
            
        #  Check the spacing. If it's specified in meters, convert to degrees.
        try:
            ## The spacing for the plot, defined in degrees. If meters specified, it's converted to degrees.
            self.spacing = float(spacing)
        except Exception:
            print "TODO"
        
        ## The community velocity model from which the data should be retrieved.
        self.cvm = cvm
    
    ##
    #  Retrieves the values for this horizontal slice and stores them in the class.
    def getplotvals(self):
        
        #  How many y and x values will we need?
        
        ## The plot width - needs to be stored as property for the plot function to work.
        self.plot_width  = self.bottomrightpoint.longitude - self.upperleftpoint.longitude
        ## The plot height - needs to be stored as a property for the plot function to work.
        self.plot_height = self.upperleftpoint.latitude - self.bottomrightpoint.latitude 
        ## The number of x points we retrieved. Stored as a property for the plot function to work.
        self.num_x = int(math.ceil(self.plot_width / self.spacing)) + 1
        ## The number of y points we retrieved. Stored as a property for the plot function to work.
        self.num_y = int(math.ceil(self.plot_height / self.spacing)) + 1
        
        #  Generate a list of points to pass to UCVM.
        ucvmpoints = []
        
        for y in xrange(0, self.num_y):
            for x in xrange(0, self.num_x):
                ucvmpoints.append(Point(self.upperleftpoint.longitude + x * self.spacing, \
                                        self.bottomrightpoint.latitude + y * self.spacing, \
                                        self.upperleftpoint.depth))
        
        ## The 2D array of retrieved material properties.
        self.materialproperties = [[MaterialProperties(-1, -1, -1) for x in xrange(self.num_x)] for x in xrange(self.num_y)] 
        
        u = UCVM()
        data = u.query(ucvmpoints, self.cvm)
        
        i = 0
        j = 0
        
        for matprop in data:
            self.materialproperties[i][j] = matprop
            j = j + 1
            if j >= self.num_x:
                j = 0
                i = i + 1
    
    ## 
    #  Plots the horizontal slice either to an image or a file name.
    # 
    #  @param property The property either as a single item array or string.
    #  @param filename The file name of the image if we're saving it. Optional.
    #  @param title The title of the plot. Optional.
    #  @param horizontal_label The horizontal label of the plot. Optional.
    #  @param color_scale The color scale for the plot (d, discretized; s, smooth). Optional.
    def plot(self, property, filename = None, title = None, horizontal_label = None, color_scale = "d"):
        
        if self.upperleftpoint.description == None:
            location_text = ""
        else:
            location_text = self.upperleftpoint.description + " "

        # Gets the better CVM description if it exists.
        try:
            cvmdesc = UCVM_CVMS[self.cvm]
        except: 
            cvmdesc = self.cvm

        if title == None:
            title = "%s%s Horizontal Slice at %.0fm" % (location_text, cvmdesc, self.upperleftpoint.depth)

        self.getplotvals()

        # Call the plot object.
        p = Plot(title, "", "", None, 10, 10)

        BOUNDS = [0, 0.2, 0.4, 0.6, 0.8, 1, 1.5, 2, 2.5, 3, 3.5, 4, 4.5, 5]
        TICKS = [0, 0.5, 1, 1.5, 2, 2.5, 3, 3.5, 4, 4.5, 5]

        if property == "vp":
            BOUNDS = [bound * 1.7 for bound in BOUNDS]
            TICKS = [tick * 1.7 for tick in TICKS]
            
        if color_scale == "s":
            colormap = cm.RdBu
            norm = mcolors.Normalize(vmin=BOUNDS[0],vmax=BOUNDS[len(BOUNDS) - 1])
        elif color_scale == "sd":
            colormap = cm.RdBu
            norm = mcolors.Normalize(vmin=self.min_val,vmax=self.max_val)      
            TICKS = [self.min_val, (self.min_val + self.max_val) / 2, self.max_val]      
        else:
            colormap = pycvm_cmapDiscretize(cm.RdBu, len(BOUNDS) - 1)
            norm = mcolors.BoundaryNorm(BOUNDS, colormap.N)  
        
        m = basemap.Basemap(projection='cyl', llcrnrlat=self.bottomrightpoint.latitude, \
                            urcrnrlat=self.upperleftpoint.latitude, \
                            llcrnrlon=self.upperleftpoint.longitude, \
                            urcrnrlon=self.bottomrightpoint.longitude, \
                            resolution='f', anchor='C')
        
        lat_ticks = np.arange(self.bottomrightpoint.latitude, self.upperleftpoint.latitude + 0.1, self.plot_height / 2)
        lon_ticks = np.arange(self.upperleftpoint.longitude, self.bottomrightpoint.longitude + 0.1, self.plot_width / 2)
    
        m.drawparallels(lat_ticks, linewidth=1.0, labels=[1,0,0,0])
        m.drawmeridians(lon_ticks, linewidth=1.0, labels=[0,0,0,1])
        m.drawstates()
        m.drawcountries()
    
        lons = np.arange(self.upperleftpoint.longitude, self.bottomrightpoint.longitude, self.spacing)
        lats = np.arange(self.bottomrightpoint.latitude, self.upperleftpoint.latitude, self.spacing)
    
        # Get the properties.
        datapoints = np.arange(self.num_x * self.num_y,dtype=float).reshape(self.num_y, self.num_x)
        
        for i in xrange(0, self.num_y):
            for j in xrange(0, self.num_x):
                if property != "poisson":
                    if color_scale == "sd":
                        datapoints[i][j] = self.materialproperties[i][j].getProperty(property)
                    else:
                        datapoints[i][j] = self.materialproperties[i][j].getProperty(property) / 1000.0
                else:
                    datapoints[i][j] = self.materialproperties[i][j].vp / self.materialproperties[i][j].vs
                    
        t = m.transform_scalar(datapoints, lons, lats, len(lons), len(lats))
        img = m.imshow(t, cmap=colormap, norm=norm)
    
        m.drawcoastlines()
    
        cax = plt.axes([0.125, 0.05, 0.775, 0.02])
        cbar = plt.colorbar(img, cax=cax, orientation='horizontal',ticks=TICKS)
        if property != "poisson":
            if horizontal_label == None:
                cbar.set_label(property.title() + " (km/s)")
            else:
                cbar.set_label(horizontal_label)
        else:
            cbar.set_label("Vp/Vs")
            
        if filename:
            plt.savefig(filename)
        else:
            plt.show()