"""
Defines the main UCVM class. This class comprises only of static methods and class methods. This
comprises most of the basic framework (model query, model loading, etc.).

This script will automatically bootstrap UCVM immediately as it may need to adjust the
LD_LIBRARY_PATH. *If* it does, then it *will* relaunch the process. Therefore, load this module
at the top of your files!

:copyright: Southern California Earthquake Center
:author:    David Gill <davidgil@usc.edu>
:created:   July 6, 2016
:modified:  August 9, 2016
"""
import sys
import os
import xmltodict
import logging
import pkg_resources
import getopt
import re
import math
import psutil

from typing import List

from ucvm.src.shared.constants import UCVM_MODEL_LIST_FILE, UCVM_MODELS_DIRECTORY, \
                                      UCVM_DEFAULT_DEM, UCVM_DEFAULT_VS30, INTERNAL_DATA_DIRECTORY
from ucvm.src.shared.properties import SeismicData
from ucvm.src.shared import display_and_raise_error, parse_xmltodict_one_or_many, is_number
from ucvm.src.model.model import Model


class UCVM:

    instantiated_models = {}  #: dict: A dictionary of instantiated models.

    @classmethod
    def bootstrap(cls) -> bool:
        """
        Bootstraps UCVM. This automatically checks to see what models we have and where the
        libraries are. It also reloads the process if need be with the new LD_LIBRARY_PATH. As such,
        this needs to be called *right away* in any utilities and command-line tools.
        :return: True, if UCVM was bootstrapped successfully. False if not.
        """
        if "ucvm_has_bootstrapped" in os.environ:
            return False

        # Open the model list file.
        try:
            with open(UCVM_MODEL_LIST_FILE, "r") as fd:
                model_xml = xmltodict.parse(fd.read())
        except FileNotFoundError:
            display_and_raise_error(1, (UCVM_MODEL_LIST_FILE,))
            sys.exit(-1)

        # By default, we assume that we want to modify the LD_LIBRARY_PATH file. If we're on Mac OS,
        # we want to modify the DYLD_LIBRARY_PATH.
        environment_variable = "LD_LIBRARY_PATH"
        if sys.platform == "darwin":
            environment_variable = "DYLD_LIBRARY_PATH"

        if environment_variable not in os.environ:
            os.environ[environment_variable] = ""

        model_list = UCVM.get_list_of_installed_models()
        model_list = model_list["velocity"] + model_list["elevation"] + model_list["vs30"]

        paths = []
        if environment_variable in os.environ:
            paths = str(os.environ[environment_variable]).split(":")

        for item in model_list:
            if ".py" not in item["file"]:
                # This is Cythonized code.
                paths.append(os.path.join(UCVM_MODELS_DIRECTORY, item["id"], "lib"))

        os.environ[environment_variable] = ":".join(paths)

        try:
            os.environ["ucvm_has_bootstrapped"] = "Yes"
            os.execv(sys.executable, [sys.executable] + sys.argv)
        except Exception as e:
            display_and_raise_error(2)
            logging.exception(e)
            sys.exit(-1)

    @classmethod
    def query(cls, points: List[SeismicData], model_string: str,
              desired_properties: List[str]=None, custom_model_query: dict=None) -> bool:
        """
        Given a list of SeismicData objects, each one containing a valid Point object, and a
        model_string to parse, this function will get the velocity, elevation, and Vs30 data.
        :param points:
        :param model_string:
        :param desired_properties:
        :return:
        """
        points_to_query = points

        if custom_model_query is None:
            models_to_query = UCVM.parse_model_string(model_string)
        else:
            models_to_query = custom_model_query

        for _, model_query in models_to_query.items():
            if desired_properties is None:
                properties_to_retrieve = ["velocity", "elevation", "vs30"]
            else:
                properties_to_retrieve = list(desired_properties)

            counter = 0
            while len(properties_to_retrieve) > 0 and counter < len(model_query) - 1:
                model_to_query = model_query[counter].split(";")
                UCVM.get_model_instance(model_to_query[0])

                if len(points_to_query) == 0:
                    # If no points to query, then just move along!
                    counter += 1
                    continue

                if len(model_to_query) == 1:
                    UCVM.instantiated_models[model_to_query[0]].query(points_to_query)
                else:
                    UCVM.instantiated_models[model_to_query[0]].query(points_to_query,
                                                                      params=model_to_query[1])

                for point in points_to_query:
                    if point.is_property_type_set("velocity"):
                        point.set_model_string(
                            ".".join([re.sub(r'(.*);(.*)', r'\1(\2)', model_query[k]) for k in
                                      model_query if is_number(k)])
                        )

                properties_to_retrieve.remove(UCVM.get_model_type(model_to_query[0]))
                counter += 1

            # Remove all points that have velocity data in them. We don't want to re-query those
            # points at all.
            points_to_query = [x for x in points_to_query if
                               not x.is_property_type_set("velocity")]

        return True

    @classmethod
    def get_model_type(cls, model: str) -> str:
        """
        Given one model string, return the type of model (velocity, elevation, vs30) as a string.
        :param str model: The model string to check.
        :return: Velocity, vs30, or elevation depending on the model type.
        """
        all_models = UCVM.get_list_of_installed_models()

        for item in all_models["velocity"]:
            if item["id"] == model:
                return "velocity"

        for item in all_models["elevation"]:
            if item["id"] == model:
                return "elevation"

        for item in all_models["vs30"]:
            if item["id"] == model:
                return "vs30"

        display_and_raise_error(19)

    @classmethod
    def get_model_instance(cls, model: str) -> Model:
        """
        Given a model string, return the instantiated model object. If the model has not been
        instantiated yet, this will instantiate it.
        :param model: The model string.
        :return: The model class.
        """
        if model in UCVM.instantiated_models:
            return UCVM.instantiated_models[model]

        model_list = UCVM.get_list_of_installed_models()
        model_list = model_list["velocity"] + model_list["elevation"] + model_list["vs30"]

        # Do a quick check just to make sure the model does, indeed, exist.
        found = False
        for item in model_list:
            if item["id"] == model:
                found = item
                break

        if not found:
            display_and_raise_error(5, (model,))

        # The model exists. Let's load it either from the .py file or from the library.
        if ".py" in found["file"]:
            new_class = __import__("ucvm.models." + found["id"] + "." +
                                   ".".join(found["file"].split(".")[:-1]), fromlist=found["class"])
            UCVM.instantiated_models[model] = getattr(new_class, found["class"])()
        else:
            new_class = __import__(found["class"], fromlist=found["class"])
            UCVM.instantiated_models[model] = \
                getattr(new_class, found["class"])(model_location=
                                                   os.path.join(UCVM_MODELS_DIRECTORY, found["id"]))

        return UCVM.instantiated_models[model]

    @classmethod
    def parse_model_string(cls, string: str) -> dict:
        """
        Parses the model string. Given a model string which can contain one or models, we need
        to construct a sequence in which each model needs to be called. For example, if a model
        relies on query by elevation, then we need to call the DEM before we call the model. This
        resolves those dependencies. Model components are defined by dots (".") and models are
        separated by commas (",").
        :param string: The model string that we are parsing.
        :return: The dictionary of models to query. The keys start at 0 and increment.
        """
        if string.strip() is "":
            return {}

        model_strings = re.split(r',\s*(?![^()]*\))', string)
        ret_dict = {}

        for i in range(0, len(model_strings)):
            ret_dict[i] = UCVM._parse_one_model_string(model_strings[i])

        return ret_dict

    @classmethod
    def _parse_one_model_string(cls, one_model_string: str) -> dict:
        """
        Parses one model string. This loads the model's ucvm_model.xml and checks certain
        properties. It then suggests a model query order with the first query key being position
        0 and going up from there. If no ucvm_model.xml is found, then an error is raised since
        it must not be installed. There must be at least one velocity model.
        :param one_model_string: One model string to parse.
        :return: The dictionary of models to query and the order. The keys start at 0 and go up.
        """
        if one_model_string.strip() is "":
            return {}

        individual_models = one_model_string.split(".")
        search_model_list = {}
        models_with_parens = {}
        installed_models = UCVM.get_list_of_installed_models()

        velocity_model = None

        for individual_model in individual_models:
            parsed = UCVM._strip_and_return_parentheses(individual_model)
            search_model_list[parsed["string"]] = {"user": parsed["string"],
                                                   "params": parsed["parenthesis"]}

        for velocity_model_installed in installed_models["velocity"]:
            if velocity_model_installed["id"] in search_model_list:
                if velocity_model is not None:
                    display_and_raise_error(4, (one_model_string,))
                else:
                    velocity_model = velocity_model_installed["id"]
                    models_with_parens[velocity_model_installed["id"]] = \
                        search_model_list[velocity_model_installed["id"]]["params"]
                search_model_list.pop(velocity_model_installed["id"], None)
            if velocity_model_installed["name"] in search_model_list:
                if velocity_model is not None:
                    display_and_raise_error(4, (one_model_string,))
                else:
                    velocity_model = velocity_model_installed["id"]
                    models_with_parens[velocity_model_installed["id"]] = \
                        search_model_list[velocity_model_installed["name"]]["params"]
                search_model_list.pop(velocity_model_installed["name"], None)

        if velocity_model is None:
            return {0: one_model_string}

        # Get the defaults for this velocity model.
        with open(os.path.join(UCVM_MODELS_DIRECTORY, velocity_model, "ucvm_model.xml")) as fd:
            info = xmltodict.parse(fd.read())

        elevation_model = parse_xmltodict_one_or_many(info, "root/internal/defaults/elevation")
        vs30_model = parse_xmltodict_one_or_many(info, "root/internal/defaults/vs30")
        query_by = parse_xmltodict_one_or_many(info, "root/internal/query_by")

        if len(elevation_model) == 0:
            elevation_model = [{"#text": UCVM_DEFAULT_DEM}]
        if len(vs30_model) == 0:
            vs30_model = [{"#text": UCVM_DEFAULT_VS30}]

        if len(query_by) == 0:
            query_by = [{"#text": "DEPTH"}]

        elevation_model = elevation_model[0]["#text"]
        vs30_model = vs30_model[0]["#text"]

        if models_with_parens[velocity_model] != "":
            velocity_model = velocity_model + ";" + models_with_parens[velocity_model]

        # Now, if our model list is still not empty, we override the elevation and Vs30 models
        # as need be.
        if len(search_model_list) > 0:
            for elevation_model_installed in installed_models["elevation"]:
                if elevation_model_installed["id"] in search_model_list or \
                   elevation_model_installed["name"] in search_model_list:
                    elevation_model = elevation_model_installed["id"]
                try:
                    search_model_list.pop(elevation_model_installed["id"], None)
                except ValueError:
                    pass
                try:
                    search_model_list.pop(elevation_model_installed["name"], None)
                except ValueError:
                    pass

            for vs30_model_installed in installed_models["vs30"]:
                if vs30_model_installed["id"] in search_model_list or \
                   vs30_model_installed["name"] in search_model_list:
                    vs30_model = vs30_model_installed["id"]
                try:
                    search_model_list.pop(vs30_model_installed["id"], None)
                except ValueError:
                    pass
                try:
                    search_model_list.pop(vs30_model_installed["name"], None)
                except ValueError:
                    pass

        # We should have a velocity model, an elevation model array (with one element), and
        # a Vs30 model with one element. Figure out the order and if a conversion is necessary.
        if (str(query_by[0]["#text"]).lower() == "depth" and "elevation" in individual_models) or \
           (str(query_by[0]["#text"]).lower() == "elevation" and "depth" in individual_models):
            return {
                0: elevation_model,
                1: velocity_model,
                2: vs30_model,
                "query_by": str(query_by[0]["#text"]).lower()
            }
        else:
            return {
                0: velocity_model,
                1: elevation_model,
                2: vs30_model,
                "query_by": None
            }

    @classmethod
    def _strip_and_return_parentheses(cls, string: str) -> dict:
        """
        Given a string like cvms4(TEST), return a dictionary with format {"string": cvms4,
        "parenthesis": TEST}. If no parenthesis, parenthesis is the empty string.
        :param string: The string to parse.
        :return: The dictionary in the format described in main comment.
        """
        first_re = re.compile(".*?\((.*?)\)")
        result = re.findall(first_re, string)

        if len(result) > 0:
            second_re = re.compile("\([^)]*\)")
            string = re.sub(second_re, "", string)
            return {"string": string, "parenthesis": result[0]}
        else:
            return {"string": string, "parenthesis": ""}

    @classmethod
    def get_list_of_installed_models(cls) -> dict:
        """
        Gets the full list of installed models.
        :return: Returns the full list of installed models.
        """
        with open(UCVM_MODEL_LIST_FILE, "r") as fd:
            model_xml = xmltodict.parse(fd.read())

        models = {"velocity": [], "elevation": [], "vs30": []}

        if model_xml["root"] is None:
            return models

        for model_type, definition in model_xml["root"].items():
            if isinstance(definition, list):
                for item in definition:
                    item = dict(item)  # Make PyCharm happy.
                    models[model_type].append({
                        "id": item["@id"],
                        "name": item["@name"],
                        "file": item["@file"],
                        "class": item["@class"]
                    })
            else:
                models[model_type].append({
                    "id": definition["@id"],
                    "name": definition["@name"],
                    "file": definition["@file"],
                    "class": definition["@class"]
                })

        return models

    @classmethod
    def print_version(cls) -> None:
        UCVM.print_with_replacements(
            "\n"
            "UCVM Version [version]\n"
            "\n"
            "Copyright (C) [year] Southern California Earthquake Center. All rights reserved.\n"
            "\n"
            "This software is licensed under the Apache 2 license. More information on this\n"
            "license can be found on the Apache website.\n"
            "\n"
        )

    @classmethod
    def print_with_replacements(cls, string: str) -> None:
        print_str = string
        print_str = print_str.replace("[version]", pkg_resources.require("ucvm")[0].version)
        print_str = print_str.replace("[year]", "20" +
                                      pkg_resources.require("ucvm")[0].version.split(".")[0])
        print(print_str)

    @classmethod
    def get_replacement_string(cls, string: str) -> str:
        ret_str = string
        ret_str = ret_str.replace("[version]", pkg_resources.require("ucvm")[0].version)
        ret_str = ret_str.replace("[year]", "20" +
                                  pkg_resources.require("ucvm")[0].version.split(".")[0])
        return ret_str

    @classmethod
    def parse_options(cls, dict_list: list, function: callable) -> dict:
        """
        Given a list of options in format [{short, long, required=True/False, value=True/False}].
        :param list dict_list: A list of dictionary options in the format described above.
        :param callable function: The usage function to call if usage
        :return: A dictionary of options in format {long: value=value/None}.
        """
        short_opt_string = ""
        short_opt_array = []
        long_opt_array = []

        ret_options = {}

        for item in dict_list:
            item = dict(item)  # Make PyCharm happy...
            short_opt_string += item["short"]
            short_opt_array.append(item["short"])
            long_opt_array.append(item["long"])
            ret_options[item["long"]] = None
            if "value" in item and item["value"] is True:
                short_opt_string += ":"
                long_opt_array[-1] += "="

        try:
            opts, _ = getopt.getopt(sys.argv[1:], short_opt_string, long_opt_array)
        except getopt.GetoptError:
            function()
            sys.exit(-1)

        for option, argument in opts:
            if option in ("-h", "--help"):
                function()
                sys.exit(0)
            elif option in ("-v", "--version"):
                UCVM.print_version()
                sys.exit(0)

            for i in range(0, len(short_opt_array)):
                if option in ("-" + short_opt_array[i], "--" + long_opt_array[i]):
                    ret_options[long_opt_array[i].strip("=")] = argument

        for item in dict_list:
            item = dict(item)
            if "required" in item and item["required"] is True and \
               ret_options[item["long"]] is None:
                function()
                raise ValueError("--" + item["long"] + " not provided or has no value. "
                                 "Please provide a value for the argument.")

        return ret_options

    @classmethod
    def create_max_seismicdata_array(cls, total_points: int=250000, processes: int=1) -> \
            List[SeismicData]:
        """
        Returns an array of SeismicData objects.
        :param total_points: The number of SeismicData tuples required.
        :param processes: TEST
        :return: A list of the SeismicData tuple objects.
        """
        return [SeismicData() for _ in range(0, cls._get_max_query(total_points, processes))]

    @classmethod
    def _get_max_query(cls, total_points: int, processes: int) -> int:
        _MAX_PERCENT_FREE = 0.33
        _SD_SIZE = 200
        free_mem = psutil.virtual_memory().free
        return min(
            min(
                math.floor((free_mem * _MAX_PERCENT_FREE) / _SD_SIZE / processes),
                total_points
            ),
            250000
        )

UCVM.bootstrap()
