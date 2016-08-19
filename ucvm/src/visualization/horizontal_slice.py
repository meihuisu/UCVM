"""
Handles generating a horizontal slice for UCVM.

:copyright: Southern California Earthquake Center
:author:    David Gill <davidgil@usc.edu>
:created:   August 12, 2016
:modified:  August 15, 2016
"""
import os
import struct
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
        if "save_file" in self.extras:
            if os.path.exists(self.extras["save_file"]):
                self.needs_extraction = False
            else:
                self.save_extraction = True

        if "query_at_once" in self.extras:
            self.QUERY_AT_ONCE = int(self.extras["query_at_once"])
        else:
            self.QUERY_AT_ONCE = self.slice_properties.num_x * self.slice_properties.num_y

        self.extracted_data = np.arange(self.slice_properties.num_x *
                                        self.slice_properties.num_y * 5, dtype=float)
        self.bounds = []
        self.ticks = []

        if "title" in self.extras:
            self.plot_title = self.extras["title"]
        else:
            self.plot_title = "Horizontal Slice Starting at (%.2f, %.2f)" % \
                              (self.origin.x_value, self.origin.y_value)

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
        save_file = parsed_dictionary["data"]["save"]

        return HorizontalSlice(origin_point, slice_properties, cvm_list,
                               plot_properties=plot_properties, save_file=save_file)

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
            UCVM.query(init_array, self.cvms, ["velocity"])
            for sd_prop in init_array[0:num_queried]:
                self.extracted_data[counter] = sd_prop.velocity_properties.vp
                self.extracted_data[counter + 1] = sd_prop.velocity_properties.vs
                self.extracted_data[counter + 2] = sd_prop.velocity_properties.density
                self.extracted_data[counter + 3] = sd_prop.velocity_properties.qp
                self.extracted_data[counter + 4] = sd_prop.velocity_properties.qs
                counter += 5
            try:
                num_queried = next(im_iterator)
            except StopIteration:
                break

        if self.save_extraction:
            with open(self.extras["file_location"], "wb") as fd:
                s = struct.pack('f' * len(self.extracted_data), *self.extracted_data)
                fd.write(s)

    def plot(self):
        if self.needs_extraction:
            self.extract()
        else:
            # Read in the already extracted data.
            with open(self.extras["file_location"], "rb") as fd:
                self.extracted_data = struct.unpack('f' * self.slice_properties.num_x *
                                                    self.slice_properties.num_y * 5 * 4, fd.read())

        init_array = UCVM.create_max_seismicdata_array(self.QUERY_AT_ONCE, 1)

        lons = np.arange(self.slice_properties.num_x * self.slice_properties.num_y,
                         dtype=float).reshape(self.slice_properties.num_y,
                                              self.slice_properties.num_x)
        lats = np.arange(self.slice_properties.num_x * self.slice_properties.num_y,
                         dtype=float).reshape(self.slice_properties.num_y,
                                              self.slice_properties.num_x)
        data = np.arange(self.slice_properties.num_x * self.slice_properties.num_y,
                         dtype=float).reshape(self.slice_properties.num_y,
                                              self.slice_properties.num_x)

        if str(self.extras["plot_properties"]["property"]).lower().strip() == "vp":
            position = 0
            self.bounds = [0, 0.35, 0.70, 1.00, 1.35, 1.70, 2.55, 3.40, 4.25, 5.10, 5.95, 6.80,
                           7.65, 8.50]
            self.ticks = [0, 0.85, 1.70, 2.55, 3.40, 4.25, 5.10, 5.95, 6.80, 7.65, 8.50]
        elif str(self.extras["plot_properties"]["property"]).lower().strip() == "vs":
            position = 1
            self.bounds = [0, 0.20, 0.40, 0.60, 0.80, 1.00, 1.50, 2.00, 2.50, 3.00, 3.50, 4.00,
                           4.50, 5.00]
            self.ticks = [0, 0.50, 1.00, 1.50, 2.00, 2.50, 3.00, 3.50, 4.00, 4.50, 5.00]
        elif str(self.extras["plot_properties"]["property"]).lower().strip() == "density":
            position = 2
            self.bounds = [0, 0.20, 0.40, 0.60, 0.80, 1.00, 1.50, 2.00, 2.50, 3.00, 3.50, 4.00,
                           4.50, 5.00]
            self.ticks = [0, 0.50, 1.00, 1.50, 2.00, 2.50, 3.00, 3.50, 4.00, 4.50, 5.00]
        elif str(self.extras["plot_properties"]["property"]).lower().strip() == "qp":
            position = 3
        elif str(self.extras["plot_properties"]["property"]).lower().strip() == "qs":
            position = 4
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
                data[j][i] = self.extracted_data[((j * self.slice_properties.num_x) + i) *
                                                 5 + position] / 1000

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
        super(HorizontalSlice, self).show_plot(lons, lats, data, True)
