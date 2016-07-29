import math
import os
import re
import struct

from ucvm.src.framework.ucvm import UCVM
from ucvm.src.shared.constants import UCVM_GRID_TYPE_CENTER, UCVM_GRID_TYPE_VERTEX, \
    UCVM_DEFAULT_PROJECTION, UCVM_DEPTH, UCVM_ELEVATION
from ucvm.src.shared.functions import ask_and_validate, is_number, is_valid_proj4_string, \
    is_acceptable_value
from ucvm.src.shared.properties import Point, SeismicData


def ask_questions() -> dict:
    """
    Asks the questions of the user that are necessary to generate the XML file for the mesh.
    :return: A dictionary containing the answers.
    """
    answers = {
        "cvm_list": "",
        "grid_type": UCVM_GRID_TYPE_CENTER,
        "spacing": 0.0,
        "initial_point": {
            "x": 0,
            "y": 0,
            "z": 0,
            "rotation": 0,
            "depth_elev": 0,
            "projection": ""
        },
        "dimensions": {
            "x": 0,
            "y": 0,
            "z": 0
        },
        "processor_dimensions": {
            "x": 0,
            "y": 0,
            "z": 0
        },
        "minimums": {
            "vp": 0,
            "vs": 0,
            "dn": 0,
            "qp": 0,
            "qs": 0
        },
        "out_dir": "",
        "mesh_name": "",
        "mesh_type": "",
        "scratch_dir": ""
    }

    print("\nGenerating a mesh requires the definition of various parameters to be defined (such \n"
          "as the origin of the mesh, the length of the mesh, and so on). The following questions\n"
          "will guide you through the definition of those parameters. At the end, you will be\n"
          "asked if you want to just generate the configuration file to make the mesh at a later\n"
          "time or if you want to generate the mesh immediately.\n")

    answers["cvm_list"] = ask_and_validate("From which velocity model(s) should this mesh be "
                                           "generated:")

    print("\nMeshes are constructed by specifying a bottom-left origin point, a rotation for the\n"
          "rectangular region, and then a width, height, and depth for the box.\n")

    answers["initial_point"]["projection"] = \
        ask_and_validate("To start, in which projection is your starting point specified?\n"
                         "The default for UCVM is WGS84 latitude and longitude. To accept\n"
                         "that projection, simply hit enter:", is_valid_proj4_string,
                         "The answer must be a valid Proj.4 projection.")

    if answers["initial_point"]["projection"].strip() is "":
        answers["initial_point"]["projection"] = UCVM_DEFAULT_PROJECTION

    answers["initial_point"]["x"] = \
        float(ask_and_validate("\nWhat is the X or longitudinal coordinate of your bottom-left\n"
                               "starting point?", is_number, "Answer must be a number."))
    answers["initial_point"]["y"] = \
        float(ask_and_validate("What is the Y or latitudinal coordinate of your bottom-left\n"
                               "starting point?", is_number, "Answer must be a number."))
    answers["initial_point"]["z"] = \
        float(ask_and_validate("What is the Z or depth/elevation coordinate of your bottom-left\n"
                               "starting point?", is_number, "Answer must be a number."))

    answers["initial_point"]["rotation"] = \
        float(ask_and_validate("What is the rotation angle, in degrees, of this box (relative to\n"
                               "the bottom-left corner)?", is_number, "Answer must a number."))

    answers["initial_point"]["depth_elev"] = \
        ask_and_validate("\nIs your Z coordinate specified as depth (default) or elevation?\n"
                         "Type 'd' or enter for depth, 'e' for elevation:", is_acceptable_value,
                         "Type the character 'd' for depth or 'e' for elevation.",
                         allowed=["d", "e", ""])

    if answers["initial_point"]["depth_elev"] is "e":
        answers["initial_point"]["depth_elev"] = "elevation"
    else:
        answers["initial_point"]["depth_elev"] = "depth"

    answers["spacing"] = ask_and_validate("\nIn meters, what should the spacing between each grid "
                                          "point be?", is_number, "The input must be a number.")

    answers["dimensions"]["x"] = int(ask_and_validate("\nHow many grid points should there be in "
                                                      "the X or longitudinal direction?", is_number,
                                                      "Answer must be a number."))
    answers["dimensions"]["y"] = int(ask_and_validate("How many grid points should there be in the "
                                                      "Y or latitudinal direction?", is_number,
                                                      "Answer must be a number."))
    answers["dimensions"]["z"] = int(ask_and_validate("How many grid points should there be in the "
                                                      "Z or depth/elevation direction?", is_number,
                                                      "Answer must be a number."))

    answers["minimums"]["vs"] = int(ask_and_validate("\nWhat should the minimum Vs, in meters, be? "
                                                     "The default is 0: ", is_number,
                                                     "Answer must be a number."))
    answers["minimums"]["vp"] = int(ask_and_validate("What should the minimum Vp, in meters, be? "
                                                     "The default is 0: ", is_number,
                                                     "Answer must be a number."))

    answers["out_dir"] = ask_and_validate("\nTo which directory should the mesh and metadata be "
                                          "saved?")
    answers["scratch_dir"] = ask_and_validate("Please provide a scratch directory for "
                                              "temporary storage:")

    answers["mesh_type"] = ask_and_validate("\nWhat type is this mesh (the most common is IJK-12)?",
                                            is_acceptable_value, "IJK-12", allowed=["IJK-12"])

    answers["mesh_name"] = ask_and_validate("\nPlease provide a name for this mesh:")

    return answers


