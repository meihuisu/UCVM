"""
Defines the tests for the models within UCVM. Each model (optionally) has a test_modelname.py file
which has a TestCase class called ModelNameTest that is dynamically loaded and added to the test
suite when this is invoked.

:copyright: Southern California Earthquake Center
:author:    David Gill <davidgil@usc.edu>
:created:   September 1, 2016
:modified:  September 20, 2016
"""
import unittest
import os
import types

from typing import List

from ucvm.src.framework.ucvm import UCVM
from ucvm.src.shared.properties import Point, SeismicData


class UCVMModelAcceptanceTest(unittest.TestCase):

    def setUp(self):
        self.data = {
            "none": []
        }


def add_acceptance_test_methods(target: unittest.TestCase, name: str):

    def custom_range(start: float, stop: float, increment: float) -> iter:
        while start < stop:
            yield start
            start += increment

    def _test_vpvs_ratio(sd_array: List[SeismicData]):
        for item in sd_array:
            if item.velocity_properties is not None \
                    and item.velocity_properties.vp is not None \
                    and item.velocity_properties.vp > 0 \
                    and item.velocity_properties.vs is not None \
                    and item.velocity_properties.vs > 0:
                target.assertGreaterEqual(
                    self=target,
                    a=item.velocity_properties.vp / item.velocity_properties.vs,
                    b=1.45,
                    msg="Vp/Vs ratio is less than 1.45 at point (%.2f, %.2f, %.2f)" %
                    (item.original_point.x_value, item.original_point.y_value,
                     item.original_point.z_value)
                )

    def _add_corner_and_depth_information():
        target.corners = {
            "bl": {
                "e": None,
                "n": None
            },
            "tr": {
                "e": None,
                "n": None
            }
        }

        target.max_depth = 0

        for _, model_dict in UCVM.parse_model_string(name).items():
            for _, model in model_dict.items():
                model_cpy = model.split(";")[0]
                if UCVM.get_model_type(model_cpy) == "velocity":
                    coverage = UCVM.get_model_instance(model_cpy).get_metadata()["coverage"]
                    if "depth" in coverage and float(coverage["depth"]) > target.max_depth:
                        target.max_depth = float(coverage["depth"])
                    elif "depth" not in coverage and 50000 < target.max_depth:
                        target.max_depth = 50000

                    if coverage["bottom_left"]["e"] is not None and \
                            (target.corners["bl"]["e"] is None or
                             float(coverage["bottom_left"]["e"]) < target.corners["bl"]["e"]):
                        target.corners["bl"]["e"] = float(coverage["bottom_left"]["e"])
                    if coverage["bottom_left"]["n"] is not None and \
                            (target.corners["bl"]["n"] is None or
                             float(coverage["bottom_left"]["n"]) < target.corners["bl"]["n"]):
                        target.corners["bl"]["n"] = float(coverage["bottom_left"]["n"])
                    if coverage["top_right"]["e"] is not None and \
                            (target.corners["tr"]["e"] is None or
                             float(coverage["top_right"]["e"]) > target.corners["tr"]["e"]):
                        target.corners["tr"]["e"] = float(coverage["top_right"]["e"])
                    if coverage["top_right"]["n"] is not None and \
                            (target.corners["tr"]["n"] is None or
                             float(coverage["top_right"]["n"]) > target.corners["tr"]["n"]):
                        target.corners["tr"]["n"] = float(coverage["top_right"]["n"])
                    break

        if target.max_depth == 0:
            target.max_depth = 50000

    def vpvs_ratio_test(_: unittest.TestCase):

        granularity = 0.001
        depth_spacing = 10000

        counter = 0
        sd_array = UCVM.create_max_seismicdata_array()

        for x_val in \
                custom_range(float(target.corners["bl"]["e"]),
                             float(target.corners["tr"]["e"]) + granularity,
                             granularity):
            for y_val in \
                    custom_range(float(target.corners["bl"]["n"]),
                                 float(target.corners["tr"]["n"]) + granularity,
                                 granularity):
                for z_val in custom_range(0, target.max_depth + depth_spacing, depth_spacing):
                    sd_array[counter].original_point = Point(x_val, y_val, z_val)
                    sd_array[counter].converted_point = Point(x_val, y_val, z_val)

                    if counter == len(sd_array) - 1:
                        UCVM.query(sd_array, name, ["velocity"])
                        _test_vpvs_ratio(sd_array)
                        counter = 0
                    else:
                        counter += 1

        _test_vpvs_ratio(sd_array[0:counter])

    _add_corner_and_depth_information()
    setattr(target, "test_" + name + "_vpvs_acceptance", types.MethodType(vpvs_ratio_test, target))


def make_suite() -> unittest.TestSuite:
    suite = unittest.TestSuite()

    models = UCVM.get_list_of_installed_models()

    for key in models:
        for model in models[key]:
            path = os.path.join(UCVM.get_model_instance(model["id"]).model_location, "test_" +
                                model["id"] + ".py")
            if os.path.exists(path):
                new_class = __import__("ucvm.models." + model["id"] + ".test_" + model["id"],
                                       fromlist=model["class"] + "Test")
                suite.addTest(
                    unittest.makeSuite(getattr(new_class, model["class"] + "Test"), "test_")
                )
            if key == "velocity":
                add_acceptance_test_methods(UCVMModelAcceptanceTest, model["id"])

    suite.addTest(unittest.makeSuite(UCVMModelAcceptanceTest, "test_"))

    return suite

if __name__ == "__main__":
    unittest.TextTestRunner(verbosity=2).run(make_suite())
