"""
USGS/NOAA Digital Elevation Model

This internal map represents a "splicing" together of two datasets:

1) ETOPO1 - Source: https://www.ngdc.noaa.gov/docucomp/page?xml=NOAA/NESDIS/NGDC/MGG/DEM/iso/xml/
                    316.xml&view=getDataView&header=none
   This data represents the global digital elevation model. ETOPO1 is vertically referenced to
   sea level, and horizontally referenced to the World Geodetic System of 1984 (WGS 84). Cell size
   for ETOPO1 is 1 arc-minute.

2) USGS National Map Data - Source: http://www.nationalmap.gov
   This data represents the California digital elevation model.

Copyright 2017 Southern California Earthquake Center

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
"""
# Python Imports
import os
import math
from typing import List

# Package Imports
import h5py

# UCVM Imports
from ucvm.src.model.elevation.elevation_model import ElevationModel
from ucvm.src.shared import ElevationProperties, SimplePoint, SimpleRotatedRectangle, \
                            calculate_bilinear_value
from ucvm.src.shared.properties import SeismicData, UCVM_DEFAULT_PROJECTION


class USGSNOAAElevationModel(ElevationModel):
    """
    Defines the USGS/NOAA digital elevation model within UCVM. The ETOPO data is stored in WGS84
    format, but the USGS data is stored in NAD83. Therefore, some additional conversions need to be
    done from within the class to make this work.
    """
    DATA_FILE = "dem.dat"     #: str: The location of the DEM data file.
    _opened_file = None

    def _get_nationalmap_data(self, datum: SeismicData) -> bool:
        """
        Attempts to get elevation properties from National Map data. If no properties can be found
        this function returns false. It does not set the properties to "None".

        Args:
            datum (SeismicData): The data point from which to get elevation properties.

        Returns:
            True, if successful, false if point not found within map.
        """
        bilinear_point = SimplePoint(
            datum.converted_point.x_value,
            datum.converted_point.y_value,
            0
        )

        if bilinear_point.x < -180 or bilinear_point.x > 180 or \
           bilinear_point.y < -90 or bilinear_point.y > 90:
            return False

        # Make sure we can get the National Map data.
        if hasattr(self._opened_file, "dem_nationalmap_" +
                   str(math.ceil(-1 * datum.converted_point.x_value)) +
                   "_" + str(math.floor(datum.converted_point.y_value))):
            use_data = getattr(self._opened_file, "dem_nationalmap_" +
                               str(-1 * math.floor(datum.converted_point.x_value)) +
                               "_" + str(math.floor(datum.converted_point.y_value)))
        else:
            return False

        rect = SimpleRotatedRectangle(
            use_data["metadata"][1][0],
            use_data["metadata"][2][0],
            0,
            use_data["metadata"][0][0],
            use_data["metadata"][0][0]
        )

        value = calculate_bilinear_value(bilinear_point, rect, use_data.data)

        # Check if not defined by DEM (i.e. in water).
        if value == 0 or value == -9999:
            return False

        datum.set_elevation_data(ElevationProperties(value, self._public_metadata["id"]))

        return True

    def _get_etopo1_data(self, datum: SeismicData) -> bool:
        """
        Attempts to get elevation properties from ETOPO1 data. If no properties can be found
        this function returns false. It does not set the properties to "None".

        Args:
            datum (SeismicData): The data point from which to get elevation properties.

        Returns:
            True, if successful, false if point not found within map.
        """
        datum.converted_point = datum.original_point.convert_to_projection(UCVM_DEFAULT_PROJECTION)

        bilinear_point = SimplePoint(
            datum.converted_point.x_value,
            datum.converted_point.y_value,
            0
        )

        if bilinear_point.x < -180 or bilinear_point.x > 180 or \
           bilinear_point.y < -90 or bilinear_point.y > 90:
            return False

        rect = SimpleRotatedRectangle(
            self._opened_file["dem_etopo1"]["metadata"][1][0],
            self._opened_file["dem_etopo1"]["metadata"][2][0],
            0,
            self._opened_file["dem_etopo1"]["metadata"][0][0],
            self._opened_file["dem_etopo1"]["metadata"][0][0]
        )

        if not hasattr(self, "etopo1_data"):
            self.etopo1_data = self._opened_file["dem_etopo1"]["data"][:,:]

        datum.set_elevation_data(ElevationProperties(
            calculate_bilinear_value(bilinear_point, rect, self.etopo1_data),
            self._public_metadata["id"]
        ))

        return True

    def _query(self, points: List[SeismicData], **kwargs) -> bool:
        """
        This is the method that all models override. It handles querying the elevation model
        and filling in the SeismicData structures.

        Args:
            points (:obj:`list` of :obj:`SeismicData`): List of SeismicData objects containing the
                points to query. These are to be populated with :obj:`ElevationProperties`:

        Returns:
            True on success, false if there is an error.
        """
        self._opened_file = h5py.File(
            os.path.join(self.get_model_dir(), "data", self.DATA_FILE), mode="r"
        )

        for point in points:
            if not self._get_nationalmap_data(point):
                if not self._get_etopo1_data(point):
                    point.set_elevation_data(ElevationProperties(None, None))

        self._opened_file.close()

        return True
