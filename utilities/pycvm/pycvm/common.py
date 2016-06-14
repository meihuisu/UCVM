##
#  @file common.py
#  @brief Common definitions and functions for all the UCVM plotting scripts.
#  @author David Gill - SCEC <davidgil@usc.edu>
#  @version 14.7.0
#
#  Provides common definitions and functions that are shared amongst all UCVM
#  plotting scripts. Facilitates easier multi-processing as well.

#  Imports
from subprocess import call, Popen, PIPE, STDOUT
import sys
import os
import multiprocessing
import math
import struct
import getopt

#  Numpy is required.
try:
    import numpy as np
except StandardError, e:
    print "ERROR: NumPy and Matplotlib must be installed on your system in order to generate these plots."
    exit(1)
    
#  Matplotlib is required.
try:
    import matplotlib.pyplot as plt
    import matplotlib.colors as mcolors
    import matplotlib.cm as cm
except StandardError, e:
    print "ERROR: NumPy and Matplotlib must be installed on your system in order to generate these plots."
    exit(1)    

#  Basemap is required.
try:
    from mpl_toolkits import basemap
except StandardError, e:
    print "ERROR: Matplotlib Toolkit must be installed on your system in order to generate these plots."
    exit(1)

#  Constants

## Known CVMs that can be installed with UCVM.
UCVM_CVMS = {"1d":"1D", \
             "1dgtl":"1D w/ Vs30 GTL", \
             "bbp1d":"Broadband Whittier Narrows 1D Model", \
             "cvms":"CVM-S4", \
             "cvmh":"CVM-H 11.9.1", \
             "cencal":"USGS Bay Area Model", \
             "cvmsi":"CVM-S4.26",
             "cvms5":"CVM-S5"}

## Constant for all material properties.
ALL_PROPERTIES = ["vp", "vs", "density"]
## Constant for just Vs.
VS = ["vs"]
## Constant for just Vp.
VP = ["vp"]
## Constant for just density.
DENSITY = ["density"]

## Version string.
VERSION = "14.7.0"

#  Class Definitions

##
#  @class Plot
#  @brief Returns a basic plot given a set of parameters.
#
#  This forms the basis for generating a plot using this suite of tools.
#  It returns a Matplotlib plot with certain parameters already set up.
class Plot:
    
    ##
    #  Initializes the Plot with a set of basic parameters. Use the
    #  addsubplot() method to add a sub-plot to the plot.
    #
    #  @param title The title for the plot.
    #  @param xlabel The label to be displayed on the x-axis.
    #  @param ylabel The label for the y-axis.
    #  @param legend A legend to be displayed on the lower left of the plot.
    #  @param width The width of the plot in inches (dpi = 100).
    #  @param height The height of the plot in inches (dpi = 100).
    def __init__(self, title = None, xlabel = None, ylabel = None, legend = None, width = 10, height = 10):
        ## Defines the figure object to which we can add subplots.
        self.figure = plt.figure(figsize=(width, height), dpi=100)
        
        if ylabel != None:
            plt.ylabel(ylabel, fontsize=14)
        
        if xlabel != None:
            plt.xlabel(xlabel, fontsize=14)  
        
        if title != None:
            plt.title(title)
        
        if legend != None:
            plt.legend(legend, loc='lower left')
            
        ## Internal counter for how many subplots we have.
        self.subplotcounter = 1
            
    ##
    #  Adds a subplot to the figure and returns it.
    # 
    #  @return The subplot that has been added to the already generated plot.
    def addsubplot(self):
        retval = self.figure.add_subplot(1, 1, 1)
        self.subplotcounter += 1;
        return retval
    
    ## 
    #  Shows the plot.
    def show(self):
        plt.show()
        
    ##
    #  Saves the figure to disk.
    #
    #  @param filename The name fo the file to save.
    def savefig(self, filename):
        plt.savefig(filename)

##
#  @class Point
#  @brief Defines a point in WGS84 latitude, longitude projection.
#
#  Allows for a point to be defined within the 3D earth structure.
#  It has a longitude, latitude, and depth as minimum parameters, but you can
#  specify a type, e.g. "LA Basin", and a description, e.g. "New point of 
#  interest".
class Point:
    
    ##
    #  Initializes a new point. Checks that the parameters are all valid and
    #  raises an error if they are not.
    # 
    #  @param longitude Longitude provided as a float.
    #  @param latitude Latitude provided as a float.
    #  @param depth The depth in meters with the surface being 0.
    #  @param type The type or classification of this location. Not required.
    #  @param code A short code for the site (unique identifier). Not required.
    #  @param description A longer description of what this point represents. Not required.
    def __init__(self, longitude, latitude, depth = 0, type = None, code = None, description = None):
        if pycvm_is_num(longitude):
            ## Longitude as a float in WGS84 projection.
            self.longitude = longitude
        else:
            raise TypeError("Longitude must be a number")
        
        if pycvm_is_num(latitude):
            ## Latitude as a float in WGS84 projection.
            self.latitude = latitude
        else:
            raise TypeError("Latitude must be a number")
        
        if pycvm_is_num(depth):
            if depth >= 0:
                ## Depth in meters below the surface. Must be greater than or equal to 0.
                self.depth = depth
            else:
                raise ValueError("Depth must be positive.")
        else:
            raise TypeError("Depth must be a number.")
        
        ## A classification or short description of what this point represents. Optional.
        self.type = type
        ## A short, unique code identifying the site. Optional.
        self.code = code
        ## A longer description of what this point represents. Optional.
        self.description = description
    
    ##
    #  String representation of the point.    
    def __str__(self):
        return "(%.4f, %.4f, %.4f)" % (float(self.longitude), float(self.latitude), float(self.depth))

