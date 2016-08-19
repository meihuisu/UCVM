"""
Defines a bare-bones plot. Every specific plot type (e.g. HorizontalSlice) derives from this
Plot class.

:copyright: Southern California Earthquake Center
:author:    David Gill <davidgil@usc.edu>
:created:   August 12, 2016
:modified:  August 16, 2016
"""
import numpy as np
import sys

from ucvm.src.shared.errors import display_and_raise_error

try:
    import matplotlib as mpl
    import matplotlib.pyplot as plt
    import matplotlib.colors as mcolors
    import matplotlib.cm as cm
except ImportError:
    display_and_raise_error(8)
    plt = None                      # Make PyCharm happy.
    mcolors = None                  # Make PyCharm happy.
    cm = None                       # Make PyCharm happy.

try:
    from mpl_toolkits import basemap
except ImportError:
    display_and_raise_error(8)
    basemap = None                  # Make PyCharm happy.


class Plot:

    _defaults = {
        "plot_width": 1000,
        "plot_height": 1000,
        "plot_xlabel": "X axis",
        "plot_ylabel": "Y axis",
        "plot_title": "Plot",
        "plot_cbar_label": "Color Bar Label"
    }

    def __init__(self, **kwargs):
        for key, value in Plot._defaults.items():
            if not hasattr(self, key):
                setattr(self, key, value)

        self.figure = plt.figure(figsize=(self.plot_width / 100, self.plot_height / 100), dpi=100)
        plt.xlabel(self.plot_xlabel, fontsize=14)
        plt.ylabel(self.plot_ylabel, fontsize=14)
        plt.title(self.plot_title)

    def show_profile(self, properties: dict) -> bool:
        """
        Displays the profile to the user.
        :param properties: The properties as a dictionary.
        :return: True if profile shown, false if error.
        """
        fig = self.figure.add_subplot(1, 1, 1)
        for key, item in properties.items():
            fig.plot(item["x_values"], item["y_values"], "-")

        plt.gca().invert_yaxis()
        plt.show()

    def show_plot(self, x_points: np.array, y_points: np.array, data: np.ndarray,
                  map_plot: bool) -> bool:
        """
        Displays the plot to the user.
        :param x_points: X co-ordinates for the data points.
        :param y_points: Y co-ordinates for the data points.
        :param data: The data points as a 1D grid, X-fast.
        :param map_plot: If set to true, the data is plotted on a map. If false, just generic data.
        :return: True, if plot was shown successfully. False if not.
        """

        print(data)

        ranges = {
            "min_lon": np.amin(x_points),
            "max_lon": np.amax(x_points),
            "min_lat": np.amin(y_points),
            "max_lat": np.amax(y_points)
        }

        if map_plot:
            # Check to see if we have pickled this particular basemap instance.
            m = basemap.Basemap(projection='cyl',
                                llcrnrlat=ranges["min_lat"],
                                urcrnrlat=ranges["max_lat"],
                                llcrnrlon=ranges["min_lon"],
                                urcrnrlon=ranges["max_lon"],
                                resolution='f',
                                anchor='C')

            lat_ticks = np.arange(ranges["min_lat"], ranges["max_lat"] + 0.01,
                                  (ranges["max_lat"] - ranges["min_lat"]) / 2)
            lon_ticks = np.arange(ranges["min_lat"], ranges["max_lat"] + 0.01,
                                  (ranges["max_lat"] - ranges["min_lat"]) / 2)

            m.drawparallels(lat_ticks, linewidth=1.0, labels=[1, 0, 0, 0])
            m.drawmeridians(lon_ticks, linewidth=1.0, labels=[0, 0, 0, 1])
            m.drawstates()
            m.drawcountries()
            m.drawcoastlines()

            colormap = cm.RdBu
            norm = mcolors.Normalize(vmin=self.bounds[0], vmax=self.bounds[len(self.bounds) - 1])

            t = m.pcolor(x_points, y_points, data, cmap=colormap, norm=norm)

            print(self.ticks, self.bounds)

            cax = plt.axes([0.125, 0.05, 0.775, 0.02])
            cbar = plt.colorbar(t, cax=cax, orientation='horizontal', ticks=self.ticks)

            cbar.set_label(self.plot_cbar_label)

        plt.show()
