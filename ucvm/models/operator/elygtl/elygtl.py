"""
Ely GTL Operator

This operator defines the Ely GTL. Roughly the GTL works as a scaling between the Vs30 at the
surface and the material properties from the model at 350m depth.

Copyright:
    Southern California Earthquake Center

Developer:
    David Gill <davidgil@usc.edu>
"""
# Python Imports
import math
from typing import List

# UCVM Imports
from ucvm.src.model.operator import OperatorModel
from ucvm.src.shared.properties import SeismicData, VelocityProperties
from ucvm.src.shared.functions import calculate_nafe_drake_density, calculate_scaled_vp


class ElyGTLOperator(OperatorModel):

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
        ely_coefficients = {
            "a": 1 / 2,
            "b": 2 / 3,
            "c": 3 / 2
        }

        depth = 350
        zmin = 0

        for datum in data:
            new_vs = datum.velocity_properties.vs
            new_vp = datum.velocity_properties.vp
            new_dn = datum.velocity_properties.density
            new_sources = {
                "vp": datum.velocity_properties.vp_source,
                "vs": datum.velocity_properties.vs_source,
                "dn": datum.velocity_properties.density_source
            }

            if new_vs is None or new_vs == 0 or new_vp is None or new_vp == 0 or \
               new_dn is None or new_dn == 0:
                continue

            if datum.converted_point.z_value < zmin:
                new_vs = ely_coefficients["a"] * datum.vs30_properties.vs30
                new_vp = ely_coefficients["a"] * calculate_scaled_vp(new_vs)
                new_dn = calculate_nafe_drake_density(new_vp)
            elif datum.converted_point.z_value < depth:
                z = (datum.converted_point.z_value - zmin) / (depth - zmin)
                f = z - math.pow(z, 2.0)
                g = math.pow(z, 2.0) + 2 * math.pow(z, 0.5) - (3 * z)
                new_vs = (z + ely_coefficients["b"] * f) * new_vs + \
                         (ely_coefficients["a"] - ely_coefficients["a"] * z +
                          ely_coefficients["c"] * g) * datum.vs30_properties.vs30
                new_vp = (z + ely_coefficients["b"] * f) * new_vp + \
                         (ely_coefficients["a"] - ely_coefficients["a"] * z +
                          ely_coefficients["c"] * g) * \
                         calculate_scaled_vp(datum.vs30_properties.vs30)
                new_dn = calculate_nafe_drake_density(new_vp)

                for key, item in new_sources.items():
                    new_sources[key] = \
                        ", ".join([x.strip() for x in new_sources[key].split(",")]) + ", Ely GTL"

            datum.set_velocity_data(
                VelocityProperties(
                    new_vp, new_vs, new_dn, datum.velocity_properties.qp,
                    datum.velocity_properties.qs, new_sources["vp"], new_sources["vs"],
                    new_sources["dn"], datum.velocity_properties.qp_source,
                    datum.velocity_properties.qs_source
                )
            )

        return True
