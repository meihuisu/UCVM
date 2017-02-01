"""
CVM-H 15.1.0 Velocity Model

The SCEC CVM-H velocity model describes seismic P- and S-wave velocities and densities, and is
comprised of basin structures embedded in tomographic and teleseismic crust and upper mantle
models. This latest release of the CVM-H (15.1.0) represents the integration of various model
components, including fully 3D waveform tomographic results.

This code is the Cython interface to the legacy CVM-H model C code. It returns equivalent
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
from ucvm.src.model.velocity import VelocityModel
from ucvm.src.shared import VelocityProperties
from ucvm.src.shared.properties import SeismicData

# Cython defs
cdef extern from "src/src/vx_sub.h":
    int vx_setup(const char *)
    int vx_cleanup()
    int vx_setgtl(int)
    int vx_register_bkg(int *)
    int vx_register_scec()
    int vx_setzmode(int)
    int vx_getcoord(void *)
    void vx_getsurface(double *, int, float *)

cdef struct vx_entry_t:
    double coor[3]
    int coor_type
    double coor_utm[3]
    float elev_cell[2]
    float topo
    float mtop
    float base
    float moho
    int data_src
    float vel_cell[3]
    float provenance
    float vp
    float vs
    double rho

class CVMH1510VelocityModel(VelocityModel):
    """
    Defines the CVM-H interface to UCVM. This class queries the legacy C code to retrieve
    the material properties and records the data to the new UCVM data structures.
    """

    def __init__(self, **kwargs):
        super().__init__(**kwargs)

        cdef char *model_path

        model_data_path = os.path.join(self.model_location, "data").encode("ASCII")
        model_path = model_data_path
        if vx_setup(model_path) != 0:
            raise RuntimeError("CVM-H 15.1.0 could not be initialized correctly.")

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
        cdef vx_entry_t entry
        cdef float vx_surf

        id_cvmh = self._public_metadata["id"]

        if "params" in kwargs:
            items = [str(x).lower().strip() for x in str(kwargs["params"]).split(",")]
            if "gtl" in items:
                vx_setgtl(1)
            else:
                vx_setgtl(0)
            if "1d" in items:
                vx_register_scec()
            else:
                vx_register_bkg(NULL)
        else:
            vx_setgtl(0)
            vx_register_bkg(NULL)

        for i in range(0, len(points)):
            if points[i].original_point.depth_elev == 0:
                vx_setzmode(1)
            else:
                vx_setzmode(0)

            entry.coor_type = 0
            entry.coor[0] = points[i].converted_point.x_value
            entry.coor[1] = points[i].converted_point.y_value
            entry.coor[2] = points[i].converted_point.z_value
            vx_getsurface(&(entry.coor[0]), entry.coor_type, &vx_surf)

            vx_getcoord(&entry)

            if entry.data_src != 0 and entry.vp != -99999:
                points[i].set_velocity_data(
                    VelocityProperties(
                        entry.vp, entry.vs, entry.rho, None, None,
                        id_cvmh, id_cvmh, id_cvmh, None, None
                    )
                )
            else:
                self._set_velocity_properties_none(points[i])

        return True

    def __del__(self):
        vx_cleanup()