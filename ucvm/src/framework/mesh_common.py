"""
Provides all the common data structures and functions for all meshing programs (both AWP and
Hercules) within UCVM.

:copyright: Southern California Earthquake Center
:author:    David Gill <davidgil@usc.edu>
:created:   July 29, 2016
:modified:  August 10, 2016
"""
import math
import humanize
import psutil
import xmltodict

from typing import List

from ucvm.src.shared.properties import Point, SeismicData
from ucvm.src.shared import UCVM_DEPTH, UCVM_ELEVATION

SEISMICDATA_SIZE = 2048     #: int: For now, let's assume that SeismicData is at least 2KB.
MAX_PERCENT_FREE = 0.33     #: float: Only use 1/3rd of available memory for SeismicData objects.


class InternalMesh(object):

    def __init__(self, mesh_info: dict):
        """
        Returns an InternalMesh object which is an internal representation of what we want our
        mesh to be (i.e. all the grid points we need to query).
        :param mesh_info: The mesh information as a dictionary.
        """
        if mesh_info is None:
            return

        origin = Point(mesh_info["initial_point"]["x"],
                       mesh_info["initial_point"]["y"],
                       mesh_info["initial_point"]["z"],
                       UCVM_ELEVATION if str(mesh_info["initial_point"]["depth_elev"]).\
                       strip().lower() == "elevation" else UCVM_DEPTH,
                       {},
                       mesh_info["initial_point"]["projection"])

        self.origin = origin.convert_to_projection(mesh_info["projection"])
        self.rotation = float(mesh_info["rotation"])
        self.num_x = int(mesh_info["dimensions"]["x"])
        self.num_y = int(mesh_info["dimensions"]["y"])
        self.num_z = int(mesh_info["dimensions"]["z"])
        self.spacing = float(mesh_info["spacing"])
        self.mesh_type = mesh_info["mesh_type"]
        self.cvm_list = mesh_info["cvm_list"]
        self.out_dir = mesh_info["out_dir"]

        self.slice_size = self.num_x * self.num_y
        self.total_size = self.slice_size * self.num_z

        # Pre-compute this for efficiency.
        self.sin_angle = math.sin(math.radians(self.rotation))
        self.cos_angle = math.cos(math.radians(self.rotation))

    @classmethod
    def from_xml_file(cls, file: str):
        with open(file, "r") as fd:
            mesh_information = xmltodict.parse(fd.read())["root"]
        return InternalMesh(mesh_information)

    @classmethod
    def from_parameters(cls, origin: Point, grid_info: dict, cvm_list: str, out_dir: str):
        im_instance = cls(None)

        im_instance.origin = origin
        im_instance.rotation = float(grid_info["rotation"])
        im_instance.num_x = int(grid_info["num_x"])
        im_instance.num_y = int(grid_info["num_y"])
        im_instance.num_z = int(grid_info["num_z"])
        im_instance.spacing = float(grid_info["spacing"])
        im_instance.cvm_list = cvm_list
        im_instance.out_dir = out_dir

        im_instance.slice_size = im_instance.num_x * im_instance.num_y
        im_instance.total_size = im_instance.slice_size * im_instance.num_z

        # Pre-compute this for efficiency.
        im_instance.sin_angle = math.sin(math.radians(im_instance.rotation))
        im_instance.cos_angle = math.cos(math.radians(im_instance.rotation))

        return im_instance

    def get_grid_file_size(self) -> dict:
        if self.mesh_type == "IJK-12":
            return {
                "display": humanize.naturalsize(self.total_size * 12, gnu=False),
                "real": self.total_size * 12
            }
        else:
            return {}

    @staticmethod
    def get_max_points_extract(num_processes: int=1) -> int:
        return math.floor((psutil.virtual_memory().free * MAX_PERCENT_FREE) / SEISMICDATA_SIZE /
                          num_processes)


class InternalMeshIterator:

    def __init__(self, im: InternalMesh, start_point: int, end_point: int, num_at_a_time: int,
                 init_array: List[SeismicData]):
        self.internal_mesh = im
        self.current_point = start_point
        self.start_point = start_point
        self.end_point = end_point
        self.num_at_a_time = num_at_a_time
        self.init_array = init_array

    def __iter__(self):
        return self

    def __next__(self) -> int:
        internal_counter = 0

        if self.current_point >= self.end_point:
            raise StopIteration()

        while internal_counter < self.num_at_a_time and self.current_point < self.end_point:
            # Get our X, Y, and Z coordinates.
            z_val = math.floor(self.current_point / self.internal_mesh.slice_size)
            y_val = math.floor((self.current_point - z_val * self.internal_mesh.slice_size) /
                               self.internal_mesh.num_x)
            x_val = self.current_point - z_val * self.internal_mesh.slice_size - \
                    y_val * self.internal_mesh.num_x

            x_point = self.internal_mesh.origin.x_value + (
                self.internal_mesh.cos_angle * x_val - self.internal_mesh.sin_angle * y_val
            ) * self.internal_mesh.spacing
            y_point = self.internal_mesh.origin.y_value + (
                self.internal_mesh.sin_angle * x_val + self.internal_mesh.cos_angle * y_val
            ) * self.internal_mesh.spacing
            z_point = z_val * self.internal_mesh.spacing

            self.init_array[internal_counter].original_point.z_value = z_point
            self.init_array[internal_counter].original_point.y_value = y_point
            self.init_array[internal_counter].original_point.x_value = x_point
            self.init_array[internal_counter].converted_point = \
                self.init_array[internal_counter].original_point

            internal_counter += 1
            self.current_point += 1

        return internal_counter
