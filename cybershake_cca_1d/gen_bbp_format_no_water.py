"""
Generates the model in BBP format. Looks for land-only points. Also plots and saves the calculated
locations of each point.

NOTE: This script will generate a 1D velocity model that exceeds the maximum layers allowed by
      default with UCVM. To fix this, and use this model with UCVM 15.10.0, ucvm_model_bbp1d.c
      needs to be modified by changing line 13 from #define UCVM_BBP1D_MAX_Z_DIM 64 to
      #define UCVM_BBP1D_MAX_Z_DIM 128. This will avoid the "invalid 1D Z dimension size" error.

To run this script to regenerate the 1D velocity profile we used for CyberShake, you will need two
things.

1) A working UCVM 15.10.0 installation. This installation requires no velocity models for this
   script to run successfully.
2) The Central California iteration 6 ASCII data file.

To run, change the variable UCVM_LOCATION to your UCVM installation directory. Also, place the
iteration 6 data file in the working directory of this script. Alternatively, you can edit DATA_FILE
to point to the iteration 6 location.

You will get two files as output:

1) cs_cca_1d_bbp.conf       - The 1D average profile over land points within the CyberShake region
                              outputted in BBP format.
2) cs_included_points.png   - A graphic depicting the included points within the CyberShake box.

The contents of the cs_cca_1d_bbp.conf file should be placed directly into the bbp1d.conf file
within the models/1d directory of UCVM. Then this model can be queried via ucvm_query -m bbp1d.

:copyright: Southern California Earthquake Center
:author:    David Gill <davidgil@usc.edu>
:created:   July 8, 2016
:modified:  July 11, 2016
"""

import math
from subprocess import Popen, PIPE, STDOUT

import numpy as np
import pyproj
import matplotlib.pyplot as plt
from mpl_toolkits import basemap

CYBERSHAKE_BOX = {
    "bl": {"e": -119.93, "n": 33.80},
    "tl": {"e": -121.51, "n": 35.52},
    "tr": {"e": -119.92, "n": 36.50},
    "br": {"e": -118.35, "n": 34.76}
}  #: dict: Defines the CyberShake box in long, lat format.

MODEL_CORNERS = {
    "tl": {"e": 504472.106530, "n": 4050290.088321},
    "bl": {"e": 779032.901477, "n": 3699450.983449},
    "tr": {"e": 905367.666914, "n": 4366503.431060},
    "br": {"e": 1181157.928029, "n": 4014713.414724}
}  #: dict: Defines the model corners in UTM format.

PLOT_CORNERS = {
    "tl": {"e": -122, "n": 37},
    "br": {"e": -118, "n": 33}
}

UCVM_LOCATION = "/Users/davidgil/ucvm-15.10.0"  #: str: Defines where UCVM has been installed.
DATA_FILE = "./CCA06.ascii"                     #: str: Defines the data file from which we read.
Z_SPACING = 500                                 #: int: Spacing in meters.
PROPERTIES = {"vp": 3, "vs": 4, "dn": 5}        #: dict: The material properties contained in model.
MAX_DIMENSIONS = {
    "X": 1024,
    "Y": 896,
    "Z": 100
}                                               #: dict: The maximum dimensions for the model.


def calc_rotation_angle(tl_cn_n: float, bl_cn_n: float, tl_cn_e: float, bl_cn_e: float) -> float:
    """
    Calculates the rotation angle of a rectangle given the top-left and bottom-left corners.
    :param float tl_cn_n: Top-left corner north.
    :param float bl_cn_n: Bottom-left corner north.
    :param float tl_cn_e: Top-left corner east.
    :param float bl_cn_e: Bottom-left corner east.
    :return: The rotation angle as a float.
    """

    north_height = float(tl_cn_n) - float(bl_cn_n)
    east_width = float(tl_cn_e) - float(bl_cn_e)

    return math.degrees(math.atan(east_width / north_height))


