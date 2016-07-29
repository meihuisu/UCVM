"""
Defines the base "legacy model". Supported legacy models include CVM-S4, CVM-S4.26.M01, and so on.

:copyright: Southern California Earthquake Center
:author:    David Gill <davidgil@usc.edu>
:created:   July 12, 2016
:modified:  July 19, 2016
"""

import xmltodict
import os
import inspect
from abc import abstractmethod
from ctypes import cdll
from typing import List

from ucvm.src.model.velocity.velocity_model import VelocityModel
from ucvm.src.shared.properties import SeismicData


class LegacyVelocityModel(VelocityModel):

    _shared_object = {
        "path": "",
        "object": None,
        "has_initialized": False
    }

    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        with open(os.path.join(os.path.dirname(inspect.getfile(self.__class__)),
                               "ucvm_model.xml")) as fd:
            doc = xmltodict.parse(fd.read())

        if not self._shared_object["has_initialized"]:
            self._shared_object["path"] = os.path.join(
                os.path.dirname(inspect.getfile(self.__class__)),
                "lib",
                str(doc["root"]["build"]["library"]).split("/")[-1] + ".so"
            )
            self._shared_object["object"] = cdll.LoadLibrary(self._shared_object["path"])

    def _query(self, data: List[SeismicData]) -> bool:
        """
        Internal (override) query method for the model.
        :param list data: A list of SeismicData classes.
        :return: True on success, false on failure.
        """
        pass

    @staticmethod
    @abstractmethod
    def get_all_models() -> List[str]:
        """
        Get all models of this type. So if we call VelocityModel.get_all_models() we get all
        velocity models registered with UCVM.
        :return: A list of string identifiers for each model.
        """
        pass
