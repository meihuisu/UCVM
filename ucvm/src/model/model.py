"""
Defines the base (inheritable) class that describes a model. Other classes can inherit from this
and expand upon it. A model is intrinsically very generic. VelocityModel, ElevationModel, and
Vs30Model all inherit from this base class. Eventually we may add attenuation models into
UCVM as well.

:copyright: Southern California Earthquake Center
:author:    David Gill <davidgil@usc.edu>
:created:   July 6, 2016
:modified:  July 18, 2016
"""

import xmltodict
import inspect
import os

from abc import abstractmethod
from typing import List

from ucvm.src.shared import UCVM_DEFAULT_PROJECTION, UCVM_DEPTH, UCVM_ELEVATION, UCVM_ELEV_ANY
from ucvm.src.shared.properties import SeismicData


class Model:
    """
    The model class provides the foundation from which all models inherit. UCVM expects all models,
    including the digital elevation model and Vs30 to conform to this abstract class.
    """
    def __init__(self, **kwargs):
        self._public_metadata = {
            "id": None,
            "name": None,
            "description": None,
            "website": None,
            "type": None,
            "references": [],
            "license": None,
            "coverage": {
                "description": None,
                "bottom_left": {
                    "e": None,
                    "n": None
                },
                "top_right": {
                    "e": None,
                    "n": None
                }
            }
        }

        self._private_metadata = {
            "projection": UCVM_DEFAULT_PROJECTION,
            "public": True,
            "defaults": {},
            "query_by": UCVM_DEPTH,
            "class": None,
            "file": None
        }

        if "model_location" in kwargs:
            self.model_location = kwargs["model_location"]
        else:
            self.model_location = os.path.join(os.path.dirname(inspect.getfile(self.__class__)))

        # Get the metadata.
        with open(os.path.join(self.model_location, "ucvm_model.xml")) as fd:
            doc = xmltodict.parse(fd.read())

        self._public_metadata["id"] = doc["root"]["information"]["id"]
        self._public_metadata["name"] = doc["root"]["information"]["identifier"]
        self._public_metadata["description"] = doc["root"]["information"]["description"]
        self._public_metadata["type"] = doc["root"]["information"]["type"]

        try:
            self._public_metadata["website"] = doc["root"]["information"]["website"]
        except KeyError:
            pass

        try:
            self._public_metadata["license"] = doc["root"]["information"]["license"]
        except KeyError:
            pass

        try:
            for _, value in doc["root"]["information"]["references"].items():
                if type(value) is list:
                    self._public_metadata["references"] += value
                else:
                    self._public_metadata["references"].append(value)
        except KeyError:
            pass

        try:
            self._public_metadata["coverage"]["description"] = \
                doc["root"]["information"]["coverage"]["description"]
            self._public_metadata["coverage"]["bottom_left"]["e"] = \
                doc["root"]["information"]["coverage"]["bottom-left"]["e"]
            self._public_metadata["coverage"]["bottom_left"]["n"] = \
                doc["root"]["information"]["coverage"]["bottom-left"]["n"]
            self._public_metadata["coverage"]["top_right"]["e"] = \
                doc["root"]["information"]["coverage"]["top-right"]["e"]
            self._public_metadata["coverage"]["top_right"]["n"] = \
                doc["root"]["information"]["coverage"]["top-right"]["n"]
        except KeyError:
            pass

        try:
            if str(doc["root"]["internal"]["projection"]).lower().strip() != "default":
                self._private_metadata["projection"] = doc["root"]["internal"]["projection"]
                if self._private_metadata["projection"] == "DEFAULT":
                    self._private_metadata["projection"] = UCVM_DEFAULT_PROJECTION
        except KeyError:
            pass

        try:
            if str(doc["root"]["internal"]["public"]).lower().strip() != "yes":
                self._private_metadata["public"] = False
        except KeyError:
            pass

        try:
            if str(doc["root"]["internal"]["query_by"]).lower().strip() == "depth":
                self._private_metadata["query_by"] = UCVM_DEPTH
            elif str(doc["root"]["internal"]["query_by"]).lower().strip() == "elevation":
                self._private_metadata["query_by"] = UCVM_ELEVATION
            elif str(doc["root"]["internal"]["query_by"]).lower().strip() == "any":
                self._private_metadata["query_by"] = UCVM_ELEV_ANY
        except KeyError:
            pass

        try:
            self._private_metadata["defaults"] = doc["root"]["internal"]["defaults"]
        except KeyError:
            pass

        self._private_metadata["class"] = doc["root"]["internal"]["class"]
        self._private_metadata["file"] = doc["root"]["internal"]["file"]

    def query(self, data: List[SeismicData], **kwargs) -> bool:
        """
        Queries the model and adds in the necessary data.
        :param list data: A list of SeismicData classes that contain Points to query.
        :return: A list of SeismicData classes.
        """
        if not isinstance(data, List[SeismicData]):
            raise TypeError("Points parameter must be a list of Point classes.")

        for datum in data:
            datum.convert_point_to_projection(self._private_metadata["projection"])
            datum.set_point_to_depth_or_elev(self._private_metadata["query_by"])

        # Now that we have converted all of the points to the model projection, let's pass them
        # into the model and retrieve back the properties.
        return self._query(data, **kwargs)

    def get_metadata(self):
        """
        Returns the array containing the metadata (id, name, description, etc.).
        :return: The metadata array.
        """
        return self._public_metadata

    def get_private_metadata(self, key: str) -> object:
        """
        Returns the model type.
        :param str key: The key in the private metadata to search for.
        :return: The model type (VELOCITY, ELEVATION, or VS30).
        """
        if key in self._private_metadata:
            return self._private_metadata[key]
        else:
            return None

    def get_model_dir(self) -> str:
        return os.path.dirname(inspect.getfile(self.__class__))

    @abstractmethod
    def _query(self, data: List[SeismicData], **kwargs) -> bool:
        """
        Internal (override) query method for the model.
        :param list data: A list of SeismicData classes.
        :return: True, if the query was successful. False if not.
        """
        pass

    def __str__(self):
        """
        Defines how the model should be described if asked to be outputted as a string.
        :return: A text description of the model.
        """
        ret_str = ""

        if self._public_metadata["id"] is not None:
            ret_str += "Model ID: %s\n" % self._public_metadata["id"]

        if self._public_metadata["name"] is not None:
            ret_str += "Model Name: %s\n" % self._public_metadata["name"]

        if self._public_metadata["description"] is not None:
            ret_str += "Description: %s\n" % " ".join(self._public_metadata["description"].split())

        if self._public_metadata["website"] is not None:
            ret_str += "Website: %s\n" % self._public_metadata["website"]

        if self._public_metadata["references"] is not None and \
           len(self._public_metadata["references"]) > 0:
            ret_str += "References: \n"
            for reference in self._public_metadata["references"]:
                ret_str += "\t- " + " ".join(reference.split()) + "\n"

        return ret_str
