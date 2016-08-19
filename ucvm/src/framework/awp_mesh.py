import os
import sys
import re
import struct
import humanize
import time
import math

from multiprocessing import cpu_count, Pool, JoinableQueue, Queue, current_process, Process

from ucvm.src.framework.ucvm import UCVM
from ucvm.src.shared.constants import UCVM_GRID_TYPE_CENTER, UCVM_GRID_TYPE_VERTEX, \
    UCVM_DEFAULT_PROJECTION, UCVM_DEPTH, UCVM_ELEVATION
from ucvm.src.shared.functions import ask_and_validate, is_number, is_valid_proj4_string, \
    is_acceptable_value
from ucvm.src.shared.properties import Point, SeismicData
from ucvm.src.framework.mesh_common import InternalMesh, InternalMeshIterator

_MPI_QUEUE = []
_MPI_RESULT_QUEUE = Queue()
_MPI_FILE_OUT = None
_MPI_RANK = -1

def ask_questions() -> dict:
    """
    Asks the questions of the user that are necessary to generate the XML file for the mesh.
    :return: A dictionary containing the answers.
    """
    answers = {
        "cvm_list": "",
        "grid_type": UCVM_GRID_TYPE_CENTER,
        "spacing": 0.0,
        "projection": "",
        "initial_point": {
            "x": 0,
            "y": 0,
            "z": 0,
            "rotation": 0,
            "depth_elev": 0,
            "projection": ""
        },
        "dimensions": {
            "x": 0,
            "y": 0,
            "z": 0
        },
        "minimums": {
            "vp": 0,
            "vs": 0,
            "dn": 0,
            "qp": 0,
            "qs": 0
        },
        "out_dir": "",
        "mesh_name": "",
        "mesh_type": "",
        "scratch_dir": ""
    }

    print("\nGenerating a mesh requires the definition of various parameters to be defined (such \n"
          "as the origin of the mesh, the length of the mesh, and so on). The following questions\n"
          "will guide you through the definition of those parameters. At the end, you will be\n"
          "asked if you want to just generate the configuration file to make the mesh at a later\n"
          "time or if you want to generate the mesh immediately.\n")

    answers["cvm_list"] = ask_and_validate("From which velocity model(s) should this mesh be "
                                           "generated:")

    print("\nMeshes are constructed by specifying a bottom-left origin point, a rotation for the\n"
          "rectangular region, and then a width, height, and depth for the box.\n")

    answers["initial_point"]["projection"] = \
        ask_and_validate("To start, in which projection is your starting point specified?\n"
                         "The default for UCVM is WGS84 latitude and longitude. To accept\n"
                         "that projection, simply hit enter:", is_valid_proj4_string,
                         "The answer must be a valid Proj.4 projection.")

    if answers["initial_point"]["projection"].strip() is "":
        answers["initial_point"]["projection"] = UCVM_DEFAULT_PROJECTION

    answers["initial_point"]["x"] = \
        float(ask_and_validate("\nWhat is the X or longitudinal coordinate of your bottom-left\n"
                               "starting point?", is_number, "Answer must be a number."))
    answers["initial_point"]["y"] = \
        float(ask_and_validate("What is the Y or latitudinal coordinate of your bottom-left\n"
                               "starting point?", is_number, "Answer must be a number."))
    answers["initial_point"]["z"] = \
        float(ask_and_validate("What is the Z or depth/elevation coordinate of your bottom-left\n"
                               "starting point?", is_number, "Answer must be a number."))

    answers["initial_point"]["rotation"] = \
        float(ask_and_validate("What is the rotation angle, in degrees, of this box (relative to\n"
                               "the bottom-left corner)?", is_number, "Answer must a number."))

    answers["initial_point"]["depth_elev"] = \
        ask_and_validate("\nIs your Z coordinate specified as depth (default) or elevation?\n"
                         "Type 'd' or enter for depth, 'e' for elevation:", is_acceptable_value,
                         "Type the character 'd' for depth or 'e' for elevation.",
                         allowed=["d", "e", ""])

    if answers["initial_point"]["depth_elev"] is "e":
        answers["initial_point"]["depth_elev"] = "elevation"
    else:
        answers["initial_point"]["depth_elev"] = "depth"

    answers["projection"] = ask_and_validate("\nWhat should your mesh projection be? The default "
                                             "is UTM WGS84 Zone 11.\nThis must be specified as "
                                             "a Proj.4 string.", is_valid_proj4_string,
                                             "The answer must be a valid Proj.4 projection.")

    answers["spacing"] = ask_and_validate("In your projection's co-ordinate system, what should "
                                          "the spacing\nbetween each grid point be?", is_number,
                                          "The input must be a number.")

    answers["dimensions"]["x"] = int(ask_and_validate("\nHow many grid points should there be in "
                                                      "the X or longitudinal direction?", is_number,
                                                      "Answer must be a number."))
    answers["dimensions"]["y"] = int(ask_and_validate("How many grid points should there be in the "
                                                      "Y or latitudinal direction?", is_number,
                                                      "Answer must be a number."))
    answers["dimensions"]["z"] = int(ask_and_validate("How many grid points should there be in the "
                                                      "Z or depth/elevation direction?", is_number,
                                                      "Answer must be a number."))

    answers["minimums"]["vs"] = int(ask_and_validate("\nWhat should the minimum Vs, in meters, be? "
                                                     "The default is 0: ", is_number,
                                                     "Answer must be a number."))
    answers["minimums"]["vp"] = int(ask_and_validate("What should the minimum Vp, in meters, be? "
                                                     "The default is 0: ", is_number,
                                                     "Answer must be a number."))

    answers["out_dir"] = ask_and_validate("\nTo which directory should the mesh and metadata be "
                                          "saved?")
    answers["scratch_dir"] = ask_and_validate("Please provide a scratch directory for "
                                              "temporary storage:")

    answers["mesh_type"] = ask_and_validate("\nWhat type is this mesh (the most common is IJK-12 "
                                            "for AWP-ODC and UCVM for UCVM meshes)?",
                                            is_acceptable_value, "IJK-12, UCVM",
                                            allowed=["IJK-12", "UCVM"])

    answers["mesh_name"] = ask_and_validate("\nPlease provide a name for this mesh:")

    return answers


