"""
E-tree generation functions.

This file contains all the common functions necessary to generate an e-tree through UCVM.

Copyright:
    Southern California Earthquake Center

Developer:
    David Gill <davidgil@usc.edu>
"""
# Python Imports
import os
import sys
import math
import copy
from datetime import datetime
from typing import List

# Package Imports
import xmltodict

# UCVM Imports
from ucvm.src.framework.ucvm import UCVM
from ucvm.src.shared.properties import SeismicData
from ucvm.src.shared.functions import ask_and_validate, is_number
from ucvm_c_common import UCVMCCommon


def etree_extract_mpi(information: dict, rows: str=None, interval: str=None) -> bool:
    from mpi4py import MPI

    comm = MPI.COMM_WORLD
    rank = comm.Get_rank()
    size = comm.Get_size()

    stats = _calculate_etree_stats(
        information, int(information["properties"]["columns"]), int(information["properties"]["rows"])
    )

    start_rc = [1, 1]
    end_rc = [information["properties"]["rows"], information["properties"]["columns"]]

    if rows is not None:
        if "-" in rows:
            parts = rows.split("-")
            start_rc = [int(parts[0]), 1]
            end_rc = [int(parts[1]), int(information["properties"]["columns"])]
        else:
            start_rc = [int(rows), 1]
            end_rc = [int(rows), int(information["properties"]["columns"])]
    elif interval is not None:
        if "-" in interval:
            parts = interval.split("-")
            parts1 = parts[0].split(",")
            start_rc = [int(parts1[0]), int(parts1[1])]
            parts2 = parts[1].split(",")
            end_rc = [int(parts2[0]), int(parts2[1])]
        else:
            interval = interval.split(",")
            start_rc = [int(interval[0]), int(interval[1])]
            end_rc = [int(interval[0]), int(interval[1])]

    if rank == 0:
        schema = "float Vp; float Vs; float density;".encode("ASCII")
        path = (information["etree_name"] + ".e").encode("ASCII")

        if sys.byteorder == "little" and sys.platform != "darwin":
            ep = UCVMCCommon.c_etree_open(path, 578)
        else:
            if start_rc[0] == 1 and start_rc[1] == 1:
                ep = UCVMCCommon.c_etree_open(path, 1538)
            else:
                ep = UCVMCCommon.c_etree_open(path, 2)

        UCVMCCommon.c_etree_registerschema(ep, schema)

        rcs_to_extract = []

        current_rc = start_rc
        while not (current_rc[0] == end_rc[0] and current_rc[1] == end_rc[1] + 1):
            if current_rc[1] > int(information["properties"]["columns"]):
                current_rc[0] += 1
                current_rc[1] = 1
            rcs_to_extract.append((current_rc[0], current_rc[1]))
            current_rc[1] += 1

        total_extracted = 0
        is_done = [False for _ in range(size - 1)]

        while True:
            data = comm.recv(source=MPI.ANY_SOURCE)

            if data["code"] == "start":
                if len(rcs_to_extract) > 0:
                    comm.send(rcs_to_extract.pop(0), dest=data["source"])
                else:
                    comm.send("done", dest=data["source"])
                    is_done[data["source"] - 1] = True
            elif data["code"] == "write":
                print("[Node %d] Writing data from node %d to file." % (rank, data["source"]), flush=True)
                _etree_writer(ep, data["data"][0], data["data"][1], data["data"][2])
                total_extracted += data["data"][2]
                print("[Node %d] Data written successfully!" % rank, flush=True)
            elif data["code"] == "new":
                if len(rcs_to_extract) > 0:
                    comm.send(rcs_to_extract.pop(0), dest=data["source"])
                else:
                    comm.send("done", dest=data["source"])
                    is_done[data["source"] - 1] = True
                print("[Node %d] Writing data from node %d to file." % (rank, data["source"]), flush=True)
                _etree_writer(ep, data["data"][0], data["data"][1], data["data"][2])
                total_extracted += data["data"][2]
                print("[Node %d] Data written successfully!" % rank, flush=True)

            if False not in is_done:
                break

        metadata_string = ("Title:%s Author:%s Date:%s %u %s %f %f %f %f %f %f %u %u %u" % (
            str(information["author"]["title"]).replace(" ", "_"),
            str(information["author"]["person"]).replace(" ", "_"),
            str(information["author"]["date"]).replace(" ", "_"), 3, "Vp(float);Vs(float);density(float)",
            float(information["corners"]["bl"]["y"]), float(information["corners"]["bl"]["x"]),
            float(information["dimensions"]["x"]), float(information["dimensions"]["y"]),
            0, float(information["dimensions"]["z"]), int(stats["max_ticks"]["width"]),
            int(stats["max_ticks"]["height"]), int(stats["max_ticks"]["depth"])
        )).encode("ASCII")

        print("[Node %d] Total number of octants extracted: %d." % (rank, total_extracted), flush=True)

        UCVMCCommon.c_etree_setappmeta(ep, metadata_string)

        UCVMCCommon.c_etree_close(ep)
    else:
        done = False
        print("[Node %d] Maximum points per section is %d." % (rank, stats["max_points"]), flush=True)
        sd_array = UCVM.create_max_seismicdata_array(stats["max_points"], 1)
        count = 0

        comm.send({"source": rank, "data": "", "code": "start"}, dest=0)

        while not done:
            row_col = comm.recv(source=0)

            if row_col == "done":
                break

            print("[Node %d] Received instruction to extract column (%d, %d)." % (rank, row_col[0], row_col[1]),
                  flush=True)
            data = _extract_mpi(rank, sd_array, information, stats, row_col[1] - 1, row_col[0] - 1)
            count += data[2]
            print("[Node %d] Finished extracting column (%d, %d)" % (rank, row_col[0], row_col[1]))
            comm.send({"source": rank, "data": data, "code": "new"}, dest=0)

        print("[Node %d] Finished extracting %d octants." % (rank, count), flush=True)

    return True


