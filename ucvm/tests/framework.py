"""
Defines all the tests for the UCVM framework. This tests basic aspects like model loading as well as more complex
aspects like proper model parsing, and so on.

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
from contextlib import redirect_stdout
from io import StringIO
import sys
import unittest

# UCVM Imports
from ucvm.src.framework.ucvm import UCVM
from ucvm.src.shared.properties import SeismicData, Point
from ucvm.src.shared.constants import UCVM_ELEVATION
from ucvm.src.shared.errors import UCVMError

try:
    import ucvm.tests.test_model as test_model
except ImportError:
    test_model = __import__("test_model")


class UCVMFrameworkTest(unittest.TestCase):
    """
    Tests that the core capabilities of the UCVM framework work. Example core capabilities include that the model
    strings can be parsed correctly, that query works and returns proper values, and so on. This makes use of the
    TestVelocityModel which allows us to verify returned values.
    """

    def test_ucvm_parse_model_string(self):
        """
        Test that UCVM can parse the model strings correctly. This just tests the actual parsing for the syntax (i.e.
        brackets, semi-colons, parameters, etc.)
        """
        # Test that one model parses correctly.
        self.assertEqual(
            UCVM.parse_model_string("velocity"),
            {0: {0: "velocity"}}
        )

        # Test that semi-colon separates models.
        self.assertEqual(
            UCVM.parse_model_string("velocity1;velocity2"),
            {0: {0: "velocity1"}, 1: {0: "velocity2"}}
        )

        # Test that operators/additional models parse correctly and that parameters parse correctly.
        self.assertEqual(
            UCVM.parse_model_string("velocity1.operator[500];velocity2"),
            {0: {0: "velocity1", 1: "operator;-;500"}, 1: {0: "velocity2"}}
        )

        # Check that parentheses allow for modifiers to apply to multiple models.
        self.assertEqual(
            UCVM.parse_model_string("(velocity1;velocity2).modifier1[50].modifier2;velocity3"),
            {
                0: {0: "velocity1", 1: "modifier1;-;50", 2: "modifier2"},
                1: {0: "velocity2", 1: "modifier1;-;50", 2: "modifier2"},
                2: {0: "velocity3"}
            }
        )

        # Second check that readjusting order still allows for parentheses to apply correctly.
        self.assertEqual(
            UCVM.parse_model_string("velocity3;(velocity1;velocity2).modifier1[50].modifier2"),
            {
                0: {0: "velocity3"},
                1: {0: "velocity1", 1: "modifier1;-;50", 2: "modifier2"},
                2: {0: "velocity2", 1: "modifier1;-;50", 2: "modifier2"}
            }
        )

        # Third check that just the parentheses allow for models to be parsed correctly.
        self.assertEqual(
            UCVM.parse_model_string("(velocity1;velocity2).modifier1[50].modifier2"),
            {
                0: {0: "velocity1", 1: "modifier1;-;50", 2: "modifier2"},
                1: {0: "velocity2", 1: "modifier1;-;50", 2: "modifier2"},
            }
        )

        # Check to make sure that if we add modifier1 to a model inside the parentheses that it adds things correctly.
        self.assertEqual(
            UCVM.parse_model_string("(velocity1.modifier1[50];velocity2).modifier2"),
            {
                0: {0: "velocity1", 1: "modifier1;-;50", 2: "modifier2"},
                1: {0: "velocity2", 1: "modifier2"}
            }
        )

        # Check to make sure that special characters in parameters don't break it...
        self.assertEqual(
            UCVM.parse_model_string("(velocity1.modifier1[1.0];velocity2).modifier2"),
            {
                0: {0: "velocity1", 1: "modifier1;-;1.0", 2: "modifier2"},
                1: {0: "velocity2", 1: "modifier2"}
            }
        )

    def test_ucvm_load_models(self):
        """
        Test that UCVM can load all the installed models without error.
        """
        installed_models = UCVM.get_list_of_installed_models()

        for key in installed_models:
            for model in installed_models[key]:
                m_py = UCVM.get_model_instance(model["id"])
                self.assertEqual(m_py.get_metadata()["name"], model["name"])
                self.assertEqual(UCVM.instantiated_models[model["id"]].get_metadata()["name"],
                                 model["name"])

    def test_ucvm_query_with_test_velocity_model(self):
        """
        Test that UCVM can query using the test velocity model and return correct material properties.
        """
        UCVM.instantiated_models["testvelocitymodel"] = test_model.TestVelocityModel()
        data_1 = [
            SeismicData(Point(-118, 34, 0))
        ]
        UCVM.query(data_1, "testvelocitymodel", ["velocity"], {
            0: {0: "testvelocitymodel"}
        })
        self.assertEqual(data_1[0].velocity_properties.vp, 34 + (-118))
        self.assertEqual(data_1[0].velocity_properties.vs, 34 - (-118))
        self.assertEqual(data_1[0].velocity_properties.density, (34 + (-118)) / 2)
        self.assertEqual(data_1[0].velocity_properties.qp, (34 - (-118)) / 4)
        self.assertEqual(data_1[0].velocity_properties.qs, (34 + (-118)) / 4)

    def test_ucvm_raises_error_on_bad_model_combinations(self):
        """
        Tests that UCVM errors out gracefully when a bad model name is called.
        """
        data_1 = [
            SeismicData(Point(-118, 34, 0))
        ]
        stdout = sys.stdout
        sys.stdout = None
        with self.assertRaises(UCVMError):
            UCVM.query(data_1, "nonexistantvelocitymodel")
        with self.assertRaises(UCVMError):
            UCVM.query(data_1, None)
        with self.assertRaises(UCVMError):
            UCVM.query(data_1, "")
        with self.assertRaises(UCVMError):
            UCVM.query(data_1, "34[]34")
        with self.assertRaises(UCVMError):
            UCVM.query(data_1, "cvms426 vs30-calc")
        sys.stdout = stdout

    def test_ucvm_raises_error_on_negative_depth(self):
        """
        Tests that you cannot pass a negative depth to UCVM. We test this two ways. First, you cannot pass a negative
        depth to a newly-constructed SeismicData object set to be depth. Second, if we modify the z_value after, we
        should get an error on query complaining about negative depth.
        """
        with self.assertRaises(ValueError):
            SeismicData(Point(-118, 34, -500))

        s = SeismicData(Point(-118, 34, 0))
        s.original_point.z_value = -500
        with self.assertRaises(ValueError):
            UCVM.query([s], "1d[SCEC]")

    def test_ucvm_select_right_models_for_query(self):
        """
        Tests that given a desired set of properties, UCVM picks the correct models to query (for example if we are
        querying a depth model by elevation, we need to get the DEM).
        """
        self.assertEqual(
            UCVM.get_models_for_query("1d.depth", ["velocity"]),
            {
                0: {0: "1d"}
            }
        )
        self.assertEqual(
            UCVM.get_models_for_query("1d", ["velocity"]),
            {
                0: {0: "1d"}
            }
        )
        self.assertEqual(
            UCVM.get_models_for_query("1d.elevation", ["velocity"]),
            {
                0: {0: "usgs-noaa", 1: "1d"}
            }
        )
        self.assertEqual(
            UCVM.get_models_for_query("usgs-noaa.1d.vs30-calc.elevation",
                                      ["velocity", "elevation", "vs30"]),
            {
                0: {0: "usgs-noaa", 1: "1d", 2: "vs30-calc"}
            }
        )

    def test_ucvm_get_model_type(self):
        """
        Tests that given a model, it correctly identifies the model type. Because we cannot guarantee that any
        operators will be installed on the user's system, we don't test for this.
        """
        self.assertEqual(UCVM.get_model_type("1d"), "velocity")
        self.assertEqual(UCVM.get_model_type("usgs-noaa"), "elevation")
        self.assertEqual(UCVM.get_model_type("wills-wald-2006"), "vs30")

    def test_ucvm_get_model_instance(self):
        """
        Tests that UCVM can retrieve a model instance correctly. Also tests to ensure that providing a bad model to
        this raises the correct error.
        """
        self.assertEqual(UCVM.get_model_instance("1d").get_metadata()["id"], "1d")
        stdout = sys.stdout
        sys.stdout = None
        with self.assertRaises(UCVMError):
            UCVM.get_model_instance("bob")
        sys.stdout = stdout

    def test_ucvm_get_list_of_installed_models(self):
        """
        Tests that we can retrieve the list of installed models. This helps verify that there are no permission
        problems reading the model XML file for example.
        """
        models = UCVM.get_list_of_installed_models()
        self.assertTrue("velocity" in models)
        self.assertTrue(len(models["velocity"]) >= 1)
        self.assertTrue("elevation" in models)
        self.assertTrue(len(models["elevation"]) >= 1)
        self.assertTrue("vs30" in models)
        self.assertTrue(len(models["vs30"]) >= 1)

    def test_ucvm_print_version(self):
        """
        Tests that the replacements are done correctly when printing UCVM's version info.
        """
        f = StringIO()
        with redirect_stdout(f):
            UCVM.print_version()
        out = f.getvalue()
        self.assertIn("17.3.0", out)
        self.assertIn("2017", out)
        self.assertIn("LICENSE", out)

    def test_ucvm_same_points_depth_or_elevation(self):
        """
        Tests that querying by elevation using the UCVM built-in DEM works. Model-specific DEMs are compared within
        their respective model test codes.
        """
        # The elevation at -118, 34 is 288.997m. We use that for the elevation test.
        s = SeismicData(Point(-118, 34, 0))
        UCVM.query([s], "1d[SCEC]")
        velocity_properties_by_depth = s.velocity_properties
        s = SeismicData(Point(-118, 34, 288.99689, UCVM_ELEVATION))
        UCVM.query([s], "1d[SCEC]")
        velocity_properties_by_elevation = s.velocity_properties

        self.assertEqual(velocity_properties_by_depth.vs, velocity_properties_by_elevation.vs)
        self.assertEqual(velocity_properties_by_depth.vp, velocity_properties_by_elevation.vp)
        self.assertEqual(velocity_properties_by_depth.density, velocity_properties_by_elevation.density)

    def test_ucvm_returns_nothing_for_elevation_in_air(self):
        """
        Tests that querying a point in air with a model that is queryable by depth returns N/A for material properties
        across the board.
        """
        s = SeismicData(Point(-118, 34, 1000, UCVM_ELEVATION))
        UCVM.query([s], "1d[SCEC].elevation")
        self.assertIsNone(s.velocity_properties.vp)
        self.assertIsNone(s.velocity_properties.vs)
        self.assertIsNone(s.velocity_properties.density)


def make_suite() -> unittest.TestSuite:
    suite = unittest.TestSuite()
    suite.addTest(unittest.makeSuite(UCVMFrameworkTest, "test_"))
    return suite

if __name__ == "__main__":
    unittest.TextTestRunner(verbosity=2).run(make_suite())