##
#  @class MaterialProperties
#  @brief Defines the possible material properties that @link UCVM UCVM @endlink can return.
#
#  Provides a class for defining the three current material properties that
#  UCVM returns and also has placeholders for Qp and Qs.
class MaterialProperties:
    
    ## 
    #  Initializes the MaterialProperties class.
    #  
    #  @param vp P-wave velocity in m/s. Must be a float.
    #  @param vs S-wave velocity in m/s. Must be a float.
    #  @param density Density in g/cm^3. Must be a float.
    #  @param qp Qp as a float. Optional.
    #  @param qs Qs as a float. Optional.
    def __init__(self, vp, vs, density, qp = None, qs = None):
       if pycvm_is_num(vp):
           ## P-wave velocity in m/s
           self.vp = float(vp)
       else:
           raise TypeError("Vp must be a number.")
       
       if pycvm_is_num(vs):
           ## S-wave velocity in m/s
           self.vs = float(vs)
       else:
           raise TypeError("Vs must be a number.")
       
       if pycvm_is_num(density):
           ## Density in g/cm^3
           self.density = float(density)
       else:
           raise TypeError("Density must be a number.")
       
       if qp != None:
           ## Qp
           self.qp = float(qp)
       else:
           self.qp = -1
           
       if qs != None:
           ## Qs
           self.qs = float(qs)
       else:
           self.qs = -1
       
    ##
    #  Defines subtraction of two MaterialProperties classes.
    #
    #  @param own This MaterialProperties class.
    #  @param other The other MaterialProperties class.
    #  @return The subtracted properties.
    def __sub__(own, other):
        return MaterialProperties(own.vp - other.vp, own.vs - other.vs, own.density - other.density, \
                                  own.qp - other.qp, own.qs - other.qs)

    ##
    #  Initializes the class from a UCVM output string line.
    #
    #  @param cls Not used. Call as MaterialProperties.fromUCVMOutput(line).
    #  @param line The line containing the material properties as generated by ucvm_query.
    #  @return A constructed MaterialProperties class.
    @classmethod
    def fromUCVMOutput(cls, line):
        new_line = line.split()
        return cls(new_line[14], new_line[15], new_line[16])
    
    ##
    #  Retrieves the corresponding property given the property as a string.
    # 
    #  @param property The property name as a string ("vs", "vp", "density", "qp", or "qs").
    #  @return The property value.
    def getProperty(self, property):               
        if property.lower() == "vs":
            return self.vs
        elif property.lower() == "vp":
            return self.vp
        elif property.lower() == "density":
            return self.density
        elif property.lower() == "qp":
            return self.qp
        elif property.lower() == "qs":
            return self.qs
        else:
            raise ValueError("Parameter property must be a valid material property unit.")
        
    ##
    #  String representation of the material properties.
    def __str__(self):
        return "Vp: %.2fm/s, Vs: %.2fm/s, Density: %.2fg/cm^3" % (self.vp, self.vs, self.density)
 
