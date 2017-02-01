"""
Bay Area Velocity Model

The USGS 3-D Geologic and Seismic Velocity Models of the San Francisco Bay region provide a
three-dimensional view of the geologic structure and physical properties of the region down to a
depth of 45 km (28 miles).

This code is the Cython interface to the legacy Bay Area model C code. It returns equivalent
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
from libc.stdlib cimport malloc, free

# UCVM Imports
from ucvm.src.model.velocity import VelocityModel
from ucvm.src.shared import VelocityProperties, ElevationProperties
from ucvm.src.shared.properties import SeismicData

# Cython defs
cdef extern from "src/libsrc/query/cvmquery.h":
    void *cencalvm_createQuery()
    void *cencalvm_errorHandler(void *)
    int cencalvm_filename(void *, const char *)
    int cencalvm_filenameExt(void *, const char *)
    int cencalvm_cacheSize(void *, const int)
    int cencalvm_cacheSizeExt(void *, const int)
    int cencalvm_open(void *)
    int cencalvm_query(void *, double **, const int, const double, const double, const double)
    int cencalvm_queryType(void *, const int)
    int cencalvm_close(void *)
    int cencalvm_destroyQuery(void *)
    int cencalvm_squash(void *, int, float)

cdef extern from "src/libsrc/query/cvmerror.h":
    char *cencalvm_error_message(void *)
    int cencalvm_error_resetStatus(void *)

cdef void *cencal_query
cdef void *cencal_error_handler
cdef double *cencal_pvals

class BayAreaVelocityModel(VelocityModel):
    """
    Defines the Bay Area interface to UCVM. This class queries the legacy C code to retrieve
    the material properties and records the data to the new UCVM data structures.
    """

    def __init__(self, **kwargs):
        super().__init__(**kwargs)

        self.CC_CACHE_SIZE = 64
        self.CENCAL_MODEL_BOTTOM = -45000
        self.CENCAL_VALS = 9
        self.CENCAL_HR_OCTANT_HEIGHT = 100.0

        cdef char *hr_model_path
        cdef char *lr_model_path
        cdef int errcode

        py_hr_model_path = os.path.join(self.model_location, "data",
                                        "USGSBayAreaVM-08.3.0.etree").encode("ASCII")
        py_lr_model_path = os.path.join(self.model_location, "data",
                                        "USGSBayAreaVMExt-08.3.0.etree").encode("ASCII")
        hr_model_path = py_hr_model_path
        lr_model_path = py_lr_model_path

        global cencal_query, cencal_pvals, cencal_error_handler

        cencal_query = cencalvm_createQuery()
        if not cencal_query:
            raise RuntimeError("CenCal_CreateQuery failed.")

        cencal_error_handler = cencalvm_errorHandler(cencal_query)
        if not cencal_error_handler:
            raise RuntimeError("CenCal_ErrorHandler failed.")

        if cencalvm_filename(cencal_query, hr_model_path) != 0:
            raise RuntimeError("Unable to set main database to " + py_hr_model_path.decode("ASCII"))

        if cencalvm_filenameExt(cencal_query, lr_model_path) != 0:
            raise RuntimeError("Unable to set ext database to " + py_lr_model_path.decode("ASCII"))

        if cencalvm_cacheSize(cencal_query, self.CC_CACHE_SIZE) != 0:
            raise RuntimeError("Could not set main cache size to " + str(self.CC_CACHE_SIZE))

        if cencalvm_cacheSizeExt(cencal_query, self.CC_CACHE_SIZE) != 0:
            raise RuntimeError("Could not set ext cache size to " + str(self.CC_CACHE_SIZE))

        if cencalvm_open(cencal_query) != 0:
            raise RuntimeError("Could not open the CenCal query.")

        cencal_pvals = <double *> malloc(self.CENCAL_VALS * cython.sizeof(double))

        cencalvm_queryType(cencal_query, 0)

    def _get_cencal_surface(self, longitude: float, latitude: float) -> float:
        """
        Given a longitude and latitude in WGS84 projection, get the Bay Area surface value from
        the embedded CenCal DEM.

        Args:
            longitude (float): The longitude in WGS84 projection.
            latitude (float): The latitude in WGS84 projection.

        Returns:
            The elevation of the surface in meters.
        """
        cdef double cencal_slimit
        cdef int cencal_smode

        cencal_slimit = 0.0
        cencal_smode = 0

        global cencal_query, cencal_pvals, cencal_error_handler

        if cencalvm_squash(cencal_query, cencal_smode, cencal_slimit) != 0:
            print(str(cencalvm_error_message(cencal_error_handler)))
            raise RuntimeError("CencalVM_Squash failed in _get_cencal_surface.")

        for i in range(0, self.CENCAL_VALS):
            cencal_pvals[i] = 0

        elev = self.CENCAL_MODEL_BOTTOM

        if cencalvm_query(cencal_query, &cencal_pvals, self.CENCAL_VALS, longitude,
                          latitude, elev) == 0:
            elev = elev + cencal_pvals[5] + self.CENCAL_HR_OCTANT_HEIGHT * 2
            prevelev = elev

            while elev >= self.CENCAL_MODEL_BOTTOM:
                if cencalvm_query(cencal_query, &cencal_pvals, self.CENCAL_VALS, longitude,
                                  latitude, elev) == 0:
                    if cencal_pvals[0] > 0 and cencal_pvals[1] > 0 and cencal_pvals[2] > 0:
                        startelev = elev
                        endelev = prevelev
                        resid = endelev - startelev
                        while resid > 1.0:
                            elev = (startelev + endelev) / 2.0
                            if cencalvm_query(cencal_query, &cencal_pvals, self.CENCAL_VALS,
                                              longitude, latitude, elev) != 0:
                                cencalvm_error_resetStatus(cencal_error_handler)
                                endelev = elev
                            else:
                                if cencal_pvals[0] <= 0 or cencal_pvals[1] <= 0 or \
                                   cencal_pvals[2] <= 0:
                                    endelev = elev
                                else:
                                    startelev = elev
                            resid = endelev - startelev
                        elev = startelev
                        break
                else:
                    cencalvm_error_resetStatus(cencal_error_handler)

                prevelev = elev
                elev = elev - self.CENCAL_HR_OCTANT_HEIGHT
        else:
            cencalvm_error_resetStatus(cencal_error_handler)

        cencal_slimit = 200000.0

        if cencalvm_squash(cencal_query, cencal_smode, cencal_slimit) != 0:
            raise RuntimeError("Could not restore original squash mode.")

        if elev - self.CENCAL_MODEL_BOTTOM <= 0.01:
            return None

        return elev

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
        cdef double cencal_slimit
        cdef int cencal_smode

        cencal_slimit = 200000.0
        cencal_smode = 0

        # Now we need to go through the material properties and add them to the SeismicData objects.
        id_cencal = self._public_metadata["id"]

        global cencal_query, cencal_pvals

        if cencalvm_squash(cencal_query, cencal_smode, cencal_slimit) != 0:
            print(str(cencalvm_error_message(cencal_error_handler)))
            raise RuntimeError("CencalVM_Squash failed in _query.")

        for i in range(0, len(points)):
            free_surface = self._get_cencal_surface(points[i].converted_point.x_value,
                                                    points[i].converted_point.y_value)

            if free_surface is not None:
                if points[0].original_point.depth_elev == 0:
                    point_elevation = free_surface - points[i].converted_point.z_value
                else:
                    point_elevation = points[i].converted_point.z_value
            else:
                self._set_velocity_properties_none(points[i])
                continue

            cencal_pvals[0] = 0
            cencal_pvals[1] = 0
            cencal_pvals[2] = 0
            cencal_pvals[3] = 0
            cencal_pvals[4] = 0

            if cencalvm_query(cencal_query, &cencal_pvals, self.CENCAL_VALS,
                              points[i].converted_point.x_value,
                              points[i].converted_point.y_value, point_elevation) == 0:
                points[i].set_velocity_data(
                    VelocityProperties(
                        cencal_pvals[0], cencal_pvals[1], cencal_pvals[2],  # From the model
                        cencal_pvals[3], cencal_pvals[4],  # CenCal actually defines Qp and Qs
                        id_cencal, id_cencal, id_cencal,  # All data comes direct from the model
                        id_cencal, id_cencal  # Defined in the model
                    )
                )
                points[i].set_elevation_data(
                    ElevationProperties(
                        point_elevation, id_cencal
                    )
                )
            else:
                self._set_velocity_properties_none(points[i])
                cencalvm_error_resetStatus(cencal_error_handler)

        return True

    def __del__(self) -> None:
        """
        Closes the file handles and deletes the pointers when the Python object is deleted.

        Returns:
            Nothing
        """
        cencalvm_close(cencal_query)
        cencalvm_destroyQuery(cencal_query)