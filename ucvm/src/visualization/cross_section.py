"""
Handles generating a cross-section for UCVM.

:copyright: Southern California Earthquake Center
:author:    David Gill <davidgil@usc.edu>
:created:   August 18, 2016
:modified:  August 18, 2016
"""
import math
import pyproj
import numpy as np

from ucvm.src.framework.ucvm import UCVM
from ucvm.src.visualization.plot import Plot
from ucvm.src.shared.properties import Point, SeismicData

from collections import namedtuple

from ucvm.src.shared.errors import display_and_raise_error

CrossSectionProperties = namedtuple("CrossSectionProperties",
                                    "width_spacing height_spacing end")


class CrossSection(Plot):

    def __init__(self, start_point: Point, end_point: Point,
                 cross_section_properties: CrossSectionProperties, cvms: str, **kwargs):
        if isinstance(start_point, Point):
            self.start_point = start_point  #: Point: The start point for the cross-section.
        else:
            display_and_raise_error(16)

        if isinstance(end_point, Point):
            self.end_point = end_point  #: Point: The end point for the cross-section.
        else:
            display_and_raise_error(16)

        if isinstance(cross_section_properties,CrossSectionProperties):
            self.cross_section_properties = cross_section_properties
            #: CrossSectionProperties: Properties for the cross-section.
        else:
            display_and_raise_error(17)

        self.cvms = cvms
        self.extras = kwargs

        self.sd_array = []
        self.extracted_data = None

        if "title" in self.extras:
            self.plot_title = self.extras["title"]
        else:
            self.plot_title = "Cross-Section from (%.2f, %.2f) to (%.2f, %.2f)" % \
                              (self.start_point.x_value, self.start_point.y_value,
                               self.end_point.x_value, self.end_point.y_value)

        super().__init__(**kwargs)

    @classmethod
    def from_dictionary(cls, dictionary: dict):
        start_point = Point(
            dictionary["start_point"]["x"],
            dictionary["start_point"]["y"],
            dictionary["start_point"]["z"],
            dictionary["start_point"]["depth_elev"],
            {},
            dictionary["start_point"]["projection"]
        )

        end_point = Point(
            dictionary["end_point"]["x"],
            dictionary["end_point"]["y"],
            dictionary["end_point"]["z"],
            dictionary["end_point"]["depth_elev"],
            {},
            dictionary["end_point"]["projection"]
        )

        cross_section_properties = dictionary["cross_section_properties"]
        cvm_list = dictionary["cvm_list"]

        return CrossSection(start_point, end_point, cross_section_properties, cvm_list)

    def extract(self):
        proj = pyproj.Proj(proj='utm', zone=11, ellps='WGS84')

        x1, y1 = proj(self.start_point.x_value, self.start_point.y_value)
        x2, y2 = proj(self.end_point.x_value, self.end_point.y_value)

        num_prof = int(
            math.sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1)) /
            self.cross_section_properties.width_spacing
        )

        for j in range(int(self.start_point.z_value), int(self.cross_section_properties.end) + 1,
                       int(self.cross_section_properties.height_spacing)):
            for i in range(0, num_prof + 1):
                x = x1 + i * (x2 - x1) / float(num_prof)
                y = y1 + i * (y2 - y1) / float(num_prof)
                lon, lat = proj(x, y, inverse=True)
                self.sd_array.append(
                    SeismicData(Point(lon, lat, j))
                )

        UCVM.query(self.sd_array, self.cvms)

        num_x = num_prof + 1
        num_y = (int(self.cross_section_properties.end) - int(self.start_point.z_value)) / \
                int(self.cross_section_properties.height_spacing) + 1

        self.extracted_data = np.arange(num_x * num_y * 5, dtype=float).reshape(num_y, num_x * 5)

        for y in range(num_y):
            for x in range(num_x):
                x_val = x * 5
                self.extracted_data[y][x_val] = \
                    self.sd_array[y * num_x + x].velocity_properties.vp
                self.extracted_data[y][x_val + 1] = \
                    self.sd_array[y * num_x + x].velocity_properties.vs
                self.extracted_data[y][x_val + 2] = \
                    self.sd_array[y * num_x + x].velocity_properties.density
                self.extracted_data[y][x_val + 3] = \
                    self.sd_array[y * num_x + x].velocity_properties.qp
                self.extracted_data[y][x_val + 4] = \
                    self.sd_array[y * num_x + x].velocity_properties.qs

    def plot(self):
        if self.extracted_data is None:
            self.extract()

        num_y = len(self.extracted_data)
        num_x = len(self.extracted_data[0]) / 5

        datapoints = np.arange(num_y * num_x, dtype=float).reshape(num_y, num_x)

