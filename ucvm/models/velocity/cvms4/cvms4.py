"""
Defines the CVM-S4 model UCVM Python interface.

:copyright: Southern California Earthquake Center
:author:    David Gill <davidgil@usc.edu>
:created:   July 12, 2016
:modified:  July 29, 2016
"""
import os
import inspect

from abc import abstractmethod
from ctypes import c_char_p, c_int, c_float, byref
from typing import List

from ucvm.src.model.velocity.legacy import LegacyVelocityModel
from ucvm.src.shared import VelocityProperties
from ucvm.src.shared.properties import SeismicData


class CVMS4VelocityModel(LegacyVelocityModel):

    def __init__(self, **kwargs):
        super().__init__(**kwargs)

        mp = c_char_p(os.path.join(os.path.dirname(inspect.getfile(self.__class__)), "data").
                      encode("ASCII"))
        errcode = c_int(0)

        if self._shared_object["has_initialized"] is False:
            self._shared_object["object"].cvms_init_(mp, byref(errcode))

        if errcode.value != 0:
            raise RuntimeError("CVM-S4 not initialized properly.")

    def _query(self, points: List[SeismicData]) -> bool:
        cvms_lon = (len(points) * c_float)()
        cvms_lat = (len(points) * c_float)()
        cvms_dep = (len(points) * c_float)()

        i = 0

        for point in points:
            cvms_lon[i] = c_float(point.converted_point.x_value)
            cvms_lat[i] = c_float(point.converted_point.y_value)
            cvms_dep[i] = c_float(point.converted_point.z_value)
            i += 1

        cvms_vp = (len(points) * c_float)()
        cvms_vs = (len(points) * c_float)()
        cvms_rho = (len(points) * c_float)()

        nn = c_int(len(points))
        errcode = c_int(0)

        self._shared_object["object"].cvms_query_(byref(nn), cvms_lon, cvms_lat, cvms_dep, cvms_vp,
                                                  cvms_vs, cvms_rho, byref(errcode))

        # Now we need to go through the material properties and add them to the SeismicData objects.
        id_s4 = self._public_metadata["id"]

        for i in range(0, len(points)):
            points[i].set_velocity_data(
                VelocityProperties(
                    cvms_vp[i], cvms_vs[i], cvms_rho[i],    # From CVM-S4
                    None, None,                             # No Qp or Qs defined
                    id_s4, id_s4, id_s4,                    # All data comes direct from CVM-S4
                    None, None                              # No Qp or Qs defined
                )
            )

        return True

    @staticmethod
    @abstractmethod
    def get_all_models() -> List[str]:
        """
        Get all models of this type. So if we call VelocityModel.get_all_models() we get all
        velocity models registered with UCVM.
        :return: A list of string identifiers for each model.
        """
        pass
