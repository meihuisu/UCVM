"""
Provides all the common data structures and functions for all meshing capabilities (both AWP and
and RWG) within UCVM.

Copyright:
    Southern California Earthquake Center

Developer:
    David Gill <davidgil@usc.edu>
"""
# Python Imports
import math
from typing import List

# Package Imports
import humanize
import psutil
import pyproj
import xmltodict

# UCVM Imports
from ucvm.src.shared.properties import Point, SeismicData
from ucvm.src.shared import UCVM_DEPTH, UCVM_ELEVATION, UCVM_DEFAULT_PROJECTION

SEISMICDATA_SIZE = 2048     #: int: For now, let's assume that SeismicData is at least 2KB.
MAX_PERCENT_FREE = 0.33     #: float: Only use 1/3rd of available memory for SeismicData objects.


class InternalMesh(object):

    def __init__(self, mesh_info: dict):
        """
        Returns an InternalMesh object which is an internal representation of what we want our
        mesh to be (i.e. all the grid points we need to query).

        Args:
            mesh_info (dict): The mesh info parsed from the XML file.
        """
        if mesh_info is None:
            return

        try:
            depth_elev = int(mesh_info["initial_point"]["depth_elev"])
        except ValueError:
            depth_elev = UCVM_ELEVATION if str(mesh_info["initial_point"]["depth_elev"]).\
                         strip().lower() == "elevation" else UCVM_DEPTH

        origin = Point(mesh_info["initial_point"]["x"],
                       mesh_info["initial_point"]["y"],
                       mesh_info["initial_point"]["z"],
                       depth_elev,
                       {},
                       mesh_info["initial_point"]["projection"])

        self.origin = origin.convert_to_projection(mesh_info["projection"])
        self.rotation = float(mesh_info["rotation"]) * -1
        self.num_x = int(mesh_info["dimensions"]["x"])
        self.num_y = int(mesh_info["dimensions"]["y"])
        self.num_z = int(mesh_info["dimensions"]["z"])
        self.spacing = float(mesh_info["spacing"])
        self.grid_type = mesh_info["grid_type"]
        self.format = mesh_info["format"]
        self.cvm_list = mesh_info["cvm_list"]
        self.out_dir = mesh_info["out_dir"]
        self.projection = mesh_info["projection"]

        if self.format == "awp":
            self.slice_size = self.num_x * self.num_y
            self.total_size = self.slice_size * self.num_z
        elif self.format == "rwg":
            self.slice_size = self.num_x * self.num_z
            self.total_size = self.slice_size * self.num_y

        # Pre-compute this for efficiency.
        self.sin_angle = math.sin(math.radians(self.rotation))
        self.cos_angle = math.cos(math.radians(self.rotation))

        self.full_size = None
        self.start_point = 0
        self.end_point = self.total_size

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
        im_instance.projection = grid_info["projection"]
        im_instance.cvm_list = cvm_list
        im_instance.out_dir = out_dir
        im_instance.grid_type = "vertex"

        im_instance.slice_size = im_instance.num_x * im_instance.num_y
        im_instance.total_size = im_instance.slice_size * im_instance.num_z

        # Pre-compute this for efficiency.
        im_instance.sin_angle = math.sin(math.radians(im_instance.rotation))
        im_instance.cos_angle = math.cos(math.radians(im_instance.rotation))

        return im_instance

    def do_slices(self, slices: str) -> bool:
        self.full_size = self.total_size
        if "-" in slices:
            parts = slices.split("-")
            self.start_point = (int(parts[0]) - 1) * self.slice_size
            self.end_point = int(parts[1]) * self.slice_size
            self.total_size = (int(parts[1]) - int(parts[0]) + 1) * self.slice_size
        else:
            self.start_point = (int(slices) - 1) * self.slice_size
            self.end_point = int(slices) * self.slice_size
            self.total_size = self.slice_size
        return True

    def do_interval(self, interval: str):
        self.full_size = self.total_size
        if "-" in interval:
            parts = interval.split("-")
            self.start_point = int(float(parts[0]) / 100 * self.total_size)
            self.end_point = int(float(parts[1]) / 100 * self.total_size)
            self.total_size = int(((float(parts[1]) - float(parts[0])) / 100) * self.total_size)
        else:
            raise ValueError("Interval must be a range (e.g. 0-10 which means generate the first 10% of the mesh).")
        return True

    def get_grid_file_size(self) -> dict:
        if self.format == "awp":
            return {
                "display": humanize.naturalsize(self.end_point * 12, gnu=False),
                "real": self.end_point * 12
            }
        if self.format == "rwg":
            return {
                "display": humanize.naturalsize(self.end_point * 4, gnu=False),
                "real": self.end_point * 4
            }

    @staticmethod
    def get_max_points_extract(num_processes: int=1) -> int:
        return math.floor((psutil.virtual_memory().free * MAX_PERCENT_FREE) / SEISMICDATA_SIZE /
                          num_processes)


