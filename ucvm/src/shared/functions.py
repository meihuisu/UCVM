import math
import numpy as np
import xmltodict
from typing import List
from collections import namedtuple

from pyproj import Proj


def parse_xmltodict_one_or_many(item: xmltodict.OrderedDict, keypath: str) -> List[dict]:
    """
    Given a base XMLtoDict object and a tag path using slashes as separators, return a list
    of dictionary items that correspond to the tags. If the path doesn't exist, None is
    returned.
    :param item: The base XMLtoDict object.
    :param keypath: The key path, separated by slashes.
    :return: The list of dictionary objects, if it exists, an empty array otherwise.
    """
    keys = keypath.split("/")
    eval_str = "item"
    ret_list = []
    for key in keys:
        eval_str += "[\"" + key + "\"]"

    try:
        new_item = eval(eval_str)
    except KeyError:
        return []
    except TypeError:
        return []
    else:
        if isinstance(new_item, list):
            for obj in new_item:
                if isinstance(obj, str):
                    ret_list.append({"#text": obj})
                else:
                    new_dict = {}
                    for key, val in dict(obj).items():
                        new_dict[key] = val
                    ret_list.append(new_dict)
        else:
            ret_dict = {}
            if isinstance(new_item, str):
                ret_list = [{"#text": new_item}]
            else:
                for key, val in new_item.items():
                    ret_dict[key] = val
                ret_list = [ret_dict]

    j = item
    j["p"] = "y"    # Fix PyCharm warning for unused parameter.

    return ret_list


def ask_and_validate(question: str, validation_function: callable=None, hint: str="", **kwargs) \
        -> str:
    """
    Asks a question and checks it against the validation function. If the validation function
    fails, it says so and also displays the hint to the user if that is provided.
    :param question: The question to ask on the command line.
    :param validation_function: The validation function to which the answer should be checked.
    :param hint: The hint to display if the user inputs the wrong data.
    :return: The answer as a string.
    """
    ask_again = True
    answer = None
    while ask_again:
        answer = input(question + " ")
        if validation_function is not None:
            if validation_function(answer, **kwargs):
                ask_again = False
            else:
                print("Sorry, that answer is not valid. Please try again.")
                if hint:
                    print("Hint: " + hint)
        else:
            ask_again = False

    return answer


def is_valid_proj4_string(projection: str) -> bool:
    if projection.strip() is "":
        return True  # A blank projection is defined to just be the UCVM default one.

    try:
        Proj(projection)
        return True
    except RuntimeError:
        return False


def is_acceptable_value(answer: str, **kwargs) -> bool:
    answer_cpy = answer
    if "lower" in kwargs:
        answer_cpy = answer_cpy.lower()
    if "allowed" in kwargs:
        return answer_cpy in kwargs["allowed"]
    return False


def is_number(s):
    """
    Checks to see if "s" is a number. If it is, then we want to return true. Otherwise, we return
    false.
    :param any s: The value to test to see if it can be converted to a float.
    :return: True if it is a number, false if not.
    """
    try:
        float(s)
        return True
    except ValueError:
        return False


def bilinear_interpolation(x: float, y: float, points: list) -> float:
    """
    From: http://stackoverflow.com/questions/8661537/how-to-perform-bilinear-interpolation-in-python

    Interpolate (x,y) from values associated with four points.

    The four points are a list of four triplets:  (x, y, value).
    The four points can be in any order.  They should form a rectangle.

    # See formula at:  http://en.wikipedia.org/wiki/Bilinear_interpolation

    :param x: X location to interpolate.
    :param y: Y location to interpolate.
    :param points: The array of points.
    :return: The interpolated value.
    """

    points = sorted(points)               # order points by x, then by y
    (x1, y1, q11), (_x1, y2, q12), (x2, _y1, q21), (_x2, _y2, q22) = points

    if x1 != _x1 or x2 != _x2 or y1 != _y1 or y2 != _y2:
        raise ValueError('points do not form a rectangle')
    if not x1 <= x <= x2 or not y1 <= y <= y2:
        raise ValueError('(x, y) not within the rectangle')

    return (q11 * (x2 - x) * (y2 - y) +
            q21 * (x - x1) * (y2 - y) +
            q12 * (x2 - x) * (y - y1) +
            q22 * (x - x1) * (y - y1)
           ) / ((x2 - x1) * (y2 - y1) + 0.0)


