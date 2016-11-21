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

from ucvm_c_common import UCVMCCommon

class TrilinearOperator(OperatorModel):
    """
    Defines the Trilinear operator for UCVM.
    """

    def _interpolate_properties(self, data: List[SeismicData], x_percent: float, y_percent: float,
                                z_percent: float) -> VelocityProperties:
        """
        Interpolates the points and sets the new material properties.

        Args:
            data (:obj:`list` of :obj:`SeismicData`): List of SeismicData objects for interpolation.
            x_percent (float): The x percentage to interpolate.
            y_percent (float): The y percentage to interpolate.
            z_percent (float): The z percentage to interpolate.

        Returns:
            The interpolated material properties.
        """
        props = ["vp", "vs", "density", "qp", "qs"]
        vel_return = VelocityProperties(
            vp=None, vp_source=None, vs=None, vs_source=None, density=None, density_source=None,
            qp=None, qs=None
        )
        for matprop in props:
            if len(data) == 4:
                if getattr(data[0].velocity_properties, matprop) is not None and \
                   getattr(data[1].velocity_properties, matprop) is not None and \
                   getattr(data[2].velocity_properties, matprop) is not None and \
                   getattr(data[3].velocity_properties, matprop) is not None:
                    setattr(vel_return, matprop, UCVMCCommon.bilinear_interpolate(
                        getattr(data[0].velocity_properties, matprop),
                        getattr(data[2].velocity_properties, matprop),
                        getattr(data[1].velocity_properties, matprop),
                        getattr(data[3].velocity_properties, matprop),
                        x_percent,
                        y_percent
                    ))
                    setattr(vel_return, matprop + "_source", "interpolated")
            elif len(data) == 8:
                if getattr(data[0].velocity_properties, matprop) is not None and \
                   getattr(data[1].velocity_properties, matprop) is not None and \
                   getattr(data[2].velocity_properties, matprop) is not None and \
                   getattr(data[3].velocity_properties, matprop) is not None and \
                   getattr(data[4].velocity_properties, matprop) is not None and \
                   getattr(data[5].velocity_properties, matprop) is not None and \
                   getattr(data[6].velocity_properties, matprop) is not None and \
                   getattr(data[7].velocity_properties, matprop) is not None:
                    setattr(vel_return, matprop, UCVMCCommon.trilinear_interpolate(
                        getattr(data[0].velocity_properties, matprop),
                        getattr(data[2].velocity_properties, matprop),
                        getattr(data[1].velocity_properties, matprop),
                        getattr(data[3].velocity_properties, matprop),
                        getattr(data[4].velocity_properties, matprop),
                        getattr(data[5].velocity_properties, matprop),
                        getattr(data[6].velocity_properties, matprop),
                        getattr(data[7].velocity_properties, matprop),
                        x_percent,
                        y_percent,
                        z_percent
                    ))
                    setattr(vel_return, matprop + "_source", "interpolated")


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

            sd_array = [
                SeismicData(Point(lat_lon_corners[0][0], lat_lon_corners[1][0],
                                  datum.original_point.z_value,
                                  datum.original_point.depth_elev)),
                SeismicData(Point(lat_lon_corners[0][1], lat_lon_corners[1][1],
                                  datum.original_point.z_value,
                                  datum.original_point.depth_elev)),
                SeismicData(Point(lat_lon_corners[0][2], lat_lon_corners[1][2],
                                  datum.original_point.z_value,
                                  datum.original_point.depth_elev)),
                SeismicData(Point(lat_lon_corners[0][3], lat_lon_corners[1][3],
                                  datum.original_point.z_value,
                                  datum.original_point.depth_elev))
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

            def contains_num_in_range(num_list: List(int), start_index: int, stop_index: int):
                for item in num_list:
                    if start_index <= item <= stop_index:
                        return True
                return False

            model_string_differences = []
            for i in range(len(sd_array)):
                if sd_array[i].model_string != datum.model_string:
                    model_string_differences.append(i)

            if sd_array[4].is_property_type_set("velocity") and \
               sd_array[5].is_property_type_set("velocity") and \
               sd_array[6].is_property_type_set("velocity") and \
               sd_array[7].is_property_type_set("velocity") and \
               sd_array[8].is_property_type_set("velocity") and \
               sd_array[9].is_property_type_set("velocity") and \
               sd_array[10].is_property_type_set("velocity") and \
               sd_array[11].is_property_type_set("velocity") and \
               contains_num_in_range(model_string_differences, 4, 11):
                datum.set_velocity_data(self._interpolate_properties(sd_array[4:11]))
            elif sd_array[0].is_property_type_set("velocity") and \
                 sd_array[1].is_property_type_set("velocity") and \
                 sd_array[2].is_property_type_set("velocity") and \
                 sd_array[3].is_property_type_set("velocity") and \
                 contains_num_in_range(model_string_differences, 0, 3):
                datum.set_velocity_data(self._interpolate_properties(sd_array[0:3]))

        return True
