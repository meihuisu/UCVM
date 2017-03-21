import xmltodict
import os
import inspect
import shutil
import copy
import ucvm.models
import urllib.request
import urllib.error
import socket
import sys

from subprocess import Popen, PIPE, STDOUT

from .model import Model
from .elevation import ElevationModel
from .velocity import VelocityModel
from .vs30 import Vs30Model

from ucvm.src.shared import UCVM_MODEL_LIST_FILE, UCVM_LIBRARY_LIST_FILE, UCVM_MODELS_DIRECTORY, \
                            UCVM_LIBRARIES_DIRECTORY, HYPOCENTER_MODEL_LIST, HYPOCENTER_PREFIX, \
                            parse_xmltodict_one_or_many


def get_list_of_installed_models() -> list:
    return_list = {
        "velocity": [],
        "elevation": [],
        "vs30": [],
        "operator": []
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

    for item in parse_xmltodict_one_or_many(model_xml, "root/operator"):
        return_list["operator"].append({item["@id"]: item})

    return return_list


def get_list_of_installable_internet_models() -> dict:
    installed_models = get_list_of_installed_models()
    model_list_xml = xmltodict.parse(urllib.request.urlopen(HYPOCENTER_MODEL_LIST))

    installable_models = {
        "velocity": [],
        "elevation": [],
        "vs30": [],
        "operator": []
    }

    for item in parse_xmltodict_one_or_many(model_list_xml, "root/model"):
        if item["id"] not in installed_models[item["type"]]:
            item["description"] = "\n".join([x.strip() for x in
                                             str(item["description"]).split("\n")])
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
        shutil.rmtree(os.path.join(UCVM_MODELS_DIRECTORY, "temp"))
        os.mkdir(os.path.join(UCVM_MODELS_DIRECTORY, "temp"))

    print("Downloading " + long_name + "...")

    error_raised = True
    while error_raised:
        try:
            urllib.request.urlretrieve(
                HYPOCENTER_PREFIX + "/models/" + model_ucvm_name + ".ucv",
                os.path.join(UCVM_MODELS_DIRECTORY, "temp", model_ucvm_name + ".ucv")
            )
        except urllib.error.URLError:
            print("\tDownload errored out. Retrying...")
            continue
        except socket.error:
            print("\tDownload errored out. Retrying...")
            continue
        error_raised = False

    """
    print("Copying " + long_name + "...")
    shutil.copy(
        "/Users/davidgil/PycharmProjects/UCVM/distribution/models/" + model_ucvm_name + ".ucv",
        os.path.join(UCVM_MODELS_DIRECTORY, "temp", model_ucvm_name + ".ucv")
    )
    """

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


def download_and_install_library(library_name: str) -> bool:
    """
    Given a library name, download and install it to ucvm.libraries.
    :param library_name: The library name to install.
    :return: True if success, false if not.
    """
    try:
        with open(UCVM_LIBRARY_LIST_FILE, "r") as fd:
            library_xml = xmltodict.parse(fd.read())
    except FileNotFoundError:
        return False

    # Don't install if it is already installed.
    for item in parse_xmltodict_one_or_many(library_xml, "root/library"):
        if item["@id"] == library_name:
            return True

    try:
        os.mkdir(os.path.join(UCVM_LIBRARIES_DIRECTORY, "temp"))
    except FileExistsError:
        shutil.rmtree(os.path.join(UCVM_LIBRARIES_DIRECTORY, "temp"))
        os.mkdir(os.path.join(UCVM_LIBRARIES_DIRECTORY, "temp"))

    print("        Downloading library " + library_name + "...")

    library_file = urllib.request.URLopener()
    library_file.retrieve(HYPOCENTER_PREFIX + "/libraries/" + library_name + ".ucv",
                          os.path.join(UCVM_LIBRARIES_DIRECTORY, "temp", library_name + ".ucv"))

    try:
        os.mkdir(os.path.join(UCVM_LIBRARIES_DIRECTORY, "temp", library_name))
    except FileExistsError:
        shutil.rmtree(os.path.join(UCVM_LIBRARIES_DIRECTORY, "temp", library_name))

    try:
        os.mkdir(os.path.join(UCVM_LIBRARIES_DIRECTORY, library_name))
    except FileExistsError:
        pass

    print("        Extracting " + library_name + "...")

    p = Popen(["tar", "-zxvf",
               os.path.join(UCVM_LIBRARIES_DIRECTORY, "temp", library_name + ".ucv"),
               "-C", os.path.join(UCVM_LIBRARIES_DIRECTORY, library_name)], stdout=PIPE,
              stderr=PIPE)
    p.wait()

    print("        Installing " + library_name + "...")

    cwd = os.getcwd()
    os.chdir(os.path.join(UCVM_LIBRARIES_DIRECTORY, library_name))

    p = Popen(["python3", "build.py"])
    p.communicate()

    os.chdir(cwd)

    os.remove(os.path.join(UCVM_LIBRARIES_DIRECTORY, "temp", library_name + ".ucv"))

    return True


def install_ucvm_model_xml(xml_file: str) -> dict:
    """
    Given a ucvm_model.xml file, install the model, including any necessary build actions
    within the UCVM models directory.
    :param xml_file: The ucvm_model.xml descriptor.
    :return: Dictionary containing the model info if successful, false or error if not.
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
        "configure": parse_xmltodict_one_or_many(doc, "root/build/configure"),
        "makefile": parse_xmltodict_one_or_many(doc, "root/build/makefile"),
        "install": parse_xmltodict_one_or_many(doc, "root/build/install"),
        "library": parse_xmltodict_one_or_many(doc, "root/build/library"),
        "setuppy": parse_xmltodict_one_or_many(doc, "root/build/setuppy"),
        "requires_lib": parse_xmltodict_one_or_many(doc, "root/build/requires/library"),
        "requires_model": parse_xmltodict_one_or_many(doc, "root/build/requires/model")
    }

    # Get the libraries.
    if len(build["requires_lib"]) != 0:
        print("    Installing pre-requisite libraries...")
        for library in build["requires_lib"]:
            download_and_install_library(library["#text"])

    if len(build["configure"]) != 0 or len(build["makefile"]) != 0:
        print("    Starting build process...")

    # We need to execute the config file(s).
    if len(build["configure"]) != 0:
        for config in build["configure"]:
            configure_path = os.path.join(os.path.dirname(xml_file),
                                          os.path.dirname(config["#text"]))
            revert_dir = os.getcwd()
            os.chdir(configure_path)

            new_env = copy.copy(os.environ)
            new_env["ETREE_INCDIR"] = os.path.join(UCVM_LIBRARIES_DIRECTORY, "euclid3", "include")
            new_env["ETREE_LIBDIR"] = os.path.join(UCVM_LIBRARIES_DIRECTORY, "euclid3", "lib")
            new_env["PROJ4_INCDIR"] = os.path.join(UCVM_LIBRARIES_DIRECTORY, "proj4", "include")
            new_env["PROJ4_LIBDIR"] = os.path.join(UCVM_LIBRARIES_DIRECTORY, "proj4", "lib")
            new_env["CPPFLAGS"] = "-I" + os.path.join(UCVM_LIBRARIES_DIRECTORY, "euclid3",
                                                      "include") + \
                                  " -I" + os.path.join(UCVM_LIBRARIES_DIRECTORY, "proj4", "include")
            new_env["LDFLAGS"] = "-L" + os.path.join(UCVM_LIBRARIES_DIRECTORY, "euclid3", "lib") + \
                                 " -L" + os.path.join(UCVM_LIBRARIES_DIRECTORY, "proj4", "lib")

            prefix = []

            if len(build["install"]) != 0:
                prefix = ["--prefix=" + str(build["install"][0]["#text"]).replace("[MODEL_DIR]",
                                                                                  new_path)]

            p = Popen([os.path.join(".", "configure")] + prefix, env=new_env)
            p.wait()

            os.chdir(revert_dir)

    # First, we execute the makefile(s).
    if len(build["makefile"]) != 0:
        for makefile in build["makefile"]:
            makefile_path = os.path.join(os.path.dirname(xml_file),
                                         os.path.dirname(makefile["#text"]))
            revert_dir = os.getcwd()
            os.chdir(makefile_path)

            p = Popen(["make"])
            p.wait()

            # If install is set, run make install.
            p = Popen(["make", "install"])
            p.wait()

            os.chdir(revert_dir)

    # Then we copy the library if it exists.
    if len(build["library"]) != 0:
        for library in build["library"]:
            shutil.copy(os.path.join(os.path.dirname(xml_file), library["#text"] + ".so"),
                        os.path.join(new_path, "lib"))

    if len(build["setuppy"]) != 0:
        print("    Executing build script for legacy model code...")
        revert_dir = os.getcwd()
        os.chdir(os.path.dirname(xml_file))
        if "anaconda" in sys.version.lower() or "continuum" in sys.version.lower():
            p = Popen(["python3", "setup.py", "install"], env=os.environ)
        else:
            p = Popen(["python3", "setup.py", "install", "--prefix=" + os.path.abspath("../../../../../../../../")])
        p.wait()
        os.chdir(revert_dir)

    # Finally, we copy the data.
    if len(build["dirs"]) != 0:
        print("    Moving model data to directory...")
        for directory in build["dirs"]:
            copy_to = os.path.join(new_path, "data")
            if "#copyto" in directory:
                try:
                    os.mkdir(os.path.join(new_path, "data", directory["#copyto"]))
                    copy_to = os.path.join(new_path, "data", directory["#copyto"])
                except FileExistsError:
                    pass
            p = Popen("mv " + os.path.join(os.path.dirname(xml_file), directory["#text"], "*") +
                      " " + copy_to, shell=True)
            p.communicate()

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

    # Fix for Anaconda hard-link issue. We need to first delete the file and create a new one to ensure that
    # we're just not writing the hard link.
    os.remove(UCVM_MODEL_LIST_FILE)

    # Now we can write the XML.
    with open(UCVM_MODEL_LIST_FILE, "w") as fd:
        fd.write(xmltodict.unparse(model_xml, pretty=True))

    # Now that the directory has been made. Copy the XML file and the class in there.
    if xml_info["file"] is not "None" and ".py" in xml_info["file"]:
        shutil.copy(os.path.join(os.path.dirname(xml_file), xml_info["file"]), new_path)

    # If there's a test_id.py file, copy that to the path.
    if os.path.exists(os.path.join(os.path.dirname(xml_file), "test_" + xml_info["id"] + ".py")):
        shutil.copy(
            os.path.join(os.path.dirname(xml_file), "test_" + xml_info["id"] + ".py"),
            new_path
        )
    if os.path.exists(os.path.join(os.path.dirname(xml_file), "test_" + xml_info["id"] + ".npy")):
        shutil.copy(
            os.path.join(os.path.dirname(xml_file), "test_" + xml_info["id"] + ".npy"),
            new_path
        )

    shutil.copy(xml_file, new_path)

    return xml_info
