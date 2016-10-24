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
from ucvm.src.shared.properties import SeismicData, UCVM_DEFAULT_PROJECTION


class USGSNOAAElevationModel(ElevationModel):

    _public_metadata = {
        "id": "usgs-noaa",
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

        self._bucket_groups = {}

    def _get_nationalmap_data(self, datum: SeismicData) -> bool:
        """
        Attempts to get elevation properties from National Map data.
        :param datum: The data point from which to get elevation properties.
        :return: True, if successful, false if not found.
        """
        bucket = "/dem/nationalmap/" + str(math.floor(datum.converted_point.x_value)) + \
                 "/" + str(math.floor(datum.converted_point.y_value) - 1)

        if bucket not in self._bucket_groups:
            group = self._opened_file.get(bucket)

            if group is None:
                return False

            self._bucket_groups[bucket] = {
                "data": np.array(group.get("data")),
                "rect": SimpleRotatedRectangle(
                    group.attrs.get("x lower left corner"),
                    group.attrs.get("y lower left corner"),
                    0,
                    group.attrs.get("cell size"),
                    group.attrs.get("cell size")
                )
            }

        bilinear_point = SimplePoint(
            datum.converted_point.x_value,
            datum.converted_point.y_value,
            0
        )

        datum.set_elevation_data(ElevationProperties(
            calculate_bilinear_value(bilinear_point, self._bucket_groups[bucket]["rect"],
                                     self._bucket_groups[bucket]["data"]),
            self._public_metadata["id"]
        ))

        return True

    def _get_etopo1_data(self, datum: SeismicData) -> bool:
        """
        Attempts to get elevation properties from etopo1 data.
        :param datum: The data point from which to get elevation properties.
        :return: True, if successful, false if not found.
        """
        bucket = "/dem/etopo1"

        if bucket not in self._bucket_groups:
            group = self._opened_file.get(bucket)

            if group is None:
                return False

            self._bucket_groups[bucket] = {
                "data": np.array(group.get("data")),
                "rect": SimpleRotatedRectangle(
                    group.attrs.get("x lower left corner"),
                    group.attrs.get("y lower left corner"),
                    0,
                    group.attrs.get("cell size"),
                    group.attrs.get("cell size")
                )
            }

        datum.converted_point = datum.original_point.convert_to_projection(UCVM_DEFAULT_PROJECTION)

        bilinear_point = SimplePoint(
            datum.converted_point.x_value,
            datum.converted_point.y_value,
            0
        )

        datum.set_elevation_data(ElevationProperties(
            calculate_bilinear_value(bilinear_point, self._bucket_groups[bucket]["rect"],
                                     self._bucket_groups[bucket]["data"]),
            self._public_metadata["id"]
        ))

        return True

    def _query(self, data: List[SeismicData], **kwargs) -> bool:
        """
        Internal (override) query method for the model.
        :param list data: A list of SeismicData classes to fill in with elevation properties.
        :return: True if function was successful, false if not.
        """
        for datum in data:
            if not self._get_nationalmap_data(datum):
                if not self._get_etopo1_data(datum):
                    datum.set_elevation_data(ElevationProperties(None, "no data"))

        return True

    def __del__(self):
        try:
            self._opened_file.close()
        except SystemError:
            pass
