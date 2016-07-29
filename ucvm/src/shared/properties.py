"""
Defines the various property classes (SeismicData, Point, etc.) that UCVM uses to represent
material properties.

:copyright: Southern California Earthquake Center
:author:    David Gill <davidgil@usc.edu>
:created:   July 6, 2016
:modified:  July 12, 2016
"""

from collections import namedtuple

try:
    import mpl_toolkits.basemap.pyproj as pyproj
except ImportError as the_err:
    print("UCVM requires PyProj to be installed. Please install PyProj and then re-run \
           this script.")
    pyproj = None  # Needed to remove the warning in PyCharm
    raise

from .constants import UCVM_DEFAULT_PROJECTION
from .functions import is_number

VelocityProperties = namedtuple("VelocityProperties", "vp vs density qp qs " +
                                "vp_source vs_source density_source qp_source qs_source")
#: namedtuple VelocityProperties: Defines all the possible returnable material properties
#                                 and sources.

ElevationProperties = namedtuple("ElevationProperties", "elevation elevation_source")
#: namedtuple ElevationProperties: Defines the returnable properties for an elevation model.

Vs30Properties = namedtuple("Vs30Properties", "vs30 vs30_source")
#: namedtuple Vs30Properties: Defines the Vs30 properties (just Vs30 value and source).

SimplePoint = namedtuple("SimplePoint", "x y z")
#: namedtuple SimplePoint: Defines a very simple point structure (x, y, and z).

SimpleRotatedRectangle = namedtuple("SimpleRotatedRectangle", "x y rotation x_spacing y_spacing")
#: namedtuple SimpleRotatedRectangle: Defines a rotated rectangle (x, y, rotation, spacing).


class Point:
    """
    Defines a point. The default projection is in WGS84 latitude and longitude.

    Represents a point on the physical 3D earth structure. By default this is just WGS84
    latitude and longitude. A point can have either a depth or elevation and also be
    provided some metadata.

    :param float x: Longitude/x co-ordinate.
    :param float y: Latitude/y co-ordinate.
    :param float z: The depth or elevation of the point.
    :param int depth_elev: A flag of either Point.DEPTH or Point.ELEVATION.
    :param dictionary metadata: A dictionary of metadata.
    :param str projection: A Proj.4 projection string indicating the point's projection.
    :Example: To declare point (-118, 34) at depth 0 using WGS 84 longitude, latitude projection, \
              do: ``Point(-118, 34, 0)``. If you wanted to declare that same point in UTM, you \
              would need to do the Proj.4 string which would be \
              ``Point(407650.4, 3762606.7, 0, projection="+proj=utm +datum=WGS84 +zone=11")``.
    """

    DEPTH = 0  #: int: Depth constant.

    ELEVATION = 1  #: int: Elevation constant.

    def __init__(self, x, y, z, depth_elev=DEPTH, metadata=None,
                 projection=None):
        if is_number(x):
            self.x_value = float(x)  #: float: X co-ordinate (set in constructor).
        else:
            raise TypeError("X co-ordinate must be a number.")

        if is_number(y):
            self.y_value = float(y)  #: float: Y co-ordinate (set in constructor).
        else:
            raise TypeError("Y co-ordinate must be a number.")

        if is_number(z):
            self.z_value = float(z)  #: float: Z co-ordinate (set in constructor). Depth/elevation.
        else:
            raise TypeError("Z co-ordinate must be a number.")

        if self.z_value < 0 and depth_elev == self.DEPTH:
            raise ValueError("Depth must be a positive number (i.e. z = 100 means 100m below the \
                              surface).")

        if depth_elev is not self.DEPTH and depth_elev is not self.ELEVATION:
            raise ValueError("The point must represent either elevation relative to sea level \
                              (Point.ELEVATION) or depth relative to the surface (Point.DEPTH)")

        if projection is None:
            self.projection = UCVM_DEFAULT_PROJECTION  #: str: Proj.4 Point projection.
        else:
            self.projection = projection

        self.depth_elev = depth_elev  #: int: Depth / elevation (DEPTH = 0, ELEVATION = 1)
        self.metadata = metadata  #: dictionary: A key-value array of metadata.

    def __str__(self, *args, **kwargs):
        """
        Returns a point in ucvm_query compatible form. This is long, lat, depth in WGS84 format.
        """
        point_to_print = self
        if self.projection is not UCVM_DEFAULT_PROJECTION:
            point_to_print = self.convert_to_projection(UCVM_DEFAULT_PROJECTION)

        return "%.4f %.4f %.4f" % (float(point_to_print.x_value), float(point_to_print.y_value),
                                   float(point_to_print.z_value))

    def convert_to_projection(self, projection: str):
        """
        Returns an equivalent point in a different projection.

        :param str projection: The new projection string in Proj.4.
        :return: A Point in the new projection.
        """
        if projection == self.projection:
            return self

        in_proj = pyproj.Proj(self.projection)
        out_proj = pyproj.Proj(projection)

        x_new, y_new = pyproj.transform(in_proj, out_proj, self.x_value, self.y_value)

        point = Point(x_new, y_new, self.z_value, self.depth_elev, self.metadata, projection)
        return point

    def get_depth_or_elevation(self):
        """
        Returns if the Point is defined by depth or elevation.

        :return: Point.DEPTH if defined by depth, Point.ELEVATION if by elevation.
        """
        return self.depth_elev


