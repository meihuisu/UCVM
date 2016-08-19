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
from ucvm.src.shared.properties import Point, SeismicData, UCVM_DEPTH

from collections import namedtuple

from ucvm.src.shared.errors import display_and_raise_error

CrossSectionProperties = namedtuple("CrossSectionProperties",
                                    "width_spacing height_spacing end property")


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

        self.bounds = []
        self.ticks = []

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
            float(dictionary["start_point"]["x"]),
            float(dictionary["start_point"]["y"]),
            float(dictionary["start_point"]["z"]),
            int(dictionary["start_point"]["depth_elev"]),
            {},
            dictionary["start_point"]["projection"]
        )

        end_point = Point(
            float(dictionary["end_point"]["x"]),
            float(dictionary["end_point"]["y"]),
            float(dictionary["end_point"]["z"]),
            int(dictionary["end_point"]["depth_elev"]),
            {},
            dictionary["end_point"]["projection"]
        )

        cross_section_properties = CrossSectionProperties(
            float(dictionary["cross_section_properties"]["width_spacing"]),
            float(dictionary["cross_section_properties"]["height_spacing"]) if
            int(dictionary["start_point"]["depth_elev"]) == UCVM_DEPTH else
            -1 * float(dictionary["cross_section_properties"]["height_spacing"]),
            float(dictionary["cross_section_properties"]["end"]),
            dictionary["cross_section_properties"]["property"]
        )
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
                    SeismicData(Point(lon, lat, j, int(self.start_point.depth_elev)))
                )

        UCVM.query(self.sd_array, self.cvms, ["elevation", "velocity"])

        num_x = num_prof + 1
        num_y = int(math.ceil(self.cross_section_properties.end - self.start_point.z_value) /
                    self.cross_section_properties.height_spacing)

        self.extracted_data = np.arange(num_x * num_y * 5, dtype=float).reshape(num_y, num_x * 5)

        for y in range(int(num_y)):
            for x in range(int(num_x)):
                x_val = x * 5
                if self.sd_array[y * num_x + x].velocity_properties is not None:
                    vp_val = self.sd_array[y * num_x + x].velocity_properties.vp
                    vs_val = self.sd_array[y * num_x + x].velocity_properties.vs
                    density_val = self.sd_array[y * num_x + x].velocity_properties.density
                    qp_val = self.sd_array[y * num_x + x].velocity_properties.qp
                    qs_val = self.sd_array[y * num_x + x].velocity_properties.qs
                else:
                    vp_val = None
                    vs_val = None
                    density_val = None
                    qp_val = None
                    qs_val = None
                self.extracted_data[y][x_val] = vp_val if vp_val is not None else -1
                self.extracted_data[y][x_val + 1] = vs_val if vs_val is not None else -1
                self.extracted_data[y][x_val + 2] = density_val if density_val is not None else -1
                self.extracted_data[y][x_val + 3] = qp_val if qp_val is not None else -1
                self.extracted_data[y][x_val + 4] = qs_val if qs_val is not None else -1

    def plot(self):
        if self.extracted_data is None:
            self.extract()

        num_y = int(len(self.extracted_data))
        num_x = int(len(self.extracted_data[0]) / 5)

        datapoints = np.arange(num_y * num_x, dtype=float).reshape(num_y, num_x)

        if str(self.cross_section_properties.property).lower().strip() == "vp":
            position = 0
            self.bounds = [0, 0.35, 0.70, 1.00, 1.35, 1.70, 2.55, 3.40, 4.25, 5.10, 5.95, 6.80,
                           7.65, 8.50]
            self.ticks = [0, 0.85, 1.70, 2.55, 3.40, 4.25, 5.10, 5.95, 6.80, 7.65, 8.50]
        elif str(self.cross_section_properties.property).lower().strip() == "vs":
            position = 1
            self.bounds = [0, 0.20, 0.40, 0.60, 0.80, 1.00, 1.50, 2.00, 2.50, 3.00, 3.50, 4.00,
                           4.50, 5.00]
            self.ticks = [0, 0.50, 1.00, 1.50, 2.00, 2.50, 3.00, 3.50, 4.00, 4.50, 5.00]
        elif str(self.cross_section_properties.property).lower().strip() == "density":
            position = 2
            self.bounds = [0, 0.20, 0.40, 0.60, 0.80, 1.00, 1.50, 2.00, 2.50, 3.00, 3.50, 4.00,
                           4.50, 5.00]
            self.ticks = [0, 0.50, 1.00, 1.50, 2.00, 2.50, 3.00, 3.50, 4.00, 4.50, 5.00]
        elif str(self.cross_section_properties.property).lower().strip() == "qp":
            position = 3
        elif str(self.cross_section_properties.property).lower().strip() == "qs":
            position = 4
        else:
            position = 0

        for y in range(num_y):
            for x in range(num_x):
                x_val = x * 5
                if int(self.start_point.depth_elev) == UCVM_DEPTH:
                    datapoints[y][x] = self.extracted_data[y][x_val + position] / 1000
                else:
                    datapoints[num_y - y - 1][x] = self.extracted_data[y][x_val + position] / 1000

        # Ok, now that we have the 2D array of lons, lats, and data, let's call on our inherited
        # classes show_plot function to actually show the plot.
        super(CrossSection, self).show_plot(None, None, datapoints, False)
