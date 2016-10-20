"""
Defines the main UCVM class. This class comprises only of static methods and class methods. This
comprises most of the basic framework (model query, model loading, etc.).

This script will automatically bootstrap UCVM immediately as it may need to adjust the
LD_LIBRARY_PATH. *If* it does, then it *will* relaunch the process. Therefore, load this module
at the top of your files!

:copyright: Southern California Earthquake Center
:author:    David Gill <davidgil@usc.edu>
:created:   July 6, 2016
:modified:  October 17, 2016
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
import copy

from typing import List

from ucvm.src.shared.constants import UCVM_MODEL_LIST_FILE, UCVM_MODELS_DIRECTORY, \
                                      UCVM_DEFAULT_DEM, UCVM_DEFAULT_VS30, UCVM_DEFAULT_VELOCITY, \
                                      INTERNAL_DATA_DIRECTORY, UCVM_DEPTH, UCVM_ELEVATION, \
                                      UCVM_ELEV_ANY
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
        if custom_model_query is None:
            models_to_query = UCVM.get_models_for_query(
                model_string,
                ["velocity", "elevation", "vs30"] if desired_properties is None
                else desired_properties
            )
        else:
            models_to_query = custom_model_query

        for _, queryable_models in models_to_query.items():
            counter = 0
            while counter < len(queryable_models):
                model_to_query = queryable_models[counter].split(";-;")
                UCVM.get_model_instance(model_to_query[0])

                if len(model_to_query) == 1:
                    UCVM.instantiated_models[model_to_query[0]].query(points)
                else:
                    UCVM.instantiated_models[model_to_query[0]].query(
                        points, params=model_to_query[1]
                    )

                for point in points:
                    if point.is_property_type_set("velocity"):
                        point.set_model_string(
                            ".".join([re.sub(r'(.*);-;(.*)', r'\1[\2]', queryable_models[k]) for k in
                                      queryable_models if is_number(k)])
                        )

                counter += 1

            points = [x for x in points if not x.is_property_type_set("velocity")]

        return True

    @classmethod
    def get_model_type(cls, model: str) -> str:
        """
        Given one model string, return the type of model (velocity, elevation, vs30, or modifier)
        as a string.
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

        for item in all_models["modifier"]:
            if item["id"] == model:
                return "modifier"

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
        model_list = model_list["velocity"] + model_list["elevation"] + model_list["vs30"] + \
                     model_list["modifier"]

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

        model_strings = re.split(r';\s*(?![^()]*\))',
                                 string.replace(".depth", "").replace(".elevation", ""))
        model_strings_expanded = []

        for model_string in model_strings:
            grouped_models = re.match(r"\(([A-Za-z0-9_\.\[\];]+)\)", model_string)
            if grouped_models:
                if len(model_string.split(")")) > 1:
                    for model_id in grouped_models.group(0).split(";"):
                        model_strings_expanded.append(
                            (model_id + model_string.split(")")[1]).
                                replace("(", "").replace(")", "")
                        )
                else:
                    for model_id in grouped_models.group(0).split(";"):
                        model_strings_expanded.append(
                            model_id.replace("(", "").replace(")", "")
                        )
            else:
                model_strings_expanded.append(model_string)

        ret_dict = {}

        for i in range(0, len(model_strings_expanded)):
            if model_strings_expanded[i].strip() is "":
                continue

            individual_models = model_strings_expanded[i].split(".")
            models_to_add = {}
            current_index = 0

            for individual_model in individual_models:
                parsed = UCVM._strip_and_return_parameters(individual_model)
                models_to_add[current_index] = parsed["string"] + (
                    ";-;" + parsed["parameters"] if parsed["parameters"] != "" else ""
                )
                current_index += 1

            ret_dict[i] = models_to_add

        return ret_dict

    @classmethod
    def get_models_for_query(cls, model_string: str, desired_properties: list) -> dict:
        """
        Given a list of models and desired properties, ascertain how we can do this.
        :param model_string: The model string in its entirety.
        :param desired_properties: The desired properties (velocity, elevation, vs30) as a list.
        :return: A dictionary containing the models.
        """
        initial_models = UCVM.parse_model_string(model_string)

        new_model_array = {}
        desired_properties_copy = copy.copy(desired_properties)

        # We assume we are querying by depth unless otherwise told.
        query_by = UCVM_ELEVATION if ".elevation" in model_string else UCVM_DEPTH
        prepend_elevation = False

        for full_index, full_list in initial_models.items():
            will_return = {}
            temp_models = {}
            new_model_array[full_index] = {}
            desired_properties = copy.copy(desired_properties_copy)
            for index, model_id in dict(full_list).items():
                model_desc = {
                    "id": str(model_id).split(";-;")[0],
                    "params": str(model_id).split(";-;")[1] if len(str(model_id).split(";-;")) > 1 \
                              else ""
                }
                model = UCVM.get_model_instance(model_desc["id"])
                if model.get_metadata()["type"] in will_return and \
                    model.get_metadata()["type"] != "modifier":
                    display_and_raise_error(20)
                elif model.get_metadata()["type"] not in will_return:
                    will_return[model.get_metadata()["type"]] = {
                        0: model_desc
                    }
                else:
                    will_return[model.get_metadata()["type"]][
                        len(will_return[model.get_metadata()["type"]])
                    ] = model_desc
                if int(model.get_private_metadata("query_by")) != query_by and \
                   int(model.get_private_metadata("query_by")) != UCVM_ELEV_ANY:
                    prepend_elevation = True
                    if "elevation" not in desired_properties:
                        desired_properties.append("elevation")

            for prop in desired_properties:
                if prop in will_return:
                    temp_models[prop] = will_return[prop]
                    desired_properties.remove(prop)

            # Check to see if any desired properties remain.
            if len(desired_properties) > 0:
                for prop in desired_properties:
                    temp_models[prop] = {0: {"id": \
                        UCVM.get_model_instance(temp_models["velocity"][0]["id"]).\
                        get_private_metadata("defaults")[prop] if \
                        "velocity" in temp_models else (UCVM_DEFAULT_DEM if prop == "elevation" \
                        else (UCVM_DEFAULT_VS30 if prop == "vs30" else (UCVM_DEFAULT_VELOCITY if \
                        prop == "velocity" else None))), "params": ""}}

            # Assemble the new array.
            if prepend_elevation:
                new_model_array[full_index][len(new_model_array[full_index])] = \
                    temp_models["elevation"][0]["id"] + \
                    (";-;" + temp_models["elevation"][0]["params"] if
                     temp_models["elevation"][0]["params"] != "" else "")
                temp_models.pop("elevation", None)

            if "velocity" in temp_models:
                new_model_array[full_index][len(new_model_array[full_index])] = \
                    temp_models["velocity"][0]["id"] + \
                    (";-;" + temp_models["velocity"][0]["params"] if
                     temp_models["velocity"][0]["params"] != "" else "")
            if "elevation" in temp_models:
                new_model_array[full_index][len(new_model_array[full_index])] = \
                    temp_models["elevation"][0]["id"] + \
                    (";-;" + temp_models["elevation"][0]["params"] if
                     temp_models["elevation"][0]["params"] != "" else "")
            if "vs30" in temp_models:
                new_model_array[full_index][len(new_model_array[full_index])] = \
                    temp_models["vs30"][0]["id"] + \
                    (";-;" + temp_models["vs30"][0]["params"] if
                     temp_models["vs30"][0]["params"] != "" else "")
            if "modifier" in will_return:
                for key, val in will_return["modifier"].items():
                    new_model_array[full_index][len(new_model_array[full_index])] = \
                        val["id"] + (";-;" + val["params"] if val["params"] != "" else "")

        return new_model_array

    @classmethod
    def _strip_and_return_parameters(cls, string: str) -> dict:
        """
        Given a string like cvms4[TEST], return a dictionary with format {"string": cvms4,
        "parameters": TEST}. If no parameters, parameters is the empty string.
        :param string: The string to parse.
        :return: The dictionary in the format described in main comment.
        """
        first_re = re.compile(".*?\[(.*?)\]")
        result = re.findall(first_re, string)

        if len(result) > 0:
            second_re = re.compile("\[[^)]*\]")
            string = re.sub(second_re, "", string)
            return {"string": string, "parameters": result[0]}
        else:
            return {"string": string, "parameters": ""}

    @classmethod
    def get_list_of_installed_models(cls) -> dict:
        """
        Gets the full list of installed models.
        :return: Returns the full list of installed models.
        """
        with open(UCVM_MODEL_LIST_FILE, "r") as fd:
            model_xml = xmltodict.parse(fd.read())

        models = {"velocity": [], "elevation": [], "vs30": [], "modifier": []}

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
