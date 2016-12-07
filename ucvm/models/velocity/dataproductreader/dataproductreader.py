"""
Data Product Reader Velocity Model

This "velocity model" reads from an AWP mesh, an RWG mesh, or an e-tree. It then retrieves the
material properties stored within the data structure. For meshes, this is trilinearly interpolated.

Copyright:
    Southern California Earthquake Center

Developer:
    David Gill <davidgil@usc.edu>
"""
# Python Imports
import os
import math
import struct
from typing import List

# Package Imports
import pyproj
import xmltodict

# UCVM Imports
from ucvm.src.model.velocity.velocity_model import VelocityModel
from ucvm.src.shared import VelocityProperties
from ucvm.src.shared.constants import UCVM_DEFAULT_PROJECTION, UCVM_DEPTH, UCVM_ELEVATION
from ucvm.src.shared.properties import SeismicData, Point
from ucvm.src.shared.errors import display_and_raise_error
from ucvm_c_common import UCVMCCommon

class DataProductReaderVelocityModel(VelocityModel):

    def __init__(self):
        self.source = ""
        self.llcorner = None
        self.dims = {}
        self.rotation = 0.0
        self.projection = UCVM_DEFAULT_PROJECTION
        self.origin_in_mesh_proj = []
        self.cos = 0.0
        self.sin = 0.0
        self.data_dir = ""
        self.corners = ()
        super().__init__()

    def _initialize(self, xml_file):
        self.source = xml_file["mesh_name"] if "mesh_name" in xml_file else xml_file["etree_name"]

        if "initial_point" in xml_file:
            self.llcorner = Point(
                xml_file["initial_point"]["x"],
                xml_file["initial_point"]["y"],
                xml_file["initial_point"]["z"],
                UCVM_ELEVATION if str(xml_file["initial_point"]["depth_elev"]).lower() == "elevation"
                else UCVM_DEPTH,
                UCVM_DEFAULT_PROJECTION if
                str(xml_file["initial_point"]["projection"]).lower() == "default" else
                str(xml_file["initial_point"]["projection"])
            )
            self.dims = {
                "x": int(xml_file["dimensions"]["x"]),
                "y": int(xml_file["dimensions"]["y"]),
                "z": int(xml_file["dimensions"]["z"]),
                "spacing": int(xml_file["spacing"])
            }
            self.rotation = float(xml_file["rotation"])
            self.cos = float(math.cos(math.radians(self.rotation)))
            self.sin = float(math.sin(math.radians(self.rotation)))

            self.projection = xml_file["projection"]

            # Find the origin point.
            p1 = pyproj.Proj(self.llcorner.projection)
            p2 = pyproj.Proj(self.projection)
            ll_x, ll_y = pyproj.transform(p1, p2, self.llcorner.x_value, self.llcorner.y_value)
            self.origin_in_mesh_proj = [ll_x, ll_y]

            # Get the four corners.
            p1 = pyproj.Proj(UCVM_DEFAULT_PROJECTION)
            p2 = pyproj.Proj("+proj=utm +datum=WGS84 +zone=11")

            ll_x, ll_y = pyproj.transform(p1, p2, self.llcorner.x_value, self.llcorner.y_value)
            corners_e = (ll_x, ll_x, ll_x + self.dims["x"] * self.dims["spacing"],
                     ll_x + self.dims["x"] * self.dims["spacing"])
            corners_n = (ll_y, ll_y + self.dims["y"] * self.dims["spacing"],
                        ll_y + self.dims["y"] * self.dims["spacing"], ll_y)
            corners_e, corners_n = pyproj.transform(p2, p1, corners_e, corners_n)

            self.corners = ((corners_e[0], corners_n[0]), (corners_e[1], corners_n[1]),
                            (corners_e[2], corners_n[2]), (corners_e[3], corners_n[3]))
        else:
            self.corners = (
                (float(xml_file["corners"]["bl"]["x"]), float(xml_file["corners"]["bl"]["y"])),
                (float(xml_file["corners"]["ul"]["x"]), float(xml_file["corners"]["ul"]["y"])),
                (float(xml_file["corners"]["ur"]["x"]), float(xml_file["corners"]["ur"]["y"])),
                (float(xml_file["corners"]["br"]["x"]), float(xml_file["corners"]["br"]["y"]))
            )

    def _awp_query(self, data: List[SeismicData]):
        p1 = pyproj.Proj(UCVM_DEFAULT_PROJECTION)
        p2 = pyproj.Proj(self.projection)

        lons_to_convert = []
        lats_to_convert = []

        for i in range(len(data)):
            lons_to_convert.append(data[i].converted_point.x_value)
            lats_to_convert.append(data[i].converted_point.y_value)

        converted_x, converted_y = pyproj.transform(p1, p2, lons_to_convert, lats_to_convert)

        fin = open(os.path.join(self.data_dir, self.source + ".awp"), "rb")

        values = {
            "tsw": 0.0,
            "tnw": 0.0,
            "tne": 0.0,
            "tse": 0.0,
            "bsw": 0.0,
            "bnw": 0.0,
            "bne": 0.0,
            "bse": 0.0
        }

        values_matprop = {
            "tsw": 0.0,
            "tnw": 0.0,
            "tne": 0.0,
            "tse": 0.0,
            "bsw": 0.0,
            "bnw": 0.0,
            "bne": 0.0,
            "bse": 0.0
        }

        v = {
            "vs": 0.0,
            "vp": 0.0,
            "density": 0.0
        }

        for i in range(len(converted_x)):
            temp_utm_e = converted_x[i] - self.origin_in_mesh_proj[0]
            temp_utm_n = converted_y[i] - self.origin_in_mesh_proj[1]

            new_point_utm_n = self.sin * temp_utm_e + self.cos * temp_utm_n
            new_point_utm_e = self.cos * temp_utm_e - self.sin * temp_utm_n

            coords, percentages = UCVMCCommon.calculate_grid_point(
                self.dims["x"] * self.dims["spacing"],
                self.dims["y"] * self.dims["spacing"],
                self.dims["z"] * self.dims["spacing"],
                new_point_utm_e,
                new_point_utm_n,
                data[i].converted_point.z_value,
                self.dims["x"],
                self.dims["y"],
                self.dims["spacing"]
            )

            # Check to see if we are outside of the model boundaries.
            if coords["x"] < 0 or coords["y"] < 0 or coords["z"] < 0 or \
                            coords["x"] > self.dims["x"] - 2 or coords["y"] > self.dims["y"] - 2 or \
                            coords["z"] > self.dims["z"] - 1:
                self._set_velocity_properties_none(data[i])
                continue

            values["tsw"] = (self.dims["z"] - coords["z"] - 1) * \
                            (self.dims["y"] * self.dims["x"]) + \
                            (coords["y"] * self.dims["x"]) + coords["x"]
            values["tse"] = (self.dims["z"] - coords["z"] - 1) * \
                            (self.dims["y"] * self.dims["x"]) + \
                            (coords["y"] * self.dims["x"]) + (coords["x"] + 1)
            values["tnw"] = (self.dims["z"] - coords["z"] - 1) * \
                            (self.dims["y"] * self.dims["x"]) + \
                            ((coords["y"] + 1) * self.dims["x"]) + coords["x"]
            values["tne"] = (self.dims["z"] - coords["z"] - 1) * \
                            (self.dims["y"] * self.dims["x"]) + \
                            ((coords["y"] + 1) * self.dims["x"]) + (coords["x"]) + 1
            values["bsw"] = (self.dims["z"] - coords["z"]) * \
                            (self.dims["y"] * self.dims["x"]) + \
                            (coords["y"] * self.dims["x"]) + coords["x"]
            values["bse"] = (self.dims["z"] - coords["z"]) * \
                            (self.dims["y"] * self.dims["x"]) + \
                            (coords["y"] * self.dims["x"]) + (coords["x"] + 1)
            values["bnw"] = (self.dims["z"] - coords["z"]) * \
                            (self.dims["y"] * self.dims["x"]) + \
                            ((coords["y"] + 1) * self.dims["x"]) + coords["x"]
            values["bne"] = (self.dims["z"] - coords["z"]) * \
                            (self.dims["y"] * self.dims["x"]) + \
                            ((coords["y"] + 1) * self.dims["x"]) + (coords["x"] + 1)

            for prop_given in ["vp", "vs", "density"]:
                add = 0 if prop_given == "vp" else 4 if prop_given == "vs" else 8
                for key in values.keys():
                    fin.seek(values[key] * 12 + add)
                    values_matprop[key] = struct.unpack("<f", fin.read(4))[0]

                v[prop_given] = UCVMCCommon.trilinear_interpolate(
                    values_matprop["tsw"], values_matprop["tse"], values_matprop["tnw"],
                    values_matprop["tne"], values_matprop["bsw"], values_matprop["bse"],
                    values_matprop["bnw"], values_matprop["bne"],
                    percentages["x"], percentages["y"], percentages["z"]
                )
                v[str(prop_given) + "_source"] = "awp: " + self.source

            data[i].set_velocity_data(
                VelocityProperties(
                    v["vp"], v["vs"], v["density"], None, None,
                    v["vp_source"], v["vs_source"], v["density_source"], None, None
                )
            )

    def _rwg_query(self, data: List[SeismicData]):
        p1 = pyproj.Proj(UCVM_DEFAULT_PROJECTION)
        p2 = pyproj.Proj(self.projection)

        lons_to_convert = []
        lats_to_convert = []

        for i in range(len(data)):
            lons_to_convert.append(data[i].converted_point.x_value)
            lats_to_convert.append(data[i].converted_point.y_value)

        converted_x, converted_y = pyproj.transform(p1, p2, lons_to_convert, lats_to_convert)

        fin_vp = open(os.path.join(self.data_dir, self.source + ".rwgvp"), "rb")
        fin_vs = open(os.path.join(self.data_dir, self.source + ".rwgvs"), "rb")
        fin_dn = open(os.path.join(self.data_dir, self.source + ".rwgdn"), "rb")

        values = {
            "tsw": 0.0,
            "tnw": 0.0,
            "tne": 0.0,
            "tse": 0.0,
            "bsw": 0.0,
            "bnw": 0.0,
            "bne": 0.0,
            "bse": 0.0
        }

        values_matprop = {
            "tsw": 0.0,
            "tnw": 0.0,
            "tne": 0.0,
            "tse": 0.0,
            "bsw": 0.0,
            "bnw": 0.0,
            "bne": 0.0,
            "bse": 0.0
        }

        v = {
            "vs": 0.0,
            "vp": 0.0,
            "density": 0.0
        }

        for i in range(len(converted_x)):
            temp_utm_e = converted_x[i] - self.origin_in_mesh_proj[0]
            temp_utm_n = converted_y[i] - self.origin_in_mesh_proj[1]

            new_point_utm_n = self.sin * temp_utm_e + self.cos * temp_utm_n
            new_point_utm_e = self.cos * temp_utm_e - self.sin * temp_utm_n

            coords, percentages = UCVMCCommon.calculate_grid_point(
                self.dims["x"] * self.dims["spacing"],
                self.dims["y"] * self.dims["spacing"],
                self.dims["z"] * self.dims["spacing"],
                new_point_utm_e,
                new_point_utm_n,
                data[i].converted_point.z_value,
                self.dims["x"],
                self.dims["y"],
                self.dims["spacing"]
            )

            # Check to see if we are outside of the model boundaries.
            if coords["x"] < 0 or coords["y"] < 0 or coords["z"] < 0 or \
                            coords["x"] > self.dims["x"] - 2 or coords["y"] > self.dims["y"] - 2 or \
                            coords["z"] > self.dims["z"] - 1:
                self._set_velocity_properties_none(data[i])
                continue

            values["tsw"] = (coords["y"] * (self.dims["z"] * self.dims["x"])) + \
                            ((self.dims["z"] - coords["z"] - 1) * self.dims["x"]) + coords["x"]
            values["tse"] = (coords["y"] * (self.dims["z"] * self.dims["x"])) + \
                            ((self.dims["z"] - coords["z"] - 1) * self.dims["x"]) + coords["x"] + 1
            values["tnw"] = ((coords["y"] + 1) * (self.dims["z"] * self.dims["x"])) + \
                            ((self.dims["z"] - coords["z"] - 1) * self.dims["x"]) + coords["x"]
            values["tne"] = ((coords["y"] + 1) * (self.dims["z"] * self.dims["x"])) + \
                            ((self.dims["z"] - coords["z"] - 1) * self.dims["x"]) + coords["x"] + 1
            values["bsw"] = (coords["y"] * (self.dims["z"] * self.dims["x"])) + \
                            ((self.dims["z"] - coords["z"]) * self.dims["x"]) + coords["x"]
            values["bse"] = (coords["y"] * (self.dims["z"] * self.dims["x"])) + \
                            ((self.dims["z"] - coords["z"]) * self.dims["x"]) + coords["x"] + 1
            values["bnw"] = ((coords["y"] + 1) * (self.dims["z"] * self.dims["x"])) + \
                            ((self.dims["z"] - coords["z"]) * self.dims["x"]) + coords["x"]
            values["bne"] = ((coords["y"] + 1) * (self.dims["z"] * self.dims["x"])) + \
                            ((self.dims["z"] - coords["z"]) * self.dims["x"]) + coords["x"] + 1

            for key in values.keys():
                fin_vp.seek(values[key] * 4)
                values_matprop[key] = struct.unpack("<f", fin_vp.read(4))[0]
            v["vp"] = UCVMCCommon.trilinear_interpolate(
                values_matprop["tsw"], values_matprop["tse"], values_matprop["tnw"],
                values_matprop["tne"], values_matprop["bsw"], values_matprop["bse"],
                values_matprop["bnw"], values_matprop["bne"],
                percentages["x"], percentages["y"], percentages["z"]
            ) * 1000
            v["vp_source"] = "rwg: " + self.source

            for key in values.keys():
                fin_vs.seek(values[key] * 4)
                values_matprop[key] = struct.unpack("<f", fin_vs.read(4))[0]
            v["vs"] = UCVMCCommon.trilinear_interpolate(
                values_matprop["tsw"], values_matprop["tse"], values_matprop["tnw"],
                values_matprop["tne"], values_matprop["bsw"], values_matprop["bse"],
                values_matprop["bnw"], values_matprop["bne"],
                percentages["x"], percentages["y"], percentages["z"]
            ) * 1000
            v["vs_source"] = "rwg: " + self.source

            for key in values.keys():
                fin_dn.seek(values[key] * 4)
                values_matprop[key] = struct.unpack("<f", fin_dn.read(4))[0]
            v["density"] = UCVMCCommon.trilinear_interpolate(
                values_matprop["tsw"], values_matprop["tse"], values_matprop["tnw"],
                values_matprop["tne"], values_matprop["bsw"], values_matprop["bse"],
                values_matprop["bnw"], values_matprop["bne"],
                percentages["x"], percentages["y"], percentages["z"]
            ) * 1000
            v["density_source"] = "rwg: " + self.source

            data[i].set_velocity_data(
                VelocityProperties(
                    v["vp"], v["vs"], v["density"], None, None,
                    v["vp_source"], v["vs_source"], v["density_source"], None, None
                )
            )

    def _etree_query(self, data: List[SeismicData]):
        obj = os.path.join(self.data_dir, self.source + ".e").encode("ASCII")
        etree = UCVMCCommon.c_etree_open(obj, 0)
        metadata = UCVMCCommon.c_etree_getappmeta(etree)
        dims = (metadata["dims"][0], metadata["dims"][1], metadata["dims"][2])
        ticks = (metadata["ticks"][0], metadata["ticks"][1], metadata["ticks"][2])

        for datum in data:
            properties = UCVMCCommon.c_etree_query(etree, datum.converted_point.x_value,
                                                   datum.converted_point.y_value,
                                                   datum.converted_point.z_value, self.corners,
                                                   dims, ticks)
            if properties is not None:
                datum.set_velocity_data(
                    VelocityProperties(properties[0], properties[1], properties[2], None, None,
                                       "etree: " + self.source, "etree: " + self.source,
                                       "etree: " + self.source, None, None)
                )
            else:
                self._set_velocity_properties_none(datum)

        UCVMCCommon.c_etree_close(etree)

    def _query(self, data: List[SeismicData], **kwargs):
        """
        This is the method that all models override. It handles querying the velocity model
        and filling in the SeismicData structures.

        Args:
            points (:obj:`list` of :obj:`SeismicData`): List of SeismicData objects containing the
                points in depth. These are to be populated with :obj:`VelocityProperties`:

        Returns:
            True on success, false if there is an error.
        """
        if "params" not in kwargs:
            display_and_raise_error(21)

        with open(kwargs["params"], "r") as fd:
            xml_in = xmltodict.parse(fd.read())
            self.data_dir = os.path.dirname(kwargs["params"])

        self._initialize(xml_in["root"])

        if xml_in["root"]["format"] == "awp":
            self._awp_query(data)
        elif xml_in["root"]["format"] == "rwg":
            self._rwg_query(data)
        elif xml_in["root"]["format"] == "etree":
            self._etree_query(data)
