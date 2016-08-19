"""
Defines the base (inheritable) class that describes a Vs30 model. Example models include the Wills-
Wald map and a model that calculates the Vs30 value based on the model's value.

:copyright: Southern California Earthquake Center
:author:    David Gill <davidgil@usc.edu>
:created:   July 19, 2016
:modified:  July 19, 2016
"""

from typing import List

from ucvm.src.model.model import Model
from ucvm.src.shared.properties import SeismicData


class Vs30Model(Model):

    def __init__(self, **kwargs):
        super().__init__(**kwargs)

    def _query(self, data: List[SeismicData], **kwargs) -> bool:
        """
        Internal (override) query method for the model.
        :param list data: A list of SeismicData classes.
        :return: Returns true on success, false on fail.
        """
        pass
