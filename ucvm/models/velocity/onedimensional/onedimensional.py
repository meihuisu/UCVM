"""
1D Velocity Model

Defines a 1D model which can either be in BBP 1D format or a list. The model file location (or if it
is an included model, just the name) is provided as a parameter.

Copyright:
    Southern California Earthquake Center

Developer:
    David Gill <davidgil@usc.edu>
"""
# Python Imports
import os
from typing import List

# Package Imports
import xmltodict

# UCVM Imports
from ucvm.src.model.velocity.velocity_model import VelocityModel
from ucvm.src.shared import VelocityProperties
from ucvm.src.shared.properties import SeismicData
from ucvm.src.shared.functions import is_number, calculate_scaled_vs, calculate_scaled_density
from ucvm.src.shared.errors import display_and_raise_error


class OneDimensionalVelocityModel(VelocityModel):

    def __init__(self, **kwargs):
        super().__init__(**kwargs)

    @classmethod
    def _parse_bbp_model(cls, text_info: str, layers: list) -> None:
        """
        Parses the BBP model format for material properties to return to UCVM.

        Args:
            text_info (str): The text representation of the BBP model.
            layers (list): The layers list that after running this function will contain the
                           material properties.

        Returns:
            Nothing
        """
        current_depth = 0
        for line in text_info.splitlines(keepends=False):
            split_str = line.split()
            if len(split_str) == 0:
                continue
            if is_number(split_str[0]):
                # Add this line.
                p_velocity = float(split_str[1]) * 1000
                density = float(split_str[3]) * 1000 if len(split_str) > 3 else \
                    calculate_scaled_density(p_velocity)
                s_velocity = float(split_str[2]) * 1000 if len(split_str) > 2 else \
                    calculate_scaled_vs(p_velocity, density)
                qp_attenuation = float(split_str[4]) if len(split_str) > 4 else None
                qs_attenuation = float(split_str[5]) if len(split_str) > 5 else None
                layers.append([current_depth, p_velocity, s_velocity, density, qp_attenuation,
                               qs_attenuation])
                current_depth += float(split_str[0]) * 1000

        layers.append(layers[-1])

    @classmethod
    def _parse_scec_model(cls, data_info: dict, layers: list) -> None:
        """
        Parses the SCEC model format for material properties to return to UCVM.

        Args:
            data_info (dict): The dictionary representation of the BBP model.
            layers (list): The layers list that after running this function will contain the
                           material properties.

        Returns:
            Nothing
        """
        layer_dict = {}
        for layer in data_info["layers"]["layer"]:
            layer_dict[int(layer["@number"])] = {
                "depth": float(layer["depth"]),
                "vp": float(layer["vp"]) if "vp" in layer else None,
                "vs": float(layer["vs"]) if "vs" in layer else None,
                "density": float(layer["density"]) if "density" in layer else None,
                "qp": float(layer["qp"]) if "qp" in layer else None,
                "qs": float(layer["qs"]) if "qs" in layer else None
            }
        last_depth = 0
        for i in range(1, len(layer_dict) + 1):
            p_velocity = layer_dict[i]["vp"] * 1000
            density = layer_dict[i]["density"] * 1000 if layer_dict[i]["density"] is not None \
                      else calculate_scaled_density(p_velocity)
            s_velocity = layer_dict[i]["vs"] * 1000 if layer_dict[i]["vs"] is not None \
                         else calculate_scaled_vs(p_velocity, density)
            qp_attenuation = layer_dict[i]["qp"]
            qs_attenuation = layer_dict[i]["qs"]
            layers.append([last_depth, p_velocity, s_velocity, density, qp_attenuation,
                           qs_attenuation])
            last_depth = layer_dict[i]["depth"] * 1000

        layers.append(layers[-1])

    @classmethod
    def _get_velocity_data(cls, depth: float, layers: list, interpolate: bool, name: str) \
            -> VelocityProperties:
        """
        Given a list of layers and depth, this function returns the velocity data for the
        1D model.

        Args:
            depth (float): The depth for which we want the properties.
            layers (list): The layers of the velocity model.
            interpolate (bool): True if we should use linear interpolation, false if not.
            name (str): The name of the model.

        Returns:
            The :obj:`VelocityProperties` for this model at the specified depth.
        """
        previous_layer = None
        current_layer = None
        for layer in layers:
            previous_layer = current_layer
            current_layer = layer
            if layer[0] > depth:
                break

        name_to_use = name if not interpolate else name + " (interpolated)"

        if previous_layer == current_layer:
            # This is the last layer so return the properties.
            return VelocityProperties(current_layer[1], current_layer[2], current_layer[3],
                                      current_layer[4], current_layer[5], name_to_use, name_to_use,
                                      name_to_use, name_to_use, name_to_use)

        # Otherwise, we need to either return the properties of the previous layer if no
        # interpolation or we need to interpolate.
        if interpolate:
            percentage = (depth - previous_layer[0]) / (current_layer[0] - previous_layer[0])
            return VelocityProperties(
                percentage * (current_layer[1] - previous_layer[1]) + previous_layer[1]
                if previous_layer[1] is not None else None,
                percentage * (current_layer[2] - previous_layer[2]) + previous_layer[2]
                if previous_layer[2] is not None else None,
                percentage * (current_layer[3] - previous_layer[3]) + previous_layer[3]
                if previous_layer[3] is not None else None,
                percentage * (current_layer[4] - previous_layer[4]) + previous_layer[4]
                if previous_layer[4] is not None else None,
                percentage * (current_layer[5] - previous_layer[5]) + previous_layer[5]
                if previous_layer[5] is not None else None,
                name_to_use, name_to_use, name_to_use, name_to_use, name_to_use
            )
        else:
            return VelocityProperties(previous_layer[1], previous_layer[2], previous_layer[3],
                                      previous_layer[4], previous_layer[5], name_to_use,
                                      name_to_use, name_to_use, name_to_use, name_to_use)

    def _query(self, data: List[SeismicData], **kwargs) -> bool:
        """
        This is the method that all models override. It handles querying the velocity model
        and filling in the SeismicData structures.

        Args:
            points (:obj:`list` of :obj:`SeismicData`): List of SeismicData objects containing the
                points in depth. These are to be populated with :obj:`VelocityProperties`:

        Returns:
            True on success, false if there is an error.
        """
        xml_file = None
        interpolation = None

        if "params" in kwargs:
            # See if the user has asked for interpolation.
            split_str = [x.strip().lower() for x in str(kwargs["params"]).split(",")]
            if len(split_str) > 1:
                if "linear" in split_str:
                    interpolation = "linear"
                    split_str.remove("linear")
                elif "none" in split_str:
                    interpolation = "none"
                    split_str.remove("none")

                paths = [
                    os.path.join(".", kwargs["params"].strip().split(",")[0] + ".mdl"),
                    os.path.join(self.model_location, "data", kwargs["params"].strip().split(",")[0] + ".mdl")
                ]
            else:
                # Let's try and find the model.
                paths = [
                    os.path.join(".", kwargs["params"] + ".mdl"),
                    os.path.join(self.model_location, "data", kwargs["params"] + ".mdl")
                ]

        else:
            kwargs["params"] = "SCEC"
            paths = [
                os.path.join(self.model_location, "data", "SCEC.mdl")
            ]

        for path in paths:
            if os.path.exists(path):
                xml_file = path
                break

        if xml_file is None:
            display_and_raise_error(13, (kwargs["params"],))

        with open(xml_file, "r") as fd:
            model_1d = xmltodict.parse(fd.read())

        parsed_model = {
            "name": str(model_1d["root"]["name"]).strip().lower(),
            "format": str(model_1d["root"]["format"]).strip().lower(),
            "interpolation": str(model_1d["root"]["interpolation"]).strip().lower()
                             if interpolation is None else interpolation,
            "layers": []
        }

        if parsed_model["format"] == "bbp":
            self._parse_bbp_model(str(model_1d["root"]["data"]), parsed_model["layers"])
        else:
            self._parse_scec_model(model_1d["root"]["data"], parsed_model["layers"])

        for i in range(0, len(data)):
            if data[i].converted_point.z_value < 0:
                self._set_velocity_properties_none(data[i])
            else:
                data[i].set_velocity_data(
                    self._get_velocity_data(data[i].converted_point.z_value, parsed_model["layers"],
                                            parsed_model["interpolation"] == "linear",
                                            parsed_model["name"])
                )

        return True
