"""
Handles generating a depth profile for UCVM.

:copyright: Southern California Earthquake Center
:author:    David Gill <davidgil@usc.edu>
:created:   August 18, 2016
:modified:  August 18, 2016
"""
import math

from collections import namedtuple

from ucvm.src.framework.ucvm import UCVM
from ucvm.src.visualization.plot import Plot
from ucvm.src.shared.properties import Point, UCVM_DEPTH, UCVM_ELEVATION
from ucvm.src.shared.errors import display_and_raise_error

DepthProfileProperties = namedtuple("DepthProfileProperties", "depth spacing properties")


class DepthProfile(Plot):

    def __init__(self, profile_point: Point, depth_profile_properties: DepthProfileProperties,
                 cvms: str, **kwargs):
        if isinstance(profile_point, Point):
            self.profile_point = profile_point  #: Point: The point for the depth profile.
        else:
            display_and_raise_error(14)

        if isinstance(depth_profile_properties, DepthProfileProperties):
            self.profile_properties = depth_profile_properties
            #: DepthProfileProperties: Properties for the depth profile.
        else:
            display_and_raise_error(15)

        self.cvms = cvms
        self.extras = kwargs

        self.sd_array = None

        if "title" in self.extras:
            self.plot_title = self.extras["title"]
        else:
            self.plot_title = "Depth Profile at (%.2f, %.2f)" % \
                              (self.profile_point.x_value, self.profile_point.y_value)

        super().__init__(**kwargs)

    @classmethod
    def from_dictionary(cls, dictionary: dict):
        profile_point = Point(
            dictionary["profile_point"]["x"],
            dictionary["profile_point"]["y"],
            dictionary["profile_point"]["z"],
            UCVM_DEPTH if dictionary["profile_point"]["depth_elev"] == "0" else UCVM_ELEVATION,
            {},
            dictionary["profile_point"]["projection"]
        )

        depth_profile_properties = DepthProfileProperties(
            float(dictionary["profile_properties"]["depth"]),
            float(dictionary["profile_properties"]["spacing"]),
            [x.strip() for x in str(dictionary["profile_properties"]["properties"]).split(",")]
        )

        cvm_list = dictionary["cvm_list"]

        if "plot" in dictionary:
            if "title" in dictionary["plot"]:
                return DepthProfile(profile_point, depth_profile_properties, cvm_list,
                                    title=dictionary["plot"]["title"])

        return DepthProfile(profile_point, depth_profile_properties, cvm_list)

    def extract(self):
        num_points = int(math.ceil((self.profile_properties.depth - self.profile_point.z_value) /
                                   self.profile_properties.spacing)) + 1
        self.sd_array = UCVM.create_max_seismicdata_array(num_points)

        for i in range(0, num_points):
            self.sd_array[i].original_point.x_value = self.profile_point.x_value
            self.sd_array[i].original_point.y_value = self.profile_point.y_value
            if self.sd_array[i].original_point.depth_elev == UCVM_DEPTH:
                self.sd_array[i].original_point.z_value = \
                    self.profile_point.z_value + (i * self.profile_properties.spacing)
            else:
                self.sd_array[i].original_point.z_value = \
                    self.profile_point.z_value - (i * self.profile_properties.spacing)

        UCVM.query(self.sd_array, self.cvms)

    def plot(self):
        if self.sd_array is None:
            self.extract()

        properties = []

        if self.profile_properties.properties[0].strip().lower() == "all" or \
           self.profile_properties.properties[0].strip().lower() == "":
            if self.sd_array[0].velocity_properties.vp is not None:
                properties.append("vp")
            if self.sd_array[0].velocity_properties.vs is not None:
                properties.append("vs")
            if self.sd_array[0].velocity_properties.density is not None:
                properties.append("density")
            if self.sd_array[0].velocity_properties.qp is not None:
                properties.append("qp")
            if self.sd_array[0].velocity_properties.qs is not None:
                properties.append("qs")
        else:
            properties = self.profile_properties.properties

        self.plot_ylabel = "Depth (m)" if self.profile_point.depth_elev == UCVM_DEPTH else \
            "Elevation (m)"
        self.plot_xlabel = "Units (m/s for velocity, m^kg^3 for density)"

        data = {}

        for property_type in properties:
            data[property_type] = {
                "y_values": [x.original_point.z_value for x in self.sd_array],
                "x_values": [getattr(x.velocity_properties, property_type) for x in self.sd_array]
            }

        super(DepthProfile, self).show_profile(data)
