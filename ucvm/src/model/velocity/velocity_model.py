"""
Defines the base (inheritable) class that describes a velocity model. This inherits from Model and
other classes (such as the LegacyModel class) inherit from this.

:copyright: Southern California Earthquake Center
:author:    David Gill <davidgil@usc.edu>
:created:   July 19, 2016
:modified:  July 19, 2016
"""

from typing import List

from ucvm.src.model.model import Model
from ucvm.src.shared.properties import SeismicData, VelocityProperties


class VelocityModel(Model):
    """
    The model class provides the foundation from which all velocity models inherit. UCVM expects
    velocity models - including the legacy models - to have a Python version.
    """

    def __init__(self, **kwargs):
        super().__init__(**kwargs)

    def _query(self, data: List[SeismicData], **kwargs) -> bool:
        """
        Internal (override) query method for the model.
        :param list data: A list of SeismicData classes.
        :return: True on success, false on failure.
        """
        pass

    @classmethod
    def _set_velocity_properties_none(cls, sd: SeismicData) -> None:
        """
        Helper method to set the SeismicData velocity properties to just be none (i.e. no
        material properties found).
        :param sd: The SeismicData object to set.
        :return: Nothing
        """
        sd.set_velocity_data(
            VelocityProperties(
                None, None, None, None, None,
                None, None, None, None, None
            )
        )
