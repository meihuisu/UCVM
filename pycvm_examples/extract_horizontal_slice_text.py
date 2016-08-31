"""
Uses the PyCVM library from UCVM 15.10.0 (although this will also work with 14.3.0 as well) to
extract a horizontal slice using four bounding corners. This will then save the data to
example_slice.txt.

NOTE: This script must be run with the UCVM utilities folder as the working directory.

:copyright: Southern California Earthquake Center
:author:    David Gill <davidgil@usc.edu>
:created:   July 22, 2016
:modified:  July 22, 2016
:target:    UCVM 14.3.0 - 15.10.0. Python 2.7 and lower.
"""

import math
import pyproj
import matplotlib.pyplot as plt
from mpl_toolkits import basemap

from pycvm import UCVM, Point

CORNERS = {
    "tl": {"lon": -121.832600, "lat": 36.457970},
    "tr": {"lon": -121.075000, "lat": 36.550000},
    "bl": {"lon": -120.371372, "lat": 34.870960},
    "br": {"lon": -119.630933, "lat": 34.938354}
}   #: dict: The four corners of the box in which to find the points.
SPACING = 1000                      #: int: Spacing in meters.
DEPTHS = [5000, 7500, 10000]        #: list: The depths, in meters, to query the model(s) for.
MODELS = ["cca"]                    #: list: A string list of models to query.

UCVM_PROJECTION = "+proj=latlong +datum=WGS84"                          #: str: UCVM projection.
UTM_PROJECTION = "+proj=utm +zone=11 +datum=WGS84 +units=m +no_defs"    #: str: UTM projection.

PLOT_FILE = "plotted_points.png"    #: str: The file name of the plot image.

HEADER = "Longitude   Latitude    Depth       Vs (m/s)    Vp (m/s)    Density (kg/m^3)"
OUT_LINE = "%-12.4f%-12.4f%-12.4f%-12.4f%-12.4f%-12.4f"


def convert_point_to_utm(point):
    """
    Convert the given point from WGS84 to UTM.
    :param point: The input point in WGS94 latitude, longitude.
    :return: The dictionary point {x, y} in UTM.
    """
    input_proj = pyproj.Proj(UCVM_PROJECTION)
    output_proj = pyproj.Proj(UTM_PROJECTION)

    new_point = pyproj.transform(input_proj, output_proj, point["lon"], point["lat"])

    return {"x": new_point[0], "y": new_point[1]}


def convert_utm_to_latlon(point):
    """
    Convert the given point from UTM to WGS84.
    :param point: The input point in UTM.
    :return: The dictionary point {lon, lat} in WGS84.
    """
    input_proj = pyproj.Proj(UTM_PROJECTION)
    output_proj = pyproj.Proj(UCVM_PROJECTION)

    new_point = pyproj.transform(input_proj, output_proj, point["x"], point["y"])

    return {"lon": new_point[0], "lat": new_point[1]}


def get_grid_point(origin, vertical_add, horizontal_add, grid_point):
    """
    Gets the grid point, given an origin point a "per y-spacing" add x and y amount to the UTM
    grid, and add_x and add_y for the "per x-spacing" and the actual x and y co-ordinates.
    :param origin: The origin point.
    :param vertical_add: The amount to add to x, y for each grid_point[y] increase from the origin.
    :param horizontal_add: The amount to add to x, y for each grid_point[x] increase on new origin.
    :param grid_point: The numerical grid point.
    :return: The point in WGS84 lat long projection.
    """
    origin_y = origin["y"] + (grid_point["y"] * vertical_add["y"])
    origin_x = origin["x"] + (grid_point["y"] * vertical_add["x"])

    utm_point = {"x": origin_x + grid_point["x"] * horizontal_add["x"],
                 "y": origin_y + grid_point["x"] * horizontal_add["y"]}

    return convert_utm_to_latlon(utm_point)


