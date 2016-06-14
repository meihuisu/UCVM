##
#  @file difference.py
#  @brief Gets the difference of two plots and save/plots the difference.
#  @author David Gill - SCEC <davidgil@usc.edu>
#  @version 14.7.0
#
#  Computes the difference between two horizontal slices or cross-sections
#  and can either save or plot the difference.

#  Imports
from common import MaterialProperties, Plot, cm, np, basemap, plt

##
#  @class Difference
#  @brief Computes the difference between two @link horizontal_slice.HorizontalSlice horizontal slices @endlink
#         or two cross sections.
#
#  Computes the difference for either plotting or saving to a file.
class Difference:
    
    ##
    #  Initializes the difference computation class and actually checks that we can,
    #  indeed, compute the difference between the two plots. The difference will be the
    #  value of the first plot minus the second plot.
    #
    #  @param firstplot The first plot provided (this will have secondplot subtracted from it).
    #  @param secondplot The plot to subtract from firstplot.
    def __init__(self, firstplot, secondplot):
        
        ## The plot type
        self.plot_type = firstplot.__class__.__name__
        
        ## A copy of one of the plots to keep some specifications.
        self.plot_specs = firstplot
        
        ## A copy of the second plot for specs.
        self.plot_specs_2 = secondplot
        
        secondplottype = secondplot.__class__.__name__
        
        if self.plot_type != "HorizontalSlice" and self.plot_type != "CrossSection":
            raise TypeError("Differences can only be done between horizontal slices and cross-sections.")
        
        if self.plot_type != secondplottype:
            raise TypeError("The first and second plots must both be horizontal slices or cross-sections.")
                
        try:
            firstplot.materialproperties
        except:
            firstplot.getplotvals()
            
        try:
            secondplot.materialproperties
        except:
            secondplot.getplotvals()
            
        #  Check that the x and y lengths are the same.
        if firstplot.num_x != secondplot.num_x:
            raise TypeError("Number of X points is not the same in each plot.")
        
        if firstplot.num_y != secondplot.num_y:
            raise TypeError("Number of Y points is not the same in each plot.")   
        
        ##  Initialize the difference holder.
        self.difference_values = [[MaterialProperties(-1, -1, -1) for x in xrange(firstplot.num_x)] for x in xrange(firstplot.num_y)]
        
        #  Get the difference and save it.
        for y in xrange(0, firstplot.num_y):
            for x in xrange(0, firstplot.num_x):
                 self.difference_values[y][x] = firstplot.materialproperties[y][x] - secondplot.materialproperties[y][x]
        
    ##
    #  Plots the difference. Can save the plot to disk if the user specifies a file name.
    #
    #  @param property The property that should be plotted.
    #  @param filename The location to which the plot should be saved. Optional.
    #  @param title A more descriptive title for the plot. Optional.
    def plot(self, property, filename = None, title = None):
        
        if self.plot_type == "HorizontalSlice":

            if title == None:
                title = "Horizontal Slice Difference Plot"

            # Call the plot object.
            p = Plot(title, "", "", None, 10, 10)
            
            colormap = cm.RdBu
        
            m = basemap.Basemap(projection='cyl', llcrnrlat=self.plot_specs.bottomrightpoint.latitude, \
                                urcrnrlat=self.plot_specs.upperleftpoint.latitude, \
                                llcrnrlon=self.plot_specs.upperleftpoint.longitude, \
                                urcrnrlon=self.plot_specs.bottomrightpoint.longitude, \
                                resolution='f', anchor='C')
        
            lat_ticks = np.arange(self.plot_specs.bottomrightpoint.latitude, \
                                  self.plot_specs.upperleftpoint.latitude + 0.1, \
                                  self.plot_specs.plot_height / 2)
            lon_ticks = np.arange(self.plot_specs.upperleftpoint.longitude, \
                                  self.plot_specs.bottomrightpoint.longitude + 0.1, \
                                  self.plot_specs.plot_width / 2)
    
            m.drawparallels(lat_ticks, linewidth=1.0, labels=[1,0,0,0])
            m.drawmeridians(lon_ticks, linewidth=1.0, labels=[0,0,0,1])
            m.drawstates()
            m.drawcountries()
    
            lons = np.arange(self.plot_specs.upperleftpoint.longitude, \
                             self.plot_specs.bottomrightpoint.longitude, \
                             self.plot_specs.spacing)
            lats = np.arange(self.plot_specs.bottomrightpoint.latitude, \
                             self.plot_specs.upperleftpoint.latitude, \
                             self.plot_specs.spacing)
    
            # Get the properties.
            datapoints = np.arange(self.plot_specs.num_x * self.plot_specs.num_y, \
                                   dtype=float).reshape(self.plot_specs.num_y, self.plot_specs.num_x)
        
            for i in xrange(0, self.plot_specs.num_y):
                for j in xrange(0, self.plot_specs.num_x):
                    datapoints[i][j] = self.difference_values[i][j].getProperty(property) / 1000.0
                    
            t = m.transform_scalar(datapoints, lons, lats, len(lons), len(lats))
            img = m.imshow(t, cmap=colormap)
    
            m.drawcoastlines()
    
            cax = plt.axes([0.125, 0.05, 0.775, 0.02])
            cbar = plt.colorbar(img, cax=cax, orientation='horizontal')
            cbar.set_label(property.title() + " (km/s)")
        
        elif self.plot_type == "CrossSection":
            
            if title == None:
                title = "Horizontal Slice Difference Plot"
                
            # Call the plot object.
            p = Plot(None, None, None, None, 10, 10)

            colormap = cm.RdBu

            plt.axes([0.1,0.7,0.8,0.25])

            # Figure out which is upper-right and bottom-left.
            ur_lat = self.plot_specs.startingpoint.latitude if self.plot_specs.startingpoint.latitude > self.plot_specs.endingpoint.latitude else self.plot_specs.endingpoint.latitude
            ur_lon = self.plot_specs.startingpoint.longitude if self.plot_specs.startingpoint.longitude > self.plot_specs.endingpoint.longitude else self.plot_specs.endingpoint.longitude
            ll_lat = self.plot_specs.startingpoint.latitude if self.plot_specs.startingpoint.latitude < self.plot_specs.endingpoint.latitude else self.plot_specs.endingpoint.latitude
            ll_lon = self.plot_specs.startingpoint.longitude if self.plot_specs.startingpoint.longitude < self.plot_specs.endingpoint.longitude else self.plot_specs.endingpoint.longitude
    
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
    
            m.plot([self.plot_specs.startingpoint.longitude,self.plot_specs.endingpoint.longitude], [self.plot_specs.startingpoint.latitude,self.plot_specs.endingpoint.latitude])
            m.plot([self.plot_specs_2.startingpoint.longitude,self.plot_specs_2.endingpoint.longitude], [self.plot_specs_2.startingpoint.latitude,self.plot_specs_2.endingpoint.latitude])
    
            valign1 = "top"
            valign2 = "bottom"
            if self.plot_specs.endingpoint.latitude < self.plot_specs.startingpoint.latitude:
                valign1 = "bottom"
                valign2 = "top"
    
            plt.text(self.plot_specs.startingpoint.longitude, self.plot_specs.startingpoint.latitude, \
                     '[S] %.1f, %.1f' % (self.plot_specs.startingpoint.longitude, self.plot_specs.startingpoint.latitude), \
                     color='k', horizontalalignment="center", verticalalignment=valign1)
    
            plt.text(self.plot_specs.endingpoint.longitude, self.plot_specs.endingpoint.latitude, \
                     '[E] %.1f, %.1f' % (self.plot_specs.endingpoint.longitude, self.plot_specs.endingpoint.latitude), \
                     color='k', horizontalalignment="center", verticalalignment=valign2)
    
            plt.axes([0.05,0.18,0.9,0.54])
    
            datapoints = np.arange(self.plot_specs.num_x * self.plot_specs.num_y,dtype=float).reshape(self.plot_specs.num_y, self.plot_specs.num_x)
            
            for i in xrange(0, self.plot_specs.num_y):
                for j in xrange(0, self.plot_specs.num_x):   
                    datapoints[i][j] = self.difference_values[i][j].getProperty(property) / 1000          
    
            img = plt.imshow(datapoints, cmap=colormap)
            
            if self.plot_specs.startingpoint.longitude != self.plot_specs.endingpoint.longitude:
                plt.xticks([0,self.plot_specs.num_x/2,self.plot_specs.num_x], ["[S] %.2f" % self.plot_specs.startingpoint.longitude, \
                            "%.2f" % ((float(self.plot_specs.endingpoint.longitude) + float(self.plot_specs.startingpoint.longitude)) / 2), \
                            "[E] %.2f" % self.plot_specs.endingpoint.longitude])
            else:
                plt.xticks([0,self.plot_specs.num_x/2,self.plot_specs.num_x], ["[S] %.2f" % self.plot_specs.startingpoint.latitude, \
                            "%.2f" % ((float(self.plot_specs.endingpoint.latitude) + float(self.plot_specs.startingpoint.latitude)) / 2), \
                            "[E] %.2f" % self.plot_specs.endingpoint.latitude])                
                
            plt.yticks([0,self.plot_specs.num_y/2,self.plot_specs.num_y], ["%.0f" % self.plot_specs.startingpoint.depth, \
                        "%.0f" % (self.plot_specs.todepth / 2000), \
                        "%.0f" % (self.plot_specs.todepth / 1000)])
    
            plt.title(title)
    
            cax = plt.axes([0.1, 0.1, 0.8, 0.02])
            cbar = plt.colorbar(img, cax=cax, orientation='horizontal')
            cbar.set_label(property.title() + " (km/s)")  
        
        if filename:
            plt.savefig(filename)
        else:
            plt.show()