def calc_width(tr_cn_n: float, tl_cn_n: float, tr_cn_e: float, tl_cn_e: float) -> float:
    """
    Calculates the width of the rectangle given the top-left and top-right corners.
    :param float tr_cn_n: Top-right corner north.
    :param float tl_cn_n: Top-left corner north.
    :param float tr_cn_e: Top-right corner east.
    :param float tl_cn_e: Top-left corner east.
    :return: The width as a float.
    """
    return math.sqrt(math.pow(float(tr_cn_n) - float(tl_cn_n), 2.0) +
                     math.pow(float(tr_cn_e) - float(tl_cn_e), 2.0))


def calc_height(tl_cn_n: float, bl_cn_n: float, tl_cn_e: float, bl_cn_e: float) -> float:
    """
    Calculates the height of the rectangle given the top-left and bottom-left corners.
    :param tl_cn_n: Top-left corner north.
    :param bl_cn_n: Bottom-left corner north.
    :param tl_cn_e: Top-left corner east.
    :param bl_cn_e: Bottom-left corner east.
    :return: The height as a float.
    """
    return math.sqrt(math.pow(float(tl_cn_n) - float(bl_cn_n), 2.0) +
                     math.pow(float(tl_cn_e) - float(bl_cn_e), 2.0))


def is_in_box(xy_coord: tuple, origin_coord: tuple, angle: float,
              width: float, height: float) -> bool:
    """
    Given the width, height, origin, and rotation of a box, check to see whether the point
    defined by (x, y) is inside that box.
    :param xy_coord: The x, y co-ordinate of the point to check if it is inside the box.
    :param origin_coord: The origin east co-ordinate of the box.
    :param angle: The rotation angle of the box.
    :param width: The width of the box.
    :param height: The height of the box.
    :return: True, if (x,y) is within the box. False if not.
    """
    new_point_e = xy_coord[0] - origin_coord[0]
    new_point_n = xy_coord[1] - origin_coord[1]

    rotated_point_e = \
        new_point_e * math.cos(math.radians(angle)) - new_point_n * math.sin(math.radians(angle))
    rotated_point_n = \
        new_point_n * math.cos(math.radians(angle)) + new_point_e * math.sin(math.radians(angle))

    return 0 <= rotated_point_e <= width and 0 <= rotated_point_n <= height


def compute_averages(land_points: list, material_properties_array: dict) -> dict:
    """
    Given a list of land points (and their corresponding X and Y co-ordinates), compute the
    average slowness for Vp and Vs, and average for density. That is then returned as a
    3-parameter array ["vp"/"vs"/"dn"][0..n] for the average at each layer.
    :param list land_points: The list of land points.
    :param list material_properties_array: The array of retrieved material properties.
    :return: The averages in the format listed in the description (an array of arrays).
    """
    # Calculate the average.
    average_slowness = {k: np.zeros(MAX_DIMENSIONS["Z"], dtype=float) for k in PROPERTIES.keys()}

    for point in land_points:
        for z_value in range(0, MAX_DIMENSIONS["Z"]):
            average_slowness["vp"][z_value] += \
                1.0 / material_properties_array["vp"][point["x"], point["y"], z_value]
            average_slowness["vs"][z_value] += \
                1.0 / material_properties_array["vs"][point["x"], point["y"], z_value]
            average_slowness["dn"][z_value] += \
                material_properties_array["dn"][point["x"], point["y"], z_value]

    # Divide the average by the number of total points on the grid.
    average_slowness["vp"] = [average_slowness["vp"][z] / len(land_points)
                              for z in range(0, MAX_DIMENSIONS["Z"])]
    average_slowness["vs"] = [average_slowness["vs"][z] / len(land_points)
                              for z in range(0, MAX_DIMENSIONS["Z"])]
    average_slowness["dn"] = [average_slowness["dn"][z] / len(land_points)
                              for z in range(0, MAX_DIMENSIONS["Z"])]

    average_slowness["vp"] = [1.0 / average_slowness["vp"][z]
                              for z in range(0, MAX_DIMENSIONS["Z"])]
    average_slowness["vs"] = [1.0 / average_slowness["vs"][z]
                              for z in range(0, MAX_DIMENSIONS["Z"])]

    return average_slowness


