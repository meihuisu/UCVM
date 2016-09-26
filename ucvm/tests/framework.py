"""
Defines all the tests for the UCVM framework. This tests basic aspects like model loading as well
as more complex aspects like proper model parsing, and so on.

:copyright: Southern California Earthquake Center
:author:    David Gill <davidgil@usc.edu>
:created:   August 9, 2016
:modified:  August 9, 2016
"""
import unittest
import os

from contextlib import redirect_stdout

from ucvm.src.framework.ucvm import UCVM
from ucvm.src.shared.properties import SeismicData, Point
from ucvm.src.shared import UCVM_ELEVATION
from ucvm.src.shared.errors import UCVMError

try:
    import ucvm.tests.test_model as test_model
except ImportError:
    test_model = __import__("test_model")


class UCVMFrameworkTest(unittest.TestCase):

    def setUp(self):
        pass

    def test_ucvm_parse_model_string(self):
        """
        Test that the model strings can be parsed correctly.
        :return: None
        """
        self.assertEqual(UCVM.parse_model_string(""), {})

    def test_ucvm_load_model(self):
        """
        Test that UCVM can load all the installed models.
        :return: None
        """
        installed_models = UCVM.get_list_of_installed_models()

        for key in installed_models:
            for model in installed_models[key]:
                m_py = UCVM.get_model_instance(model["id"])
                self.assertEqual(m_py.get_metadata()["name"], model["name"])
                self.assertEqual(UCVM.instantiated_models[model["id"]].get_metadata()["name"],
                                 model["name"])

    def test_ucvm_query_with_test_velocity_model(self):
        UCVM.instantiated_models["testvelocitymodel"] = test_model.TestVelocityModel()
        data_1 = [
            SeismicData(Point(-118, 34, 0))
        ]
        UCVM.query(data_1, "testvelocitymodel", ["velocity"], {
            0: {0: "testvelocitymodel", "query_by": "depth"}
        })
        self.assertEqual(data_1[0].velocity_properties.vp, 34 + (-118))
        self.assertEqual(data_1[0].velocity_properties.vs, 34 - (-118))
        self.assertEqual(data_1[0].velocity_properties.density, (34 + (-118)) / 2)
        self.assertEqual(data_1[0].velocity_properties.qp, (34 - (-118)) / 4)
        self.assertEqual(data_1[0].velocity_properties.qs, (34 + (-118)) / 4)


def make_suite() -> unittest.TestSuite:
    suite = unittest.TestSuite()
    suite.addTest(unittest.makeSuite(UCVMFrameworkTest, "test_ucvm_"))
    return suite

if __name__ == "__main__":
    unittest.TextTestRunner(verbosity=2).run(make_suite())