def etree_extract_single(information: dict, rows: str=None, interval: str=None) -> bool:
    schema = "float Vp; float Vs; float density;".encode("ASCII")
    path = (information["etree_name"] + ".e").encode("ASCII")

    start_rc = [1, 1]
    end_rc = [information["properties"]["rows"], information["properties"]["columns"]]

    if rows is not None:
        if "-" in rows:
            parts = rows.split("-")
            start_rc = [int(parts[0]), 1]
            end_rc = [int(parts[1]), int(information["properties"]["columns"])]
        else:
            start_rc = [int(rows), 1]
            end_rc = [int(rows), int(information["properties"]["columns"])]
    elif interval is not None:
        if "-" in interval:
            parts = interval.split("-")
            parts1 = parts[0].split(",")
            start_rc = [int(parts1[0]), int(parts1[1])]
            parts2 = parts[1].split(",")
            end_rc = [int(parts2[0]), int(parts2[1])]
        else:
            interval = interval.split(",")
            start_rc = [int(interval[0]), int(interval[1])]
            end_rc = [int(interval[0]), int(interval[1])]

    if sys.byteorder == "little" and sys.platform != "darwin":
        ep = UCVMCCommon.c_etree_open(path, 578)
    else:
        if start_rc[0] == 1 and start_rc[1] == 1:
            ep = UCVMCCommon.c_etree_open(path, 1538)
        else:
            ep = UCVMCCommon.c_etree_open(path, 2)

    UCVMCCommon.c_etree_registerschema(ep, schema)

    octant_count = 0

    stats = _calculate_etree_stats(
        information, int(information["properties"]["columns"]), int(information["properties"]["rows"])
    )

    sd_array = UCVM.create_max_seismicdata_array(stats["max_points"], 1)

    print("Maximum points per section is %d" % stats["max_points"])

    current_rc = start_rc

    while not (current_rc[0] == end_rc[0] and current_rc[1] == end_rc[1] + 1):
        if current_rc[1] > int(information["properties"]["columns"]):
            current_rc[0] += 1
            current_rc[1] = 1

        count = _extract_single(ep, sd_array, information, stats, current_rc[1] - 1, current_rc[0] - 1)

        if count is not None:
            octant_count += count
        else:
            raise Exception("HELP!")

        current_rc[1] += 1

    print(str(octant_count) + " octants were extracted.")

    metadata_string = ("Title:%s Author:%s Date:%s %u %s %f %f %f %f %f %f %u %u %u" % (
        str(information["author"]["title"]).replace(" ", "_"), str(information["author"]["person"]).replace(" ", "_"),
        str(information["author"]["date"]).replace(" ", "_"), 3, "Vp(float);Vs(float);density(float)",
        float(information["corners"]["bl"]["y"]), float(information["corners"]["bl"]["x"]),
        float(information["dimensions"]["x"]), float(information["dimensions"]["y"]),
        0, float(information["dimensions"]["z"]), int(stats["max_ticks"]["width"]),
        int(stats["max_ticks"]["height"]), int(stats["max_ticks"]["depth"])
    )).encode("ASCII")

    UCVMCCommon.c_etree_setappmeta(ep, metadata_string)

    UCVMCCommon.c_etree_close(ep)

    return True


