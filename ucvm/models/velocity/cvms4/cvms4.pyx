"""
CVM-S4 Velocity Model

CVM-SCEC version 4 (CVM-S4), also known as SCEC CVM-4, is a 3D seismic velocity model.
The current version is CVM-S4 was released in 2006 and was originally posted for download from the
SCEC Data Center SCEC 3D Velocity Models Site.

This code is the Cython interface to the legacy CVM-S4 Fortran code. It returns equivalent material
properties to UCVM.

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
from libc.stdlib cimport malloc, free

# UCVM Imports
from ucvm.src.model.velocity.legacy import VelocityModel
from ucvm.src.shared import VelocityProperties
from ucvm.src.shared.properties import SeismicData

# Cython defs
cdef extern from "src/cvms.h":
    void cvms_init_(char *, int *)
    void cvms_version_(char *, int *)
    void cvms_query_(int *, float *, float *, float *, float *, float *, float *, int *)


class CVMS4VelocityModel(VelocityModel):
    """
    Defines the CVM-S4 interface to UCVM. This class queries the legacy Fortran code to retrieve
    the material properties and records the data to the new UCVM data structures.
    """

    def __init__(self, **kwargs):
        super().__init__(**kwargs)

        cdef char *model_path
        cdef int errcode

        py_model_path = os.path.join(self.model_location, "data").encode("ASCII")
        model_path = py_model_path
        errcode = 0

        cvms_init_(model_path, &errcode)

        if errcode != 0:
            raise RuntimeError("CVM-S4 not initialized properly.")

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
        cdef int nn

        cdef float *lon
        cdef float *lat
        cdef float *dep
        cdef float *vp
        cdef float *vs
        cdef float *density

        cdef int retcode

        nn = len(points)
        retcode = 0

        lon = <float *> malloc(len(points)*cython.sizeof(float))
        for i in range(0, len(points)):
            lon[i] = points[i].converted_point.x_value

        lat = <float *> malloc(len(points)*cython.sizeof(float))
        for i in range(0, len(points)):
            lat[i] = points[i].converted_point.y_value

        dep = <float *> malloc(len(points)*cython.sizeof(float))
        for i in range(0, len(points)):
            dep[i] = points[i].converted_point.z_value

        vs = <float *> malloc(len(points)*cython.sizeof(float))
        for i in range(0, len(points)):
            vs[i] = 0.0

        vp = <float *> malloc(len(points)*cython.sizeof(float))
        for i in range(0, len(points)):
            vp[i] = 0.0

        density = <float *> malloc(len(points)*cython.sizeof(float))
        for i in range(0, len(points)):
            density[i] = 0.0

        cvms_query_(&nn, lon, lat, dep, vp, vs, density, &retcode)

        # Now we need to go through the material properties and add them to the SeismicData objects.
        id_s4 = self._public_metadata["id"]

        for i in range(0, len(points)):
            if points[i].converted_point.z_value >= 0:
                points[i].set_velocity_data(
                    VelocityProperties(
                        vp[i], vs[i], density[i],  # From CVM-S4
                        None, None,  # No Qp or Qs defined
                        id_s4, id_s4, id_s4,  # All data comes direct from CVM-S4
                        None, None  # No Qp or Qs defined
                    )
                )
            else:
                # CVM-S4 has no information on negative depth points.
                self._set_velocity_properties_none(points[i])

        free(lon)
        free(lat)
        free(dep)
        free(vs)
        free(vp)
        free(density)

        return True
