"""
CVM-S4.26.M01 Velocity Model

Iteration 26 of Po Chen and En-Jui Lee's tomographic inversions of CVM-S4, combined with a sequence
of rules that are aimed at recovering the GTL from CVM-S4.

This code is the Cython interface to the legacy CVM-S4.26.M01 model C code. It returns equivalent
material properties to UCVM.

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
from typing import List

# Cython Imports
cimport cython
from libc.stdlib cimport malloc, free

# UCVM Imports
from ucvm.src.model.velocity import VelocityModel
from ucvm.src.shared import VelocityProperties
from ucvm.src.shared.properties import SeismicData

# Cython defs
cdef extern from "src/src/cvmsi.h":
    int cvmsi_init(const char *)
    int cvmsi_query(cvmsi_point_t *, cvmsi_data_t *, int)
    int cvmsi_finalize()

cdef struct cvmsi_point_t:
  double coord[3]

cdef struct cvmsi_index_t:
  int coord[3]

cdef struct cvmsi_prop_t:
  float vp
  float vs
  float rho
  float diff_vp
  float diff_vs
  float diff_rho

cdef struct cvmsi_data_t:
  cvmsi_index_t xyz
  cvmsi_prop_t prop

class CVMS426M01VelocityModel(VelocityModel):
    """
    Defines the CVM-S4.26.M01 interface to UCVM. This class queries the legacy C code to retrieve
    the material properties and records the data to the new UCVM data structures.
    """

    def __init__(self, **kwargs):
        super().__init__(**kwargs)

        cdef char *model_data_path

        model_py_path = os.path.join(self.model_location, "data", "i26").encode("ASCII")
        model_data_path = model_py_path
        if cvmsi_init(model_data_path) != 0:
            raise RuntimeError("CVM-S4.26.M01 could not be initialized correctly.")

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
        cdef int num_pts
        cdef cvmsi_point_t *pts
        cdef cvmsi_data_t *data

        num_pts = len(points)

        pts = <cvmsi_point_t *> malloc(num_pts*cython.sizeof(cvmsi_point_t))
        data = <cvmsi_data_t *> malloc(num_pts*cython.sizeof(cvmsi_data_t))

        for i in range(len(points)):
            pts[i].coord[0] = points[i].converted_point.x_value
            pts[i].coord[1] = points[i].converted_point.y_value
            pts[i].coord[2] = points[i].converted_point.z_value

        cvmsi_query(pts, data, num_pts)

        id_s426m01 = self._public_metadata["id"]

        for i in range(0, len(points)):
            if points[i].converted_point.z_value >= 0:
                points[i].set_velocity_data(
                    VelocityProperties(
                        data[i].prop.vp, data[i].prop.vs, data[i].prop.rho,  # From CVM-S4.26.M01
                        None, None,  # No Qp or Qs defined
                        id_s426m01, id_s426m01, id_s426m01,  # All data comes from CVM-S4.26.M01
                        None, None  # No Qp or Qs defined
                    )
                )
            else:
                # CVM-S4.26.M01 has no information on negative depth points.
                self._set_velocity_properties_none(points[i])

        free(pts)
        free(data)

        return True

    def __del__(self):
        cvmsi_finalize()
