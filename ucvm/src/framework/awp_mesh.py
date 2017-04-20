"""
AWP mesh generation functions.

This file contains all the common functions necessary to generate an AWP-compatible mesh through
UCVM. It handles gridding, projections, etc.

Copyright 2017 Southern California Earthquake Center

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
"""
# Python Imports
import os
import sys
import struct
import math
import time
from multiprocessing import current_process
from typing import List

# Package Imports
import humanize
import xmltodict
import pyproj

# UCVM Imports
from ucvm.src.framework.ucvm import UCVM
from ucvm.src.shared.constants import UCVM_DEFAULT_PROJECTION
from ucvm.src.shared.functions import ask_and_validate, is_number, is_valid_proj4_string, \
    is_acceptable_value, get_utm_zone_for_lon
from ucvm.src.shared.properties import SeismicData, VelocityProperties
from ucvm.src.framework.mesh_common import InternalMesh, AWPInternalMeshIterator, \
    RWGInternalMeshIterator


def mesh_extract_mpi(information: dict, start_end: tuple) -> bool:
    """
    Extracts the parameters for the MPI process.

    Args:
        information (dict): The dictionary containing the metadata defining the extraction.
        start_end (tuple): The tuple defining the start and end poing.

    Returns:
        True, when successful. Raises an error if the extraction fails.
    """
    from mpi4py import MPI

    comm = MPI.COMM_WORLD
    rank = comm.Get_rank()

    internal_mesh = InternalMesh(information)
    max_pts = 250000

    sd_array = [SeismicData() for _ in range(0, max_pts)]

    print(
        "[Node %d] Responsible for extracting %d grid points. We can extract %d at once.\nStarting extraction..." % (
            rank, start_end[1] - start_end[0], max_pts
        ), flush=True
    )

    information["minimums"]["vp"] = float(information["minimums"]["vp"])
    information["minimums"]["vs"] = float(information["minimums"]["vs"])

    if information["format"] == "rwg":
        _mesh_extract_mpi_rwg(sd_array, information, internal_mesh, start_end)
    elif information["format"] == "awp":
        _mesh_extract_mpi_awp(sd_array, information, internal_mesh, start_end)
    else:
        raise ValueError("Invalid mesh format.")

    print(
        "[Node %d] Extraction is done!" % rank
    )