def _etree_writer(ep: int, etree_pnts: dict, props: List[SeismicData], n: int):
    for i in range(n):
        UCVMCCommon.c_etree_insert(
            ep, etree_pnts[i]["x"], etree_pnts[i]["y"], etree_pnts[i]["z"], etree_pnts[i]["level"],
            props[i].velocity_properties.vp, props[i].velocity_properties.vs,
            props[i].velocity_properties.density
        )


def _extract_mpi(rank: int, sd_array: List[SeismicData], cfg: dict, stats: dict, column: int, row: int) \
        -> (dict, list, int, int):
    from mpi4py import MPI

    comm = MPI.COMM_WORLD

    level = int(stats["max_level"])
    edgesize = stats["max_length"] / (1 << level)
    edgetics = 1 << (31 - level)
    extracted = 0
    ztics = 0

    ret_dict = {}
    ret_list = []

    print("[Node %d] Extracting row %d, column %d..." % (rank, row + 1, column + 1), flush=True)

    num_points, etree_addrs = _get_grid(sd_array, cfg, stats, level, column, row, ztics)

    if num_points > stats["max_points"]:
        print("ERROR: Num points exceeds max points")

    while ztics < stats["max_ticks"]["depth"]:
        if ztics % (1 << (31 - level)) != 0:
            print("ERROR: Ztics")

        print("[Node %d]\tQuerying velocity model for %d points" % (rank, num_points), flush=True)

        UCVM.query(sd_array[0:num_points], cfg["cvm_list"], ["velocity"])

        scanlevel = _get_level(cfg, stats, sd_array, num_points)

        if scanlevel < int(stats["min_level"]):
            scanlevel = int(stats["min_level"])
        elif scanlevel > int(stats["max_level"]):
            scanlevel = int(stats["max_level"])

        if level < scanlevel:
            print("[Node %d]\tIncreasing scan level to %d" % (rank, scanlevel), flush=True)
            level = scanlevel
            edgesize = stats["max_length"] / (1 << level)
            edgetics = 1 << (31 - level)
            num_points, etree_addrs = _get_grid(sd_array, cfg, stats, level, column, row, ztics)
            print("[Node %d]\tQuerying velocity model for %d points" % (rank, num_points), flush=True)
            UCVM.query(sd_array[0:num_points], cfg["cvm_list"], ["velocity"])
        elif level > scanlevel:
            for l in range(scanlevel, level):
                if ztics % (1 << (31 - l)) == 0 and \
                   ztics + (1 << (31 - l)) <= stats["max_ticks"]["depth"]:
                    print("[Node %d]\tDecreasing scan level to %d" % (rank, l), flush=True)
                    level = l
                    edgesize = stats["max_length"] / (1 << level)
                    edgetics = 1 << (31 - level)
                    num_points, etree_addrs = _get_grid(sd_array, cfg, stats, level, column, row, ztics)
                    print("[Node %d]\tQuerying velocity model for %d points" % (rank, num_points), flush=True)
                    UCVM.query(sd_array[0:num_points], cfg["cvm_list"], ["velocity"])
                    break

        for i in range(num_points):
            ret_list.append(copy.copy(sd_array[i]))
            ret_dict[len(ret_list) - 1] = copy.copy(etree_addrs[i])

        if len(ret_list) > 100000:
            comm.send({"source": rank, "data": (ret_dict, ret_list, len(ret_list)), "code": "write"}, dest=0)
            ret_list = []
            ret_dict = {}

        extracted += num_points

        ztics += edgetics

        for item in sd_array:
            item.original_point.z_value = (ztics * stats["tick_size"]) + edgesize / 2.0
        for _, item in etree_addrs.items():
            item["z"] = ztics

    if ztics != stats["max_ticks"]["depth"]:
        raise Exception("Ticks mismatch")

    return ret_dict, ret_list, len(ret_list), extracted


