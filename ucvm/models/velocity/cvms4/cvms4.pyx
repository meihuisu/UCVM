import time
import os
cimport cython

from typing import List

from libc.stdlib cimport malloc, free

from ucvm.src.model.velocity.legacy import VelocityModel
from ucvm.src.shared import VelocityProperties
from ucvm.src.shared.properties import SeismicData

cdef extern from "fortran_src/cvms.h":
    void cvms_init_(char *, int *)
    void cvms_version_(char *, int *)
    void cvms_query_(int *, float *, float *, float *, float *, float *, float *, int *)


class CVMS4VelocityModel(VelocityModel):

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
            points[i].set_velocity_data(
                VelocityProperties(
                    vp[i], vs[i], density[i],  # From CVM-S4
                    None, None,  # No Qp or Qs defined
                    id_s4, id_s4, id_s4,  # All data comes direct from CVM-S4
                    None, None  # No Qp or Qs defined
                )
            )

        free(lon)
        free(lat)
        free(dep)
        free(vs)
        free(vp)
        free(density)

        return True