def mesh_extract_single(information: dict, **kwargs) -> bool:
    """
    Given a dictionary containing the relevant parameters for the extraction, extract the material
    properties for a single process.
    :param information: The dictionary containing the metadata defining the extraction.
    :return: True, when successful. It will raise an error if the extraction is not successful.
    """
    internal_mesh = InternalMesh(information)

    progress = {
        "last": 0,
        "max": InternalMesh.get_max_points_extract()
    }

    sd_array = UCVM.create_max_seismicdata_array(internal_mesh.total_size, 1)

    internal_mesh_iter = InternalMeshIterator(internal_mesh, 0, internal_mesh.total_size,
                                              len(sd_array), sd_array)

    pattern = re.compile('[\W_]+')
    file_out = pattern.sub('', str(information["mesh_name"]).lower()) + ".data"

    print("There are a total of " + humanize.intcomma(internal_mesh.total_size) + " grid points "
          "to extract.\nWe can extract " + humanize.intcomma(progress["max"]) + " points at once.\n"
          "Starting extraction...\n")

    with open(os.path.join(information["out_dir"], file_out), "wb") as fd:
        while progress["last"] < internal_mesh_iter.end_point:
            count = next(internal_mesh_iter)

            progress["last"] += count

            if "custom_model_order" in kwargs:
                UCVM.query(sd_array[0:count], information["cvm_list"], ["velocity"],
                           kwargs["custom_model_order"])
            else:
                UCVM.query(sd_array[0:count], information["cvm_list"], ["velocity"])

            fl_array = []
            for s in sd_array[0:count]:
                fl_array.append(s.velocity_properties.vp)
                fl_array.append(s.velocity_properties.vs)
                fl_array.append(s.velocity_properties.density)

            s = struct.pack('f' * len(fl_array), *fl_array)
            fd.write(s)

            print("%-4.2f" % ((progress["last"] / internal_mesh_iter.end_point) * 100.0) +
                  "% complete. Wrote " + humanize.intcomma(count) + " grid points.")

    print("\nExtraction done.")

    if information["mesh_type"] == "IJK-12":
        print("\nExpected file size is " + internal_mesh.get_grid_file_size()["display"] + ". " +
              "Actual size is " +
              humanize.naturalsize(os.path.getsize(os.path.join(information["out_dir"], file_out)),
                                   gnu=False) + ".")

        if internal_mesh.get_grid_file_size()["real"] == \
                os.path.getsize(os.path.join(information["out_dir"], file_out)):
            print("File sizes match!")
        else:
            print("ERROR! File sizes DO NOT MATCH!")

    return True


def _mesh_extract_mpi_iterator(next_task: dict) -> None:
    """
    Given an internal mesh object and an iterator, this extracts the material properties and
    queues up the data to be written.
    :param i_mesh:
    :param mesh_iterator:
    :return:
    """
    from mpi4py import MPI

    comm = MPI.COMM_WORLD
    rank = comm.Get_rank()

    mesh_iterator = next_task["iterator"]
    next(mesh_iterator)
    i_mesh = next_task["internalmesh"]

    sys.stdout.write(
        "[Node %5d] Thread ID %s extracting %s points." % (rank, current_process().name,
        humanize.intcomma(mesh_iterator.end_point - mesh_iterator.start_point))
    )
    sys.stdout.flush()

    """UCVM.query(mesh_iterator.init_array[0:mesh_iterator.end_point - mesh_iterator.start_point],
               i_mesh.cvm_list, ["velocity"])

    fl_array = []
    for s in mesh_iterator.init_array[0:mesh_iterator.end_point - mesh_iterator.start_point]:
        fl_array.append(s.velocity_properties.vp)
        fl_array.append(s.velocity_properties.vs)
        fl_array.append(s.velocity_properties.density)
    _MPI_RESULT_QUEUE.put({
        "start": mesh_iterator.start_point,
        "data": fl_array
    })"""

    sys.stdout.write(
        "[Node %5d] Thread ID %s done extracting %s points." % (rank, current_process().name,
        humanize.intcomma(mesh_iterator.end_point - mesh_iterator.start_point))
    )
    sys.stdout.flush()


