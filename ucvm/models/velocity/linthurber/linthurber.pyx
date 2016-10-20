"""
Defines the Python implementation of the Lin-Thurber velocity model.

:copyright: Southern California Earthquake Center
:author:    David Gill <davidgil@usc.edu>
:created:   October 12, 2016
:modified:  October 12, 2016
"""
import os
cimport cython

from typing import List

from ucvm.src.model.velocity.velocity_model import VelocityModel
from ucvm.src.shared.properties import SeismicData, VelocityProperties

cdef extern from "src/src/cvmlt.h":
    int cvmlt_init(const char *)
    int cvmlt_finalize()
    int cvmlt_query(void *pnt, void *data)

cdef struct cvmlt_point_t:
    double coord[3]

cdef struct cvmlt_data_t:
    float vp
    float vs
    float rho

class LinThurberVelocityModel(VelocityModel):

    def __init__(self, **kwargs):
        super().__init__(**kwargs)

        py_model_path = os.path.join(self.model_location, "data").encode("ASCII")
        model_path = py_model_path

        # Initialize CVM-LT.
        cvmlt_init(model_path)

    def _query(self, points: List[SeismicData], **kwargs) -> bool:
        """
        Internal (override) query method for the model.
        :param list data: A list of SeismicData classes.
        :return: True on success, false on failure.
        """
        cdef cvmlt_point_t mpnt
        cdef cvmlt_data_t mdata

        id_cvmlt = self._public_metadata["id"]

        for i in range(0, len(points)):
            mpnt.coord[0] = points[i].converted_point.x_value
            mpnt.coord[1] = points[i].converted_point.y_value
            mpnt.coord[2] = points[i].converted_point.z_value

            if mpnt.coord[2] < 0:
                self._set_velocity_properties_none(points[i])
                continue

            if cvmlt_query(&mpnt, &mdata) == 0 and mdata.vp > 0 and mdata.vs > 0 and mdata.rho > 0:
                points[i].set_velocity_data(
                    VelocityProperties(
                        mdata.vp, mdata.vs, mdata.rho, None, None,
                        id_cvmlt, id_cvmlt, id_cvmlt, None, None
                    )
                )
            else:
                self._set_velocity_properties_none(points[i])

        return True

    def __del__(self):
        cvmlt_finalize()
