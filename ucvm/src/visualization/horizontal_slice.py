"""
Handles generating a horizontal slice for UCVM.

:copyright: Southern California Earthquake Center
:author:    David Gill <davidgil@usc.edu>
:created:   August 12, 2016
<<<<<<< HEAD
:modified:  September 8, 2016
"""
import os

import xmltodict
import numpy as np

from collections import namedtuple

from ucvm.src.framework.ucvm import UCVM
from ucvm.src.visualization.plot import Plot
from ucvm.src.shared.constants import UCVM_DEFAULT_PROJECTION
from ucvm.src.shared.properties import Point
from ucvm.src.shared.errors import display_and_raise_error
from ucvm.src.framework.mesh_common import InternalMesh, InternalMeshIterator

SliceProperties = namedtuple("SliceProperties", "num_x num_y spacing rotation")


class HorizontalSlice(Plot):

    def __init__(self, origin_point: Point, slice_properties: SliceProperties, cvms: str,
                 **kwargs):
        if isinstance(origin_point, Point):
            self.origin = origin_point  #: Point: The bottom-left origin point for the slice.
        else:
            display_and_raise_error(6)

        if isinstance(slice_properties, SliceProperties):
            self.slice_properties = slice_properties  #: SliceProperties: Properties for slice.
        else:
            display_and_raise_error(7)

        self.cvms = cvms        #: str: The list of community velocity models to query.
        self.extras = kwargs    #: dict: Save any extra parameters

        # Check to see if we need to extract the data before we plot it.
        self.needs_extraction = True    #: bool: True to extract data before plotting.
        self.save_extraction = False    #: bool: True if we want to save the extraction.
        if "save_file" in self.extras and self.extras["save_file"] is not None:
            if os.path.exists(self.extras["save_file"]):
                self.needs_extraction = False
            else:
                self.save_extraction = True

        if "query_at_once" in self.extras:
            self.QUERY_AT_ONCE = int(self.extras["query_at_once"])
        else:
            self.QUERY_AT_ONCE = self.slice_properties.num_x * self.slice_properties.num_y

        if self.extras["plot"]["property"] == "elevation" or \
                        self.extras["plot"]["property"] == "vs30":
            self.extracted_data = np.zeros(self.slice_properties.num_x *
                                           self.slice_properties.num_y, dtype="<f8")
        else:
            self.extracted_data = np.zeros((self.slice_properties.num_x *
                                           self.slice_properties.num_y) * 6, dtype="<f8")
        self.bounds = []
        self.ticks = []

        if "title" in self.extras["plot"]:
            self.plot_title = self.extras["plot"]["title"]
        else:
            self.plot_title = "Horizontal Slice Starting at (%.2f, %.2f)" % \
                              (self.origin.x_value, self.origin.y_value)

        self.plot_xlabel = None
        self.plot_ylabel = None
        self.plot_cbar_label = None

        super().__init__(**kwargs)

    @classmethod
    def from_dictionary(cls, parsed_dictionary: dict):
        """
        From a dictionary, return an instance of the HorizontalSlice class. Note that you will still
        need to call extract and the other functions separately. This just configures everything.
        :param parsed_dictionary: The already parsed dictionary.
        :return: An instance of HorizontalSlice
        """
        origin_point = Point(
            parsed_dictionary["bottom_left_point"]["x"],
            parsed_dictionary["bottom_left_point"]["y"],
            parsed_dictionary["bottom_left_point"]["z"],
            parsed_dictionary["bottom_left_point"]["depth_elev"],
            {},
            parsed_dictionary["bottom_left_point"]["projection"]
        )

        slice_properties = SliceProperties(
            num_x=int(parsed_dictionary["properties"]["num_x"]),
            num_y=int(parsed_dictionary["properties"]["num_y"]),
            spacing=float(parsed_dictionary["properties"]["spacing"]),
            rotation=float(parsed_dictionary["properties"]["rotation"])
        )

        cvm_list = parsed_dictionary["cvm_list"]

        plot_properties = parsed_dictionary["plot"]
        try:
            save_file = parsed_dictionary["data"]["save"]
        except KeyError:
            save_file = None

        return HorizontalSlice(origin_point, slice_properties, cvm_list,
                               plot=plot_properties, save_file=save_file)

    @classmethod
    def from_xml_file(cls, xml_file: str):
        """
        Reads the XML file and returns an instance of this class.
        :param xml_file: The XML file to read.
        :return: An instance of HorizontalSlice.
        """
        try:
            with open(xml_file, "r") as fd:
                info = xmltodict.parse(fd.read())["root"]
        except FileNotFoundError:
            return None

        return HorizontalSlice.from_dictionary(info)

    def extract(self):
        init_array = UCVM.create_max_seismicdata_array(self.QUERY_AT_ONCE, 1)

        im = InternalMesh.from_parameters(
            self.origin,
            {
                "num_x": self.slice_properties.num_x,
                "num_y": self.slice_properties.num_y,
                "num_z": 1,
                "rotation": self.slice_properties.rotation,
                "spacing": self.slice_properties.spacing
            },
            self.cvms,
            ""
        )
        im_iterator = InternalMeshIterator(im, 0, im.total_size, len(init_array), init_array)

        counter = 0
        num_queried = next(im_iterator)
        while num_queried > 0:
            if self.extras["plot"]["property"] == "elevation":
                UCVM.query(init_array, self.cvms, ["elevation"])
                for sd_prop in init_array[0:num_queried]:
                    self.extracted_data[counter] = sd_prop.elevation_properties.elevation
                    counter += 1
            elif self.extras["plot"]["property"] == "vs30":
                UCVM.query(init_array, self.cvms, ["vs30"])
                for sd_prop in init_array[0:num_queried]:
                    self.extracted_data[counter] = sd_prop.vs30_properties.vs30
                    counter += 1
            else:
                UCVM.query(init_array, self.cvms, ["elevation", "velocity"])
                for sd_prop in init_array[0:num_queried]:
                    self.extracted_data[counter] = sd_prop.velocity_properties.vp
                    self.extracted_data[counter + 1] = sd_prop.velocity_properties.vs
                    self.extracted_data[counter + 2] = sd_prop.velocity_properties.density
                    self.extracted_data[counter + 3] = sd_prop.velocity_properties.qp
                    self.extracted_data[counter + 4] = sd_prop.velocity_properties.qs
                    self.extracted_data[counter + 5] = sd_prop.elevation_properties.elevation
                    counter += 6

            try:
                num_queried = next(im_iterator)
            except StopIteration:
                break

        if self.save_extraction:
            with open(self.extras["save_file"], "wb") as fd:
                np.save(fd, self.extracted_data)

    def plot(self):
        if self.needs_extraction:
            self.extract()
        else:
            # Read in the already extracted data.
            with open(self.extras["save_file"], "rb") as fd:
                self.extracted_data = np.load(fd)

        init_array = UCVM.create_max_seismicdata_array(self.QUERY_AT_ONCE, 1)

        lons = np.zeros(self.slice_properties.num_x * self.slice_properties.num_y,
                        dtype=float).reshape(self.slice_properties.num_y,
                                             self.slice_properties.num_x)
        lats = np.zeros(self.slice_properties.num_x * self.slice_properties.num_y,
                        dtype=float).reshape(self.slice_properties.num_y,
                                             self.slice_properties.num_x)
        data = np.zeros(self.slice_properties.num_x * self.slice_properties.num_y,
                        dtype=float).reshape(self.slice_properties.num_y,
                                             self.slice_properties.num_x)
        topography = None

        if "features" in self.extras["plot"] and \
           "topography" in self.extras["plot"]["features"] and \
           str(self.extras["plot"]["features"]["topography"]).lower().strip() == "yes":
            topography = np.zeros(self.slice_properties.num_x * self.slice_properties.num_y,
                                  dtype="<f8").reshape(self.slice_properties.num_y,
                                                       self.slice_properties.num_x)

        if str(self.extras["plot"]["property"]).lower().strip() == "vp":
            position = 0
            self.bounds = [0, 0.35, 0.70, 1.00, 1.35, 1.70, 2.55, 3.40, 4.25, 5.10, 5.95, 6.80,
                           7.65, 8.50]
            self.ticks = [0, 0.85, 1.70, 2.55, 3.40, 4.25, 5.10, 5.95, 6.80, 7.65, 8.50]
            self.plot_cbar_label = "Vp (km/s)"
        elif str(self.extras["plot"]["property"]).lower().strip() == "vs":
            position = 1
            self.bounds = [0, 0.20, 0.40, 0.60, 0.80, 1.00, 1.50, 2.00, 2.50, 3.00, 3.50, 4.00,
                           4.50, 5.00]
            self.ticks = [0, 0.50, 1.00, 1.50, 2.00, 2.50, 3.00, 3.50, 4.00, 4.50, 5.00]
            self.plot_cbar_label = "Vs (km/s)"
        elif str(self.extras["plot"]["property"]).lower().strip() == "density":
            position = 2
            self.bounds = [0, 0.20, 0.40, 0.60, 0.80, 1.00, 1.50, 2.00, 2.50, 3.00, 3.50, 4.00,
                           4.50, 5.00]
            self.ticks = [0, 0.50, 1.00, 1.50, 2.00, 2.50, 3.00, 3.50, 4.00, 4.50, 5.00]
            self.plot_cbar_label = "Density (kg/m^3)"
        elif str(self.extras["plot"]["property"]).lower().strip() == "qp":
            position = 3
        elif str(self.extras["plot"]["property"]).lower().strip() == "qs":
            position = 4
        elif str(self.extras["plot"]["property"]).lower().strip() == "elevation":
            position = 0
            self.bounds = [-4, -3, -2, -1, 0, 1, 2, 3, 4]
            self.ticks = [-4, -3, -2, -1, 0, 1, 2, 3, 4]
            self.plot_cbar_label = "Elevation (km)"
        else:
            position = 0

        im = InternalMesh.from_parameters(
            self.origin,
            {
                "num_x": self.slice_properties.num_x,
                "num_y": self.slice_properties.num_y,
                "num_z": 1,
                "rotation": self.slice_properties.rotation,
                "spacing": self.slice_properties.spacing
            },
            self.cvms,
            ""
        )
        im_iterator = InternalMeshIterator(im, 0, im.total_size, len(init_array), init_array)

        num_queried = next(im_iterator)
        i = 0
        j = 0
        while num_queried > 0:
            for datum in init_array[0:num_queried]:
                new_pt = datum.original_point.convert_to_projection(UCVM_DEFAULT_PROJECTION)
                lons[j][i] = new_pt.x_value
                lats[j][i] = new_pt.y_value

                if str(self.extras["plot"]["property"]).lower().strip() == "elevation":
                    data[j][i] = self.extracted_data[j * self.slice_properties.num_x + i] / 1000
                else:
                    data[j][i] = self.extracted_data[((j * self.slice_properties.num_x) + i) *
                                                     6 + position] / 1000
                    if topography is not None:
                        topography[j][i] = self.extracted_data[
                                               ((j * self.slice_properties.num_x) + i) * 6 + 5
                                           ] / 1000

                i += 1
                if i == self.slice_properties.num_x:
                    i = 0
                    j += 1

            try:
                num_queried = next(im_iterator)
            except StopIteration:
                break

        # Ok, now that we have the 2D array of lons, lats, and data, let's call on our inherited
        # classes show_plot function to actually show the plot.
        super(HorizontalSlice, self).show_plot(lons, lats, data, True, topography=topography)
