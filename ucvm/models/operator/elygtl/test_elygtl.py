"""
Defines the tests for the Ely GTL operator within UCVM.

Copyright:
    Southern California Earthquake Center

Developer:
    David Gill <davidgil@usc.edu>
"""
# UCVM Imports
from ucvm.models.elygtl.elygtl import ElyGTLOperator
from ucvm.src.shared.properties import SeismicData, Vs30Properties, VelocityProperties, Point
from ucvm.src.shared.test import UCVMTestCase


class ElyGTLOperatorTest(UCVMTestCase):
    """
    Defines the test cases for the Ely GTL operator.
    """
    description = "Ely GTL"

    def test_elygtl_query(self):
        """
        Tests that the Ely GTL delivers the correct scaling.

        Returns:
            None
        """
        self._test_start("test of Ely GTL scaling parameters")

        test_property = SeismicData()
        test_property.vs30_properties = Vs30Properties(vs30=100, vs30_source="test")
        test_property.velocity_properties = VelocityProperties(vs=1000, vp=2000, density=2000,
                                                               qp=None, qs=None, vs_source="test",
                                                               vp_source="test",
                                                               density_source="test",
                                                               qp_source=None, qs_source=None)
        test_property.converted_point = Point(-118, 34, 0)
        test_property.original_point = Point(-118, 34, 0)

        e = ElyGTLOperator()
        e._query([test_property])

        self.assertEqual(test_property.velocity_properties.vs, 50)
        self.assertIn("Ely GTL", test_property.velocity_properties.vs_source)

        self._test_end()
