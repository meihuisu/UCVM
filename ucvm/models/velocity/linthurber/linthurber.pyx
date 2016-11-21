"""
Lin-Thurber Velocity Model

This code is the Cython interface to the legacy Lin-Thurber model C code. It returns equivalent
material properties to UCVM.

Copyright:
    Southern California Earthquake Center

Developer:
    David Gill <davidgil@usc.edu>
"""
# Python Imports
import os
from typing import List

# Cython Imports
cimport cython

# UCVM Imports
from ucvm.src.model.velocity.velocity_model import VelocityModel
from ucvm.src.shared.properties import SeismicData, VelocityProperties

# Cython defs
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
    """
    Defines the Lin-Thurber interface to UCVM. This class queries the legacy C code to retrieve
    the material properties and records the data to the new UCVM data structures.
    """

    def __init__(self, **kwargs):
        super().__init__(**kwargs)

        py_model_path = os.path.join(self.model_location, "data").encode("ASCII")
        model_path = py_model_path

        # Initialize CVM-LT.
        cvmlt_init(model_path)

    def _query(self, points: List[SeismicData], **kwargs) -> bool:
        """
        This is the method that all models override. It handles querying the velocity model
        and filling in the SeismicData structures.

        Args:
            points (:obj:`list` of :obj:`SeismicData`): List of SeismicData objects containing the
                points in depth. These are to be populated with :obj:`VelocityProperties`:

        Returns:
            True on success, false if there is an error.
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