def _mesh_extract_mpi_awp(sd_array: List[SeismicData], information: dict, im: InternalMesh, start_end: tuple) -> bool:
    """
    Extract an AWP mesh using MPI. Internal method.

    Args:

    Returns:
        True, if successful. Raises an error if not successful.
    """
    from mpi4py import MPI

    comm = MPI.COMM_WORLD
    rank = comm.Get_rank()

    file_out = os.path.join(information["out_dir"], information["mesh_name"]) + ".awp"

    im_iter = AWPInternalMeshIterator(im, start_end[0], start_end[1], len(sd_array), sd_array)

    progress = start_end[0]
    sqrt2 = math.sqrt(2)

    fh = MPI.File.Open(MPI.COMM_WORLD, file_out, amode=MPI.MODE_WRONLY | MPI.MODE_CREATE)

    while progress < im_iter.end_point:
        count = next(im_iter)

        UCVM.query(sd_array[0:count], information["cvm_list"], ["velocity"])

        fl_array = []
        for s in sd_array[0:count]:
            if s.velocity_properties is not None and s.velocity_properties.vs is not None and \
               s.velocity_properties.vs < information["minimums"]["vs"]:
                s.set_velocity_data(
                    VelocityProperties(
                        information["minimums"]["vp"], information["minimums"]["vs"],
                        s.velocity_properties.density, s.velocity_properties.qp,
                        s.velocity_properties.qs, s.velocity_properties.vp_source,
                        s.velocity_properties.vs_source, s.velocity_properties.density_source,
                        s.velocity_properties.qp_source, s.velocity_properties.qs_source
                    )
                )

            fl_array.append(s.velocity_properties.vp)
            fl_array.append(s.velocity_properties.vs)
            fl_array.append(s.velocity_properties.density)

            if s.velocity_properties is None or s.velocity_properties.vp is None or \
               s.velocity_properties.vs is None or s.velocity_properties.density is None:
                print("[Node %d] Attention! %.3f, %.3f, %.3f has no material properties." % (
                    rank, s.original_point.x_value, s.original_point.y_value, s.original_point.z_value
                ), flush=True)
            if s.velocity_properties is not None and s.velocity_properties.vp is not None and \
               s.velocity_properties.vs is not None and s.velocity_properties.vp / s.velocity_properties.vs < sqrt2:
                print("[Node %d] Warning: %.3f, %.3f, %.3f has a Vp/Vs ratio of less than sqrt(2)." % (
                    rank, s.original_point.x_value, s.original_point.y_value, s.original_point.z_value
                ), flush=True)
        s = struct.pack('f' * len(fl_array), *fl_array)
        fh.Write_at(progress * 12, s)
        fh.Sync()

        progress += count

        print("[Node %d] %-4.2f" % (rank, ((progress - start_end[0]) / (start_end[1] - start_end[0])) * 100.0) +
              "% complete. Wrote " + humanize.intcomma(count) + " more grid points.", flush=True)

    comm.Barrier()
    fh.Close()

    if rank == 0:
        print("\n[Node " + str(rank) + "] Extraction job fully complete.")

        # print("\n[Node " + str(rank) + "] Expected file size is " + im.get_grid_file_size()["display"] + ". " +
        #       "Actual size is " + humanize.naturalsize(os.path.getsize(os.path.join(file_out)), gnu=False) + ".",
        #       flush=True)

        # if im.get_grid_file_size()["real"] == os.path.getsize(file_out):
        #     print("Generated file size matches the expected file size.")
        # else:
        #     print("ERROR! File sizes DO NOT MATCH!")

    return True


