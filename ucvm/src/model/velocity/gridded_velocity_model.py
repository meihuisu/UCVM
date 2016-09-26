"""
Defines the base (inheritable) class that describes a standard gridded velocity model. This
inherits from VelocityModel. This reads configuration information from data/config.xml. It assumes
that the model projection was originally in UTM. It also automatically calculates rotation and
trilinearly interpolates if need be.

:copyright: Southern California Earthquake Center
:author:    David Gill <davidgil@usc.edu>
:created:   August 31, 2016
:modified:  August 31, 2016
"""

import os
import xmltodict
import math

from typing import List

import numpy as np

from ucvm.src.model.velocity.velocity_model import VelocityModel
from ucvm.src.shared.properties import SeismicData, VelocityProperties
from ucvm.src.shared.errors import display_and_raise_error


class GriddedVelocityModel(VelocityModel):

    def __init__(self, **kwargs):
        super().__init__(**kwargs)

        self.material_properties = {
            "vp": None,
            "vs": None,
            "density": None,
            "qp": None,
            "qs": None
        }

        # Let's read in the configuration file.
        if not os.path.exists(os.path.join(self.model_location, "data", "config.xml")):
            display_and_raise_error(18)

        with open(os.path.join(self.model_location, "data", "config.xml"), "r") as xml_file:
            self.config_dict = xmltodict.parse(xml_file.read())
            self.config_dict = self.config_dict["root"]

        # See what data we can serve.
        for key in self.material_properties:
            if os.path.exists(os.path.join(self.model_location, "data",
                                           self.config_dict["model"], key + ".dat")):
                # Let's load this into memory.
                with open(os.path.join(self.model_location, "data", self.config_dict["model"],
                                       key + ".dat"), "rb") as data_file:
                    self.material_properties[key] = np.load(data_file)

        angle = math.atan(
            (float(self.config_dict["corners"]["top_left"]["e"]) -
             float(self.config_dict["corners"]["bottom_left"]["e"])) /
            (float(self.config_dict["corners"]["top_left"]["n"]) -
             float(self.config_dict["corners"]["bottom_left"]["n"]))
        )

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

    def _query(self, data: List[SeismicData], **kwargs) -> bool:
        """
        Internal (override) query method for the model.
        :param list data: A list of SeismicData classes.
        :return: True on success, false on failure.
        """
        for sd_object in data:
            temp_utm_e = sd_object.converted_point.x_value - self.model_properties["origin"]["e"]
            temp_utm_n = sd_object.converted_point.y_value - self.model_properties["origin"]["n"]
            new_point_utm_n = self.model_properties["angles"]["sin"] * temp_utm_e + \
                self.model_properties["angles"]["cos"] * temp_utm_n
            new_point_utm_e = self.model_properties["angles"]["cos"] * temp_utm_e - \
                self.model_properties["angles"]["sin"] * temp_utm_n
            coords = {
                "x": int(math.floor(
                     new_point_utm_e / self.model_properties["dimensions"]["width"] *
                     (int(self.config_dict["dimensions"]["x"]) - 1))
                ),
                "y": int(math.floor(
                     new_point_utm_n / self.model_properties["dimensions"]["height"] *
                     (int(self.config_dict["dimensions"]["y"]) - 1))
                ),
                "z": int((float(self.config_dict["dimensions"]["depth"]) / float(
                    self.config_dict["dimensions"]["z_interval"]) - 1) - math.floor(
                    sd_object.converted_point.z_value /
                    float(self.config_dict["dimensions"]["z_interval"])))
            }

            # Check to see if we are outside of the model boundaries.
            if coords["x"] < 0 or coords["y"] < 0 or \
               coords["x"] > int(self.config_dict["dimensions"]["x"]) - 2 or \
               coords["y"] > int(self.config_dict["dimensions"]["y"]) - 2:
                self._set_velocity_properties_none(sd_object)
                continue

            # Get percentages for trilinear interpolation.
            percentages = {
                "x":
                    math.fmod(new_point_utm_e, self.model_properties["spacing"]["x"]) /
                    self.model_properties["spacing"]["x"],
                "y":
                    math.fmod(new_point_utm_n, self.model_properties["spacing"]["y"]) /
                    self.model_properties["spacing"]["y"],
                "z":
                    math.fmod(sd_object.converted_point.z_value,
                              float(self.config_dict["dimensions"]["z_interval"])) /
                    float(self.config_dict["dimensions"]["z_interval"])
            }

            props_to_return = {
                "vp": None,
                "vp_source": None,
                "vs": None,
                "vs_source": None,
                "density": None,
                "density_source": None,
                "qp": None,
                "qp_source": None,
                "qs": None,
                "qs_source": None
            }

            for key, value in self.material_properties.items():
                if value is not None:
                    props_to_return[key] = self._interp_trilinear(percentages, [
                        value[coords["x"], coords["y"], coords["z"]],
                        value[coords["x"] + 1, coords["y"], coords["z"]],
                        value[coords["x"], coords["y"] + 1, coords["z"]],
                        value[coords["x"] + 1, coords["y"] + 1, coords["z"]],
                        value[coords["x"], coords["y"], coords["z"] - 1],
                        value[coords["x"] + 1, coords["y"], coords["z"] - 1],
                        value[coords["x"], coords["y"] + 1, coords["z"] - 1],
                        value[coords["x"] + 1, coords["y"] + 1, coords["z"] - 1]
                    ])
                    props_to_return[key + "_source"] = self.get_metadata()["id"]

            sd_object.set_velocity_data(
                VelocityProperties(
                    props_to_return["vp"], props_to_return["vs"], props_to_return["density"],
                    props_to_return["qp"], props_to_return["qs"],
                    props_to_return["vp_source"], props_to_return["vs_source"],
                    props_to_return["density_source"], props_to_return["qp_source"],
                    props_to_return["qs_source"]
                )
            )

    def _interp_trilinear(self, percentages: dict, corner_vals: list) -> float:
        """
        Does trilinear interpolation specifically for these types of gridded velocity models.
        :param percentages: The X, Y, and Z percentages for the interpolation.
        :param corner_vals: The corner values. Top layer first, then bottom layer.
        :return: A float representing the trilinearly interpolated material property.
        """
        # Interpolate planes.
        temp = {
            "tp": self._bilinear_interpolation_helper((percentages["x"], percentages["y"]),
                                                      (corner_vals[0], corner_vals[2],
                                                       corner_vals[3], corner_vals[1])),
            "bp": self._bilinear_interpolation_helper((percentages["x"], percentages["y"]),
                                                      (corner_vals[4], corner_vals[6],
                                                       corner_vals[7], corner_vals[5]))
        }
        return self._linear_interpolation_helper(percentages["z"], temp["tp"], temp["bp"])

    def _bilinear_interpolation_helper(self, percents: tuple, plane: tuple) -> float:
        """
        Bilinearly interpolates given X, Y percentages and a plane defined as a tuple starting
        from the bottom-left point and rotating clockwise.
        :param tuple percents: The percentages (X, Y) to interpolate.
        :param tuple plane: The plane, defined as a tuple of floats, starting bottom-left clockwise.
        :return: A float representing the bilinearly interpolated value.
        """
        temp = {
            "bx": self._linear_interpolation_helper(percents[0], plane[0], plane[3]),
            "tx": self._linear_interpolation_helper(percents[0], plane[1], plane[2])
        }
        return self._linear_interpolation_helper(percents[1], temp["bx"], temp["tx"])

    @classmethod
    def _linear_interpolation_helper(cls, percent: float, val1: float, val2: float) -> float:
        """
        Linearly interpolates between val1 and val2. This is then used to form the bilinear
        interpolation, which then forms the trilinear interpolation. Helper function that is hidden.
        :param float percent: The percentage (from 0 to 1) to interpolate.
        :param float val1: The first value.
        :param float val2: The second value.
        :return: The linearly interpolated value.
        """
        if percent > 1:
            percent /= 100
        return (1 - percent) * val1 + percent * val2
