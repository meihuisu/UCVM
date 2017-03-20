"""
Defines shared objects and functions (like velocity material properties) that are used between
models and the "other" UCVM functions and classes.

:copyright: Southern California Earthquake Center
:author:    David Gill <davidgil@usc.edu>
:created:   July 19, 2016
:modified:  October 5, 2016
"""

from .constants import UCVM_DEFAULT_PROJECTION, UCVM_DEPTH, UCVM_ELEVATION, UCVM_MODEL_LIST_FILE, \
                       UCVM_MODELS_DIRECTORY, HYPOCENTER_MODEL_LIST, HYPOCENTER_PREFIX, \
                       UCVM_LIBRARIES_DIRECTORY, UCVM_LIBRARY_LIST_FILE, UCVM_ELEV_ANY
from .functions import is_number, bilinear_interpolation, calculate_bilinear_value, \
                       parse_xmltodict_one_or_many
from .properties import VelocityProperties, ElevationProperties, Vs30Properties, ZProperties, SimplePoint, \
                        SimpleRotatedRectangle
from .errors import display_and_raise_error