def _mesh_extract_mpi_rwg(sd_array: List[SeismicData], information: dict, im: InternalMesh, start_end: tuple) -> bool:
    """
    Extract a RWG mesh using MPI. Internal method.

    Args:

    Returns:
        True, if successful. Raises an error if not successful.
    """
    from mpi4py import MPI

    comm = MPI.COMM_WORLD
    rank = comm.Get_rank()

    file_out_vp = os.path.join(information["out_dir"], information["mesh_name"]) + ".rwgvp"
    file_out_vs = os.path.join(information["out_dir"], information["mesh_name"]) + ".rwgvs"
    file_out_dn = os.path.join(information["out_dir"], information["mesh_name"]) + ".rwgdn"

    im_iter = RWGInternalMeshIterator(im, start_end[0], start_end[1], len(sd_array), sd_array)

    progress = start_end[0]
    sqrt2 = math.sqrt(2)

    fh_vp = MPI.File.Open(MPI.COMM_WORLD, file_out_vp, amode=MPI.MODE_WRONLY | MPI.MODE_CREATE)
    fh_vs = MPI.File.Open(MPI.COMM_WORLD, file_out_vs, amode=MPI.MODE_WRONLY | MPI.MODE_CREATE)
    fh_dn = MPI.File.Open(MPI.COMM_WORLD, file_out_dn, amode=MPI.MODE_WRONLY | MPI.MODE_CREATE)

    while progress < im_iter.end_point:
        count = next(im_iter)

        UCVM.query(sd_array[0:count], information["cvm_list"], ["velocity"], None)

        vp_array = []
        vs_array = []
        dn_array = []
        for s in sd_array[0:count]:
            if s.velocity_properties is not None and s.velocity_properties.vs is not None and \
               s.velocity_properties.vs < information["minimums"]["vs"]:
                s.set_velocity_data(
                    VelocityProperties(
                        information["minimums"]["vp"], information["minimums"]["vs"],
                        s.velocity_properties.density, s.velocity_properties.qp,
                        s.velocity_properties.qs, s.velocity_properties.vp_source,
                        s.velocity_properties.vs_source, s.velocity_properties.density_source,
                        s.velocity_properties.qp_source, s.velocity_properties.qs_source
                    )
                )

            vp_array.append(s.velocity_properties.vp / 1000)
            vs_array.append(s.velocity_properties.vs / 1000)
            dn_array.append(s.velocity_properties.density / 1000)

            if s.velocity_properties is None or s.velocity_properties.vp is None or \
               s.velocity_properties.vs is None or s.velocity_properties.density is None:
                print("[Node %d] Attention! %.3f, %.3f, %.3f has no material properties." % (
                    rank, s.original_point.x_value, s.original_point.y_value, s.original_point.z_value
                ), flush=True)
            if s.velocity_properties is not None and s.velocity_properties.vp is not None and \
               s.velocity_properties.vs is not None and s.velocity_properties.vp / s.velocity_properties.vs < sqrt2:
                print("[Node %d] Warning: %.3f, %.3f, %.3f has a Vp/Vs ratio of less than sqrt(2)." % (
                    rank, s.original_point.x_value, s.original_point.y_value, s.original_point.z_value
                ), flush=True)
        s = struct.pack('f' * len(vp_array), *vp_array)
        fh_vp.Write_at(progress * 4, s)
        fh_vp.Sync()
        s = struct.pack('f' * len(vs_array), *vs_array)
        fh_vs.Write_at(progress * 4, s)
        fh_vs.Sync()
        s = struct.pack('f' * len(dn_array), *dn_array)
        fh_dn.Write_at(progress * 4, s)
        fh_dn.Sync()

        progress += count

        print("[Node %d] %-4.2f" % (rank, ((progress - start_end[0]) / (start_end[1] - start_end[0])) * 100.0) +
              "% complete. Wrote " + humanize.intcomma(count) + " more grid points.", flush=True)

    comm.Barrier()

    fh_vp.Close()
    fh_vs.Close()
    fh_dn.Close()

    if rank == 0:
        print("\n[Node " + str(rank) + "] Extraction job fully complete.")

        # print("\n[Node " + str(rank) + "] Expected file size is " + im.get_grid_file_size()["display"] + ". " +
        #       "Actual size is " + humanize.naturalsize(os.path.getsize(
        #       os.path.join(information["out_dir"], file_out_vp)), gnu=False) + ".", flush=True)

        # if im.get_grid_file_size()["real"] == \
        #    os.path.getsize(os.path.join(information["out_dir"], file_out_vp)) and \
        #    im.get_grid_file_size()["real"] == \
        #    os.path.getsize(os.path.join(information["out_dir"], file_out_vs)) and \
        #    im.get_grid_file_size()["real"] == \
        #    os.path.getsize(os.path.join(information["out_dir"], file_out_dn)):
        #     print("Generated file size matches the expected file size.")
        # else:
        #     print("ERROR! File sizes DO NOT MATCH!")

    return True


def mesh_extract_single(information: dict, slices: str=None, interval: str=None, **kwargs) -> bool:
    """
    Given a dictionary containing the relevant parameters for the extraction, extract the material
    properties for a single process.

    Args:
        information (dict): The dictionary containing the metadata defining the extraction.

    Returns:
        True, when successful. It will raise an error if the extraction is not successful.
    """
    internal_mesh = InternalMesh(information)

    if slices is not None:
        internal_mesh.do_slices(slices)
    elif interval is not None:
        internal_mesh.do_interval(interval)

    sd_array = UCVM.create_max_seismicdata_array(min(internal_mesh.total_size, 250000), 1)

    print("\nThere are a total of " + humanize.intcomma(internal_mesh.total_size) + " grid points "
          "to extract.\nWe can extract " + humanize.intcomma(len(sd_array)) + " points at once.\n"
          "\nStarting extraction...\n")

    information["minimums"]["vp"] = float(information["minimums"]["vp"])
    information["minimums"]["vs"] = float(information["minimums"]["vs"])

    if internal_mesh.format == "awp":
        _mesh_extract_single_awp(sd_array, information, internal_mesh, slices, interval)
    elif internal_mesh.format == "rwg":
        _mesh_extract_single_rwg(sd_array, information, internal_mesh)

    print("\nExtraction done.")

    return True


