"""
Defines the various property classes (SeismicData, Point, etc.) that UCVM uses to represent
material properties.

Copyright 2017 Southern California Earthquake Center

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
"""
from collections import namedtuple

try:
    import pyproj
except ImportError as the_err:
    print("UCVM requires PyProj to be installed. Please install PyProj and then re-run \
           this script.")
    pyproj = None  # Needed to remove the warning in PyCharm
    raise

from .constants import UCVM_DEFAULT_PROJECTION
from ucvm.src.shared import UCVM_DEPTH, UCVM_ELEVATION, UCVM_ELEV_ANY

VelocityProperties = namedtuple("VelocityProperties", "vp vs density qp qs " +
                                "vp_source vs_source density_source qp_source qs_source")
#: namedtuple VelocityProperties: Defines all the possible returnable material properties
#                                 and sources.

ElevationProperties = namedtuple("ElevationProperties", "elevation elevation_source")
#: namedtuple ElevationProperties: Defines the returnable properties for an elevation model.

Vs30Properties = namedtuple("Vs30Properties", "vs30 vs30_source")
#: namedtuple Vs30Properties: Defines the Vs30 properties (just Vs30 value and source).

ZProperties = namedtuple("ZProperties", "z10 z25")
#: namedtuple ZProperties: Defines the Z properties (either 1.0 or 2.5).

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

    Parameters:
        x (float): Longitude/x co-ordinate.
        y (float): Latitude/y co-ordinate.
        z (float): The depth or elevation of the point.
        depth_elev (int): A flag of either UCVM_DEPTH or UCVM_ELEVATION.
        metadata (dict): A dictionary of metadata.
        projection (str): A Proj.4 projection string indicating the point's projection.

    Example:
        To declare point (-118, 34) at depth 0 using WGS 84 longitude, latitude projection, do:

        Point(-118, 34, 0)

        If you wanted to declare that same point in UTM, you would need to specify the Proj.4 string which would be:

        Point(407650.4, 3762606.7, 0, projection="+proj=utm +datum=WGS84 +zone=11").
    """

    loaded_projections = {}

    def __init__(self, x: float, y: float, z: float, depth_elev: int=UCVM_DEPTH,
                 metadata: dict=None, projection: str=None):
        try:
            self.x_value = float(x)  #: float: X co-ordinate (set in constructor).
        except ValueError:
            raise TypeError("X co-ordinate must be a number.")

        try:
            self.y_value = float(y)  #: float: Y co-ordinate (set in constructor).
        except ValueError:
            raise TypeError("Y co-ordinate must be a number.")

        try:
            self.z_value = float(z)  #: float: Z co-ordinate (set in constructor). Depth/elevation.
        except ValueError:
            raise TypeError("Z co-ordinate must be a number.")

        if self.z_value < 0 and int(depth_elev) == UCVM_DEPTH:
            raise ValueError("Depth must be a positive number (i.e. z = 100 means 100m below the "
                             "surface).")

        if int(depth_elev) != UCVM_DEPTH and int(depth_elev) != UCVM_ELEVATION:
            raise ValueError("The point must represent either elevation relative to sea level "
                             "(UCVM_ELEVATION) or depth relative to the surface (UCVM_DEPTH)")

        if projection is None:
            self.projection = UCVM_DEFAULT_PROJECTION  #: str: Proj.4 Point projection.
        else:
            self.projection = projection

        self.depth_elev = depth_elev  #: int: Depth / elevation (UCVM_DEPTH = 0, UCVM_ELEVATION = 1)
        self.metadata = metadata  #: dictionary: A key-value array of metadata.

    def convert_to_projection(self, projection: str):
        """
        Returns an equivalent point in a different projection.

        Parameters:
            projection (str): The new projection string in Proj.4.

        Returns:
            Point: A Point in the new projection.
        """
        if projection == self.projection:
            return self

        if self.projection in Point.loaded_projections:
            in_proj = Point.loaded_projections[self.projection]
        else:
            in_proj = pyproj.Proj(self.projection)
            Point.loaded_projections[self.projection] = in_proj

        if projection in Point.loaded_projections:
            out_proj = Point.loaded_projections[projection]
        else:
            out_proj = pyproj.Proj(projection)
            Point.loaded_projections[projection] = out_proj

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

    Parameters:

        point (Point): The point to which these properties belong.
        extras (dict): Possible extra parameters (i.e. metadata you wished to attach to this object).

    Example:
        To fill a SeismicData object with the material properties from model "foo" at (-118, 34, 0) of
        (Vp: 1000, Vs: 1000, Density: 1000), the code would be:

        s = SeismicData(Point(-118, 34, 0), "foo") |br|
        s.set_velocity_data(VelocityProperties(1000, 1000, 1000, "foo", "foo", "foo"))
    """

    def __init__(self, point: Point=None, extras: dict=None):
        if point is not None:
            self.original_point = point         #: Point: The original point given.
            self.converted_point = None         #: Point: If a different projection is required, this holds that point.
        elif point is not None:
            raise TypeError("The point must be an instance of the Point class")
        else:
            self.original_point = Point(-118, 34, 0)
            self.converted_point = None

        self.elevation_properties = None          #: ElevationProperties: Elevation property data.
        self.velocity_properties = None           #: VelocityProperties: Material property data.
        self.vs30_properties = None               #: Vs30Properties: Vs30 property data.
        self.z_properties = None                  #: ZProperties: Z property data.
        self.model_string = None                  #: str: The string from which the data came.

        if extras is not None:
            self.extras = extras        #: dictionary: Extra parameters (Vs30, elevation, etc.).
        else:
            self.extras = {}

    def set_elevation_data(self, elevation_properties: ElevationProperties) -> bool:
        """
        Sets the elevation data for this particular seismic data instance.

        Parameters:
            elevation_properties (namedtuple): The ElevationProperties namedtuple to set.

        Returns:
            bool: True when successful. An error is raised if not.
        """
        if not isinstance(elevation_properties, ElevationProperties):
            raise TypeError("Elevation properties must be a namedtuple of type "
                            "ElevationProperties.")

        self.elevation_properties = elevation_properties
        return True

    def set_velocity_data(self, velocity_properties: VelocityProperties) -> bool:
        """
        Sets the velocity data for this particular seismic data instance.

        Parameters:
            velocity_properties (namedtuple): The VelocityProperties namedtuple to set.

        Returns:
            bool: True when successful. An error is raised if not.
        """
        if not isinstance(velocity_properties, VelocityProperties):
            raise TypeError("Velocity properties must be a namedtuple of type "
                            "VelocityProperties.")

        self.velocity_properties = velocity_properties
        return True

    def set_vs30_data(self, vs30_properties: Vs30Properties) -> bool:
        """
        Sets the Vs30 data for this particular seismic data instance.

        Parameters:
            vs30_properties (namedtuple): The Vs30Properties namedtuple to set.

        Returns:
            bool: True when successful. An error is raised if not.
        """
        if not isinstance(vs30_properties, Vs30Properties):
            raise TypeError("Vs30 properties must be a namedtuple of type "
                            "Vs30Properties.")

        self.vs30_properties = vs30_properties
        return True

    def set_z_data(self, z_properties: ZProperties) -> bool:
        """
        Sets the Z1.0 and Z2.5 property data for this particular instance.

        Parameters:
            z_properties (namedtuple): The ZProperties namedtuple to set.

        Returns:
            bool: True when successful. An error is raised if not.
        """
        if not isinstance(z_properties, ZProperties):
            raise TypeError("Z properties must be a namedtuple of type ZProperties.")

        self.z_properties = z_properties
        return True

    def set_model_string(self, model_string: str) -> bool:
        """
        Sets the model string from which this data came (e.g. "cvms426.usgs-noaa").

        Parameters:
            model_string (str): The model string.

        Returns:
            bool: True
        """
        self.model_string = model_string
        return True

    def is_property_type_set(self, property_type: str) -> bool:
        """
        Check to see if the given property type is set. Valid property types include velocity, elevation, and vs30. That
        is, has this SeismicData object been populated with values of that type?

        Parameters:
            property_type (str): The property type. Valid values include velocity, elevation, and vs30.

        Returns:
            bool: True if type is set, false if not.
        """
        if property_type == "velocity":
            return self.velocity_properties is not None and \
                   self.velocity_properties.vp is not None
        elif property_type == "elevation":
            return self.elevation_properties is not None
        elif property_type == "vs30":
            return self.vs30_properties is not None

        return False

    @classmethod
    def from_old_ucvm(cls, output_string, point=None, model=None):
        """
        Generates a new instance of SeismicData, from the old UCVM output.

        Parameters:
        :param str output_string: The string returned by UCVM.
        :param Point point: The point to which this SeismicData belongs.
        :param str model: The model identifier as a string.
        :return: A new instance of SeismicData.
        """
        pass
        #line_array = output_string.split()
        #return cls(line_array[14], line_array[15], line_array[16], point, model)

    def __str__(self, *args, **kwargs):
        """
        Prints out the seismic data in the form of long, lat, depth/elevation (100 d for depth,
        -100 e for elevation), Vp, Vs, density.
        """
        return "(%.4f, %.4f, %.4f)" % (float(self.velocity_properties.vp),
                                       float(self.velocity_properties.vs),
                                       float(self.velocity_properties.density))

    def convert_point_to_projection(self, projection: str) -> None:
        """
        Given a projection, sets converted_point to be a copy of this object's Point but converted to the new
        projection.

        Parameters:
            projection (str): The projection as a Proj.4 string.

        Returns:
            None
        """
        if self.original_point.projection == projection:
            self.converted_point = Point(self.original_point.x_value, self.original_point.y_value,
                                         self.original_point.z_value, self.original_point.depth_elev,
                                         self.original_point.metadata,
                                         self.original_point.projection)
        else:
            self.converted_point = self.original_point.convert_to_projection(projection)

    def set_point_to_depth_or_elev(self, depth_or_elev: int=UCVM_DEPTH) -> bool:
        """
        Sets the point to either be a depth or elevation. Elevation_properties must be set for a conversion to happen.
        If it is not set, and a conversion is required, false is returned.

        Parameters:
            depth_or_elev (int): Either UCVM_DEPTH or UCVM_ELEVATION.

        Returns:
            bool: True if conversion was not needed or conversion was successful. False if there was an error (e.g.
            elevation properties were not set).
        """
        if self.converted_point.get_depth_or_elevation() == depth_or_elev or \
           depth_or_elev == UCVM_ELEV_ANY:
            return True

        # A conversion is necessary.
        if self.elevation_properties is None or self.elevation_properties.elevation is None:
            return False

        # We have the elevation properties. Let's convert.
        self.converted_point.z_value = \
            self.elevation_properties.elevation - self.converted_point.z_value

        self.converted_point.depth_or_elev = depth_or_elev

        return True
