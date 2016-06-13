##
#   Handles loading and querying the velocity models.

from ucvm.api.common import Point, MaterialProperty, DEFAULT_PROJECTION

import sys
import getopt

def usage():
    print(get_string("query_usage"))
    return

def query(points, model):
    material_properties = []
       
    for point in points:
        converted_point = point
        if point.projection != DEFAULT_PROJECTION:
            converted_point = point.to_projection(DEFAULT_PROJECTION)
        
        
       
    return material_properties

def cmd_query():
    # Check the options. We will act on them later.
    model = ""
    
    try:
        opts, args = getopt.getopt(sys.argv[1:], "", ["model"])
        for o, a in opts:
            if o in ("--model"):
                model = a
    except getopt.GetoptError:
        usage()
        
    if model == "":
        ucvm_raise_error("param_model")