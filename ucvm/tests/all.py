"""
Launches all the UCVM tests. Please note that since the test suite is comprehensive, this can take
awhile to run.

:copyright: Southern California Earthquake Center
:author:    David Gill <davidgil@usc.edu>
:created:   August 9, 2016
:modified:  August 9, 2016
"""
import unittest

framework = __import__("framework")
mesh = __import__("mesh")


def suite():
    suite2 = unittest.TestSuite()
    suite2.addTest(framework.make_suite())
    suite2.addTest(mesh.make_suite())
    return suite2

if __name__ == "__main__":
    unittest.TextTestRunner(verbosity=2).run(suite())
