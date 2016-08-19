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
        self.assertEqual(UCVM.parse_model_string("cvms4"),
                         {0: {0: "cvms4", 1: "usgs_noaa", 2: "vs30_calc", "query_by": None}})
        self.assertEqual(UCVM.parse_model_string("cvms4.depth"),
                         {0: {0: "cvms4", 1: "usgs_noaa", 2: "vs30_calc", "query_by": None}})
        self.assertEqual(UCVM.parse_model_string("cvms4.elevation"),
                         {0: {0: "usgs_noaa", 1: "cvms4", 2: "vs30_calc", "query_by": "depth"}})
        self.assertEqual(UCVM.parse_model_string("cvms4(Extra Stuff).vs30_calc"),
                         {0: {0: "cvms4;Extra Stuff", 1: "usgs_noaa", 2: "vs30_calc",
                              "query_by": None}})
        self.assertEqual(UCVM.parse_model_string("cvms4(ES),cvms4"),
                         {0: {0: "cvms4;ES", 1: "usgs_noaa", 2: "vs30_calc", "query_by": None},
                          1: {0: "cvms4", 1: "usgs_noaa", 2: "vs30_calc", "query_by": None}})

    def test_ucvm_load_model(self):
        """
        Test that UCVM can load both C models (like CVM-S4) and Python models (like USGS/NOAA).
        :return: None
        """
        m_py = UCVM.get_model_instance("usgs_noaa")
        self.assertEqual(m_py.get_metadata()["name"], "USGS/NOAA Digital Elevation Model")
        self.assertEqual(UCVM.instantiated_models["usgs_noaa"].get_metadata()["name"],
                         "USGS/NOAA Digital Elevation Model")
        m_py = UCVM.get_model_instance("usgs_noaa")
        self.assertEqual(m_py.get_metadata()["name"], "USGS/NOAA Digital Elevation Model")
        self.assertEqual(UCVM.instantiated_models["usgs_noaa"].get_metadata()["name"],
                         "USGS/NOAA Digital Elevation Model")
        m_c = UCVM.get_model_instance("cvms4")
        self.assertEqual(m_c.get_metadata()["name"], "CVM-S4")
        self.assertEqual(UCVM.instantiated_models["cvms4"].get_metadata()["name"], "CVM-S4")

    def test_ucvm_query_capabilities(self):
        """
        Tests UCVM's query capabilities.
        :return: None
        """
        data_1 = []
        self.assertTrue(UCVM.query(data_1, "cvms4"))
        data_2 = [
            SeismicData(Point(-118, 34, 0))
        ]
        UCVM.query(data_2, "cvms4")
        self.assertEqual(data_2[0].velocity_properties.vs, 213)
        self.assertAlmostEqual(data_2[0].elevation_properties.elevation, 276.99999870)
        self.assertAlmostEqual(data_2[0].vs30_properties.vs30, 354.63709123)
        data_3 = [
            SeismicData(Point(-118, 34, 276.99, UCVM_ELEVATION))
        ]
        UCVM.query(data_3, "cvms4.elevation")
        self.assertEqual(data_3[0].velocity_properties.vs, 213)
        self.assertAlmostEqual(data_3[0].elevation_properties.elevation, 276.99999870)
        self.assertAlmostEqual(data_3[0].vs30_properties.vs30, 354.63709123)
        with redirect_stdout(open(os.devnull, "w")):
            with self.assertRaises(UCVMError):
                UCVM.query(data_3, "cvms4jibberish.elevation")
        data_4 = [
            SeismicData(Point(-118, 34, 0)),
            SeismicData(Point(-118, 34, 16)),
            SeismicData(Point(-118, 34, 20)),
            SeismicData(Point(-118, 34, 100000))
        ]
        UCVM.query(data_4, "1d(Whittier Narrows)")
        self.assertEqual(data_4[0].velocity_properties.vs, 450)
        self.assertEqual(data_4[0].velocity_properties.vp, 1700)
        self.assertEqual(data_4[0].velocity_properties.density, 2000)
        self.assertEqual(data_4[0].velocity_properties.qp, 45.0)
        self.assertEqual(data_4[0].velocity_properties.qs, 22.5)
        self.assertEqual(data_4[1].velocity_properties.vs, 950)
        self.assertEqual(data_4[1].velocity_properties.vp, 1900)
        self.assertEqual(data_4[1].velocity_properties.density, 2100)
        self.assertEqual(data_4[1].velocity_properties.qp, 95.0)
        self.assertEqual(data_4[1].velocity_properties.qs, 47.5)
        self.assertEqual(data_4[2].velocity_properties.vs, 1150)
        self.assertEqual(data_4[2].velocity_properties.vp, 2000)
        self.assertEqual(data_4[2].velocity_properties.density, 2200)
        self.assertEqual(data_4[2].velocity_properties.qp, 115.0)
        self.assertEqual(data_4[2].velocity_properties.qs, 57.5)
        self.assertEqual(data_4[3].velocity_properties.vs, 4500)
        self.assertEqual(data_4[3].velocity_properties.vp, 7800)
        self.assertEqual(data_4[3].velocity_properties.density, 3200)
        self.assertEqual(data_4[3].velocity_properties.qp, 450.0)
        self.assertEqual(data_4[3].velocity_properties.qs, 225.0)
        UCVM.query(data_4, "1d(Whittier Narrows,linear)")
        self.assertEqual(data_4[0].velocity_properties.vs, 450)
        self.assertEqual(data_4[0].velocity_properties.vp, 1700)
        self.assertEqual(data_4[0].velocity_properties.density, 2000)
        self.assertEqual(data_4[0].velocity_properties.qp, 45.0)
        self.assertEqual(data_4[0].velocity_properties.qs, 22.5)
        self.assertEqual(data_4[1].velocity_properties.vs, 1050)
        self.assertEqual(data_4[1].velocity_properties.vp, 1950)
        self.assertEqual(data_4[1].velocity_properties.density, 2150)
        self.assertEqual(data_4[1].velocity_properties.qp, 105.0)
        self.assertEqual(data_4[1].velocity_properties.qs, 52.5)
        self.assertEqual(data_4[2].velocity_properties.vs, 1150)
        self.assertEqual(data_4[2].velocity_properties.vp, 2000)
        self.assertEqual(data_4[2].velocity_properties.density, 2200)
        self.assertEqual(data_4[2].velocity_properties.qp, 115.0)
        self.assertEqual(data_4[2].velocity_properties.qs, 57.5)
        self.assertEqual(data_4[3].velocity_properties.vs, 4500)
        self.assertEqual(data_4[3].velocity_properties.vp, 7800)
        self.assertEqual(data_4[3].velocity_properties.density, 3200)
        self.assertEqual(data_4[3].velocity_properties.qp, 450.0)
        self.assertEqual(data_4[3].velocity_properties.qs, 225.0)
        data_5 = [
            SeismicData(Point(-118, 34, 276.95, UCVM_ELEVATION)),
            SeismicData(Point(-118, 34, 256.95, UCVM_ELEVATION)),
            SeismicData(Point(-118, 34, -1000000, UCVM_ELEVATION))
        ]
        UCVM.query(data_5, "1d(Whittier Narrows).elevation")
        self.assertEqual(data_5[0].velocity_properties.vs, 450)
        self.assertEqual(data_5[0].velocity_properties.vp, 1700)
        self.assertEqual(data_5[0].velocity_properties.density, 2000)
        self.assertEqual(data_5[0].velocity_properties.qp, 45.0)
        self.assertEqual(data_5[0].velocity_properties.qs, 22.5)
        self.assertEqual(data_5[1].velocity_properties.vs, 1150)
        self.assertEqual(data_5[1].velocity_properties.vp, 2000)
        self.assertEqual(data_5[1].velocity_properties.density, 2200)
        self.assertEqual(data_5[1].velocity_properties.qp, 115.0)
        self.assertEqual(data_5[1].velocity_properties.qs, 57.5)
        self.assertEqual(data_5[2].velocity_properties.vs, 4500)
        self.assertEqual(data_5[2].velocity_properties.vp, 7800)
        self.assertEqual(data_5[2].velocity_properties.density, 3200)
        self.assertEqual(data_5[2].velocity_properties.qp, 450.0)
        self.assertEqual(data_5[2].velocity_properties.qs, 225.0)

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
    suite.addTest(unittest.makeSuite(UCVMFrameworkTest, "test_"))
    return suite

if __name__ == "__main__":
    unittest.TextTestRunner(verbosity=2).run(make_suite())