def _mesh_extract_single_awp(sd_array: List[SeismicData], information: dict, im: InternalMesh, slices: str=None,
                             interval: str=None) -> bool:
    """
    Takes an InternalMesh object, the mesh information file, and the iterator, and generates, using
    one core only, the mesh in AWP-ODC format.

    Args:
        information (dict): The mesh information dictionary (from the XML config file).
        im (InternalMesh): The internal representation of the AWP mesh.
        im_iter (AWPInternalMeshIterator): The internal mesh iterator that was generated from im.

    Returns:
        Nothing
    """
    file_out = information["mesh_name"] + ".awp"

    start_point = 0
    end_point = im.total_size

    if slices is not None:
        if "-" in slices:
            parts = slices.split("-")
            start_point = (int(parts[0]) - 1) * im.slice_size
            end_point = (int(parts[1])) * im.slice_size
        else:
            start_point = (int(slices) - 1) * im.slice_size
            end_point = int(slices) * im.slice_size
    elif interval is not None:
        if "-" in interval:
            parts = interval.split("-")
            start_point = (int(parts[0]) / 100) * im.full_size
            end_point = (int(parts[1]) / 100) * im.full_size
        else:
            raise ValueError("Interval must be a range (e.g. 0-10 which means generate the first 10% of the mesh).")

    im_iter = AWPInternalMeshIterator(im, start_point, end_point, len(sd_array), sd_array)
    progress = start_point

    progress = 0
    sqrt2 = math.sqrt(2)

    with open(os.path.join(information["out_dir"], file_out), "ab+") as fd:
        while progress < im.total_size:
            count = next(im_iter)

            UCVM.query(sd_array[0:count], information["cvm_list"], ["velocity"])

            fl_array = []
            for s in sd_array[0:count]:
                if s.velocity_properties is not None and s.velocity_properties.vs is not None and \
                   s.velocity_properties.vs < information["minimums"]["vs"]:
                    s.set_velocity_data(
                        VelocityProperties(
                            information["minimums"]["vp"], information["minimums"]["vs"],
                            s.velocity_properties.density, s.velocity_properties.qp,
                            s.velocity_properties.qs, s.velocity_properties.vp_source,
                            s.velocity_properties.vs_source, s.velocity_properties.density_source,
                            s.velocity_properties.qp_source, s.velocity_properties.qs_source
                        )
                    )

                fl_array.append(s.velocity_properties.vp)
                fl_array.append(s.velocity_properties.vs)
                fl_array.append(s.velocity_properties.density)

                if s.velocity_properties is None or s.velocity_properties.vp is None or \
                   s.velocity_properties.vs is None or s.velocity_properties.density is None:
                    print("Attention! %.3f, %.3f, %.3f has no material properties." % (
                        s.original_point.x_value, s.original_point.y_value, s.original_point.z_value
                    ))
                if s.velocity_properties is not None and s.velocity_properties.vp is not None and \
                   s.velocity_properties.vs is not None and s.velocity_properties.vp / s.velocity_properties.vs < sqrt2:
                    print("Warning: %.3f, %.3f, %.3f has a Vp/Vs ratio of less than sqrt(2)." % (
                        s.original_point.x_value, s.original_point.y_value, s.original_point.z_value
                    ))
            s = struct.pack('f' * len(fl_array), *fl_array)
            fd.seek(progress * 12)
            fd.write(s)

            progress += count

            print("%-4.2f" % ((progress / (im_iter.end_point - start_point)) * 100.0) +
                  "% complete. Wrote " + humanize.intcomma(count) + " more grid points.")

        print("\nExpected file size is " + im.get_grid_file_size()["display"] + ". " +
              "Actual size is " + humanize.naturalsize(os.path.getsize(
              os.path.join(information["out_dir"], file_out)), gnu=False) + ".")

        if im.get_grid_file_size()["real"] == \
           os.path.getsize(os.path.join(information["out_dir"], file_out)):
            print("Generated file size matches the expected file size.")
        else:
            print("ERROR! File sizes DO NOT MATCH!")

    return True