def generate_and_save_plot(point_list, filename):
    """
    Generates and saves the plot of points in point list. This also puts a red box around the
    point region.
    :param point_list: The list of points.
    :param filename: The file name to which this plot should be saved.
    :return: Nothing.
    """
    plt.figure(figsize=(10, 10), dpi=100)
    map_base = basemap.Basemap(projection='cyl',
                               llcrnrlat=33.5,
                               urcrnrlat=37.5,
                               llcrnrlon=-122,
                               urcrnrlon=-118,
                               resolution='f', anchor='C')

    map_base.drawparallels([33.5, 35.5, 37.5], linewidth=1.0, labels=[1, 0, 0, 0])
    map_base.drawmeridians([-122, -120, -118], linewidth=1.0, labels=[0, 0, 0, 1])

    map_base.drawstates()
    map_base.drawcountries()
    map_base.drawcoastlines()

    lon_array = []
    lat_array = []

    for point in point_list:
        lon_array.append(point["lon"])
        lat_array.append(point["lat"])

    map_base.plot(lon_array, lat_array, "bo", markersize=0.1)

    # Plot the bounding box.
    lon_array = [CORNERS["tl"]["lon"], CORNERS["tr"]["lon"], CORNERS["br"]["lon"],
                 CORNERS["bl"]["lon"], CORNERS["tl"]["lon"]]
    lat_array = [CORNERS["tl"]["lat"], CORNERS["tr"]["lat"], CORNERS["br"]["lat"],
                 CORNERS["bl"]["lat"], CORNERS["tl"]["lat"]]
    map_base.plot(lon_array, lat_array, "r-")

    # Add the title.
    plt.title("Grid Points Included in Text File")

    plt.savefig(filename)

    print("Plot saved.")


def main():
    """
    Generates the slice given the constants above.
    :return: Zero on success.
    """
    corners_utm = {key: convert_point_to_utm(value) for key, value in CORNERS.items()}

    angle = {
        "x": math.atan2(corners_utm["br"]["y"] - corners_utm["bl"]["y"],
                        corners_utm["br"]["x"] - corners_utm["bl"]["x"]),
        "y": math.atan2(corners_utm["tl"]["y"] - corners_utm["bl"]["y"],
                        corners_utm["tl"]["x"] - corners_utm["bl"]["x"])
    }

    num = {
        "x": math.ceil(math.sqrt(math.pow(corners_utm["br"]["y"] - corners_utm["bl"]["y"], 2.0) +
                                 math.pow(corners_utm["br"]["x"] - corners_utm["bl"]["x"], 2.0)) /
                       SPACING),
        "y": math.ceil(math.sqrt(math.pow(corners_utm["tl"]["y"] - corners_utm["bl"]["y"], 2.0) +
                                 math.pow(corners_utm["tl"]["x"] - corners_utm["bl"]["x"], 2.0)) /
                       SPACING)
    }

    horizontal_add = {
        "x": SPACING * math.cos(angle["x"]),
        "y": SPACING * math.sin(angle["x"])
    }

    vertical_add = {
        "x": SPACING * math.cos(angle["y"]),
        "y": SPACING * math.sin(angle["y"])
    }

    plotted_points = []

    for depth in DEPTHS:
        points_to_query = []
        for y_grid in range(0, int(num["y"])):
            for x_grid in range(0, int(num["x"])):
                current_grid_point = get_grid_point(corners_utm["bl"],
                                                    {"x": vertical_add["x"],
                                                     "y": vertical_add["y"]},
                                                    {"x": horizontal_add["x"],
                                                     "y": horizontal_add["y"]},
                                                    {"x": x_grid, "y": y_grid})
                points_to_query.append(Point(current_grid_point["lon"], current_grid_point["lat"],
                                             depth))
                if depth == DEPTHS[0]:
                    plotted_points.append(current_grid_point)

        # Now we need to query these points with UCVM.
        ucvm_object = UCVM()
        properties = ucvm_object.query(points_to_query, ",".join(MODELS))

        file_out = open("-".join(MODELS) + "_" + str(depth) + "_slice.txt", "w")
        file_out.write(HEADER + "\n")

        for i in range(0, len(properties)):
            file_out.write(OUT_LINE % (points_to_query[i].longitude, points_to_query[i].latitude,
                                       points_to_query[i].depth, properties[i].vp, properties[i].vs,
                                       properties[i].density) + "\n")

        file_out.close()

    generate_and_save_plot(plotted_points, PLOT_FILE)

    return 0

if __name__ == "__main__":
    main()
