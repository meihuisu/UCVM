"""
Handles generating a difference slice or cross-section for UCVM.

Copyright:
    Southern California Earthquake Center

Author:
    David Gill <davidgil@usc.edu>
"""
# Python Imports
import math

# Package Imports
import numpy as np

# UCVM Imports
from ucvm.src.framework.ucvm import UCVM
from ucvm.src.shared.constants import UCVM_DEFAULT_PROJECTION
from ucvm.src.visualization.plot import Plot, plt
from ucvm.src.visualization.horizontal_slice import HorizontalSlice
from ucvm.src.framework.mesh_common import InternalMesh, AWPInternalMeshIterator


class Difference(Plot):

    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.dataset1 = None
        self.dataset2 = None
        self.extracted_data = None
        self.bounds = []
        self.ticks = []
        self.plot_cbar_label = ""

    @classmethod
    def between_two_horizontal_slices(cls, h1: HorizontalSlice, h2: HorizontalSlice):
        d = Difference()

        d.extracted_data = np.zeros(len(h1.extracted_data))

        for i in range(len(d.extracted_data)):
            if not math.isnan(h2.extracted_data[i]) and not math.isnan(h1.extracted_data[i]):
                d.extracted_data[i] = (h1.extracted_data[i] / h2.extracted_data[i] - 1) * 100
            else:
                d.extracted_data[i] = float("NaN")

        d.dataset1 = h1
        d.dataset2 = h2

        return d

    def plot(self, prop: str="vp", basic: bool=False):

        init_array = UCVM.create_max_seismicdata_array(self.dataset1.slice_properties.num_x *
                                                       self.dataset1.slice_properties.num_y, 1)

        lons = np.zeros(self.dataset1.slice_properties.num_x * self.dataset1.slice_properties.num_y,
                        dtype=float).reshape(self.dataset1.slice_properties.num_y,
                                             self.dataset1.slice_properties.num_x)
        lats = np.zeros(self.dataset1.slice_properties.num_x * self.dataset1.slice_properties.num_y,
                        dtype=float).reshape(self.dataset1.slice_properties.num_y,
                                             self.dataset1.slice_properties.num_x)
        data = np.zeros(self.dataset1.slice_properties.num_x * self.dataset1.slice_properties.num_y,
                        dtype=float).reshape(self.dataset1.slice_properties.num_y,
                                             self.dataset1.slice_properties.num_x)

        self.bounds = [-20, -15, -10, -5, 0, 5, 10, 15, 20]
        self.ticks = self.bounds
        self.plot_cbar_label = "Percentage difference."

        if str(prop).lower().strip() == "vp":
            position = 0
        elif str(prop).lower().strip() == "vs":
            position = 1
        elif str(prop).lower().strip() == "density":
            position = 2
        elif str(prop).lower().strip() == "qp":
            position = 3
        elif str(prop).lower().strip() == "qs":
            position = 4
        elif str(prop).lower().strip() == "elevation":
            position = 0
        elif str(prop).lower().strip() == "vs30":
            position = 0
        else:
            position = 0

        im = InternalMesh.from_parameters(
            self.dataset1.origin,
            {
                "num_x": self.dataset1.slice_properties.num_x,
                "num_y": self.dataset1.slice_properties.num_y,
                "num_z": 1,
                "rotation": self.dataset1.slice_properties.rotation,
                "spacing": self.dataset1.slice_properties.spacing,
                "projection": self.dataset1.origin.projection
            },
            self.dataset1.cvms,
            ""
        )
        im_iterator = AWPInternalMeshIterator(im, 0, im.total_size, len(init_array), init_array)

        num_queried = next(im_iterator)
        i = 0
        j = 0
        while num_queried > 0:
            for datum in init_array[0:num_queried]:
                new_pt = datum.original_point.convert_to_projection(UCVM_DEFAULT_PROJECTION)
                lons[j][i] = new_pt.x_value
                lats[j][i] = new_pt.y_value

                data[j][i] = self.extracted_data[(j * self.dataset1.slice_properties.num_x + i) * 6 + position]

                i += 1
                if i == self.dataset1.slice_properties.num_x:
                    i = 0
                    j += 1

            try:
                num_queried = next(im_iterator)
            except StopIteration:
                break

        # Ok, now that we have the 2D array of lons, lats, and data, let's call on our inherited
        # classes show_plot function to actually show the plot.
        return super(Difference, self).show_plot(lons, lats, data, True, basic=basic)

    def plot_histogram(self, prop: str="vp", basic: bool=False) -> tuple:
        data = []

        if str(prop).lower().strip() == "vp":
            position = 0
        elif str(prop).lower().strip() == "vs":
            position = 1
        elif str(prop).lower().strip() == "density":
            position = 2
        elif str(prop).lower().strip() == "qp":
            position = 3
        elif str(prop).lower().strip() == "qs":
            position = 4
        elif str(prop).lower().strip() == "elevation":
            position = 0
        elif str(prop).lower().strip() == "vs30":
            position = 0
        else:
            position = 0

        not_zero_counter = 0

        for i in range(self.dataset1.slice_properties.num_x * self.dataset1.slice_properties.num_y):
            if not math.isnan(self.extracted_data[i * 6 + position]):
                if self.extracted_data[i * 6 + position] != 0:
                    data.append(self.extracted_data[i * 6 + position])
                    not_zero_counter += 1

        plt.hist(np.clip(data, -100, 100),
                 bins=[-110, -100, -90, -80, -70, -60, -50, -40, -30, -20, -10, 0, 10, 20, 30, 40, 50, 60,
                       70, 80, 90, 100, 110], facecolor="green")
        plt.grid(True)
        plt.xticks([-100, -50, 0, 50, 100], ["< -100", "-50", "0", "50", "100 >"])
        plt.xlim([-110, 110])

        return not_zero_counter, self.dataset1.slice_properties.num_x * self.dataset1.slice_properties.num_y
