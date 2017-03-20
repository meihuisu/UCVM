"""
Vs30 Calculated From Model

Defines a Vs30 model which consists of the Vs30 information calculated direct from the model itself.
This calculates the slowness at the top 30 meters and returns that information.

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
from typing import List

# UCVM Imports
from ucvm.src.framework.ucvm import UCVM
from ucvm.src.model.vs30.vs30_model import Vs30Model
from ucvm.src.shared import Vs30Properties, UCVM_DEPTH
from ucvm.src.shared.properties import SeismicData, Point


class Vs30CalcModel(Vs30Model):
    """
    Defines the operator that calculates the Vs30 data directly from the model.
    """

    def __init__(self, **kwargs):
        super().__init__(**kwargs)

    def _query(self, data: List[SeismicData], **kwargs) -> bool:
        """
        This is the method that all models override. It handles querying the velocity model
        and filling in the SeismicData structures.

        Args:
            points (:obj:`list` of :obj:`SeismicData`): List of SeismicData objects containing the
                points. These are to be populated with :obj:`Vs30Properties`:

        Returns:
            True on success, false if there is an error.
        """
        for datum in data:
            if datum.model_string is not None:
                query_points = [
                    SeismicData(
                        Point(
                            datum.original_point.x_value,
                            datum.original_point.y_value,
                            z,
                            UCVM_DEPTH,
                            datum.original_point.metadata,
                            datum.original_point.projection
                        )
                    ) for z in range(0, 30)
                ]

                for point in query_points:
                    point.set_elevation_data(datum.elevation_properties)

                UCVM.query(query_points, datum.model_string, ["velocity"])

                if query_points[0].velocity_properties.vs is None or \
                   query_points[0].velocity_properties.vs == 0:
                    datum.vs30_properties = Vs30Properties(None, None)
                    continue

                avg_slowness = 0

                for point in query_points:
                    avg_slowness += 1.0 / float(point.velocity_properties.vs)

                datum.vs30_properties = Vs30Properties(1.0 / (avg_slowness / len(query_points)),
                                                       self._public_metadata["id"])
            else:
                datum.vs30_properties = Vs30Properties(None, None)

        return True
