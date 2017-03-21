"""
Defines all the possible errors that can be called with UCVM. This makes for easy error referencing.

:copyright: Southern California Earthquake Center
:author:    David Gill <davidgil@usc.edu>
:created:   August 9, 2016
:modified:  October 17, 2016
"""

_ERROR_LIST = {
    1: "Error initializing UCVM. Could not read model file at %s. Please try reinstalling UCVM."
       "If that doesn't work,",
    2: "Error initializing UCVM. Could not relaunch process.",
    3: "No velocity model provided in model string %s. Please add a velocity model. If that "
       "doesn't work,",
    4: "Too many velocity models in model string %s. Please have only one velocity model. If that "
       "doesn't work,",
    5: "Could not find model %s. Please try again. If that doesn't work,",
    6: "Origin point not specified correctly for horizontal slice. Please try again. If that "
       "doesn't work,",
    7: "The horizontal slice properties are not specified correctly. Please try again. If that "
       "doesn't work,",
    8: "NumPy, Matplotlib, and Basemap must be installed on your system in order to generate these plots. "
       "Please try installing these package. If that doesn't work,",
    9: "The point provided must be an instance of the UCVMPoint namedtuple. Please revise your "
       "code and try again. If that doesn't work,",
    10: "%s must be a number.",
    11: "Depth must be a positive number (i.e. z = 100 means 100m below the surface).",
    12: "The point must represent either elevation relative to sea level (UCVM_ELEVATION) "
        "or depth relative to the surface (UCVM_DEPTH).",
    13: "Model %s was not found. Please try installing the model. If you believe this message to "
        "be in error,",
    14: "Point not specified correctly for depth profile. Please try again. If that doesn't work,",
    15: "Depth profile properties not specified correctly. Please try again. If that doesn't work,",
    16: "Starting and ending points must be of type Point. Please try again. If that doesn't work,",
    17: "Cross-section properties not specified correctly. Please try again. If that doesn't work,",
    18: "Could not load config.xml for gridded velocity model. Please try launching UCVM again. If "
        "that doesn't work,",
    19: "Model type not found. Please try again. If that doesn't work,",
    20: "Two or more of the same model type were provided in the same model string (e.g. "
        "cvms4.cvms426 which combines two velocity models). Please fix and try again.",
    21: "No file was specified as a parameter to the data product reader model. Please call the "
        "model like this: dataproductreader[xml file name]. If that doesn't work,",
    22: "Could not load model. Please try reinstalling the model by running ucvm_model_manager -a %s. "
        "If that doesn't work,"
}


class UCVMError(Exception):
    pass


def display_and_raise_error(errcode: int, replacements: tuple=None) -> None:
    """
    Displays the error defined by code "errcode".
    :param errcode: The error code.
    :param replacements: A tuple of replacements. Optional.
    :return: Nothing.
    """
    if replacements is None:
        replacements = ()

    if _ERROR_LIST[errcode][:-1] is not ".":
        print(_ERROR_LIST[errcode] % replacements +
              " please contact software@scec.org with error code " + str(errcode) + ".")
    else:
        print(_ERROR_LIST[errcode] % replacements +
              " Please contact software@scec.org with error code " + str(errcode) + ".")

    raise UCVMError(errcode)