def _mesh_extract_mpi_write_data() -> None:
    from mpi4py import MPI

    comm = MPI.COMM_WORLD
    rank = comm.Get_rank()

    global _MPI_RESULT_QUEUE

    while not _MPI_QUEUE.empty() or not _MPI_RESULT_QUEUE.empty():
        next_task = _MPI_RESULT_QUEUE.get()

        print("[Node %5d] Writing %s points to data file." %
              (rank, humanize.intcomma(int(len(next_task["data"]) / 3))))
        sys.stdout.flush()

        _MPI_FILE_OUT.Write_at(int(next_task["start"] * 12),
                               struct.pack('f' * len(next_task["data"]), *next_task["data"]))

        print("[Node %5d] Finished writing %s points to data file." %
              (rank, humanize.intcomma(int(len(next_task["data"]) / 3))))
        sys.stdout.flush()


def test_multiprocessing_shared_object(item: dict) -> bool:
    print("HERE")
    return True


def mesh_extract_mpi(i_mesh: InternalMesh, max_points_per_cpu: int, start_end: tuple,
                     file_out: str) -> bool:
    """
    Given a dictionary containing the relevant parameters for the extraction, extract the material
    properties for a single process.
    :param i_mesh:
    :param max_points_per_cpu:
    :param start_end:
    :param file_out:
    :return: True, when successful. It will raise an error if the extraction is not successful.
    """
    from mpi4py import MPI

    global _MPI_FILE_OUT, _MPI_QUEUE, _MPI_RESULT_QUEUE
    _MPI_FILE_OUT = MPI.File.Open(MPI.COMM_WORLD, file_out, amode=MPI.MODE_WRONLY | MPI.MODE_CREATE)

    comm = MPI.COMM_WORLD
    rank = comm.Get_rank()

    num_cpus = cpu_count() - 1
    sd_array = []
    current_point = 0
    num_jobs = 0

    print("[Node %5d] Initializing temporary arrays of buffer objects." % rank)
    sys.stdout.flush()

    # We will build a list of iterators.
    for i in range(0, num_cpus):
        sd_array.append([SeismicData() for _ in range(0, max_points_per_cpu)])

    print("[Node %5d] Temporary array completed. Enqueuing jobs." % rank)
    sys.stdout.flush()

    while current_point < start_end[1] - start_end[0]:
        sys.stdout.flush()
        if num_cpus * max_points_per_cpu > start_end[1] - start_end[0] - current_point:
            temp_points_left = start_end[1] - start_end[0] - current_point
            for i in range(0, num_cpus):
                if i < num_cpus - 1:
                    _MPI_QUEUE.append({
                        "internalmesh": i_mesh,
                        "iterator": InternalMeshIterator(
                            i_mesh, current_point,
                            current_point + int(math.floor(temp_points_left / num_cpus)),
                            int(math.floor(temp_points_left / num_cpus)),
                            sd_array[i]
                        )
                    })

                    current_point += int(math.floor(temp_points_left / num_cpus))
                else:
                    _MPI_QUEUE.append({
                        "internalmesh": i_mesh,
                        "iterator": InternalMeshIterator(
                            i_mesh, current_point,
                            int(start_end[1]),
                            int(start_end[1] - current_point),
                            sd_array[i]
                        )
                    })
                    current_point += start_end[1] - current_point
                num_jobs += 1
        else:
            for i in range(0, num_cpus):
                _MPI_QUEUE.append({
                    "internalmesh": i_mesh,
                    "iterator": InternalMeshIterator(
                        i_mesh, current_point, max_points_per_cpu, max_points_per_cpu, sd_array[i]
                    )
                })
                current_point += max_points_per_cpu
                num_jobs += 1

    print("[Node %5d] %d jobs enqueued. Initializing threads and starting extraction." %
          (rank, num_jobs))
    sys.stdout.flush()

    # Initialize the models.
    UCVM.query([], i_mesh.cvm_list, ["velocity"])

    p = Pool()
    p.map(test_multiprocessing_shared_object, _MPI_QUEUE)
    p.close()
    p.join()

    #_mesh_extract_mpi_write_data()

    #_MPI_QUEUE.join()

    print("[Node %5d] MPI extraction complete." % rank)
    sys.stdout.flush()

    if rank == 0:
        if i_mesh.mesh_type == "IJK-12":
            print("\nExpected file size is " + i_mesh.get_grid_file_size()["display"] + ". " +
                  "Actual size is " + humanize.naturalsize(os.path.getsize(file_out), gnu=False) +
                  ".")

            if i_mesh.get_grid_file_size()["real"] == \
                    os.path.getsize(file_out):
                print("File sizes match!")
            else:
                print("ERROR! File sizes DO NOT MATCH!")