def get_list_of_lat_lons_in_model(model_meta: dict, input_proj: pyproj.Proj,
                                  output_proj: pyproj.Proj) -> list:
    """
    Returns a list of latitudes and longitudes within the model. This basically converts from the
    X, Y, Z format of the text model to lat, long that can be queried with UCVM.
    :param dict model_meta: The model metadata, including width, height, and cos and sin.
    :param Proj.4 input_proj: The input projection as a Proj. 4 object.
    :param Proj.4 output_proj: The output projection as a Proj. 4 object.
    :return: An array of dictionary latitudes and longitudes.
    """
    print("Generating lat, long point list from model.")

    points = []

    for y_value in range(0, MAX_DIMENSIONS["Y"]):
        for x_value in range(0, MAX_DIMENSIONS["X"]):
            x_calc = x_value * model_meta["width"]
            y_calc = y_value * model_meta["height"]

            add_x = model_meta["cos_rotation"] * x_calc - model_meta["sin_rotation"] * y_calc
            add_y = model_meta["sin_rotation"] * x_calc + model_meta["cos_rotation"] * y_calc

            utm_point_e = MODEL_CORNERS["bl"]["e"] + add_x
            utm_point_n = MODEL_CORNERS["bl"]["n"] + add_y

            new_point = pyproj.transform(input_proj, output_proj, utm_point_e, utm_point_n)
            points.append({"x": x_value, "y": y_value, "long": new_point[0], "lat": new_point[1],
                           "utm_e": utm_point_e, "utm_n": utm_point_n})

    return points


def query_ucvm_for_land_points(all_points: list, cybershake_utm: dict, cs_metadata: dict) -> list:
    """
    Given a list of all points (converted to lat, long) find all points within the CyberShake
    UTM box.
    :param list all_points: A list of all the points in the model.
    :param dict cybershake_utm: The four corners of the CyberShake box in UTM.
    :param dict cs_metadata: A dictionary describing the metadata of the CyberShake box.
    :return: A list of all the land points.
    """
    proc = Popen([UCVM_LOCATION + "/bin/ucvm_query", "-f", UCVM_LOCATION + "/conf/ucvm.conf",
                  "-m", "1d"], stdout=PIPE, stdin=PIPE, stderr=STDOUT)

    text_points = ""

    for point in all_points:
        text_points += "%.5f %.5f %.5f\n" % (point["long"], point["lat"], 0)

    text_points = text_points.encode("ASCII")

    output = str(proc.communicate(input=bytes(text_points))[0], "ASCII")
    output = output.split("\n")[1:-1]

    land_points = []

    for i in range(0, len(output)):
        split_line = output[i].split()
        if float(split_line[3]) >= 0 and \
                is_in_box((all_points[i]["utm_e"], all_points[i]["utm_n"]),
                          (cybershake_utm["bl"]["e"], cybershake_utm["bl"]["n"]),
                          cs_metadata["angle"], cs_metadata["width"], cs_metadata["height"]):
            land_points.append(all_points[i])

    print("UCVM queried for land-based points.")

    return land_points


def convert_model_to_array() -> dict:
    """
    Converts the text model into a 3D NumPy array of material properties.
    :return: The 3D array of material properties.
    """
    mat_array = {k: np.zeros((MAX_DIMENSIONS["X"], MAX_DIMENSIONS["Y"], MAX_DIMENSIONS["Z"]),
                             dtype=np.float) for k in PROPERTIES.keys()}

    print("Reading in text file data...")

    # Read in the actual material property data.
    with open(DATA_FILE, "r") as open_file:
        for line in open_file:
            components = line.split()
            for key, value in PROPERTIES.items():
                mat_array[key][np.int(components[0]) - 1, np.int(components[1]) - 1,
                               MAX_DIMENSIONS["Z"] - np.int(components[2])] = \
                               np.float(components[value])

    print("Array of material properties generated.")

    return mat_array


