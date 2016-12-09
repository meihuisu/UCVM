"""
Defines testing models for use with the UCVM testing framework. The idea is that we know what
each property should be.

In the case of TestVelocityModel:

Vp = Lat + Lon + Depth
Vs = Lat - Lon
Density = (Lat + Lon) / 2
Qp = (Lat - Lon) / 4
Qs = (Lat + Lon) / 4

Vp_source = TestVelocityModel_Vp
Vs_source = TestVelocityModel_Vs
Density_source = TestVelocityModel_Density
Qp_source = TestVelocityModel_Qp
Qs_source = TestVelocityModel_Qs

In the case of TestElevationModel:

Elevation = Abs(Lon) + Abs(Lat)
Elevation_source = TestElevationModel

In the case of TestVs30Model:
Vs30 = Abs(Lon - Lat)
Vs30_source = TestVs30Model

Copyright:
    Southern California Earthquake Center

Author:
    David Gill <davidgil@usc.edu>
"""
# Python Imports
from typing import List

# UCVM Imports
from ucvm.src.model.velocity.velocity_model import VelocityModel
from ucvm.src.shared.properties import SeismicData
from ucvm.src.shared import VelocityProperties, UCVM_DEPTH, UCVM_DEFAULT_PROJECTION


class TestVelocityModel(VelocityModel):

    def __init__(self, **kwargs):
        self._public_metadata = {
            "id": "testvelocitymodel",
            "name": "TestVelocityModel",
            "description": "Tests the velocity model.",
            "website": "http://www.scec.org",
            "references": ["TEST"],
            "license": "Test License",
            "type": "velocity"
        }

        self._private_metadata = {
            "projection": UCVM_DEFAULT_PROJECTION,
            "public": True,
            "defaults": {},
            "query_by": UCVM_DEPTH
        }

        if self._public_metadata is None:   # Fix incorrect PyCharm error.
            super().__init__(**kwargs)

    def _query(self, data: List[SeismicData], **kwargs) -> bool:
        for datum in data:
            datum.set_velocity_data(
                VelocityProperties(
                    datum.converted_point.y_value + datum.converted_point.x_value +
                    datum.converted_point.z_value,
                    datum.converted_point.y_value - datum.converted_point.x_value,
                    (datum.converted_point.y_value + datum.converted_point.x_value) / 2,
                    (datum.converted_point.y_value - datum.converted_point.x_value) / 4,
                    (datum.converted_point.y_value + datum.converted_point.x_value) / 4,
                    "TestVelocityModel_Vp", "TestVelocityModel_Vs", "TestVelocityModel_Density",
                    "TestVelocityModel_Qp", "TestVelocityModel_Qs"
                )
            )

        return True