def calculate_bilinear_value(point: namedtuple, rectangle: namedtuple,
                             data_array: np.array) -> float:
    """
    Given a rotated rectangle and a point within that rectangle, calculate the bilinearly
    interpolated value given the data array.
    :param SimplePoint point: The point at which the query should be done.
    :param RotatedRectangle rectangle: The rotated rectangle.
    :param np.array data_array: The array of data values from which to query.
    :return: The calculated data value as a float.
    """
    new_point_x = point.x - rectangle.x
    new_point_y = point.y - rectangle.y

    rotated_point_x = \
        new_point_x * math.cos(math.radians(rectangle.rotation)) - \
        new_point_y * math.sin(math.radians(rectangle.rotation))
    rotated_point_y = \
        new_point_y * math.cos(math.radians(rectangle.rotation)) + \
        new_point_x * math.sin(math.radians(rectangle.rotation))

    width, height = np.shape(data_array)

    if (width - 1) * rectangle.x_spacing < rotated_point_x or rotated_point_x < 0 or \
       (height - 1) * rectangle.y_spacing < rotated_point_y or rotated_point_y < 0:
        # Falls outside of the range. Return None.
        return None

    gridded_x = rotated_point_x / rectangle.x_spacing
    gridded_y = rotated_point_y / rectangle.y_spacing

    # Handle edge cases.

    # We want a point right at the vertex.
    if math.floor(gridded_x) == math.ceil(gridded_x) and \
       math.floor(gridded_y) == math.ceil(gridded_y):
        return data_array[int(gridded_y), int(gridded_x)]

    # We want a point right between two vertices.
    if math.floor(gridded_y) == math.ceil(gridded_y):
        percent = gridded_x - math.floor(gridded_x)
        return (1 - percent) * data_array[(math.floor(gridded_x), int(gridded_y))] + \
               percent * data_array[(math.ceil(gridded_x), int(gridded_y))]
    elif math.floor(gridded_x) == math.ceil(gridded_x):
        percent = gridded_y - math.floor(gridded_y)
        return (1 - percent) * data_array[(int(gridded_x), math.floor(gridded_y))] + \
               percent * data_array[(int(gridded_x), math.ceil(gridded_y))]

    # This is a valid point within the rectangle. Let's find it now.
    llgridpoint = (math.floor(gridded_y), math.floor(gridded_x))
    llgridpoint = (llgridpoint[0], llgridpoint[1],
                   data_array[llgridpoint[0], llgridpoint[1]])

    lrgridpoint = (math.floor(gridded_y), math.ceil(gridded_x))
    lrgridpoint = (lrgridpoint[0], lrgridpoint[1],
                   data_array[lrgridpoint[0], lrgridpoint[1]])

    ulgridpoint = (math.ceil(gridded_y), math.floor(gridded_x))
    ulgridpoint = (ulgridpoint[0], ulgridpoint[1],
                   data_array[ulgridpoint[0], ulgridpoint[1]])

    urgridpoint = (math.ceil(gridded_y), math.ceil(gridded_x))
    urgridpoint = (urgridpoint[0], urgridpoint[1],
                   data_array[urgridpoint[0], urgridpoint[1]])

    # Do bilinear interpolation...
    return bilinear_interpolation(gridded_y, gridded_x,
                                  [llgridpoint, lrgridpoint, ulgridpoint, urgridpoint])


def calculate_scaled_density(vp: float) -> float:
    """
    Calculates the scaled density parameter based on Vp.
    :param vp: The P-wave velocity in m/s.
    :return: The scaled density parameter.
    """
    return 1865.0 + 0.1579 * vp


def calculate_scaled_vs(vp: float, density: float) -> float:
    """
    Calculates the scaled Vs parameter based on Vp and density.
    :param vp: The P-wave velocity in m/s.
    :param density: The density.
    :return: The scaled Vs parameter.
    """
    nu = 0.40 - ((density - 2060.0) * 0.15 / 440.0)

    if density < 2060.0:
        nu = 0.40
    elif density > 2500.0:
        nu = 0.25

    return vp * math.sqrt((0.5 - nu) / (1.0 - nu))
