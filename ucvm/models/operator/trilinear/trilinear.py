"""
Trilinear Interpolation

This operator queries the four points surrounding the queried point(s) and if the material
properties come from a different model, in one of the points, then we do trilinear interpolation
between the model boundaries.

Copyright:
    Southern California Earthquake Center

Developer:
    David Gill <davidgil@usc.edu>
"""
# Python Imports
from typing import List

# Package Imports
from pyproj import Proj, transform

# UCVM Imports
from ucvm.src.framework.ucvm import UCVM
from ucvm.src.model.operator import OperatorModel
from ucvm.src.shared.properties import SeismicData, Point, VelocityProperties
from ucvm.src.shared.functions import is_number, get_utm_zone_for_lon
from ucvm.src.shared.constants import UCVM_DEFAULT_PROJECTION


class TrilinearOperator(OperatorModel):

    def _query(self, data: List[SeismicData], **kwargs) -> bool:
        """
        This is the method that all models override. It handles retrieving the queried models'
        properties and adjusting the SeismicData structures as need be.

        Args:
            points (:obj:`list` of :obj:`SeismicData`): List of SeismicData objects containing the
                points queried. These are to be adjusted if needed.

        Returns:
            True on success, false if there is an error.
        """
        spacing = 500
        if "params" in kwargs:
            if is_number(kwargs["params"]):
                spacing = kwargs["params"]

        proj_in = Proj(UCVM_DEFAULT_PROJECTION)

        for datum in data:
            proj_out = Proj("+proj=utm +datum=WGS84 +zone=%d" % (
                            get_utm_zone_for_lon(datum.converted_point.x_value)
                       ))
            utm_coords = transform(proj_in, proj_out, datum.converted_point.x_value,
                                   datum.converted_point.y_value)

            # Get the four corners.
            corners_utm_e = [
                utm_coords[0] - spacing / 2, utm_coords[0] - spacing / 2,
                utm_coords[0] + spacing / 2, utm_coords[0] + spacing / 2
            ]
            corners_utm_n = [
                utm_coords[1] - spacing / 2, utm_coords[1] + spacing / 2,
                utm_coords[1] - spacing / 2, utm_coords[1] + spacing / 2
            ]

            lat_lon_corners = transform(proj_out, proj_in, corners_utm_e, corners_utm_n)

            # Generate the cube to query.
            sd_array = [
                SeismicData(Point(lat_lon_corners[0][0], lat_lon_corners[1][0],
                                  datum.original_point.z_value - spacing / 2,
                                  datum.original_point.depth_elev)),
                SeismicData(Point(lat_lon_corners[0][1], lat_lon_corners[1][1],
                                  datum.original_point.z_value - spacing / 2,
                                  datum.original_point.depth_elev)),
                SeismicData(Point(lat_lon_corners[0][2], lat_lon_corners[1][2],
                                  datum.original_point.z_value - spacing / 2,
                                  datum.original_point.depth_elev)),
                SeismicData(Point(lat_lon_corners[0][3], lat_lon_corners[1][3],
                                  datum.original_point.z_value - spacing / 2,
                                  datum.original_point.depth_elev)),
                SeismicData(Point(lat_lon_corners[0][0], lat_lon_corners[1][0],
                                  datum.original_point.z_value + spacing / 2,
                                  datum.original_point.depth_elev)),
                SeismicData(Point(lat_lon_corners[0][1], lat_lon_corners[1][1],
                                  datum.original_point.z_value + spacing / 2,
                                  datum.original_point.depth_elev)),
                SeismicData(Point(lat_lon_corners[0][2], lat_lon_corners[1][2],
                                  datum.original_point.z_value + spacing / 2,
                                  datum.original_point.depth_elev)),
                SeismicData(Point(lat_lon_corners[0][3], lat_lon_corners[1][3],
                                  datum.original_point.z_value + spacing / 2,
                                  datum.original_point.depth_elev)),
            ]

            # Now query for the material properties.
            UCVM.query(sd_array, datum.model_string, ["velocity"])

            print(sd_array)

        return True
