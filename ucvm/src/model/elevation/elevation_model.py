"""
Defines the base (inheritable) class that describes a digital elevation model. This inherits from
the model class and various bundled elevation models (such as the USGS/NOAA DEM) inherit from this
class.

:copyright: Southern California Earthquake Center
:author:    David Gill <davidgil@usc.edu>
:created:   July 19, 2016
:modified:  July 19, 2016
"""

from typing import List

from ucvm.src.model.model import Model
from ucvm.src.shared import ElevationProperties


class ElevationModel(Model):

    def _query(self, points) -> List[ElevationProperties]:
        """
        Internal (override) query method for the model.
        :param list points: A list of Point classes.
        :return: A list of SeismicData classes.
        """
        pass

    @staticmethod
    def get_all_models() -> List[str]:
        """
        Get all models of this type. So if we call VelocityModel.get_all_models() we get all
        velocity models registered with UCVM.
        :return: A list of string identifiers for each model.
        """
        pass