class AWPInternalMeshIterator:

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

        convert_array_x = []
        convert_array_y = []

        while internal_counter < self.num_at_a_time and self.current_point < self.end_point:
            # Get our X, Y, and Z coordinates.
            z_val = math.floor(self.current_point / self.internal_mesh.slice_size)
            y_val = math.floor((self.current_point - z_val * self.internal_mesh.slice_size) /
                               self.internal_mesh.num_x)
            x_val = self.current_point - z_val * self.internal_mesh.slice_size - \
                    y_val * self.internal_mesh.num_x

            if self.internal_mesh.grid_type == "center":
                x_val += 0.5
                y_val += 0.5

            x_point = self.internal_mesh.origin.x_value + (
                self.internal_mesh.cos_angle * x_val - self.internal_mesh.sin_angle * y_val
            ) * self.internal_mesh.spacing
            y_point = self.internal_mesh.origin.y_value + (
                self.internal_mesh.sin_angle * x_val + self.internal_mesh.cos_angle * y_val
            ) * self.internal_mesh.spacing
            z_point = z_val * self.internal_mesh.spacing

            self.init_array[internal_counter].original_point.depth_elev = \
                self.internal_mesh.origin.depth_elev
            self.init_array[internal_counter].original_point.z_value = z_point
            convert_array_x.append(x_point)
            convert_array_y.append(y_point)
            self.init_array[internal_counter].original_point.projection = \
                self.internal_mesh.projection

            internal_counter += 1
            self.current_point += 1

        if self.internal_mesh.projection in Point.loaded_projections:
            in_proj = Point.loaded_projections[self.internal_mesh.projection]
        else:
            in_proj = pyproj.Proj(self.internal_mesh.projection)
            Point.loaded_projections[self.internal_mesh.projection] = in_proj

        if UCVM_DEFAULT_PROJECTION in Point.loaded_projections:
            out_proj = Point.loaded_projections[UCVM_DEFAULT_PROJECTION]
        else:
            out_proj = pyproj.Proj(UCVM_DEFAULT_PROJECTION)
            Point.loaded_projections[UCVM_DEFAULT_PROJECTION] = out_proj

        x_new, y_new = pyproj.transform(in_proj, out_proj, convert_array_x, convert_array_y)

        for i in range(len(x_new)):
            self.init_array[i].original_point.x_value = x_new[i]
            self.init_array[i].original_point.y_value = y_new[i]
            self.init_array[i].original_point.projection = UCVM_DEFAULT_PROJECTION

        return internal_counter


class RWGInternalMeshIterator:

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

        convert_array_x = []
        convert_array_y = []

        while internal_counter < self.num_at_a_time and self.current_point < self.end_point:
            # Get our X, Y, and Z coordinates.
            y_val = math.floor(self.current_point / self.internal_mesh.slice_size)
            z_val = math.floor((self.current_point - y_val * self.internal_mesh.slice_size) /
                               self.internal_mesh.num_x)
            x_val = self.current_point - y_val * self.internal_mesh.slice_size - \
                    z_val * self.internal_mesh.num_x

            if self.internal_mesh.grid_type == "center":
                x_val += 0.5
                y_val += 0.5

            x_point = self.internal_mesh.origin.x_value + (
                self.internal_mesh.cos_angle * x_val - self.internal_mesh.sin_angle * y_val
            ) * self.internal_mesh.spacing
            y_point = self.internal_mesh.origin.y_value + (
                self.internal_mesh.sin_angle * x_val + self.internal_mesh.cos_angle * y_val
            ) * self.internal_mesh.spacing
            z_point = z_val * self.internal_mesh.spacing

            self.init_array[internal_counter].original_point.depth_elev = \
                self.internal_mesh.origin.depth_elev
            self.init_array[internal_counter].original_point.z_value = z_point
            convert_array_x.append(x_point)
            convert_array_y.append(y_point)
            self.init_array[internal_counter].original_point.projection = \
                self.internal_mesh.projection

            internal_counter += 1
            self.current_point += 1

        if self.internal_mesh.projection in Point.loaded_projections:
            in_proj = Point.loaded_projections[self.internal_mesh.projection]
        else:
            in_proj = pyproj.Proj(self.internal_mesh.projection)
            Point.loaded_projections[self.internal_mesh.projection] = in_proj

        if UCVM_DEFAULT_PROJECTION in Point.loaded_projections:
            out_proj = Point.loaded_projections[UCVM_DEFAULT_PROJECTION]
        else:
            out_proj = pyproj.Proj(UCVM_DEFAULT_PROJECTION)
            Point.loaded_projections[UCVM_DEFAULT_PROJECTION] = out_proj

        x_new, y_new = pyproj.transform(in_proj, out_proj, convert_array_x, convert_array_y)

        for i in range(len(x_new)):
            self.init_array[i].original_point.x_value = x_new[i]
            self.init_array[i].original_point.y_value = y_new[i]
            self.init_array[i].original_point.projection = UCVM_DEFAULT_PROJECTION

        return internal_counter