##
#  @class UCVM
#  @brief Python functions to interact with the underlying C code.
#
#  Provides a Python mechanism for calling the underlying C programs and
#  getting their output in a format that is readily and easily interpreted
#  by other classes.
class UCVM:
    
    ##
    #  Initializes the UCVM class and reads in all the available models that have
    #  been installed.
    #  
    #  @param install_dir The base installation directory of UCVM.
    #  @param config_file The location of the UCVM configuration file.
    def __init__(self, install_dir = None, config_file = None):
        if install_dir != None:
            ## Location of the UCVM binary directory.
            self.binary_dir = install_dir + "/bin"
        else:
            self.binary_dir = "../bin"
            
        
        if config_file != None:
            ## Location of the UCVM configuration file.
            self.config = config_file
        else:
            self.config = "../conf/ucvm.conf"
        
        if install_dir != None:
            ## List of all the installed CVMs.
            self.models = [x for x in os.listdir(install_dir + "/model")]
        else:
            self.models = [x for x in os.listdir("../model")]
            
        self.models.remove("ucvm")
    ##
    #  Queries UCVM given a set of points and a CVM to query. If the CVM does not exist,
    #  this function will throw an error. The set of points must be an array of the 
    #  @link Point Point @endlink class. This function returns an array of @link MaterialProperties
    #  MaterialProperties @endlink.
    #
    #  @param point_list An array of @link Point Points @endlink for which UCVM should query.
    #  @param cvm The CVM from which this data should be retrieved.
    #  @return An array of @link MaterialProperties @endlink.
    def query(self, point_list, cvm):
        
        shared_object = "../model/" + cvm + "/lib/lib" + cvm + ".so"
        properties = []
        
        # Can we load this library dynamically and bypass the C code entirely?
        if os.path.isfile(shared_object):
            import ctypes
            #obj = ctypes.cdll.LoadLibrary(shared_object)
            #print obj
        
        proc = Popen([self.binary_dir + "/ucvm_query", "-f", self.config, "-m", cvm], stdout=PIPE, stdin=PIPE, stderr=STDOUT)
        
        text_points = ""
        
        if isinstance(point_list, Point):
            point_list = [point_list]
         
        for point in point_list:
            text_points += "%.5f %.5f %.5f\n" % (point.longitude, point.latitude, point.depth)
        
        output = proc.communicate(input=text_points)[0]
        output = output.split("\n")[1:-1]
        
        for line in output:
            properties.append(MaterialProperties.fromUCVMOutput(line))
        
        if len(properties) == 1:
            return properties[0]
        
        return properties
    
    ##
    #  Gets the Vs30 values for a given set of points and a CVM to query. If
    #  the CVM does not exist, this function will throw an error. The set of
    #  points is an array of @link Point Points @endlink. The function returns
    #  the Vs30 values as floats.
    #
    #  @param point_list An array of @link Point Points @endlink to query.
    #  @param cvm The CVM from which the Vs30 data should be retrieved.
    #  @return An array of floats which correspond to the points provided.
    def vs30(self, point_list, cvm):
        
        proc = Popen([self.binary_dir + "/vs30_query", "-f", self.config, "-m", cvm], stdout=PIPE, stdin=PIPE, stderr=STDOUT)
        
        text_points = ""
        floats = []
        
        if isinstance(point_list, Point):
            point_list = [point_list]
        
        for point in point_list:
            text_points += "%.5f %.5f\n" % (point.longitude, point.latitude)
            
        output = proc.communicate(input=text_points)[0]
        output = output.split("\n")[:-1]
        
        for line in output:
            floats.append(float(line.split()[2]))
        
        if len(floats) == 1:
            return floats[0]
            
        return floats
    
    ##
    #  Gets the basin depths for a given set of points, CVM, and desired Vs.
    #  If the CVM does not exist, an error is given. The set of points is an
    #  array of @link Point Points @endlink. The function returns the depths
    #  as floats.
    # 
    #  @param point_list An array of @link Point Points @endlink to query.
    #  @param cvm The CVM from which the depths should come.
    #  @param vs_threshold The Vs threshold to check for (e.g. Z1.0 = 1000).
    #  @return An array of floats which correspond to the depths.
    def basin_depth(self, point_list, cvm, vs_threshold):
        
        proc = Popen([self.binary_dir + "/basin_query", "-f", self.config, "-m", \
                      cvm, "-v", "%.0f" % vs_threshold], stdout=PIPE, stdin=PIPE, stderr=STDOUT)
        
        text_points = ""
        floats = []
        
        if isinstance(point_list, Point):
            point_list = [point_list]
        
        for point in point_list:
            text_points += "%.5f %.5f\n" % (point.longitude, point.latitude)
            
        output = proc.communicate(input=text_points)[0]
        output = output.split("\n")[:-1]
        
        for line in output:
            floats.append(float(line.split()[2]))
        
        if len(floats) == 1:
            return floats[0]
        
        return floats

#  Function Definitions

##
#  Displays an error message and exits the program.
#  @param message The error message to be displayed.
def pycvm_display_error(message):
    print "An error has occurred while executing this script. The error was:\n"
    print message
    print "\nPlease contact software@scec.org and describe both the error and a bit"
    print "about the computer you are running CVM-S5 on (Linux, Mac, etc.)."
    exit(0)
    
##
#  Returns true if value is a number. False otherwise.
#
#  @param value The value to test if numeric or not.
#  @return True if it is a number, false if not.
def pycvm_is_num(value):
    try:
        float(value)
        return True
    except Exception:
        return False
    
##
#  Returns the discrete colormap.
#
#  @param cmap The colormap to use.
#  @param N The number of discretized intervals.
def pycvm_cmapDiscretize(cmap, N):
    cdict = cmap._segmentdata.copy()
    # N colors
    colors_i = np.linspace(0,1.,N)
    # N+1 indices
    indices = np.linspace(0,1.,N+1)
    for key in ('red','green','blue'):
        # Find the N colors
        D = np.array(cdict[key])
        colors = np.interp(colors_i, D[:,0], D[:,1])
        #I = sp.interpolate.interp1d(D[:,0], D[:,1])
        #colors = I(colors_i)
        # Place these colors at the correct indices.
        A = np.zeros((N+1,3), float)
        A[:,0] = indices
        A[1:,1] = colors
        A[:-1,2] = colors
        # Create a tuple for the dictionary.
        L = []
        for l in A:
            L.append(tuple(l))
        cdict[key] = tuple(L)
    # Return colormap object.
    return mcolors.LinearSegmentedColormap('colormap',cdict,1024)