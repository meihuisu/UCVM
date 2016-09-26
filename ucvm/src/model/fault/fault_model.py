"""
Provides the definition of a fault. Eventually this can be expanded to include ruptures, etc. but
for now it just specifies the fault latitude and longitude points. The faults are hard-coded in here
for expediency but they will be modularized in the future.

:copyright: Southern California Earthquake Center
:author:    David Gill <davidgil@usc.edu>
:created:   September 20, 2016
:modified:  September 20, 2016
"""
from typing import List


class Fault:

    def __init__(self):
        self.FAULTS = {
            "saf": {
                "long_name": "San Andreas Fault",
                "coordinates": """
                    -124.0692, 40.0215
                    -123.6931, 38.9088
                    -122.7984, 38.0440
                    -122.4702, 37.6879
                    -121.5380, 36.8455
                    -120.4327, 35.8997
                    -120.2965, 35.7239
                    -119.4005, 34.9342
                    -118.9448, 34.8228
                    -118.1165, 34.5794
                    -117.2898, 34.1083
                    -116.5017, 33.9611
                    -115.9181, 33.5088
                """
            }
        }

    def get_fault_by_id(self, name: str) -> List[tuple]:
        if name in self.FAULTS:
            ret_list = []
            for line in self.FAULTS[name]["coordinates"].splitlines(keepends=False):
                if line.strip() == "":
                    continue
                split_strip_line = [x.strip() for x in line.split(",")]
                ret_list.append((float(split_strip_line[0]), float(split_strip_line[1])))
            return ret_list
        else:
            return None

    def get_all_faults(self) -> dict:
        ret_dict = {}
        for name in self.FAULTS:
            ret_dict[name] = []
            for line in self.FAULTS[name]["coordinates"].splitlines(keepends=False):
                if line.strip() == "":
                    continue
                split_strip_line = [x.strip() for x in line.split(",")]
                ret_dict[name].append((float(split_strip_line[0]), float(split_strip_line[1])))
        return ret_dict
