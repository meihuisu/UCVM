"""
Defines the base (inheritable) class that describes a standard gridded velocity model. This
inherits from VelocityModel. This reads configuration information from data/config.xml. It assumes
that the model projection was originally in UTM. It also automatically calculates rotation and
trilinearly interpolates if need be.

Copyright:
    Southern California Earthquake Center

Developer:
    David Gill <davidgil@usc.edu>
"""
# Python Imports
import time
import os
import math
from typing import List

# Package Imports
import xmltodict
import tables

# UCVM Imports
from ucvm.src.model.velocity.velocity_model import VelocityModel
from ucvm.src.shared.properties import SeismicData, VelocityProperties
from ucvm.src.shared.errors import display_and_raise_error
from ucvm.src.shared.functions import calculate_nafe_drake_density
from ucvm.src.shared.constants import UCVM_DEFAULT_PROJECTION

from ucvm_c_common import UCVMCCommon


class GriddedVelocityModel(VelocityModel):
    """
    Defines the gridded velocity model class which is currently being used to serve up Po and
    En-Jui's models but it could be used to serve up any kind of discretized velocity model in
    the future.
    """

    def __init__(self, **kwargs):
        super().__init__(**kwargs)

        # Let's read in the configuration file.
        if not os.path.exists(os.path.join(self.model_location, "data", "config.xml")):
            display_and_raise_error(18)

        with open(os.path.join(self.model_location, "data", "config.xml"), "r") as xml_file:
            self.config_dict = xmltodict.parse(xml_file.read())
            self.config_dict = self.config_dict["root"]

        angle = math.atan(
            (float(self.config_dict["corners"]["top_left"]["e"]) -
             float(self.config_dict["corners"]["bottom_left"]["e"])) /
            (float(self.config_dict["corners"]["top_left"]["n"]) -
             float(self.config_dict["corners"]["bottom_left"]["n"]))
        )

        self.config_dict["zone"] = int(self.config_dict["zone"])
        self.config_dict["dimensions"]["depth"] = float(self.config_dict["dimensions"]["depth"])
        self.config_dict["dimensions"]["x"] = int(self.config_dict["dimensions"]["x"])
        self.config_dict["dimensions"]["y"] = int(self.config_dict["dimensions"]["y"])
        self.config_dict["dimensions"]["z"] = int(self.config_dict["dimensions"]["z"])
        self.config_dict["dimensions"]["z_interval"] = int(self.config_dict["dimensions"]["z_interval"])

        self.model_properties = {
            "angles": {
                "cos": math.cos(angle),
                "sin": math.sin(angle)
            },
            "dimensions": {
                "width":
                    math.sqrt(
                        math.pow(float(self.config_dict["corners"]["top_right"]["n"]) -
                                 float(self.config_dict["corners"]["top_left"]["n"]), 2.0) +
                        math.pow(float(self.config_dict["corners"]["top_right"]["e"]) -
                                 float(self.config_dict["corners"]["top_left"]["e"]), 2.0)
                    ),
                "height":
                    math.sqrt(
                        math.pow(float(self.config_dict["corners"]["top_left"]["n"]) -
                                 float(self.config_dict["corners"]["bottom_left"]["n"]), 2.0) +
                        math.pow(float(self.config_dict["corners"]["top_left"]["e"]) -
                                 float(self.config_dict["corners"]["bottom_left"]["e"]), 2.0)
                    )
            },
            "origin": {
                "e": float(self.config_dict["corners"]["bottom_left"]["e"]),
                "n": float(self.config_dict["corners"]["bottom_left"]["n"])
            }
        }

        self.model_properties["spacing"] = {
            "x": self.model_properties["dimensions"]["width"] / (
                float(self.config_dict["dimensions"]["x"]) - 1),
            "y": self.model_properties["dimensions"]["height"] / (
                float(self.config_dict["dimensions"]["y"]) - 1)
        }

    def _query(self, points: List[SeismicData], **kwargs) -> bool:
        """
        This is the method that all models override. It handles querying the velocity model
        and filling in the SeismicData structures.

        Args:
            points (:obj:`list` of :obj:`SeismicData`): List of SeismicData objects containing the
                points in depth. These are to be populated with :obj:`VelocityProperties`:

        Returns:
            True on success, false if there is an error.
        """
        stime = time.time()

        self._opened_file = tables.open_file(
            os.path.join(self.get_model_dir(), "data", self._public_metadata["id"] + ".dat"), "r"
        )

        model_has = []
        if hasattr(self._opened_file.root, "vp"):
            model_has.append("vp")
        if hasattr(self._opened_file.root, "vs"):
            model_has.append("vs")
        if hasattr(self._opened_file.root, "density"):
            model_has.append("density")

        mesh = {}
        for m in model_has:
            mesh[m] = {}

        for sd_object in points:

            x_value = sd_object.converted_point.x_value
            y_value = sd_object.converted_point.y_value
            if sd_object.converted_point.projection == UCVM_DEFAULT_PROJECTION:
                # If we are given this in not UTM, that's the sign to use the Fortran code.
                x_value, y_value = UCVMCCommon.fortran_convert_ll_utm(sd_object.converted_point.x_value,
                                                                      sd_object.converted_point.y_value,
                                                                      self.config_dict["zone"])

            temp_utm_e = x_value - self.model_properties["origin"]["e"]
            temp_utm_n = y_value - self.model_properties["origin"]["n"]

            new_point_utm_n = self.model_properties["angles"]["sin"] * temp_utm_e + \
                self.model_properties["angles"]["cos"] * temp_utm_n
            new_point_utm_e = self.model_properties["angles"]["cos"] * temp_utm_e - \
                self.model_properties["angles"]["sin"] * temp_utm_n

            coords, percentages = UCVMCCommon.calculate_grid_point(
                self.model_properties["dimensions"]["width"],
                self.model_properties["dimensions"]["height"],
                self.config_dict["dimensions"]["depth"],
                new_point_utm_e,
                new_point_utm_n,
                sd_object.converted_point.z_value,
                self.config_dict["dimensions"]["x"],
                self.config_dict["dimensions"]["y"],
                self.config_dict["dimensions"]["z_interval"]
            )

            # Check to see if we are outside of the model boundaries.
            if coords["x"] < 0 or coords["y"] < 0 or coords["z"] < 0 or \
               coords["x"] > self.config_dict["dimensions"]["x"] - 2 or \
               coords["y"] > self.config_dict["dimensions"]["y"] - 2 or \
               coords["z"] > self.config_dict["dimensions"]["z"] - 1:
                self._set_velocity_properties_none(sd_object)
                continue

            v = VelocityProperties(vp=None, vp_source=None, vs=None, vs_source=None,
                                   density=None, density_source=None, qp=None,
                                   qp_source=None, qs=None, qs_source=None)

            for prop_given in model_has:
                if coords["z"] not in mesh[prop_given]:
                    l = getattr(self._opened_file.root, prop_given)
                    mesh[prop_given][coords["z"]] = l[coords["z"], :, :]
                if coords["z"] - 1 not in mesh[prop_given]:
                    l = getattr(self._opened_file.root, prop_given)
                    mesh[prop_given][coords["z"] - 1] = l[coords["z"] - 1, :, :]

                if coords["z"] + 1 in mesh[prop_given]:
                    del mesh[prop_given][coords["z"] + 1]

                # Interpolate
                p = getattr(v, prop_given)
                p = UCVMCCommon.trilinear_interpolate(
                    mesh[prop_given][coords["z"]][coords["x"]][coords["y"]],
                    mesh[prop_given][coords["z"]][coords["x"] + 1][coords["y"]],
                    mesh[prop_given][coords["z"]][coords["x"]][coords["y"] + 1],
                    mesh[prop_given][coords["z"]][coords["x"] + 1][coords["y"] + 1],
                    mesh[prop_given][coords["z"] - 1][coords["x"]][coords["y"]],
                    mesh[prop_given][coords["z"] - 1][coords["x"] + 1][coords["y"]],
                    mesh[prop_given][coords["z"] - 1][coords["x"]][coords["y"] + 1],
                    mesh[prop_given][coords["z"] - 1][coords["x"] + 1][coords["y"] + 1],
                    percentages["x"], percentages["y"], percentages["z"]
                )
                p = getattr(v, prop_given + "_source")
                p = self.get_metadata()["id"]

            if v.density is None and v.vp is not None:
                v.density = calculate_nafe_drake_density(v.vp)

            sd_object.set_velocity_data(v)

        self._opened_file.close()