def _extract_single(ep: int, sd_array: List[SeismicData], cfg: dict, stats: dict, column: int, row: int) -> int:
    level = int(stats["max_level"])
    edgesize = stats["max_length"] / (1 << level)
    edgetics = 1 << (31 - level)
    extracted = 0
    ztics = 0

    print("Extracting row %d, column %d..." % (row + 1, column + 1), flush=True)

    num_points, etree_addrs = _get_grid(sd_array, cfg, stats, level, column, row, ztics)

    if num_points > stats["max_points"]:
        print("ERROR: Num points exceeds max points")

    while ztics < stats["max_ticks"]["depth"]:
        if ztics % (1 << (31 - level)) != 0:
            print("ERROR: Ztics")

        print("\tQuerying velocity model for %d points" % num_points, flush=True)

        UCVM.query(sd_array[0:num_points], cfg["cvm_list"], ["velocity"])

        scanlevel = _get_level(cfg, stats, sd_array, num_points)

        if scanlevel < int(stats["min_level"]):
            scanlevel = int(stats["min_level"])
        elif scanlevel > int(stats["max_level"]):
            scanlevel = int(stats["max_level"])

        if level < scanlevel:
            print("\tIncreasing scan level to %d" % scanlevel)
            level = scanlevel
            edgesize = stats["max_length"] / (1 << level)
            edgetics = 1 << (31 - level)
            num_points, etree_addrs = _get_grid(sd_array, cfg, stats, level, column, row, ztics)
            print("\tQuerying velocity model for %d points" % num_points, flush=True)
            UCVM.query(sd_array[0:num_points], cfg["cvm_list"], ["velocity"])
        elif level > scanlevel:
            for l in range(scanlevel, level):
                if ztics % (1 << (31 - l)) == 0 and \
                   ztics + (1 << (31 - l)) <= stats["max_ticks"]["depth"]:
                    print("\tDecreasing scan level to %d" % l)
                    level = l
                    edgesize = stats["max_length"] / (1 << level)
                    edgetics = 1 << (31 - level)
                    num_points, etree_addrs = _get_grid(sd_array, cfg, stats, level, column, row, ztics)
                    print("\tQuerying velocity model for %d points" % num_points, flush=True)
                    UCVM.query(sd_array[0:num_points], cfg["cvm_list"], ["velocity"])
                    break

        print("\tWriting points to e-tree file", flush=True)
        _etree_writer(ep, etree_addrs, sd_array, num_points)
        extracted += num_points

        ztics += edgetics

        for item in sd_array:
            item.original_point.z_value = (ztics * stats["tick_size"]) + edgesize / 2.0
        for _, item in etree_addrs.items():
            item["z"] = ztics

    if ztics != stats["max_ticks"]["depth"]:
        raise Exception("Ticks mismatch")

    return extracted


