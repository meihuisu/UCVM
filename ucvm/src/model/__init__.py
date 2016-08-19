import xmltodict
import os
import inspect
import shutil
import ucvm.models
import urllib.request

from subprocess import Popen, PIPE
from distutils.dir_util import copy_tree

from .model import Model
from .elevation import ElevationModel
from .velocity import VelocityModel
from .vs30 import Vs30Model

from ucvm.src.shared import UCVM_MODEL_LIST_FILE, UCVM_MODELS_DIRECTORY, HYPOCENTER_MODEL_LIST, \
                            HYPOCENTER_PREFIX, parse_xmltodict_one_or_many


def get_list_of_installed_models() -> list:
    return_list = {
        "velocity": [],
        "elevation": [],
        "vs30": []
    }

    try:
        with open(UCVM_MODEL_LIST_FILE, "r") as fd:
            model_xml = xmltodict.parse(fd.read())
    except FileNotFoundError:
        return return_list

    for item in parse_xmltodict_one_or_many(model_xml, "root/velocity"):
        return_list["velocity"].append({item["@id"]: item})

    for item in parse_xmltodict_one_or_many(model_xml, "root/elevation"):
        return_list["elevation"].append({item["@id"]: item})

    for item in parse_xmltodict_one_or_many(model_xml, "root/vs30"):
        return_list["vs30"].append({item["@id"]: item})

    return return_list


def get_list_of_installable_internet_models() -> dict:
    installed_models = get_list_of_installed_models()
    model_list_xml = xmltodict.parse(urllib.request.urlopen(HYPOCENTER_MODEL_LIST))

    installable_models = {
        "velocity": [],
        "elevation": [],
        "vs30": []
    }

    for item in parse_xmltodict_one_or_many(model_list_xml, "root/model"):
        ucvm_name = item["file"].split(".")[0]
        if ucvm_name not in installed_models[item["type"]]:
            item["description"] = "\n".join([x.strip() for x in
                                             str(item["description"]).split("\n")])
            item.update({"id": ucvm_name})
            installable_models[item["type"]].append(item)

    return installable_models


def install_internet_ucvm_model(model_ucvm_name: str, long_name: str) -> bool:
    """
    Given a model name, this function downloads and installs it. Note that this assumes that
    model_ucvm_name actually exists!
    :param model_ucvm_name: The name of the model to download.
    :param long_name: The long version of the name of the model to download.
    :return: True if installation was successful, false if not.
    """
    try:
        os.mkdir(os.path.join(UCVM_MODELS_DIRECTORY, "temp"))
    except FileExistsError:
        pass

    print("Downloading " + long_name + "...")

    model_file = urllib.request.URLopener()
    model_file.retrieve(HYPOCENTER_PREFIX + "/models/" + model_ucvm_name + ".ucv",
                        os.path.join(UCVM_MODELS_DIRECTORY, "temp", model_ucvm_name + ".ucv"))

    try:
        os.mkdir(os.path.join(UCVM_MODELS_DIRECTORY, "temp", model_ucvm_name))
    except FileExistsError:
        shutil.rmtree(os.path.join(UCVM_MODELS_DIRECTORY, "temp", model_ucvm_name))

    print("Extracting " + long_name + "...")

    p = Popen(["tar", "-zxvf",
               os.path.join(UCVM_MODELS_DIRECTORY, "temp", model_ucvm_name + ".ucv"),
               "-C", os.path.join(UCVM_MODELS_DIRECTORY, "temp", model_ucvm_name)], stdout=PIPE,
              stderr=PIPE)
    p.wait()

    ret_code = install_ucvm_model_xml(os.path.join(UCVM_MODELS_DIRECTORY, "temp",
                                                   model_ucvm_name, "ucvm_model.xml"))

    os.remove(os.path.join(UCVM_MODELS_DIRECTORY, "temp", model_ucvm_name + ".ucv"))
    shutil.rmtree(os.path.join(UCVM_MODELS_DIRECTORY, "temp", model_ucvm_name))

    return ret_code


