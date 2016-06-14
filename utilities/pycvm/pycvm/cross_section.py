##
#  @file cross_section.py
#  @brief Plots a cross section to an image or a pre-existing plot.
#  @author David Gill - SCEC <davidgil@usc.edu>
#  @version 14.7.0
#
#  Allows for generation of a cross section between two points.

#  Imports
from common import Plot, Point, MaterialProperties, UCVM, UCVM_CVMS, \
                   math, pycvm_cmapDiscretize, cm, mcolors, basemap, np, plt

try:
    import pyproj
except StandardError, e:
    print "ERROR: PyProj must be installed for this script to work."
    exit(1)

##
#  @class CrossSection
#  @brief Plots a cross section between two @link common.Point Points @endlink.
#
#  Generates a cross section that can either be saved as a file or displayed
#  to the user. 
class CrossSection:
    
    ##
    #  Initializes the cross section class.
    #
    #  @param startingpoint The @link common.Point starting point @endlink from which this plot should start.
    #  @param endingpoint The @link common.Point ending point @endlink at which this plot will end.
    #  @param todepth The ending depth, in meters, where this plot should end.
    #  @param hspacing The discretization interval in meters, horizontally.
    #  @param vspacing The discretization interval in meters, vertically.
    #  @param cvm The CVM from which to retrieve these material properties.
    def __init__(self, startingpoint, endingpoint, todepth, hspacing, vspacing, cvm):
        if not isinstance(startingpoint, Point):
            raise TypeError("The starting point must be an instance of Point.")
        else:
            ## Defines the @link common.Point starting point @endlink for the cross section.
            self.startingpoint = startingpoint
            
        if not isinstance(endingpoint, Point):
            raise TypeError("The ending point must be an instance of Point.")
        else:
            ## Defines the @link common.Point ending point @endlink for the cross section.
            self.endingpoint = endingpoint
        
        if (todepth - self.startingpoint.depth) % vspacing != 0:
            raise ValueError("%s\n%s\n%s" % ("The spacing value does not divide evenly into the requested depth. ", \
                          "Please make sure that the depth (%.2f - %.2f) divided by the spacing " % (todepth, startingpoint.depth), \
                          "%.2f has no remainder" % (vspacing)))
        else:
            ## Defines the depth to which the plot should go in meters.
            self.todepth = todepth
            
        ## The vertical discretization of the plot, in meters.
        self.vspacing = vspacing
        
        ## The horizontal discretization of the plot, in meters.
        self.hspacing = hspacing
        
        ## The CVM to use (must be installed with UCVM).
        self.cvm = cvm

    ## 
    #  Generates the depth profile in a format that is ready to plot.
    def getplotvals(self):

        point_list = []

        proj = pyproj.Proj(proj='utm', zone=11, ellps='WGS84')

        x1, y1 = proj(self.startingpoint.longitude, self.startingpoint.latitude)
        x2, y2 = proj(self.endingpoint.longitude, self.endingpoint.latitude)

        num_prof = int(math.sqrt((x2-x1)*(x2-x1) + \
                                 (y2-y1)*(y2-y1))/self.hspacing)
        
        for j in xrange(int(self.startingpoint.depth), int(self.todepth) + 1, int(self.vspacing)):
            for i in xrange(0, num_prof + 1):
                x = x1 + i*(x2-x1)/float(num_prof)
                y = y1 + i*(y2-y1)/float(num_prof)
                lon, lat = proj(x, y, inverse=True)
                point_list.append(Point(lon, lat, j))
                
        u = UCVM()
        data = u.query(point_list, self.cvm)
        
        ## Private number of x points.
        self.num_x = num_prof + 1
        ## Private number of y points.
        self.num_y = (int(self.todepth) - int(self.startingpoint.depth)) / int(self.vspacing) + 1
        
        ## The 2D array of retrieved material properties.
        self.materialproperties = [[MaterialProperties(-1, -1, -1) for x in xrange(self.num_x)] for x in xrange(self.num_y)] 
        
        for y in xrange(0, self.num_y):
            for x in xrange(0, self.num_x):   
                self.materialproperties[y][x] = data[y * self.num_x + x]     
                
    ## 
    #  Plots the horizontal slice either to an image or a file name.
    # 
    #  @param property The property either as a single item array or string.
    #  @param filename The file name of the image if we're saving it. Optional.
    #  @param title The title for the plot. Optional.
    #  @param color_scale The color scale to use. Optional.
    def plot(self, property, filename = None, title = None, color_scale = "d"):
        
        if self.startingpoint.description == None:
            location_text = ""
        else:
            location_text = self.startingpoint.description + " "

        # Gets the better CVM description if it exists.
        try:
            cvmdesc = UCVM_CVMS[self.cvm]
        except: 
            cvmdesc = self.cvm

        if title == None:
            title = "%s%s Cross Section from (%.2f, %.2f) to (%.2f, %.2f)" % (location_text, cvmdesc, self.startingpoint.longitude, \
                        self.startingpoint.latitude, self.endingpoint.longitude, self.endingpoint.latitude)
            
        self.getplotvals()
        
        # Call the plot object.
        p = Plot(None, None, None, None, 10, 10)

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
    
        plt.axes([0.1,0.7,0.8,0.25])
    
        # Figure out which is upper-right and bottom-left.
        ur_lat = self.startingpoint.latitude if self.startingpoint.latitude > self.endingpoint.latitude else self.endingpoint.latitude
        ur_lon = self.startingpoint.longitude if self.startingpoint.longitude > self.endingpoint.longitude else self.endingpoint.longitude
        ll_lat = self.startingpoint.latitude if self.startingpoint.latitude < self.endingpoint.latitude else self.endingpoint.latitude
        ll_lon = self.startingpoint.longitude if self.startingpoint.longitude < self.endingpoint.longitude else self.endingpoint.longitude
    
        # Add 1% to each for good measure.
        ur_lat = ur_lat + 0.03 * ur_lat
        ur_lon = ur_lon - 0.015 * ur_lon
        ll_lat = ll_lat - 0.03 * ll_lat
        ll_lon = ll_lon + 0.015 * ll_lon
    
        # Plot map up top.
        m = basemap.Basemap(projection='cyl', llcrnrlat=ll_lat, urcrnrlat=ur_lat, \
                            llcrnrlon=ll_lon, urcrnrlon=ur_lon, \
                            resolution='f', anchor='C')
    
        lat_ticks = np.arange(ll_lat, ur_lat + 0.1, (ur_lat - ll_lat))
        lon_ticks = np.arange(ll_lon, ur_lon + 0.1, (ur_lon - ll_lon))
    
        m.drawparallels(lat_ticks, linewidth=1.0, labels=[1,0,0,0])
        m.drawmeridians(lon_ticks, linewidth=1.0, labels=[0,0,0,1])
        m.drawstates()
        m.drawcountries()
    
        m.drawcoastlines()
        m.drawmapboundary(fill_color='aqua')
        m.fillcontinents(color='brown',lake_color='aqua')
    
        m.plot([self.startingpoint.longitude,self.endingpoint.longitude], [self.startingpoint.latitude,self.endingpoint.latitude])
    
        valign1 = "top"
        valign2 = "bottom"
        if self.endingpoint.latitude < self.startingpoint.latitude:
            valign1 = "bottom"
            valign2 = "top"
    
        plt.text(self.startingpoint.longitude, self.startingpoint.latitude, \
                 '[S] %.1f, %.1f' % (self.startingpoint.longitude, self.startingpoint.latitude), \
                 color='k', horizontalalignment="center", verticalalignment=valign1)
    
        plt.text(self.endingpoint.longitude, self.endingpoint.latitude, \
                 '[E] %.1f, %.1f' % (self.endingpoint.longitude, self.endingpoint.latitude), \
                 color='k', horizontalalignment="center", verticalalignment=valign2)
    
        plt.axes([0.05,0.18,0.9,0.54])
    
        datapoints = np.arange(self.num_x * self.num_y,dtype=float).reshape(self.num_y, self.num_x)
            
        for y in xrange(0, self.num_y):
            for x in xrange(0, self.num_x):   
                datapoints[y][x] = self.materialproperties[y][x].getProperty(property) / 1000          
    
        img = plt.imshow(datapoints, cmap=colormap, norm=norm)
        plt.xticks([0,self.num_x/2,self.num_x], ["[S] %.2f" % self.startingpoint.longitude, \
                                                 "%.2f" % ((float(self.endingpoint.longitude) + float(self.startingpoint.longitude)) / 2), \
                                                 "[E] %.2f" % self.endingpoint.longitude])
        plt.yticks([0,self.num_y/2,self.num_y], ["%.0f" % self.startingpoint.depth, \
                                                 "%.0f" % (self.todepth / 2000), \
                                                 "%.0f" % (self.todepth / 1000)])
    
        plt.title(title)
    
        cax = plt.axes([0.1, 0.1, 0.8, 0.02])
        cbar = plt.colorbar(img, cax=cax, orientation='horizontal',ticks=TICKS)
        if property != "poisson":
            cbar.set_label(property.title() + " (km/s)")
        else:
            cbar.set_label("Vp/Vs")
            
        plt.show()
        