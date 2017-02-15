"""
Defines all the tests for the UCVM meshing framework. This checks to see that the meshing
capabilities work and that things like the InternalMesh class are working.

Copyright:
    Southern California Earthquake Center

Developer:
    David Gill <davidgil@usc.edu>
"""
# Python Imports
import unittest
import xmltodict
import os
import struct
import inspect
import math
from contextlib import redirect_stdout

# UCVM Imports
from ucvm.src.framework.ucvm import UCVM
from ucvm.src.framework.mesh_common import InternalMesh, AWPInternalMeshIterator, RWGInternalMeshIterator
from ucvm.src.shared.properties import SeismicData, Point
from ucvm.src.framework.awp_mesh import mesh_extract_single

import ucvm.tests

try:
    import ucvm.tests.test_model as test_model
except ImportError:
    test_model = __import__("test_model")


class UCVMMeshTest(unittest.TestCase):

    dir = os.path.dirname(inspect.getfile(ucvm.tests))

    def setUp(self):
        self.im_1 = InternalMesh.from_xml_file(
            os.path.join(self.dir, "data", "simple_mesh_ijk12_unrotated.xml")
        )
        self.sd = [SeismicData(Point(-118, 34, 0)) for _ in range(0, 101505)]
        self.im_1.out_dir = os.path.join(self.dir, "scratch")
        self.im_1_iterator_1 = AWPInternalMeshIterator(self.im_1, 0, 101505, 1005, self.sd)
        self.im_1_iterator_2 = AWPInternalMeshIterator(self.im_1, 0, 101505, 101505, self.sd)
        self.im_2 = InternalMesh.from_xml_file(
            os.path.join(self.dir, "data", "simple_mesh_ijk12_rotated.xml")
        )
        self.sd2 = [SeismicData(Point(-118, 34, 0)) for _ in range(0, 51005)]
        self.im_2.out_dir = os.path.join(self.dir, "scratch")
        self.im_2_iterator_1 = AWPInternalMeshIterator(self.im_2, 0, 51005, 505, self.sd2)
        self.im_2_iterator_2 = AWPInternalMeshIterator(self.im_2, 0, 51005, 51005, self.sd2)
        self.im_3 = InternalMesh.from_xml_file(
            os.path.join(self.dir, "data", "simple_utm_mesh_ijk12_rotated.xml")
        )
        self.sd3 = [SeismicData(Point(-118, 34, 0)) for _ in range(0, 101505)]
        self.im_3.out_dir = os.path.join(self.dir, "scratch")
        self.im_3_iterator_1 = AWPInternalMeshIterator(self.im_3, 0, 101505, 202, self.sd3)
        self.im_3_iterator_2 = AWPInternalMeshIterator(self.im_3, 0, 101505, 101505, self.sd3)

    def test_internal_mesh_basics(self):
        """
        Tests that basic mesh parameters work (e.g. that rotation works and returns the expected point and that the
        number of mesh points is correct).
        """
        self.assertEqual(self.im_1.rotation, 0)
        self.assertEqual(self.im_1.spacing, 0.01)
        self.assertEqual(self.im_1.format, "awp")
        self.assertEqual(self.im_1.slice_size, 20301)
        self.assertEqual(self.im_1.total_size, 101505)
        self.assertEqual(self.im_1.get_grid_file_size()["real"], 1218060)
        self.assertEqual(self.im_2.rotation, -45)
        self.assertEqual(self.im_2.spacing, 0.01)
        self.assertEqual(self.im_2.format, "awp")
        self.assertEqual(self.im_2.slice_size, 10201)
        self.assertEqual(self.im_2.total_size, 51005)
        self.assertEqual(self.im_2.get_grid_file_size()["real"], 612060)
        self.assertEqual(self.im_3.rotation, -30)
        self.assertEqual(self.im_3.spacing, 3)
        self.assertEqual(self.im_3.format, "awp")
        self.assertEqual(self.im_3.slice_size, 20301)
        self.assertEqual(self.im_3.total_size, 101505)
        self.assertEqual(self.im_3.get_grid_file_size()["real"], 1218060)

        self.assertAlmostEqual(self.im_3.origin.x_value, 407650.39665729157, 4)
        self.assertAlmostEqual(self.im_3.origin.y_value, 3762606.6598763773, 4)

    def test_internal_mesh_iterator(self):
        """
        Tests that the UCVM mesh iterators work.
        """
        next(self.im_1_iterator_1)
        self.assertAlmostEqual(self.sd[0].original_point.x_value, -118)
        self.assertAlmostEqual(self.sd[0].original_point.y_value, 34)
        self.assertAlmostEqual(self.sd[0].original_point.z_value, 0)
        self.assertAlmostEqual(self.sd[1004].original_point.x_value, -116)
        self.assertAlmostEqual(self.sd[1004].original_point.y_value, 34.04)
        self.assertAlmostEqual(self.sd[1004].original_point.z_value, 0)
        next(self.im_1_iterator_1)
        self.assertAlmostEqual(self.sd[0].original_point.x_value, -118)
        self.assertAlmostEqual(self.sd[0].original_point.y_value, 34.05)
        self.assertAlmostEqual(self.sd[0].original_point.z_value, 0)
        self.assertAlmostEqual(self.sd[1004].original_point.x_value, -116)
        self.assertAlmostEqual(self.sd[1004].original_point.y_value, 34.09)
        self.assertAlmostEqual(self.sd[1004].original_point.z_value, 0)
        next(self.im_1_iterator_2)
        self.assertAlmostEqual(self.sd[101504].original_point.x_value, -116)
        self.assertAlmostEqual(self.sd[101504].original_point.y_value, 35)
        self.assertAlmostEqual(self.sd[101504].original_point.z_value, 0.04)
        with self.assertRaises(StopIteration):
            next(self.im_1_iterator_2)
        next(self.im_2_iterator_1)
        self.assertAlmostEqual(self.sd2[100].original_point.x_value, -118 + 0.707106781)
        self.assertAlmostEqual(self.sd2[100].original_point.y_value, 34 - 0.707106781)
        self.assertEqual(self.sd2[100].original_point.z_value, 0)
        next(self.im_2_iterator_2)
        self.assertAlmostEqual(self.sd2[51004].original_point.x_value, -118 + 2 * 0.707106781)
        self.assertAlmostEqual(self.sd2[51004].original_point.y_value, 34)
        self.assertEqual(self.sd2[51004].original_point.z_value, 0.04)
        with self.assertRaises(StopIteration):
            next(self.im_2_iterator_2)
        next(self.im_3_iterator_1)
        self.assertAlmostEqual(self.sd3[200].original_point.x_value, -117.99434261156043, places=5)
        self.assertAlmostEqual(self.sd3[200].original_point.y_value, 33.99734, places=5)
        self.assertEqual(self.sd3[200].original_point.z_value, 0)
        next(self.im_3_iterator_2)
        self.assertAlmostEqual(self.sd3[101504].original_point.x_value, -117.99274586863415, places=5)
        self.assertAlmostEqual(self.sd3[101504].original_point.y_value, 33.999696, places=5)
        self.assertEqual(self.sd3[101504].original_point.z_value, 12)
        with self.assertRaises(StopIteration):
            next(self.im_3_iterator_2)

    def test_awp_rwg_equivalent(self):
        """
        Quick verification that the AWP and RWG formats are equivalent (i.e. same material properties).
        """
        sd_awp = [SeismicData(Point(-118, 34, 0)) for _ in range(0, 101505)]
        sd_rwg = [SeismicData(Point(-119, 35, 0)) for _ in range(0, 101505)]
        awp_im = InternalMesh.from_xml_file(
            os.path.join(self.dir, "data", "simple_mesh_ijk12_unrotated.xml")
        )
        awp_iterator = AWPInternalMeshIterator(awp_im, 0, 101505, 101505, sd_awp)
        rwg_im = InternalMesh.from_xml_file(
            os.path.join(self.dir, "data", "simple_mesh_rwg_unrotated.xml")
        )
        rwg_iterator = RWGInternalMeshIterator(rwg_im, 0, 101505, 101505, sd_rwg)
        next(awp_iterator)
        next(rwg_iterator)

        counter = 0

        for item_awp in sd_awp:
            # For time reasons, don't test every point. Test every 250 points.
            if counter == 250:
                counter = 0
            else:
                counter += 1
                continue

            found = False
            for item_rwg in sd_rwg:
                if math.isclose(item_rwg.original_point.x_value, item_awp.original_point.x_value) and \
                   math.isclose(item_rwg.original_point.y_value, item_awp.original_point.y_value) and \
                   math.isclose(item_rwg.original_point.z_value, item_awp.original_point.z_value):
                    found = True
                    sd_rwg.remove(item_rwg)
                    break
            if not found:
                print(item_awp.original_point.x_value, item_awp.original_point.y_value, item_awp.original_point.z_value)
                self.assertTrue(False)

        self.assertTrue(True)

    def test_generate_simple_mesh_ijk12_unrotated(self):
        """
        Generates a simple IJK-12 Cartesian mesh, unrotated, and makes sure that the material properties are correct.
        """
        UCVM.instantiated_models["testvelocitymodel"] = test_model.TestVelocityModel()
        with open(os.path.join(self.dir, "data", "simple_mesh_ijk12_unrotated.xml")) as fd:
            simple_mesh_ijk12_xml = xmltodict.parse(fd.read())
        simple_mesh_ijk12_xml = simple_mesh_ijk12_xml["root"]
        simple_mesh_ijk12_xml["out_dir"] = os.path.join(self.dir, "scratch")
        with redirect_stdout(open(os.devnull, "w")):
            self.assertTrue(mesh_extract_single(simple_mesh_ijk12_xml, custom_model_order={
                0: {0: "testvelocitymodel"}
            }))

        start_point = (-118, 34)
        spacing = 0.01

        # Check that the mesh is valid.
        with open(os.path.join(self.dir, "scratch", "simple_mesh_unrotated.awp"), "rb") as fd:
            for z_val in range(0, 5):
                for y_val in range(0, 101):
                    for x_val in range(0, 201):
                        self.assertAlmostEqual(struct.unpack("f", fd.read(4))[0],
                                               (start_point[1] + y_val * spacing) +
                                               (start_point[0] + x_val * spacing) + z_val * spacing,
                                               places=4)
                        self.assertAlmostEqual(struct.unpack("f", fd.read(4))[0],
                                               (start_point[1] + y_val * spacing) -
                                               (start_point[0] + x_val * spacing), places=4)
                        self.assertAlmostEqual(struct.unpack("f", fd.read(4))[0],
                                               ((start_point[1] + y_val * spacing) +
                                               (start_point[0] + x_val * spacing)) / 2, places=4)

    def test_generate_simple_mesh_ijk12_rotated(self):
        """
        Generates a simple IJK-12 Cartesian mesh, rotated, and makes sure that the material properties are correct.
        """
        UCVM.instantiated_models["testvelocitymodel"] = test_model.TestVelocityModel()
        with open(os.path.join(self.dir, "data", "simple_mesh_ijk12_rotated.xml")) as fd:
            simple_mesh_ijk12_xml = xmltodict.parse(fd.read())
        simple_mesh_ijk12_xml = simple_mesh_ijk12_xml["root"]
        simple_mesh_ijk12_xml["out_dir"] = os.path.join(self.dir, "scratch")
        with redirect_stdout(open(os.devnull, "w")):
            self.assertTrue(mesh_extract_single(simple_mesh_ijk12_xml, custom_model_order={
                0: {0: "testvelocitymodel"}
            }))

        i_test = InternalMesh.from_xml_file(
            os.path.join(self.dir, "data", "simple_mesh_ijk12_rotated.xml")
        )

        with open(os.path.join(self.dir, "scratch", "simple_mesh_rotated.awp"), "rb") as fd:
            for z_val in range(0, 5):
                for y_val in range(0, 101):
                    for x_val in range(0, 101):
                        x_prop = -118 + (i_test.cos_angle * x_val - i_test.sin_angle * y_val) * 0.01
                        y_prop = 34 + (i_test.sin_angle * x_val + i_test.cos_angle * y_val) * 0.01

                        self.assertAlmostEqual(struct.unpack("f", fd.read(4))[0],
                                               y_prop + x_prop + z_val * 0.01,
                                               places=4)
                        self.assertAlmostEqual(struct.unpack("f", fd.read(4))[0],
                                               y_prop - x_prop, places=4)
                        self.assertAlmostEqual(struct.unpack("f", fd.read(4))[0],
                                               (y_prop + x_prop) / 2, places=4)

    def test_generate_simple_utm_mesh_ijk12_rotated(self):
        """
        Generates a simple IJK-12 Cartesian mesh, rotated, UTM, and makes sure that the material properties are correct.
        """
        UCVM.instantiated_models["testvelocitymodel"] = test_model.TestVelocityModel()
        UCVM.instantiated_models["testvelocitymodel"]._private_metadata["projection"] = \
            "+proj=utm +zone=11 +datum=WGS84"
        with open(os.path.join(self.dir, "data", "simple_utm_mesh_ijk12_rotated.xml")) as fd:
            simple_mesh_ijk12_xml = xmltodict.parse(fd.read())
        simple_mesh_ijk12_xml = simple_mesh_ijk12_xml["root"]
        simple_mesh_ijk12_xml["out_dir"] = os.path.join(self.dir, "scratch")
        with redirect_stdout(open(os.devnull, "w")):
            self.assertTrue(mesh_extract_single(simple_mesh_ijk12_xml, custom_model_order={
                0: {0: "testvelocitymodel"}
            }))

        i_test = InternalMesh.from_xml_file(
            os.path.join(self.dir, "data", "simple_utm_mesh_ijk12_rotated.xml")
        )

        with open(os.path.join(self.dir, "scratch", "simple_utm_mesh_rotated.awp"), "rb") as fd:
            for z_val in range(0, 5):
                for y_val in range(0, 101):
                    for x_val in range(0, 201):
                        x_prop = 407650.39665729157 + (i_test.cos_angle * x_val -
                                                       i_test.sin_angle * y_val) * 3.0
                        y_prop = 3762606.6598763773 + (i_test.sin_angle * x_val +
                                                       i_test.cos_angle * y_val) * 3.0

                        self.assertAlmostEqual(struct.unpack("f", fd.read(4))[0],
                                               y_prop + x_prop + z_val * 3.0,
                                               places=0)
                        self.assertAlmostEqual(struct.unpack("f", fd.read(4))[0],
                                               y_prop - x_prop, places=0)
                        self.assertAlmostEqual(struct.unpack("f", fd.read(4))[0],
                                               (y_prop + x_prop) / 2, places=0)

    def test_generate_simple_utm_mesh_rwg_rotated(self):
        """
        Generates a simple RWG mesh, rotated, UTM, and makes sure that the material properties are correct.
        """
        UCVM.instantiated_models["testvelocitymodel"] = test_model.TestVelocityModel()
        UCVM.instantiated_models["testvelocitymodel"]._private_metadata["projection"] = \
            "+proj=utm +zone=11 +datum=WGS84"
        with open(os.path.join(self.dir, "data", "simple_utm_mesh_ijk12_rotated.xml")) as fd:
            simple_mesh_rwg_xml = xmltodict.parse(fd.read())
        simple_mesh_rwg_xml = simple_mesh_rwg_xml["root"]
        simple_mesh_rwg_xml["out_dir"] = os.path.join(self.dir, "scratch")
        simple_mesh_rwg_xml["format"] = "rwg"
        with redirect_stdout(open(os.devnull, "w")):
            self.assertTrue(mesh_extract_single(simple_mesh_rwg_xml, custom_model_order={
                0: {0: "testvelocitymodel"}
            }))

        i_test = InternalMesh.from_xml_file(
            os.path.join(self.dir, "data", "simple_utm_mesh_ijk12_rotated.xml")
        )

        with open(os.path.join(self.dir, "scratch", "simple_utm_mesh_rotated.rwgvp"), "rb") as fdvp, \
            open(os.path.join(self.dir, "scratch", "simple_utm_mesh_rotated.rwgvs"), "rb") as fdvs, \
            open(os.path.join(self.dir, "scratch", "simple_utm_mesh_rotated.rwgdn"), "rb") as fddn:
            for y_val in range(0, 101):
                for z_val in range(0, 5):
                    for x_val in range(0, 201):
                        x_prop = 407650.39665729157 + (i_test.cos_angle * x_val -
                                                       i_test.sin_angle * y_val) * 3.0
                        y_prop = 3762606.6598763773 + (i_test.sin_angle * x_val +
                                                       i_test.cos_angle * y_val) * 3.0

                        self.assertAlmostEqual(struct.unpack("f", fdvp.read(4))[0],
                                               (y_prop + x_prop + z_val * 3.0) / 1000,
                                               places=0)
                        self.assertAlmostEqual(struct.unpack("f", fdvs.read(4))[0],
                                               (y_prop - x_prop) / 1000, places=0)
                        self.assertAlmostEqual(struct.unpack("f", fddn.read(4))[0],
                                               ((y_prop + x_prop) / 2) / 1000, places=0)


def make_suite(path: str=None) -> unittest.TestSuite:
    suite = unittest.TestSuite()
    if path is not None:
        UCVMMeshTest.dir = path
    suite.addTest(unittest.makeSuite(UCVMMeshTest, "test_"))
    return suite

if __name__ == "__main__":
    unittest.TextTestRunner(verbosity=2).run(make_suite())