def ask_questions() -> dict:
    """
    Asks the questions of the user that are necessary to generate the XML file for the e-tree.

    Returns:
        A dictionary containing the answers.
    """
    answers = {
        "cvm_list": "",
        "projection": "geo-bilinear",
        "corners": {
            "bl": {
                "x": 0.0,
                "y": 0.0
            },
            "ul": {
                "x": 0.0,
                "y": 0.0
            },
            "ur": {
                "x": 0.0,
                "y": 0.0
            },
            "br": {
                "x": 0.0,
                "y": 0.0
            }
        },
        "dimensions": {
            "x": 0,
            "y": 0,
            "z": 0
        },
        "properties": {
            "max_frequency": 0,
            "parts_per_wavelength": 0,
            "max_octant_size": 0,
            "columns": 0,
            "rows": 0
        },
        "minimums": {
            "vp": 0,
            "vs": 0
        },
        "author": {
            "title": "",
            "person": "",
            "date": ""
        },
        "out_dir": "",
        "etree_name": ""
    }

    print(
        "\nGenerating an e-tree requires the definition of various parameters to be defined\n"
        "(such as the origin of the mesh, the length of the mesh, and so on). The following\n"
        "questions will guide you through the definition of those parameters. At the end, you\n"
        "will be asked if you want to just generate the configuration file to make the e-tree\n"
        "at a later time or if you want to generate the e-tree immediately.\n"
    )

    answers["cvm_list"] = ask_and_validate(
        "From which velocity model(s) should this mesh be generated:"
    )

    print(
        "\nE-trees are constructed by specifying four corners as well as the width, height, and\n"
        "depth of the desired volume."
    )

    answers["corners"]["bl"]["x"] = ask_and_validate(
        "\nWhat is the longitude of the bottom-left corner of the volume?"
    )
    answers["corners"]["bl"]["y"] = ask_and_validate(
        "What is the latitude of the bottom-left corner of the volume?"
    )
    answers["corners"]["ul"]["x"] = ask_and_validate(
        "What is the longitude of the upper-left corner of the volume?"
    )
    answers["corners"]["ul"]["y"] = ask_and_validate(
        "What is the latitude of the upper-left corner of the volume?"
    )
    answers["corners"]["ur"]["x"] = ask_and_validate(
        "What is the longitude of the upper-right corner of the volume?"
    )
    answers["corners"]["ur"]["y"] = ask_and_validate(
        "What is the latitude of the upper-right corner of the volume?"
    )
    answers["corners"]["br"]["x"] = ask_and_validate(
        "What is the longitude of the bottom-right corner of the volume?"
    )
    answers["corners"]["br"]["y"] = ask_and_validate(
        "What is the latitude of the bottom-right corner of the volume?"
    )

    answers["dimensions"]["x"] = float(ask_and_validate(
        "\nPlease provide the width (X dimension) of the e-tree in meters:",
        is_number, "Answer must be a positive number."
    ))
    answers["dimensions"]["y"] = float(ask_and_validate(
        "Please provide the height (Y dimension) of the e-tree in meters:",
        is_number, "Answer must be a positive number."
    ))
    answers["dimensions"]["z"] = float(ask_and_validate(
        "Please provide the depth (Z dimension) of the e-tree in meters:",
        is_number, "Answer must be a positive number."
    ))

    answers["minimums"]["vs"] = int(ask_and_validate(
        "\nWhat should the minimum Vs, in meters per second, be? The default is 0: ",
        is_number, "Answer must be a number."
    ))
    answers["minimums"]["vp"] = int(ask_and_validate(
        "What should the minimum Vp, in meters per second, be? The default is 0: ",
        is_number, "Answer must be a number."
    ))

    answers["properties"]["max_frequency"] = float(ask_and_validate(
        "\nWhat is the maximum simulation frequency this e-tree will be used for?",
        is_number, "Answer must be a number."
    ))
    answers["properties"]["parts_per_wavelength"] = float(ask_and_validate(
        "How many parts per wavelength must this e-tree contain?",
        is_number, "Answer must be a number."
    ))
    answers["properties"]["max_octant_size"] = float(ask_and_validate(
        "What is the maximum size that each octant can be (in meters)?",
        is_number, "Answer must be a number."
    ))

    answers["properties"]["columns"] = float(ask_and_validate(
        "\nWhat should the column size for extraction be?", is_number, "Answer must be a number."
    ))
    answers["properties"]["rows"] = float(ask_and_validate(
        "What should the row size for extraction be?", is_number, "Answer must be a number."
    ))

    answers["author"]["title"] = ask_and_validate(
        "\nWhat is the title for this e-tree?"
    )
    answers["author"]["person"] = ask_and_validate(
        "Who is the author of this e-tree?"
    )
    answers["author"]["date"] = datetime.now()

    answers["out_dir"] = ask_and_validate("\nTo which directory should the e-tree and metadata be "
                                          "saved?")
    answers["etree_name"] = ask_and_validate("Please provide a name for this e-tree:")

    # Output the summary.
    print(
        "E-tree configuration complete! The e-tree that you have specified is as follows:\n\n"
        "Name:               " + answers["etree_name"] + "\n"
        "Output directory:   " + answers["out_dir"] + "\n"
        "Bottom-left point:  " + answers["corners"]["bl"]["x"] + ", " +
        answers["corners"]["bl"]["y"] + "\n"
        "Top-left point:     " + answers["corners"]["ul"]["x"] + ", " +
        answers["corners"]["ul"]["y"] + "\n"
        "Top-right point:    " + answers["corners"]["ur"]["x"] + ", " +
        answers["corners"]["ur"]["y"] + "\n"
        "Bottom-right point: " + answers["corners"]["br"]["x"] + ", " +
        answers["corners"]["br"]["y"] + "\n"
        "Dimensions:\n"
        "    Width:   " + str(answers["dimensions"]["x"]) + "m\n"
        "    Height:  " + str(answers["dimensions"]["y"]) + "m\n"
        "    Depth:   " + str(answers["dimensions"]["z"]) + "m\n"
        "E-tree Parameters:\n"
        "    Maximum frequency:    " + str(answers["properties"]["max_frequency"]) + "\n"
        "    Parts per wavelength: " + str(answers["properties"]["parts_per_wavelength"]) + "\n"
        "    Maximum octant size:  " + str(answers["properties"]["max_octant_size"]) + "\n"
        "Finally, the material properties in this mesh will be no lower than:\n"
        "    Vp: " + str(answers["minimums"]["vp"]) + "\n"
        "    Vs: " + str(answers["minimums"]["vs"])
    )

    filename = answers["etree_name"] + ".xml"
    kml_filename = answers["etree_name"] + ".kml"

    with open(os.path.join(".", filename), "w") as fd:
        fd.write(xmltodict.unparse({"root": answers}, pretty=True))

    # Create the KML.
    kml_xml = {
        "kml": {
            "@xmlns": "http://www.opengis.net/kml/2.2",
            "@xmlns:gx": "http://www.google.com/kml/ext/2.2",
            "@xmlns:kml": "http://www.opengis.net/kml/2.2",
            "@xmlns:atom": "http://www.w3.org/2005/Atom",
            "Document": {
                "name": answers["etree_name"] + ".kml",
                "StyleMap": {
                    "Pair": [
                        {
                            "key": "normal",
                            "styleUrl": "#s_ylw-pushpin"
                        },
                        {
                            "key": "highlight",
                            "styleUrl": "#s_ylw-pushpin_hl1"
                        }
                    ]
                },
                "Style": [
                    {
                        "@id": "s_ylw-pushpin",
                        "IconStyle": {
                            "scale": 1.1,
                            "Icon": {
                                "href": "http://maps.google.com/mapfiles/kml/pushpin/ylw-pushpin." +
                                        "png"
                            },
                            "hotSpot": {
                                "@x": "20",
                                "@y": "2",
                                "@xunits": "pixels",
                                "@yunits": "pixels"
                            }
                        },
                        "LineStyle": {
                            "color": "ff000000",
                            "width": 5
                        }
                    },
                    {
                        "@id": "s_ylw-pushpin_hl1",
                        "IconStyle": {
                            "scale": 1.3,
                            "Icon": {
                                "href": "http://maps.google.com/mapfiles/kml/pushpin/ylw-pushpin." +
                                        "png"
                            },
                            "hotSpot": {
                                "@x": "20",
                                "@y": "2",
                                "@xunits": "pixels",
                                "@yunits": "pixels"
                            }
                        },
                        "LineStyle": {
                            "color": "ff000000",
                            "width": 5
                        }
                    }
                ],
                "Placemark": [
                    {
                        "name": answers["etree_name"] + " Bottom Line",
                        "styleURL": "#m_ylw-pushpin0",
                        "LineString": {
                            "tessellate": 0,
                            "coordinates": str(answers["corners"]["bl"]["x"]) + "," +
                                           str(answers["corners"]["bl"]["y"]) + ",0 " +
                                           str(answers["corners"]["br"]["x"]) + "," +
                                           str(answers["corners"]["br"]["y"]) + ",0"
                        }
                    },
                    {
                        "name": answers["etree_name"] + " Left Line",
                        "styleURL": "#m_ylw-pushpin0",
                        "LineString": {
                            "tessellate": 0,
                            "coordinates": str(answers["corners"]["bl"]["x"]) + "," +
                                           str(answers["corners"]["bl"]["y"]) + ",0 " +
                                           str(answers["corners"]["ul"]["x"]) + "," +
                                           str(answers["corners"]["ul"]["y"]) + ",0"
                        }
                    },
                    {
                        "name": answers["etree_name"] + " Top Line",
                        "styleURL": "#m_ylw-pushpin0",
                        "LineString": {
                            "tessellate": 0,
                            "coordinates": str(answers["corners"]["ul"]["x"]) + "," +
                                           str(answers["corners"]["ul"]["y"]) + ",0 " +
                                           str(answers["corners"]["ur"]["x"]) + "," +
                                           str(answers["corners"]["ur"]["y"]) + ",0"
                        }
                    },
                    {
                        "name": answers["etree_name"] + " Right Line",
                        "styleURL": "#m_ylw-pushpin0",
                        "LineString": {
                            "tessellate": 0,
                            "coordinates": str(answers["corners"]["ur"]["x"]) + "," +
                                           str(answers["corners"]["ur"]["y"]) + ",0 " +
                                           str(answers["corners"]["br"]["x"]) + "," +
                                           str(answers["corners"]["br"]["y"]) + ",0"
                        }
                    }
                ]
            }
        }
    }

    with open(os.path.join(".", kml_filename), "w") as fd:
        fd.write(xmltodict.unparse(kml_xml, pretty=True))

    print(
        "\nYour e-tree metadata file has been saved to " + filename + ".\n"
        "A KML file showing the four corners and bounding region of the e-tree has been saved " +
        "to\n" + kml_filename + ". You can use the XML file to generate and regenerate the " +
        "e-tree, and the\nKML file to visualize the boundaries."
    )


