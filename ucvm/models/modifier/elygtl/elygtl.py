"""
Defines the Ely GTL model modifier.

:copyright: Southern California Earthquake Center
:author:    David Gill <davidgil@usc.edu>
:created:   October 13, 2016
:modified:  October 13, 2016
"""
from typing import List

from ucvm.src.model.modifier import ModifierModel
from ucvm.src.shared.properties import SeismicData


class ElyGTLModifier(ModifierModel):

    def __init__(self, **kwargs):
        super().__init__(**kwargs)

    def _query(self, data: List[SeismicData], **kwargs) -> bool:
        """
        Internal (override) query method for the model.
        :param list data: A list of SeismicData classes to fill in with elevation properties.
        :return: True if function was successful, false if not.
        """
        pass
