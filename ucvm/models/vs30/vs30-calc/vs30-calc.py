"""
Defines a Vs30 model which consists of the Vs30 information calculated direct from the model itself.
This calculates the slowness at the top 30 meters and returns that information.

:copyright: Southern California Earthquake Center
:author:    David Gill <davidgil@usc.edu>
:created:   July 21, 2016
:modified:  September 6, 2016
"""

from typing import List

from ucvm.src.framework.ucvm import UCVM
from ucvm.src.model.vs30.vs30_model import Vs30Model
from ucvm.src.shared import Vs30Properties, UCVM_DEPTH
from ucvm.src.shared.properties import SeismicData, Point


class Vs30CalcModel(Vs30Model):

    def __init__(self, **kwargs):
        super().__init__(**kwargs)

    def _query(self, data: List[SeismicData], **kwargs) -> bool:
        """
        Internal (override) query method for the model.
        :param list data: A list of SeismicData classes to fill in with elevation properties.
        :return: True if function was successful, false if not.
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
