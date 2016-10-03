"""
Defines a bare-bones plot. Every specific plot type (e.g. HorizontalSlice) derives from this
Plot class.

:copyright: Southern California Earthquake Center
:author:    David Gill <davidgil@usc.edu>
:created:   August 12, 2016
:modified:  August 16, 2016
"""
import numpy as np

from ucvm.src.shared.errors import display_and_raise_error
from ucvm.src.model.fault import Fault

try:
    import matplotlib as mpl
    import matplotlib.pyplot as plt
    import matplotlib.colors as mcolors
    import matplotlib.cm as cm
    from matplotlib.colors import LightSource
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
        plt.xlabel(self.plot_xlabel if isinstance(self.plot_xlabel, str) else "", fontsize=14)
        plt.ylabel(self.plot_ylabel if isinstance(self.plot_ylabel, str) else "", fontsize=14)
        plt.title(self.plot_title if isinstance(self.plot_title, str) else "")

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
                  map_plot: bool, **kwargs) -> bool:
        """
        Displays the plot to the user.
        :param x_points: X co-ordinates for the data points.
        :param y_points: Y co-ordinates for the data points.
        :param data: The data points as a 1D grid, X-fast.
        :param map_plot: If set to true, the data is plotted on a map. If false, just generic data.
        :return: True, if plot was shown successfully. False if not.
        """
        ranges = {
            "min_lon": np.amin(x_points),
            "max_lon": np.amax(x_points),
            "min_lat": np.amin(y_points),
            "max_lat": np.amax(y_points)
        }

        colormap = cm.RdBu
        norm = mcolors.Normalize(vmin=self.bounds[0], vmax=self.bounds[len(self.bounds) - 1])
        faults = False

        if hasattr(self, "extras"):
            if "plot" in self.extras:
                if "features" in self.extras["plot"]:
                    if "colormap" in self.extras["plot"]["features"]:
                        colormap = getattr(
                            cm, self.extras["plot"]["features"]["colormap"]
                        )
                    if "scale" in self.extras["plot"]["features"]:
                        if str(self.extras["plot"]["features"]["scale"]).lower().strip() \
                                == "discrete":
                            colormap = self._cmapDiscretize(colormap, len(self.bounds) - 1)
                            norm = mcolors.BoundaryNorm(self.bounds, colormap.N)
                    if "faults" in self.extras["plot"]["features"]:
                        if str(self.extras["plot"]["features"]["faults"]).lower().strip() == "yes":
                            faults = True

        colormap.set_bad("gray", 1)

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
            lon_ticks = np.arange(ranges["min_lon"], ranges["max_lon"] + 0.01,
                                  (ranges["max_lon"] - ranges["min_lon"]) / 2)

            m.drawparallels(lat_ticks, linewidth=1.0, labels=[1, 0, 0, 0])
            m.drawmeridians(lon_ticks, linewidth=1.0, labels=[0, 0, 0, 1])

            data_cpy = np.ma.masked_invalid(data)

            if "topography" in kwargs and kwargs["topography"] is not None:
                # create light source object.
                ls = LightSource(azdeg=315, altdeg=45)
                # convert data to rgb array including shading from light source.
                # (must specify color map)
                low_indices = kwargs["topography"] < 0
                kwargs["topography"][low_indices] = 0
                rgb = ls.shade_rgb(colormap(norm(data_cpy)), kwargs["topography"],
                                   blend_mode="overlay", vert_exag=2)
                t = m.imshow(rgb, cmap=colormap, norm=norm)
            else:
                t = m.pcolormesh(x_points, y_points, data_cpy, cmap=colormap, norm=norm)

            m.drawstates()
            m.drawcountries()
            m.drawcoastlines()

            # If we want the fault lines we need to plot them.
            if faults:
                for key, fault_coords in Fault().get_all_faults().items():
                    fault_lons = []
                    fault_lats = []
                    for x, y in fault_coords:
                        fault_lons.append(x)
                        fault_lats.append(y)
                    m.plot(fault_lons, fault_lats, "k-", linewidth=1)
        else:
            t = plt.imshow(data, cmap=colormap, norm=norm)

        cax = plt.axes([0.125, 0.05, 0.775, 0.02])
        cbar = plt.colorbar(t, cax=cax, orientation='horizontal', ticks=self.ticks)

        cbar.set_label(self.plot_cbar_label if isinstance(self.plot_cbar_label, str) else "")

        plt.show()

    @classmethod
    def _cmapDiscretize(cls, cmap: cm, n: int) -> mcolors.LinearSegmentedColormap:
        """
        Generates a discrete colormap with N breaks in it.
        :param cm cmap: The colormap to make discrete.
        :param int n: The number of segments.
        :return: The new LinearSegmentedColormap to use.
        """
        cdict = cmap._segmentdata.copy()
        # N colors
        colors_i = np.linspace(0,1.,n)
        # N+1 indices
        indices = np.linspace(0,1.,n+1)
        for key in ('red','green','blue'):
            # Find the N colors
            D = np.array(cdict[key])
            colors = np.interp(colors_i, D[:,0], D[:,1])
            #I = sp.interpolate.interp1d(D[:,0], D[:,1])
            #colors = I(colors_i)
            # Place these colors at the correct indices.
            A = np.zeros((n+1,3), float)
            A[:,0] = indices
            A[1:,1] = colors
            A[:-1,2] = colors
            # Create a tuple for the dictionary.
            L = []
            for l in A:
                L.append(tuple(l))
            cdict[key] = tuple(L)
        # Return colormap object.
        return mcolors.LinearSegmentedColormap('colormap', cdict, 1024)