def generate_and_save_plot(land_points: list, filename: str) -> None:
    """
    Generates and saves the plot showing exactly which points were included in the calculation.
    It also shows the CyberShake box so one can see exactly how they all fit in that box.
    :param list land_points: The list of land-based points to plot.
    :param str filename: The file name to which the data should be saved.
    :return: Nothing
    """
    plt.figure(figsize=(10, 10), dpi=100)
    map_base = basemap.Basemap(projection='cyl',
                               llcrnrlat=PLOT_CORNERS["br"]["n"],
                               urcrnrlat=PLOT_CORNERS["tl"]["n"],
                               llcrnrlon=PLOT_CORNERS["tl"]["e"],
                               urcrnrlon=PLOT_CORNERS["br"]["e"],
                               resolution='f', anchor='C')

    map_base.drawparallels([PLOT_CORNERS["tl"]["n"],
                            (PLOT_CORNERS["br"]["n"] + PLOT_CORNERS["tl"]["n"]) / 2,
                            PLOT_CORNERS["br"]["n"]], linewidth=1.0, labels=[1, 0, 0, 0])
    map_base.drawmeridians([PLOT_CORNERS["tl"]["e"],
                            (PLOT_CORNERS["br"]["e"] + PLOT_CORNERS["tl"]["e"]) / 2,
                            PLOT_CORNERS["br"]["e"]], linewidth=1.0, labels=[0, 0, 0, 1])
    map_base.drawstates()
    map_base.drawcountries()
    map_base.drawcoastlines()

    lon_array = []
    lat_array = []

    for land_point in land_points:
        lon_array.append(land_point["long"])
        lat_array.append(land_point["lat"])

    map_base.plot(lon_array, lat_array, "bo", markersize=0.1)

    # Plot the CyberShake box.
    lon_array = [CYBERSHAKE_BOX["tl"]["e"], CYBERSHAKE_BOX["tr"]["e"], CYBERSHAKE_BOX["br"]["e"],
                 CYBERSHAKE_BOX["bl"]["e"], CYBERSHAKE_BOX["tl"]["e"]]
    lat_array = [CYBERSHAKE_BOX["tl"]["n"], CYBERSHAKE_BOX["tr"]["n"], CYBERSHAKE_BOX["br"]["n"],
                 CYBERSHAKE_BOX["bl"]["n"], CYBERSHAKE_BOX["tl"]["n"]]
    map_base.plot(lon_array, lat_array, "r-")

    # Add the title.
    plt.title("Grid Points Included in Average")

    plt.savefig(filename)

    print("Plot saved.")


def output_model_in_bbp_format(averages: dict, filename: str) -> None:
    """
    Given an array of 1D averages, this function takes that and outputs them in BBP format.
    :param list averages: The 1D array of averages.
    :param str filename: The file name to which the data should be saved.
    :return: Nothing
    """
    bbp_output = """# Central California 1D average velocity model over land-based points.

# Name of model.
version=CyberShake Central California 1D Velocity Model Land-Based Points Only

# Number of layers.
num_z=[num_layers]

# Interpolation method.
interpolation=linear

# Start of model.

--MODEL--

""".replace("[num_layers]", str(MAX_DIMENSIONS["Z"] + 1))

    current_layer = 0

    bbp_output += "0.0001 %.4f %.4f %.4f 0.0000 0.0000\n" % (averages["vp"][0] / 1000.0,
                                                             averages["vs"][0] / 1000.0,
                                                             averages["dn"][0] / 1000.0)

    for i in range(1, MAX_DIMENSIONS["Z"]):
        bbp_output += "%.4f %.4f %.4f %.4f 0.0000 0.0000\n" % \
                      (Z_SPACING / 1000.0, averages["vp"][i] / 1000.0, averages["vs"][i] / 1000.0,
                       averages["dn"][i] / 1000.0)
        current_layer += 1

    bbp_output += "999.0000 %.4f %.4f %.4f 0.0000 0.0000\n" % \
                  (averages["vp"][MAX_DIMENSIONS["Z"] - 1] / 1000.0,
                   averages["vs"][MAX_DIMENSIONS["Z"] - 1] / 1000.0,
                   averages["dn"][MAX_DIMENSIONS["Z"] - 1] / 1000.0)

    file_to_save = open(filename, "w")
    file_to_save.write(bbp_output)
    file_to_save.close()

    print("BBP 1D format data file saved.")


