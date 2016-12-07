"""
Handles generating a difference slice or cross-section for UCVM.

Copyright:
    Southern California Earthquake Center

Author:
    David Gill <davidgil@usc.edu>
"""
# UCVM Imports
from ucvm.src.visualization.plot import Plot


class Difference(Plot):

    def __init__(self, definition: dict, **kwargs):
        super().__init__(**kwargs)

    @classmethod
    def from_xml_file(cls, xml_file: str):
        pass

    def extract(self):
        pass
