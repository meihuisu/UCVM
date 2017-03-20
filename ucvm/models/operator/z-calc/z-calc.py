"""
Z1.0 and Z2.5 Calculated From Model

Retrieves Z1.0 and Z2.5 data directly from the model. This requires knowledge of where the surface is in order
to satisfy this!

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
from ucvm.src.model.operator.operator_model import OperatorModel
from ucvm.src.shared import ZProperties, UCVM_DEPTH
from ucvm.src.shared.properties import SeismicData, Point


class ZOperator(OperatorModel):
    """
    Defines the operator that calculates the Z1.0 and Z2.5 data directly from the model.
    """

    def __init__(self, **kwargs):
        super().__init__(**kwargs)

    @classmethod
    def _get_z_data(cls, p: Point, model: str, spacing: int=10, depth: int=50000) -> (float, float):
        """
        Gets the Z1.0 and Z2.5 data for a given point which has all the projection, metadata, x, y, etc. specified.

        Args:
            p (Point): The point for which the Z1.0 and Z2.5 values should be found.
            model (str): The model to query to find the Z1.0 and Z2.5 at this point.
            spacing (int): The vertical spacing at which the queries should be made. The default is one meter.
            depth (int): Check for Vs values down to this depth. The default is 70 kilometers.

        Returns:
            A tuple containing the Z1.0 (first) and Z2.5 (second) when successful. None if an error occurs.
        """
        velocities_to_find = (1000, 2500)
        depths = {}

        query_points = [SeismicData(Point(p.x_value, p.y_value, z * spacing, UCVM_DEPTH, p.metadata, p.projection))
                        for z in range(0, int(depth / spacing) + 1)]
        UCVM.query(query_points, model.replace(".z-calc", ""), ["velocity"])

        for target in velocities_to_find:
            for point in query_points:
                if point.velocity_properties is not None and point.velocity_properties.vs is not None:
                    if point.velocity_properties.vs >= target:
                        depths[target] = point.converted_point.z_value
                        break
            if target not in depths:
                depths[target] = depth

        return depths

    def _query(self, data: List[SeismicData], **kwargs) -> bool:
        """
        This is the method that all models override. It handles querying the velocity model
        and filling in the SeismicData structures.

        Args:
            data (:obj:`list` of :obj:`SeismicData`): List of SeismicData objects containing the points. These are to
                be populated with :obj:`ZProperties`:

        Returns:
            True on success, false if there is an error.
        """
        for datum in data:
            if datum.model_string is not None:
                z_data = self._get_z_data(datum.original_point, datum.model_string)
                datum.set_z_data(
                    ZProperties(z_data[1000], z_data[2500])
                )

        return True