class SeismicData:
    """
    Represents the seismic data retrieved from a specific model of the Earth.

    Holds material properties and other data (such as elevation and Vs30) for a specific
    point on the Earth. A SeismicData object also contains the model from which the data was
    retrieved for easy reference.

    :param Point point: The point to which these properties belong.
    :param dictionary extras: Possible extra parameters (e.g. Vs30, elevation, etc.).
    :Example: To return material properties from model "foo" at (-118, 34, 0) of (Vp: 1000, \
              Vs: 1000, Density: 1000), the code would be \
              ``SeismicData(MaterialProperties(1000, 1000, 1000), None, Point(-118, 34, 0), \
              "foo")``.
    """

    def __init__(self, point: Point, extras: dict=None):
        if point is not None and isinstance(point, Point):
            self.original_point = point           #: Point: The point for the material properties.
            self.converted_point = Point          #: Point: Converted point to be used for queries.
        elif point is not None:
            raise TypeError("The point must be an instance of the Point class")

        self.elevation_properties = None          #: ElevationProperties: Elevation property data.
        self.velocity_properties = None           #: VelocityProperties: Material property data.
        self.vs30_properties = None               #: Vs30Properties: Vs30 property data.
        self.model_dictionary = None              #: dict: The model(s) from which the data came.

        if extras is not None:
            self.extras = extras        #: dictionary: Extra parameters (Vs30, elevation, etc.).
        else:
            self.extras = {}

    def set_elevation_data(self, elevation_properties: ElevationProperties) -> bool:
        """
        Sets the elevation data for this particular seismic data instance.
        :param namedtuple elevation_properties: The ElevationProperties namedtuple to set.
        :return: True when successful, error raised if not.
        """
        if not isinstance(elevation_properties, ElevationProperties):
            raise TypeError("Elevation properties must be a namedtuple of type "
                            "ElevationProperties.")

        self.elevation_properties = elevation_properties
        return True

    def set_velocity_data(self, velocity_properties: VelocityProperties) -> bool:
        """
        Sets the velocity data for this particular seismic data instance.
        :param namedtuple velocity_properties: The VelocityProperties namedtuple to set.
        :return: True when successful, error raised if not.
        """
        if not isinstance(velocity_properties, VelocityProperties):
            raise TypeError("Velocity properties must be a namedtuple of type "
                            "VelocityProperties.")

        self.velocity_properties = velocity_properties
        return True

    def set_vs30_data(self, vs30_properties: Vs30Properties) -> bool:
        """
        Sets the Vs30 data for this particular seismic data instance.
        :param namedtuple vs30_properties: The Vs30Properties namedtuple to set.
        :return: True when successful, error raised if not.
        """
        if not isinstance(vs30_properties, Vs30Properties):
            raise TypeError("Vs30 properties must be a namedtuple of type "
                            "Vs30Properties.")

        self.vs30_properties = vs30_properties
        return True

    def set_model_dictionary(self, model_dictionary: dict) -> bool:
        """

        :param model_dictionary:
        :return:
        """
        self.model_dictionary = model_dictionary
        return True

    @classmethod
    def from_old_ucvm(cls, output_string, point=None, model=None):
        """
        Generates a new instance of SeismicData, from the old UCVM output.
        :param str output_string: The string returned by UCVM.
        :param Point point: The point to which this SeismicData belongs.
        :param str model: The model identifier as a string.
        :return: A new instance of SeismicData.
        """
        line_array = output_string.split()
        return cls(line_array[14], line_array[15], line_array[16], point, model)

    def __str__(self, *args, **kwargs):
        """
        Prints out the seismic data in the form of long, lat, depth/elevation (100 d for depth,
        -100 e for elevation), Vp, Vs, density.
        """
        return "(%.4f, %.4f, %.4f)" % (float(self.velocity_properties.vp),
                                       float(self.velocity_properties.vs),
                                       float(self.velocity_properties.density))

    def get_property(self, prop) -> float:
        """
        Given a property (vp, vs, density, etc.) returns the value for that property.
        :param str prop: The property to retrieve.
        :return: The value for the property.
        """
        if prop == "vp":
            return self.velocity_properties.vp
        elif prop == "vs":
            return self.velocity_properties.vs
        elif prop == "dn":
            return self.velocity_properties.density

    def convert_point_to_projection(self, projection: str) -> None:
        """
        Given a projection, sets converted point property to be the new projection.
        :param projection: The projection as a string.
        :return: Nothing.
        """
        self.converted_point = self.original_point.convert_to_projection(projection)