def _get_grid(sd_array: List[SeismicData], cfg: dict, stats: dict, level: int, column: int,
              row: int, ztics: int) -> (int, dict):
    """
    Internal utility which generates the query grid based on the z level (in tics), the column and
    row, as well as the desired level. It returns the number of queried points (which cannot be
    greater than the maximum points in the stats dictionary), as well as a dictionary of the
    e-tree address locations.

    Args:
        sd_array (`obj`:List of `obj`:SeismicData): The list of SeismicData objects to fill with
            the X, Y, and Z values in geo-bilinear projection.
        cfg (dict): The configuration XML file as a dictionary.
        stats (dict): The statistics as calculated from _get_etree_stats.
        level (int): Our curent extraction resolution level.
        column (int): The column to extract.
        row (int): The row to extract.
        ztics (int): The z level to extract in tics.

    Returns:
        A tuple containing an int with the number of points extracted, and a dictionary of the
        e-tree address locations for each lat, lon.
    """
    edgesize = stats["max_length"] / (1 << level)
    edgetics = 1 << (31 - level)
    num_points = 0
    etree_pnts = {}

    grid_z = (ztics * stats["tick_size"]) + edgesize / 2.0

    imin = int(column * stats["column_ticks"])
    jmin = int(row * stats["row_ticks"])
    imax = int(imin + stats["column_ticks"])
    jmax = int(jmin + stats["row_ticks"])

    for j in range(jmin, jmax, edgetics):
        for i in range(imin, imax, edgetics):
            x_val = (i + (edgetics / 2.0)) * stats["tick_size"]
            y_val = (j + (edgetics / 2.0)) * stats["tick_size"]
            new_y, new_x = UCVMCCommon.c_etree_bilinear_xy2geo(
                x_val, y_val,
                (
                    (float(cfg["corners"]["bl"]["x"]), float(cfg["corners"]["bl"]["y"])),
                    (float(cfg["corners"]["ul"]["x"]), float(cfg["corners"]["ul"]["y"])),
                    (float(cfg["corners"]["ur"]["x"]), float(cfg["corners"]["ur"]["y"])),
                    (float(cfg["corners"]["br"]["x"]), float(cfg["corners"]["br"]["y"]))
                ),
                (
                    float(cfg["dimensions"]["x"]),
                    float(cfg["dimensions"]["y"]),
                    float(cfg["dimensions"]["z"])
                )
            )
            sd_array[num_points].original_point.x_value = new_x
            sd_array[num_points].original_point.y_value = new_y
            sd_array[num_points].original_point.z_value = grid_z

            etree_pnts[num_points] = {
                "x": i,
                "y": j,
                "z": ztics,
                "level": level,
                "type": 1
            }

            num_points += 1

    return num_points, etree_pnts


