"""
Defines the Wills-Wald Vs30 model. This is the default Vs30 model within UCVM.

:copyright: Southern California Earthquake Center
:author:    David Gill <davidgil@usc.edu>
:created:   October 18, 2016
:modified:  October 18, 2016
"""
import os
import math
from typing import List

from ucvm.src.model.vs30.vs30_model import Vs30Model
from ucvm.src.shared import Vs30Properties
from ucvm.src.shared.functions import bilinear_interpolation
from ucvm.src.shared.properties import SeismicData

import numpy as np


class WillsWaldModel(Vs30Model):

    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self._query_array = np.load(os.path.join(self.get_model_dir(), "data", "vs30.dat"))

    def _query(self, data: List[SeismicData], **kwargs) -> bool:
        """
        Internal (override) query method for the model.
        :param list data: A list of SeismicData classes to fill in with elevation properties.
        :return: True if function was successful, false if not.
        """
        for datum in data:
            if datum.converted_point.x_value < -130 or datum.converted_point.x_value > -110 or \
               datum.converted_point.y_value < 27 or datum.converted_point.y_value > 47:
                datum.set_vs30_data(Vs30Properties(None, None))

            # Get four corners.
            x_vals = [
                (math.floor(datum.converted_point.x_value * 100) / 100 - (-130)) / 0.01,
                (math.ceil(datum.converted_point.x_value * 100) / 100 - (-130)) / 0.01
            ]
            y_vals = [
                (math.floor(datum.converted_point.y_value * 100) / 100 - 27) / 0.01,
                (math.ceil(datum.converted_point.y_value * 100) / 100 - 27) / 0.01
            ]

            ret_val = 0

            if x_vals[0] == x_vals[1] and y_vals[0] == y_vals[1]:
                ret_val = self._query_array[int(y_vals[0])][int(x_vals[0])]
            elif x_vals[0] == x_vals[1]:
                t = (datum.converted_point.y_value - \
                    (math.floor(datum.converted_point.y_value * 100) / 100)) * 100
                ret_val = (1 - t) *self._query_array[int(y_vals[0])][int(x_vals[0])] + \
                          t * self._query_array[int(y_vals[1])][int(x_vals[0])]
            elif y_vals[0] == y_vals[1]:
                t = (datum.converted_point.x_value - \
                    (math.floor(datum.converted_point.x_value * 100) / 100)) * 100
                ret_val = (1 - t) *self._query_array[int(y_vals[0])][int(x_vals[0])] + \
                          t * self._query_array[int(y_vals[0])][int(x_vals[1])]
            else:
                # Bilinearly interpolate.
                ret_val = bilinear_interpolation(
                    (datum.converted_point.x_value - (-130)) * 100,
                    (datum.converted_point.y_value - 27) * 100,
                    [
                        (x_vals[0], y_vals[0], self._query_array[int(y_vals[0])][int(x_vals[0])]),
                        (x_vals[1], y_vals[0], self._query_array[int(y_vals[0])][int(x_vals[1])]),
                        (x_vals[0], y_vals[1], self._query_array[int(y_vals[1])][int(x_vals[0])]),
                        (x_vals[1], y_vals[1], self._query_array[int(y_vals[1])][int(x_vals[1])])
                    ]
                )

            if ret_val > 0:
                datum.vs30_properties = Vs30Properties(ret_val, self._public_metadata["id"])
            else:
                datum.vs30_properties = Vs30Properties(None, None)

        return True
