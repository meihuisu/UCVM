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
    def _get_z_data(cls, p: Point, model: str, spacing: int=50, depth: int=70000) -> dict:
        """
        Gets the Z1.0 and Z2.5 data for a given point which has all the projection, metadata, x, y, etc. specified.

        Args:
            p (Point): The point for which the Z1.0 and Z2.5 values should be found.
            model (str): The model to query to find the Z1.0 and Z2.5 at this point.
            spacing (int): The vertical spacing at which the queries should be made. The default is one meter.
            depth (int): Check for Vs values down to this depth. The default is 70 kilometers.

        Returns:
            dict: A dict containing the Z1.0 (first) and Z2.5 (second) when successful. None if an error occurs.
        """
        _interval_size = 1000
        _velocities_to_find = (1000, 2500)

        #This if statement is a toggle to select three different algorithms for finding Z-values:
        #   1st occurrence, 2nd occurrence, last occurrence (probably the same as 2nd for all 17.5 models)
        if False:
            #This one checks for 1st occurrence
            _current_interval = 0
            _depths = {1000: depth, 2500: depth}

            while _current_interval < depth:
                _query_points = [SeismicData(Point(p.x_value, p.y_value, _current_interval + (z * spacing), UCVM_DEPTH,
                                 None, p.projection)) for z in range(0, int(_interval_size / spacing) + 1)]
                UCVM.query(_query_points, model.replace(".z-calc", ""), ["velocity"])

                for target in _velocities_to_find:
                    for point in _query_points:
                        if point.velocity_properties is not None and point.velocity_properties.vs is not None:
                            if point.velocity_properties.vs >= target and _depths[target] == depth:
                                _depths[target] = point.converted_point.z_value
                                break

                if _depths[1000] != depth and _depths[2500] != depth:
                    return _depths
                else:
                    _current_interval += _interval_size
        elif False:
            #This one checks for last occurrence
            _current_interval = depth
            _depths = {1000: 0, 2500: 0}
            while _current_interval > 0:
                _query_points = [SeismicData(Point(p.x_value, p.y_value, _current_interval - (z * spacing), UCVM_DEPTH,
                                 None, p.projection)) for z in range(0, int(_interval_size / spacing) + 1)]
                UCVM.query(_query_points, model.replace(".z-calc", ""), ["velocity"])

                for target in _velocities_to_find:
                    for point in _query_points:
                        if point.velocity_properties is not None and point.velocity_properties.vs is not None:
                            if point.velocity_properties.vs <= target and _depths[target] == 0:
                                #This will give us the first point which is less than the target; we want the previous
                                #(deeper) point to be consistent with Z2.5 def
                                _depths[target] = point.converted_point.z_value + spacing
                                break

                if _depths[1000] != 0 and _depths[2500] != 0:
                    return _depths
                else:
                    _current_interval -= _interval_size
        else:
            #This one checks for 2nd occurrence
            _current_interval = 0
            _depths = {1000: depth, 2500: depth}
            _flags = {1000: 0, 2500: 0}

            while _current_interval < depth:
                _query_points = [SeismicData(Point(p.x_value, p.y_value, _current_interval + (z * spacing), UCVM_DEPTH,
                                 None, p.projection)) for z in range(0, int(_interval_size / spacing) + 1)]
                UCVM.query(_query_points, model.replace(".z-calc", ""), ["velocity"])

                for target in _velocities_to_find:
                    for point in _query_points:
                        if point.velocity_properties is not None and point.velocity_properties.vs is not None:
                            if point.velocity_properties.vs >= target and _flags[target] == 0:
                                _depths[target] = point.converted_point.z_value
                                _flags[target] = 1
                            elif point.velocity_properties.vs < target and _flags[target] == 1:
                                _flags[target] = 2
                            elif point.velocity_properties.vs >= target and _flags[target] == 2:
                                _depths[target] = point.converted_point.z_value
                                _flags[target] = 3
                                break

                if _flags[2500] == 3:
                    return _depths
                else:
                    _current_interval += _interval_size

        #If we get here, we didn't find valid depths

        return _depths

    def _query(self, data: List[SeismicData], **kwargs) -> bool:
        """
        This is the method that all models override. It handles querying the velocity model
        and filling in the SeismicData structures.

        Args:
            points (:obj:`list` of :obj:`SeismicData`): List of SeismicData objects containing the
                points. These are to be populated with :obj:`ZProperties`:

        Returns:
            True on success, false if there is an error.
        """
        for datum in data:
            if datum.model_string is not None:
                if datum.velocity_properties is not None and datum.velocity_properties.vs is not None:
                    z_data = self._get_z_data(datum.original_point, datum.model_string)
                    datum.set_z_data(ZProperties(z_data[1000], z_data[2500]))
                else:
                    datum.set_z_data(ZProperties(None, None))

        return True
