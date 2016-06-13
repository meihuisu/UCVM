##
#   Provides the common routines and classes for UCVM.
#   
#   @author     David Gill
#   @created    04/26/2016
#   @revised    04/26/2016

##  Imports
import pyproj

##  Constants
DEFAULT_PROJECTION = "+proj=latlong +ellps=WGS84 +datum=WGS84"

##
#   @class Point
#   @brief Defines a point, a projection, and some additional metadata.
#
#   Allows for a point to be defined within the 3D earth structure.
#   It has an x and y coordinate, as well as a depth. A projection also
#   needs to be specified so that conversions can be made as necessary. Finally,
#   metadata can be provided to uniquely identify the point (metadata is a key-value array).
class Point:
    
    ##
    #   Initializes a new point. Checks that the parameters are all valid and
    #   raises an error if they are not.
    # 
    #   @param x The x coordinate provided as a float.
    #   @param y The y coordinate provided as a float.
    #   @param depth The depth in meters with the surface being 0.
    #   @param proj The coordinate's projection information in Proj.4 format.
    #   @param metadata An array of metadata in key-value format.
    def __init__(self, x, y, depth = 0, proj = DEFAULT_PROJECTION, metadata = ()):
        if ucvm_is_num(x):
            ## X coordinate as a float.
            self.x = x
        else:
            raise TypeError("X coordinate must be a number")
        
        if ucvm_is_num(y):
            ## Y coordinate as a float.
            self.y = y
        else:
            raise TypeError("Y coordinate must be a number")
        
        if ucvm_is_num(depth):
            if depth >= 0:
                ## Depth in meters below the surface. Must be greater than or equal to 0.
                self.depth = depth
            else:
                raise ValueError("Depth must be positive.")
        else:
            raise TypeError("Depth must be a number.")
        
        ## Projection information string.
        self.projection = proj
        
        ## Metadata array in key-value format.
        self.metadata = metadata
    
    ##
    #   String representation of the point.    
    def __str__(self):
        return "(%.4f, %.4f, %.4f)" % (float(self.x), float(self.y), float(self.depth))
    
    ##
    #   Returns a Point in a different projection.
    def to_projection(self, newproj = DEFAULT_PROJECTION):
        p_from = pyproj.Proj(newproj)
        p_to = pyproj.Proj(self.projection)
        new_x, new_y = pyproj.transform(p_from, p_to, self.x, self.y, self.depth)
        return Point(new_x, new_y, self.depth, newproj, self.metadata)

#   @class MaterialProperty
#   @brief Defines available material properties (Vp, Vs, density, Qp, and Qs)
#
#   Allows for material properties to be defined within the 3D earth structure.
#   Also, we include the original point for easy reference.
class MaterialProperty:
    
    def __init__(self, vp, vs, density, qp = None, qs = None, point = None):
        if ucvm_is_num(vp):
            self.vp = vp
        else:
            raise TypeError("Vp material property must be a number.")

        if ucvm_is_num(vs):
            self.vs = vs
        else:
            raise TypeError("Vs material property must be a number.")

        if ucvm_is_num(density):
            self.density = density
        else:
            raise TypeError("Density material property must be a number.")

        if ucvm_is_num(qp):
            self.qp = qp
        else:
            raise TypeError("Qp material property must be a number.")
        
        if ucvm_is_num(qs):
            self.qs = qs
        else:
            raise TypeError("Qs material property must be a number.")

        self.point = point

##
#   Returns true if value is a number. False otherwise.
#
#   @param value The value to test if numeric or not.
#   @return True if it is a number, false if not.
def ucvm_is_num(value):
    try:
        float(value)
        return True
    except Exception:
        return False
    
##
#   Raises an error and displays it to the user. Also exits gracefully.
#
#   @param id The id of the message to display to the user.
#   @param code Return code upon exit. Optional.
def ucvm_raise_error(id, code = -1):
    print("Error: " + id.upper() + " - " + get_string(id) + 
          "\n\nPlease contact software@scec.org for more assistance.")
    sys.exit(code)
    