def _mesh_extract_single_rwg(sd_array: List[SeismicData], information: dict, im: InternalMesh) -> bool:
    """
    Takes an InternalMesh object, the mesh information file, and the iterator, and generates, using
    one core only, the mesh in RWG format.

    Args:
        information (dict): The mesh information dictionary (from the XML config file).
        im (InternalMesh): The internal representation of the RWG mesh.

    Returns:
        Nothing
    """
    file_out_vp = information["mesh_name"] + ".rwgvp"
    file_out_vs = information["mesh_name"] + ".rwgvs"
    file_out_dn = information["mesh_name"] + ".rwgdn"

    im_iter = RWGInternalMeshIterator(im, 0, im.total_size, len(sd_array), sd_array)

    progress = 0

    with open(os.path.join(information["out_dir"], file_out_vp), "wb") as fd_vp, \
         open(os.path.join(information["out_dir"], file_out_vs), "wb") as fd_vs, \
         open(os.path.join(information["out_dir"], file_out_dn), "wb") as fd_dn:
        while progress < im_iter.end_point:
            count = next(im_iter)
            progress += count

            UCVM.query(sd_array[0:count], information["cvm_list"], ["velocity"], None)

            vp_array = []
            vs_array = []
            dn_array = []
            for s in sd_array[0:count]:
                if s.velocity_properties is not None and s.velocity_properties.vs is not None and \
                   s.velocity_properties.vs < information["minimums"]["vs"]:
                    s.set_velocity_data(
                        VelocityProperties(
                            information["minimums"]["vp"], information["minimums"]["vs"],
                            s.velocity_properties.density, s.velocity_properties.qp,
                            s.velocity_properties.qs, s.velocity_properties.vp_source,
                            s.velocity_properties.vs_source, s.velocity_properties.density_source,
                            s.velocity_properties.qp_source, s.velocity_properties.qs_source
                        )
                    )

                vp_array.append(s.velocity_properties.vp / 1000)
                vs_array.append(s.velocity_properties.vs / 1000)
                dn_array.append(s.velocity_properties.density / 1000)

                if s.velocity_properties.vp is None:
                    print("Attention! %.3f, %.3f, %.3f has no material properties." % (
                        s.original_point.x_value, s.original_point.y_value, s.original_point.z_value
                    ))
                if s.velocity_properties is not None and s.velocity_properties.vp is not None and \
                   s.velocity_properties.vs is not None and s.velocity_properties.vp / s.velocity_properties.vs < 1.45:
                    print("Warning: %.3f, %.3f, %.3f has a Vp/Vs ratio of less than 1.45." % (
                        s.original_point.x_value, s.original_point.y_value, s.original_point.z_value
                    ))
            s = struct.pack('f' * len(vp_array), *vp_array)
            fd_vp.write(s)
            s = struct.pack('f' * len(vs_array), *vs_array)
            fd_vs.write(s)
            s = struct.pack('f' * len(dn_array), *dn_array)
            fd_dn.write(s)

            print("%-4.2f" % ((progress / im_iter.end_point) * 100.0) +
                  "% complete. Wrote " + humanize.intcomma(count) + " more grid points.")

        print("\nExpected file size is " + im.get_grid_file_size()["display"] + ". " +
              "Actual size is " + humanize.naturalsize(os.path.getsize(
              os.path.join(information["out_dir"], file_out_vp)), gnu=False) + ".")

        if im.get_grid_file_size()["real"] == \
           os.path.getsize(os.path.join(information["out_dir"], file_out_vp)) and \
           im.get_grid_file_size()["real"] == \
           os.path.getsize(os.path.join(information["out_dir"], file_out_vs)) and \
           im.get_grid_file_size()["real"] == \
           os.path.getsize(os.path.join(information["out_dir"], file_out_dn)):
            print("Generated file size matches the expected file size.")
        else:
            print("ERROR! File sizes DO NOT MATCH!")

    return True