def _get_level(cfg: dict, stats: dict, sd_array: List[SeismicData], num_points: int) -> int:
    """
    Given the minimum found velocity in sd_array, calculate the level that is needed to properly
    represent it.

    Args:
        cfg (dict): The configuration XML file as a dictionary.
        stats (dict): The statistics as calculated using the _calculate_etree_stats function.
        sd_array (`obj`:List of `obj`:SeismicData): The extracted SeismicData objects containing
            the material properties to check for.
        num_points (int): The number of points to look through in sd_array.

    Returns:
        The level as an integer.
    """
    if num_points == 0:
        raise Exception("Num points is 0.")

    vs_min = sd_array[0].velocity_properties.vs
    min_index = 0

    for i in range(num_points):
        if sd_array[i].velocity_properties.vs < vs_min:
            vs_min = sd_array[i].velocity_properties.vs
            min_index = i

    return int(math.log(
        float(stats["max_length"]) /
        (vs_min / (float(cfg["properties"]["parts_per_wavelength"]) * float(cfg["properties"]["max_frequency"])))
    ) / math.log(2.0) + 1)


def _calculate_etree_stats(information: dict, cols: int, rows: int) -> dict:
    """
    Given the XML description of the desired e-tree as a dictionary as well as the number of columns
    and rows per processor, this function calculates the statistics (e.g. the tick size).

    Args:
        information (dict): The XML description of the desired e-tree as a dictionary.
        cols (int): The number of columns to extract.
        rows (int): The number of rows to extract.

    Returns:
        A dictionary containing the statistics.
    """
    if float(information["dimensions"]["x"]) > float(information["dimensions"]["y"]):
        max_length = float(information["dimensions"]["x"])
    else:
        max_length = float(information["dimensions"]["y"])

    tick_size = max_length / (1 << 31)

    column_ticks = float(information["dimensions"]["x"]) / cols / tick_size
    row_ticks = float(information["dimensions"]["y"]) / rows / tick_size

    min_edgesize = float(information["minimums"]["vs"]) / \
                   (float(information["properties"]["parts_per_wavelength"]) *
                    float(information["properties"]["max_frequency"]))
    max_edgesize = float(information["properties"]["max_octant_size"])
    if max_edgesize > column_ticks * tick_size:
        max_edgesize = column_ticks * tick_size

    max_level = math.ceil(math.log(max_length / min_edgesize) / math.log(2.0))
    min_level = math.ceil(math.log(max_length / max_edgesize) / math.log(2.0))

    max_ticks = {
        "width": (float(information["dimensions"]["x"]) / max_length) * (1 << 31),
        "height": (float(information["dimensions"]["y"]) / max_length) * (1 << 31),
        "depth": (float(information["dimensions"]["z"]) / max_length) * (1 << 31)
    }

    maxrez = 1 << (31 - max_level)
    max_points = int((column_ticks / maxrez) * (row_ticks / maxrez))

    return {
        "max_length": max_length,
        "tick_size": tick_size,
        "min_level": min_level,
        "min_edgesize": min_edgesize,
        "max_level": max_level,
        "max_edgesize": max_edgesize,
        "maxrez": maxrez,
        "max_points": max_points,
        "max_ticks": max_ticks,
        "column_ticks": column_ticks,
        "row_ticks": row_ticks
    }