def install_ucvm_model_xml(xml_file: str) -> bool:
    """
    Given a ucvm_model.xml file, install the model, including any necessary build actions
    within the UCVM models directory.
    :param xml_file: The ucvm_model.xml descriptor.
    :return: True if the operation was successful, false or error if not.
    """
    with open(xml_file) as fd:
        doc = xmltodict.parse(fd.read())

    xml_info = {
        "id": doc["root"]["information"]["id"],
        "type": doc["root"]["information"]["type"],
        "name": doc["root"]["information"]["identifier"],
        "file": doc["root"]["internal"]["file"],
        "class": doc["root"]["internal"]["class"]
    }

    print("Installing %s..." % xml_info["name"])

    new_path = os.path.join(os.path.dirname(inspect.getfile(ucvm.models)), xml_info["id"])

    try:
        os.mkdir(new_path)
        os.mkdir(os.path.join(new_path, "lib"))
        os.mkdir(os.path.join(new_path, "data"))
    except FileExistsError:
        pass
    except OSError as e:
        raise e

    # Now, let's handle the build, if we're requested to do so.
    build = {
        "dirs": parse_xmltodict_one_or_many(doc, "root/build/data/directory"),
        "makefile": parse_xmltodict_one_or_many(doc, "root/build/makefile"),
        "library": parse_xmltodict_one_or_many(doc, "root/build/library"),
        "setuppy": parse_xmltodict_one_or_many(doc, "root/build/setuppy")
    }

    # First, we execute the makefile(s).
    if len(build["makefile"]) != 0:
        for makefile in build["makefile"]:
            makefile_path = os.path.join(os.path.dirname(xml_file),
                                         os.path.dirname(makefile["#text"]))
            revert_dir = os.getcwd()
            os.chdir(makefile_path)
            p = Popen(["make"], stdout=PIPE, stderr=PIPE)
            p.wait()
            os.chdir(revert_dir)

    # Then we copy the library if it exists.
    if len(build["library"]) != 0:
        for library in build["library"]:
            shutil.copy(os.path.join(os.path.dirname(xml_file), library["#text"] + ".so"),
                        os.path.join(new_path, "lib"))

    # Finally, we copy the data.
    if len(build["dirs"]) != 0:
        for directory in build["dirs"]:
            copy_to = os.path.join(new_path, "data")
            if "#copyto" in directory:
                try:
                    os.mkdir(os.path.join(new_path, "data", directory["#copyto"]))
                    copy_to = os.path.join(new_path, "data", directory["#copyto"])
                except FileExistsError:
                    pass
            copy_tree(os.path.join(os.path.dirname(xml_file), directory["#text"]), copy_to)

    if len(build["setuppy"]) != 0:
        revert_dir = os.getcwd()
        os.chdir(os.path.dirname(xml_file))
        p = Popen(["python3", "setup.py", "install", "--user"], stdout=PIPE, stderr=PIPE)
        p.wait()
        os.chdir(revert_dir)

    # Append the model to the model list.
    with open(UCVM_MODEL_LIST_FILE, "r") as fd:
        model_xml = xmltodict.parse(fd.read())

    if model_xml["root"] is None:
        model_xml["root"] = {xml_info["type"]: []}
    elif xml_info["type"] not in model_xml["root"]:
        model_xml["root"][xml_info["type"]] = []

    if isinstance(model_xml["root"][xml_info["type"]], list):
        model_xml["root"][xml_info["type"]].append({
            "@id": xml_info["id"],
            "@name": xml_info["name"],
            "@file": xml_info["file"],
            "@class": xml_info["class"]
        })
    else:
        model_xml["root"][xml_info["type"]] = [model_xml["root"][xml_info["type"]], {
            "@id": xml_info["id"],
            "@name": xml_info["name"],
            "@file": xml_info["file"],
            "@class": xml_info["class"]
        }]

    with open(UCVM_MODEL_LIST_FILE, "w") as fd:
        fd.write(xmltodict.unparse(model_xml, pretty=True))

    # Now that the directory has been made. Copy the XML file and the class in there.
    if xml_info["file"] is not "None" and ".py" in xml_info["file"]:
        shutil.copy(os.path.join(os.path.dirname(xml_file), xml_info["file"]), new_path)
    shutil.copy(xml_file, new_path)

    return True
