import unittest

from ucvm.src.shared.properties import SeismicData, VelocityProperties


def assert_velocity_properties(test_case: unittest.TestCase, data: SeismicData,
                               velocity_prop: VelocityProperties):
    test_case.assertIsNotNone(data)
    test_case.assertIsNotNone(data.velocity_properties)
    test_case.assertIsNotNone(velocity_prop)

    for prop in dir(data.velocity_properties):
        if "_" in prop[0:1] or prop == "count" or prop == "index":
            continue

        if getattr(data.velocity_properties, prop) is None:
            test_case.assertIsNone(getattr(velocity_prop, prop))
        else:
            try:
                float(getattr(data.velocity_properties, prop))
                test_case.assertAlmostEqual(getattr(data.velocity_properties, prop),
                                            getattr(velocity_prop, prop), 3)
            except ValueError:
                test_case.assertEqual(getattr(data.velocity_properties, prop),
                                      getattr(velocity_prop, prop))