def mesh_extract(information: dict) -> bool:
    """
    Given a dictionary containing the relevant parameters for the extraction, extract the material
    properties.
    :param information: The dictionary containing the metadata defining the extraction.
    :return: True, when successful. It will raise an error if the extraction is not successful.
    """
    seismic_data_grid = []

    utm_origin_point = Point(
        information["initial_point"]["x"],
        information["initial_point"]["y"],
        information["initial_point"]["z"],
        UCVM_DEPTH if information["initial_point"]["depth_elev"] is "depth" else UCVM_ELEVATION,
        {},
        information["initial_point"]["projection"]
    ).convert_to_projection("+proj=utm +datum=WGS84 +zone=11")

    sin_angle = math.sin(float(information["initial_point"]["rotation"]))
    cos_angle = math.cos(float(information["initial_point"]["rotation"]))
    spacing = float(information["spacing"])

    for y_val in range(0, int(information["dimensions"]["y"])):
        for x_val in range(0, int(information["dimensions"]["x"])):
            seismic_data_grid.append(
                SeismicData(
                    Point(
                        utm_origin_point.x_value + x_val * spacing * cos_angle,
                        utm_origin_point.y_value + y_val * spacing * sin_angle,
                        utm_origin_point.z_value, utm_origin_point.depth_elev, {},
                        "+proj=utm +datum=WGS84 +zone=11"
                    ).convert_to_projection(UCVM_DEFAULT_PROJECTION)
                )
            )

    parsed_model_string = UCVM.parse_model_string(information["cvm_list"])
    parsed_model_string = {
        "velocity": parsed_model_string["velocity"],
        "elevation": [],
        "vs30": []
    }

    pattern = re.compile('[\W_]+')
    file_out = pattern.sub('', str(information["mesh_name"]).lower()) + ".data"

    with open(os.path.join(information["out_dir"], file_out), "wb") as fd:
        for z_value in range(0, int(information["dimensions"]["z"])):
            print(z_value)
            for data_point in seismic_data_grid:
                data_point.original_point.z_value = float(information["initial_point"]["z"]) + \
                                                    (spacing * z_value)
            UCVM.query(seismic_data_grid, parsed_model_string)

            fl_array = []
            for s in seismic_data_grid:
                fl_array.append(s.velocity_properties.vp)
                fl_array.append(s.velocity_properties.vs)
                fl_array.append(s.velocity_properties.density)

            s = struct.pack('f' * len(fl_array), *fl_array)
            fd.write(s)
