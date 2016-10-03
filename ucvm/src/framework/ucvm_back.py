"""
Defines the UCVM base class which handles the main lifting of
"""

import getopt
import sys
import os
from typing import List, Dict
from collections import OrderedDict

import pkg_resources
import xmltodict

try:
    import mpl_toolkits.basemap.pyproj as pyproj
except ImportError as the_err:
    print("UCVM requires PyProj to be installed. Please install PyProj and then re-run \
           this script.")
    pyproj = None  # Needed to remove the warning in PyCharm
    raise

from ucvm.src.framework.bootstrap import bootstrap
from ucvm.src.shared.properties import SeismicData
from ucvm.src.model import UCVM_MODEL_LIST_FILE


class UCVM:

    initialized_models = {}     #: dict: Dictionary of already initialized model classes.

    def __init__(self):
        bootstrap()

    @classmethod
    def bootstrap(cls):
        if "ucvm_has_bootstrapped" in os.environ:
            return 1

        try:
            with open(UCVM_MODEL_LIST_FILE, "r") as fd:
                model_xml = xmltodict.parse(fd.read())
        except FileNotFoundError:
            print("Error launching UCVM. Please contact software@scec.org with error code 1.")
            sys.exit(-1)

        environment_variable = "LD_LIBRARY_PATH"
        if sys.platform == "darwin":
            environment_variable = "DYLD_LIBRARY_PATH"

        paths_to_add = []

        for item in parse_xmltodict_one_or_many(model_xml, "root/velocity"):
            if "@library" in item:
                paths_to_add.append(os.path.join(os.path.dirname(inspect.getfile(ucvm.models)),
                                                 item["@library"]))

        for item in parse_xmltodict_one_or_many(model_xml, "root/elevation"):
            if "@library" in item:
                paths_to_add.append(os.path.join(os.path.dirname(inspect.getfile(ucvm.models)),
                                                 item["@library"]))

        for item in parse_xmltodict_one_or_many(model_xml, "root/vs30"):
            if "@library" in item:
                paths_to_add.append(os.path.join(os.path.dirname(inspect.getfile(ucvm.models)),
                                                 item["@library"]))

        if environment_variable not in os.environ:
            os.environ[environment_variable] = ""

        for path in paths_to_add:
            if path not in os.environ[environment_variable]:
                os.environ[environment_variable] = ":".join(
                    str(os.environ[environment_variable]).split(":") + [path]
                )

        try:
            os.execv(sys.executable, [sys.executable] + sys.argv)
            os.environ["ucvm_has_bootstrapped"] = 1
        except Exception as e:
            print("Error launching UCVM. Please contact software@scec.org with error code 2.")
            print("The exception was:")
            print(str(e))
            sys.exit(1)

    @classmethod
    def query(cls, points: List[SeismicData], models: OrderedDict) -> bool:
        """
        Given a list of points and a model, returns a list of SeismicData objects.
        :param list points: A list of points (UCVM will auto-convert projections).
        :param list models: An ordered dictionary of models to query.
        :return: The list of SeismicData objects including the corresponding points.
        """
        for point in points:
            point.set_model_dictionary(models)

        for model_type, list_of_models in models.items():
            for model in list_of_models:
                if model not in cls.initialized_models:
                    cls.initialized_models[model] = model()
                cls.initialized_models[model].query(points)

        return True

    @staticmethod
    def get_installed_models() -> Dict[str, list]:
        """
        Generates a list of all known/installed models. This includes models found online as well
        as models installed via the custom method.
        :return: A dictionary of all the installed models from the installed.xml file.
        """
        with open(UCVM_MODEL_LIST_FILE, "r") as fd:
            model_xml = xmltodict.parse(fd.read())

        models = {"velocity": [], "elevation": [], "vs30": []}

        if model_xml["root"] is None:
            return models

        for model_type, definition in model_xml["root"].items():
            models[model_type].append({
                "id": definition["@id"],
                "file": definition["@file"],
                "class": definition["@class"]
            })

        return models

    @staticmethod
    def get_class_from_id_class(identifier: str, file: str, class_name: str) -> callable:
        """

        :param identifier:
        :param file:
        :param class_name:
        :return:
        """
        new_class = __import__("ucvm.models." + identifier + "." + file, fromlist=class_name)
        return getattr(new_class, class_name)

    @staticmethod
    def parse_model_string(model_str: str) -> OrderedDict:
        """
        Given a model string like cvms4 or CVM-S4.vs30_calc, get the model - or models - to which
        that string corresponds.
        :param model_str: The model string to parse.
        :return: The list of models that this string represents.
        """
        initial_model_split = model_str.split(".")
        ret_dict = OrderedDict([("velocity", []), ("elevation", []), ("vs30", [])])

        for model_class, the_array in UCVM.get_installed_models().items():
            for value in the_array:
                value = dict(value)  # Make PyCharm happy...
                if value["id"] in initial_model_split:
                    the_class = UCVM.get_class_from_id_class(value["id"],
                                                             ".".join(value["file"].
                                                                      split(".")[:-1]),
                                                             value["class"])
                    ret_dict[model_class].append(the_class)

                    # Instantiate and see if there are defaults.
                    new_object = the_class()
                    if new_object.get_private_metadata("defaults") is not None:
                        for default_type, default_class in \
                                new_object.get_private_metadata("defaults").items():
                            new_dict = UCVM.parse_model_string(default_class)
                            ret_dict["velocity"] += new_dict["velocity"]
                            ret_dict["elevation"] += new_dict["elevation"]
                            ret_dict["vs30"] += new_dict["vs30"]

        return ret_dict

    @staticmethod
    def print_version() -> None:
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

    @staticmethod
    def print_with_replacements(string: str) -> None:
        print_str = string
        print_str = print_str.replace("[version]", pkg_resources.require("ucvm")[0].version)
        print_str = print_str.replace("[year]", "20" +
                                      pkg_resources.require("UCVM")[0].version.split(".")[0])
        print(print_str)

    @staticmethod
    def parse_options(dict_list: list, function: callable) -> dict:
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
