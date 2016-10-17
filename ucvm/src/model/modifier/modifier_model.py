"""
Defines the base (inheritable) class that describes a modifier "model". This inherits from
the model class and various models (such as the Ely GTL) inherit from this.

:copyright: Southern California Earthquake Center
:author:    David Gill <davidgil@usc.edu>
:created:   October 13, 2016
:modified:  October 13, 2016
"""
import os
import xmltodict

from typing import List

from ucvm.src.model.model import Model
from ucvm.src.shared.properties import SeismicData


class ModifierModel(Model):

    def __init__(self, **kwargs):
        super().__init__(**kwargs)

        # Get the metadata.
        with open(os.path.join(self.model_location, "ucvm_model.xml")) as fd:
            doc = xmltodict.parse(fd.read())

        self._private_metadata["depends"] = {}

        try:
            self._private_metadata["depends"]["velocity"] = \
                doc["root"]["internal"]["depends"]["velocity"]
        except KeyError:
            self._private_metadata["depends"]["velocity"] = None

        try:
            self._private_metadata["depends"]["elevation"] = \
                doc["root"]["internal"]["depends"]["elevation"]
        except KeyError:
            self._private_metadata["depends"]["elevation"] = None

        try:
            self._private_metadata["depends"]["vs30"] = \
                doc["root"]["internal"]["depends"]["vs30"]
        except KeyError:
            self._private_metadata["depends"]["vs30"] = None

    def _query(self, points: List[SeismicData], **kwargs) -> bool:
        """
        Internal (override) query method for the model.
        :param list points: A list of Point classes.
        :return: A list of SeismicData classes.
        """
        pass