def main() -> int:
    """
    The main function for our program.
    :return: Zero when successful.
    """

    print("Calculating angles, heights, projections, etc.")

    # Try and match the inversion projection as close as possible.
    input_proj = pyproj.Proj("+proj=utm +zone=10 +ellps=clrk66 +datum=NAD27 +units=m +no_defs")
    output_proj = pyproj.Proj("+proj=latlong +datum=WGS84")

    cybershake_box_utm = {
        "tl": {"e": 0.0, "n": 0.0},
        "bl": {"e": 0.0, "n": 0.0},
        "tr": {"e": 0.0, "n": 0.0},
        "br": {"e": 0.0, "n": 0.0}
    }

    # Convert from WGS84 lat, long format to the UTM projection that the inversion uses.
    for key, value in CYBERSHAKE_BOX.items():
        new_point = pyproj.transform(output_proj, input_proj, value["e"], value["n"])
        cybershake_box_utm[key] = {"e": new_point[0], "n": new_point[1]}

    # Calculate the metadata for the CyberShake box.
    cs_metadata = {
        "angle": calc_rotation_angle(cybershake_box_utm["tl"]["n"], cybershake_box_utm["bl"]["n"],
                                     cybershake_box_utm["tl"]["e"], cybershake_box_utm["bl"]["e"]),
        "width": calc_width(cybershake_box_utm["tr"]["n"], cybershake_box_utm["tl"]["n"],
                            cybershake_box_utm["tr"]["e"], cybershake_box_utm["tl"]["e"]),
        "height": calc_height(cybershake_box_utm["tl"]["n"], cybershake_box_utm["bl"]["n"],
                              cybershake_box_utm["tl"]["e"], cybershake_box_utm["bl"]["e"])
    }

    # Calculate the metadata for the model boundaries (including rotation, width, and height).
    model_metadata = {
        "width": calc_width(MODEL_CORNERS["tr"]["n"],
                            MODEL_CORNERS["tl"]["n"],
                            MODEL_CORNERS["tr"]["e"],
                            MODEL_CORNERS["tl"]["e"]) / MAX_DIMENSIONS["X"],
        "height": calc_height(MODEL_CORNERS["tl"]["n"],
                              MODEL_CORNERS["bl"]["n"],
                              MODEL_CORNERS["tl"]["e"],
                              MODEL_CORNERS["bl"]["e"]) / MAX_DIMENSIONS["Y"],
        "sin_rotation": math.sin(math.radians(math.fabs(calc_rotation_angle(
            MODEL_CORNERS["tl"]["n"], MODEL_CORNERS["bl"]["n"], MODEL_CORNERS["tl"]["e"],
            MODEL_CORNERS["bl"]["e"])))),
        "cos_rotation": math.cos(math.radians(math.fabs(calc_rotation_angle(
            MODEL_CORNERS["tl"]["n"], MODEL_CORNERS["bl"]["n"], MODEL_CORNERS["tl"]["e"],
            MODEL_CORNERS["bl"]["e"]))))
    }

    # Get the list of all the land-based points.
    land_points = query_ucvm_for_land_points(
        get_list_of_lat_lons_in_model(model_metadata, input_proj, output_proj),
        cybershake_box_utm,
        cs_metadata
    )

    # Compute averages and output them in BBP format.
    output_model_in_bbp_format(compute_averages(land_points, convert_model_to_array()),
                               "cs_cca_1d_bbp.conf")

    # Now, we also need to save an image of the points we included. As well, we need to draw lines
    # around the CyberShake box so that way people can see it.
    generate_and_save_plot(land_points, "cs_included_points.png")

    print("All tasks complete!")

    return 0

if __name__ == "__main__":
    main()
