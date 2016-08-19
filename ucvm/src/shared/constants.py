import os
import inspect
import pkg_resources

import ucvm.models
import ucvm.src.framework

UCVM_DEFAULT_PROJECTION = "+proj=latlong +datum=WGS84"  #: str: The default Proj.4 projection.
UCVM_DEFAULT_DEM = "usgs_noaa"                          #: str: The default UCVM elevation model.
UCVM_DEFAULT_VS30 = "vs30_calc"                         #: str: The default Vs30 model.
UCVM_DEFAULT_INTERPOLATION = "trilinear"                #: str: The default interpolation method.

UCVM_DEPTH = 0                                          #: int: UCVM constant for query by depth.
UCVM_ELEVATION = 1                                      #: int: UCVM constant for query by elev.

UCVM_GRID_TYPE_CENTER = 0                               #: int: Meshing grid type center.
UCVM_GRID_TYPE_VERTEX = 1                               #: int: Meshing grid type vertex.

UCVM_MODELS_DIRECTORY = os.path.dirname(inspect.getfile(ucvm.models))        #: str: Model dir.
UCVM_MODEL_LIST_FILE = os.path.join(UCVM_MODELS_DIRECTORY, "installed.xml")  #: str: XML model list.
HYPOCENTER_PREFIX = "http://hypocenter.usc.edu/research/ucvm/" + \
                     pkg_resources.require("ucvm")[0].version                #: str: D/l location.
HYPOCENTER_MODEL_LIST = HYPOCENTER_PREFIX + "/model_list.xml"                #: str: Model list loc.
INTERNAL_DATA_DIRECTORY = os.path.join(
    os.path.dirname(inspect.getfile(ucvm.src.framework)), "internal_data"
)