def ask_questions() -> dict:
    """
    Asks the questions of the user that are necessary to generate the XML file for the mesh.

    Returns:
        A dictionary containing the answers.
    """
    answers = {
        "cvm_list": "",
        "grid_type": "center",
        "spacing": 0.0,
        "projection": "",
        "initial_point": {
            "x": 0,
            "y": 0,
            "z": 0,
            "depth_elev": 0,
            "projection": ""
        },
        "rotation": 0,
        "dimensions": {
            "x": 0,
            "y": 0,
            "z": 0
        },
        "minimums": {
            "vp": 0,
            "vs": 0
        },
        "format": "",
        "out_dir": "",
        "mesh_name": ""
    }

    print("\nGenerating a mesh requires the definition of various parameters to be defined (such \n"
          "as the origin of the mesh, the length of the mesh, and so on). The following questions\n"
          "will guide you through the definition of those parameters. At the end, you will be\n"
          "asked if you want to just generate the configuration file to make the mesh at a later\n"
          "time or if you want to generate the mesh immediately.\n")

    # Ask about CVMs.
    answers["cvm_list"] = ask_and_validate(
        "From which velocity model(s) should this mesh be generated:"
    )

    print("\nMeshes are constructed by specifying a bottom-left origin point, a rotation for the\n"
          "rectangular region, and then a width, height, and depth for the box.\n")

    # Ask about initial point projection.
    answers["initial_point"]["projection"] = \
        ask_and_validate(
            "To start, in which projection is your starting point specified? The default for UCVM\n"
            "is WGS84 latitude and longitude. To accept that projection, simply hit enter:",
            is_valid_proj4_string, "The answer must be a valid Proj.4 projection."
        )

    if answers["initial_point"]["projection"].strip() is "":
        answers["initial_point"]["projection"] = UCVM_DEFAULT_PROJECTION

    answers["initial_point"]["x"] = \
        float(ask_and_validate(
            "\nWhat is the X or longitudinal coordinate of your bottom-left starting point?",
            is_number, "Answer must be a number."
        ))
    answers["initial_point"]["y"] = \
        float(ask_and_validate(
            "What is the Y or latitudinal coordinate of your bottom-left starting point?",
            is_number, "Answer must be a number."
        ))
    answers["initial_point"]["z"] = \
        float(ask_and_validate(
            "What is the Z or depth/elevation coordinate of your bottom-left starting point?",
            is_number, "Answer must be a number."
        ))

    answers["initial_point"]["depth_elev"] = \
        ask_and_validate(
            "\nIs your Z coordinate specified as depth (default) or elevation? Type 'd' or enter "
            "for depth,\n'e' for elevation:", is_acceptable_value,
            "Type the character 'd' for depth or 'e' for elevation.", allowed=["d", "e", ""]
        )

    answers["initial_point"]["depth_elev"] = \
        "elevation" if answers["initial_point"]["depth_elev"] == "e" else "depth"

    # Get the gridding type.
    answers["grid_type"] = ask_and_validate(
        "\nBy default, UCVM queries the center of each grid point to get the material properties\n"
        "(so it is an average of the cell). UCVM can query at each vertex instead. Type 'c' or\n"
        "hit enter to accept the center grid type, or type 'v' or 'vertex' for a point at each\n"
        "corner:", is_acceptable_value,
        "Type 'c' or hit enter for center, 'v' or vertex for vertex.",
        allowed=["c", "center", "v", "vertex", ""]
    )

    if answers["grid_type"] == "" or answers["grid_type"] == "c":
        answers["grid_type"] = "center"
    else:
        answers["grid_type"] = "vertex"

    # Get default UTM zone.
    in_proj = pyproj.Proj(answers["initial_point"]["projection"])
    out_proj = pyproj.Proj(UCVM_DEFAULT_PROJECTION)

    zone_lon, _ = pyproj.transform(
        in_proj, out_proj, answers["initial_point"]["x"], answers["initial_point"]["y"]
    )
    default_zone = str(get_utm_zone_for_lon(zone_lon))

    answers["projection"] = ask_and_validate(
        "\nWhat should your mesh projection be? The default is UTM WGS84 Zone " + default_zone +
        ".\nHit enter to accept this projection or specify your own, as a Proj.4 string:",
        is_valid_proj4_string, "The answer must be a valid Proj.4 projection."
    )

    if answers["projection"] == "":
        answers["projection"] = "+proj=utm +datum=WGS84 +zone=" + default_zone

    answers["rotation"] = \
        float(ask_and_validate(
            "\nWhat is the rotation angle, in degrees, of this box (relative to the bottom-left " +
            "corner)?", is_number, "Answer must a number."
        ))

    answers["spacing"] = ask_and_validate(
        "In your projection's co-ordinate system, what should the spacing between each grid "
        "\npoint be?", is_number, "The input must be a number.")

    answers["dimensions"]["x"] = \
        int(ask_and_validate(
            "\nHow many grid points should there be in the X or longitudinal direction?",
            is_number, "Answer must be a number."
        ))
    answers["dimensions"]["y"] = \
        int(ask_and_validate(
            "How many grid points should there be in the Y or latitudinal direction?",
            is_number, "Answer must be a number."
        ))
    answers["dimensions"]["z"] = \
        int(ask_and_validate(
            "How many grid points should there be in the Z or depth/elevation direction?",
            is_number, "Answer must be a number."
        ))

    answers["minimums"]["vs"] = \
        int(ask_and_validate(
            "\nWhat should the minimum Vs, in meters, be? The default is 0: ",
            is_number, "Answer must be a number."
        ))
    answers["minimums"]["vp"] = \
        int(ask_and_validate(
            "What should the minimum Vp, in meters, be? The default is 0: ",
            is_number, "Answer must be a number."
        ))

    answers["format"] = ask_and_validate(
        "\nIn which format would you like this mesh? Type 'awp' for AWP-ODC, 'rwg' for a Graves'\n"
        "format mesh:", is_acceptable_value, allowed=["awp", "rwg"]
    )
    answers["out_dir"] = ask_and_validate("To which directory should the mesh and metadata be "
                                          "saved?")
    answers["mesh_name"] = ask_and_validate("Please provide a name for this mesh:")

    # Calculate the four corners.
    corner_ll = [answers["initial_point"]["x"], answers["initial_point"]["y"]]
    p1 = pyproj.Proj(answers["initial_point"]["projection"])
    p2 = pyproj.Proj(answers["projection"])

    corner_origin = pyproj.transform(p1, p2, corner_ll[0], corner_ll[1])

    sin_angle = math.sin(math.radians(answers["rotation"]))
    cos_angle = math.cos(math.radians(answers["rotation"]))

    add_x = 0
    add_y = int(answers["dimensions"]["y"]) * int(answers["spacing"])
    corner_ul = [corner_origin[0] + (add_x * cos_angle - add_y * sin_angle),
                 corner_origin[1] + (add_y * cos_angle + add_x * sin_angle)]
    corner_ul = pyproj.transform(p2, p1, corner_ul[0], corner_ul[1])

    add_x = int(answers["dimensions"]["x"]) * int(answers["spacing"])
    add_y = int(answers["dimensions"]["y"]) * int(answers["spacing"])
    corner_ur = [corner_origin[0] + (add_x * cos_angle - add_y * sin_angle),
                 corner_origin[1] + (add_y * cos_angle + add_x * sin_angle)]
    corner_ur = pyproj.transform(p2, p1, corner_ur[0], corner_ur[1])

    add_x = int(answers["dimensions"]["x"]) * int(answers["spacing"])
    add_y = 0
    corner_lr = [corner_origin[0] + (add_x * cos_angle - add_y * sin_angle),
                 corner_origin[1] + (add_y * cos_angle + add_x * sin_angle)]
    corner_lr = pyproj.transform(p2, p1, corner_lr[0], corner_lr[1])

    # Output the summary.
    print(
        "Mesh configuration complete! The mesh that you have specified is as follows:\n\n"
        "Name:               " + answers["mesh_name"] + "\n"
        "Format:             " + answers["format"] + "\n"
        "Output directory:   " + answers["out_dir"] + "\n"
        "Bottom-left point:  " + str(answers["initial_point"]["x"]) + ", " +
        str(answers["initial_point"]["y"]) + " at " + str(answers["initial_point"]["z"]) + " " +
        answers["initial_point"]["depth_elev"] + " in " +
        answers["initial_point"]["projection"] + "\n"
        "Mesh projection:    " + answers["projection"] + "\n"
        "Dimensions:\n"
        "    Spacing: " + str(answers["spacing"]) + "\n"
        "    Width:   " + str(answers["dimensions"]["x"]) +
        " (" + str(int(int(answers["dimensions"]["x"]) * int(answers["spacing"]))) + "m)\n"
        "    Height:  " + str(answers["dimensions"]["y"]) +
        " (" + str(int(int(answers["dimensions"]["y"]) * int(answers["spacing"]))) + "m)\n"
        "    Depth:   " + str(answers["dimensions"]["z"]) +
        " (" + str(int(int(answers["dimensions"]["z"]) * int(answers["spacing"]))) + "m)\n"
        "Rotation angle:     " + str(answers["rotation"]) + "\n"
        "Grid type:          " + answers["grid_type"] + "\n"
        "The four corners of the box will be: \n"
        "    Bottom-left corner:  %.5f, %.5f" % (corner_ll[0], corner_ll[1]) + "\n"
        "    Top-left corner:     %.5f, %.5f" % (corner_ul[0], corner_ul[1]) + "\n"
        "    Top-right corner:    %.5f, %.5f" % (corner_ur[0], corner_ur[1]) + "\n"
        "    Bottom-right corner: %.5f, %.5f" % (corner_lr[0], corner_lr[1]) + "\n"
        "Finally, the material properties in this mesh will be no lower than:\n"
        "    Vp: " + str(answers["minimums"]["vp"]) + "\n"
        "    Vs: " + str(answers["minimums"]["vs"])
    )

    filename = answers["mesh_name"] + ".xml"
    kml_filename = answers["mesh_name"] + ".kml"

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
                "name": answers["mesh_name"] + ".kml",
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
                        "name": answers["mesh_name"] + " Bottom Line",
                        "styleURL": "#m_ylw-pushpin0",
                        "LineString": {
                            "tessellate": 0,
                            "coordinates": str(corner_ll[0]) + "," + str(corner_ll[1]) + ",0 "  +
                                           str(corner_lr[0]) + "," + str(corner_lr[1]) + ",0"
                        }
                    },
                    {
                        "name": answers["mesh_name"] + " Left Line",
                        "styleURL": "#m_ylw-pushpin0",
                        "LineString": {
                            "tessellate": 0,
                            "coordinates": str(corner_ll[0]) + "," + str(corner_ll[1]) + ",0 " +
                                           str(corner_ul[0]) + "," + str(corner_ul[1]) + ",0"
                        }
                    },
                    {
                        "name": answers["mesh_name"] + " Top Line",
                        "styleURL": "#m_ylw-pushpin0",
                        "LineString": {
                            "tessellate": 0,
                            "coordinates": str(corner_ul[0]) + "," + str(corner_ul[1]) + ",0 " +
                                           str(corner_ur[0]) + "," + str(corner_ur[1]) + ",0"
                        }
                    },
                    {
                        "name": answers["mesh_name"] + " Right Line",
                        "styleURL": "#m_ylw-pushpin0",
                        "LineString": {
                            "tessellate": 0,
                            "coordinates": str(corner_ur[0]) + "," + str(corner_ur[1]) + ",0 " +
                                           str(corner_lr[0]) + "," + str(corner_lr[1]) + ",0"
                        }
                    }
                ]
            }
        }
    }

    with open(os.path.join(".", kml_filename), "w") as fd:
        fd.write(xmltodict.unparse(kml_xml, pretty=True))

    print(
        "\nYour mesh metadata file has been saved to " + filename + ".\n"
        "A KML file showing the four corners and bounding region of the mesh has been saved to\n" +
        kml_filename + ". You can use the XML file to generate and regenerate the mesh, and the\n" +
        "KML file to visualize the boundaries."
    )

    return answers
