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
from ucvm.src.shared.properties import SeismicData


class ElevationModel(Model):

    def _query(self, points: List[SeismicData], **kwargs) -> List[ElevationProperties]:
        """
        Internal (override) query method for the model.
        :param list points: A list of Point classes.
        :return: A list of SeismicData classes.
        """
        pass
