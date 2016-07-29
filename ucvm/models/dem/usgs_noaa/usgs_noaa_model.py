"""
Defines the USGS/NOAA DEM. This elevation data is from the data sampled at 1/3 arc-second
resolution directly from the USGS website: nationalmap.gov.

:copyright: Southern California Earthquake Center
:author:    David Gill <davidgil@usc.edu>
:created:   July 19, 2016
:modified:  July 19, 2016
"""

import math
from typing import List

import h5py
import numpy as np
import os

from ucvm.src.model.elevation.elevation_model import ElevationModel
from ucvm.src.shared import ElevationProperties, SimplePoint, SimpleRotatedRectangle, \
                            calculate_bilinear_value
from ucvm.src.shared.properties import SeismicData


class USGSNOAAElevationModel(ElevationModel):

    _public_metadata = {
        "id": "usgs_noaa",
        "name": "USGS/NOAA Digital Elevation Model",
        "description": "This elevation data is from the data sampled at 1/3 arc-second "
                       "resolution directly from the USGS website: nationalmap.gov.",
        "website": "http://www.nationalmap.gov",
        "references": ["The National Map (http://www.nationalmap.gov)"]
    }   #: dict: Describes the public metadata associated with this elevation model.

    _private_metadata = {
        "projection": "+proj=latlong +datum=NAD83 +ellps=GRS80",
        "public": True
    }   #: dict: Describes the private metadata (such as projection) associated with this model.

    DATA_FILE = "usgs_noaa_dem.dat"                 #: str: The location of the DEM data file.
    _opened_file = None                             #: h5py.File: The opened HDF5 file.

    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self._opened_file = h5py.File(os.path.join(self.get_model_dir(), "data", self.DATA_FILE),
                                      "r")

    def _query(self, data: List[SeismicData]) -> bool:
        """
        Internal (override) query method for the model.
        :param list data: A list of SeismicData classes to fill in with elevation properties.
        :return: True if function was successful, false if not.
        """
        for datum in data:
            bucket = "/dem/" + str(math.floor(datum.converted_point.x_value)) + \
                     "/" + str(math.floor(datum.converted_point.y_value) - 1)
            group = self._opened_file.get(bucket)

            if group is None:
                datum.set_elevation_data(ElevationProperties(None, "no data"))
                continue

            opened_data = np.array(group.get("data"))

            bilinear_point = SimplePoint(datum.converted_point.x_value,
                                         datum.converted_point.y_value,
                                         0)

            bilinear_rect = SimpleRotatedRectangle(group.attrs.get("x lower left corner"),
                                                   group.attrs.get("y lower left corner"),
                                                   0,
                                                   group.attrs.get("cell size"),
                                                   group.attrs.get("cell size"))

            datum.set_elevation_data(
                ElevationProperties(
                    calculate_bilinear_value(bilinear_point, bilinear_rect, opened_data),
                    self._public_metadata["id"]
                )
            )

        return True

    def __del__(self):
        self._opened_file.close()
