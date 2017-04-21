"""
Defines a bare-bones plot. Every specific plot type (e.g. HorizontalSlice) derives from this Plot class.

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
import os
import subprocess
import numpy as np

from ucvm.src.shared.errors import display_and_raise_error
from ucvm.src.model.fault import Fault

try:
    import matplotlib as mpl
    import matplotlib as mpl
    proc = subprocess.Popen(
        ["python3", "-c", "import matplotlib as mpl;mpl.use(\'qt4agg\');import matplotlib.pyplot as plt;plt.figure()"],
        stdout=subprocess.PIPE, stderr=subprocess.PIPE
    )
    if proc.communicate()[1].decode("UTF-8") == "":
        mpl.use("qt4agg")
    else:
        proc = subprocess.Popen(
             ["python3", "-c",
              "import matplotlib as mpl;mpl.use(\'qt5agg\');import matplotlib.pyplot as plt;plt.figure()"],
             stdout=subprocess.PIPE, stderr=subprocess.PIPE
        )
        if proc.communicate()[1].decode("UTF-8") == "":
            mpl.use("qt5agg")
        else:
            mpl.use("agg")
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
    from mpl_toolkits.basemap import cm as basemapcm
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

        if plt.gcf().get_figwidth() == 640 or plt.gcf().get_figwidth() == 6.4:
            plt.close(plt.gcf())
            self.figure = plt.figure(figsize=(self.plot_width / 100, self.plot_height / 100), dpi=100)

    def show_profile(self, properties: dict, **kwargs) -> bool:
        """
        Displays the profile to the user.
        :param properties: The properties as a dictionary.
        :return: True if profile shown, false if error.
        """
        save = None
        if hasattr(self, "extras"):
            if "plot" in self.extras:
                if "save" in self.extras["plot"]:
                    save = self.extras["plot"]["save"]
            if "data" in self.extras:
                if "save" in self.extras["data"]:
                    if str(self.extras["data"]["save"]).lower() == "y":
                        save = os.path.join(self.extras["data"]["location"], self.extras["data"]["name"] + ".png")

        plt.axes([0.2, 0.125, 0.70, 0.80])
        for key, item in properties.items():
            plt.gca().plot(item["x_values"], item["y_values"], "-",
                           color=item["info"]["color"], label=item["info"]["label"])

        plt.gca().invert_yaxis()
        plt.gca().set_ylim([item["y_values"][len(item["y_values"]) - 1], item["y_values"][0]])
        plt.gca().set_xlim([0, 8500])
        plt.xlabel(self.plot_xlabel if isinstance(self.plot_xlabel, str) else "", fontsize=14)
        plt.ylabel(self.plot_ylabel if isinstance(self.plot_ylabel, str) else "", fontsize=14)
        plt.title(self.plot_title if isinstance(self.plot_title, str) else "")
        plt.legend(loc="upper right")

        if not save:
            plt.show()
        else:
            plt.savefig(save)

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
        save = False

        if "basic" in kwargs:
            basic = kwargs["basic"]
        else:
            basic = False

        if hasattr(self, "extras"):
            if "data" in self.extras:
                if "save" in self.extras["data"]:
                    if str(self.extras["data"]["save"]).lower() == "y":
                        save = os.path.join(self.extras["data"]["location"], self.extras["data"]["name"] + ".png")
            if "plot" in self.extras:
                if "save" in self.extras["plot"]:
                    save = self.extras["plot"]["save"]
                if "features" in self.extras["plot"]:
                    if "colormap" in self.extras["plot"]["features"]:
                        try:
                            colormap = getattr(
                                cm, self.extras["plot"]["features"]["colormap"]
                            )
                        except AttributeError:
                            colormap = getattr(
                                basemapcm, self.extras["plot"]["features"]["colormap"]
                            )
                    if "scale" in self.extras["plot"]["features"]:
                        if str(self.extras["plot"]["features"]["scale"]).lower().strip() \
                                == "discrete":
                            colormap = self._cmapDiscretize(colormap, len(self.bounds) - 1)
                            norm = mcolors.BoundaryNorm(self.bounds, colormap.N)
                    if "faults" in self.extras["plot"]["features"]:
                        if str(self.extras["plot"]["features"]["faults"]).lower().strip() == "yes":
                            faults = True

        if map_plot:
            colormap.set_bad("gray", 1)

            if not basic:
                plt.axes([0.15, 0.2, 0.70, 0.70])

            # Check to see if we have pickled this particular basemap instance.
            m = basemap.Basemap(projection='cyl',
                                llcrnrlat=ranges["min_lat"],
                                urcrnrlat=ranges["max_lat"],
                                llcrnrlon=ranges["min_lon"],
                                urcrnrlon=ranges["max_lon"],
                                resolution='f',
                                anchor='C')

            if not basic:
                lat_ticks = np.arange(ranges["min_lat"], ranges["max_lat"],
                                      (ranges["max_lat"] - ranges["min_lat"]) / 2)
                lat_ticks = np.append(lat_ticks, [ranges["max_lat"]])
                lon_ticks = np.arange(ranges["min_lon"], ranges["max_lon"],
                                      (ranges["max_lon"] - ranges["min_lon"]) / 2)
                lon_ticks = np.append(lon_ticks, [ranges["max_lon"]])

                lat_ticks = [round(x, 2) for x in lat_ticks]
                lon_ticks = [round(x, 2) for x in lon_ticks]

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
                for y in range(len(rgb)):
                    for x in range(len(rgb[0])):
                        rgb[y][x][0] *= 0.90
                        rgb[y][x][1] *= 0.90
                        rgb[y][x][2] *= 0.90
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

            if not basic:
                plt.xlabel(self.plot_xlabel if isinstance(self.plot_xlabel, str) else "", fontsize=14)
                plt.ylabel(self.plot_ylabel if isinstance(self.plot_ylabel, str) else "", fontsize=14)
                plt.title(self.plot_title if isinstance(self.plot_title, str) else "")
        else:
            colormap.set_bad("gray", 1)
            data_cpy = np.ma.masked_invalid(data)
            data_cpy = np.ma.masked_less_equal(data_cpy, 0)

            plt.axes([0.1, 0.7, 0.8, 0.25])

            ll_lat = kwargs["boundaries"]["sp"][1] if kwargs["boundaries"]["sp"][1] < \
                     kwargs["boundaries"]["ep"][1] else kwargs["boundaries"]["ep"][1]
            ll_lon = kwargs["boundaries"]["sp"][0] if kwargs["boundaries"]["sp"][0] < \
                     kwargs["boundaries"]["ep"][0] else kwargs["boundaries"]["ep"][0]
            ur_lat = kwargs["boundaries"]["sp"][1] if kwargs["boundaries"]["sp"][1] > \
                     kwargs["boundaries"]["ep"][1] else kwargs["boundaries"]["ep"][1]
            ur_lon = kwargs["boundaries"]["sp"][0] if kwargs["boundaries"]["sp"][0] > \
                     kwargs["boundaries"]["ep"][0] else kwargs["boundaries"]["ep"][0]

            m = basemap.Basemap(projection='cyl',
                                llcrnrlat=ll_lat - 0.1 * ll_lat,
                                urcrnrlat=ur_lat + 0.1 * ur_lat,
                                llcrnrlon=ll_lon + 0.05 * ll_lon,
                                urcrnrlon=ur_lon - 0.05 * ur_lon,
                                resolution='f', anchor='C')

            m.etopo()

            m.plot([kwargs["boundaries"]["sp"][0], kwargs["boundaries"]["ep"][0]],
                   [kwargs["boundaries"]["sp"][1], kwargs["boundaries"]["ep"][1]],
                   color="k", linewidth=2)

            plt.axes([0.15, 0.25, 0.7, 0.35])
            plt.xlabel(self.plot_xlabel if isinstance(self.plot_xlabel, str) else "", fontsize=12)
            plt.ylabel(self.plot_ylabel if isinstance(self.plot_ylabel, str) else "", fontsize=12)
            plt.title(self.plot_title if isinstance(self.plot_title, str) else "")

            # If the aspect ratio is too extreme, set it to auto.
            if kwargs["yticks"][0][2] / kwargs["xticks"][0][2] < 1/8 or \
               kwargs["yticks"][0][2] / kwargs["xticks"][0][2] > 8/1:
                t = plt.imshow(data_cpy, cmap=colormap, norm=norm, aspect="auto")
            else:
                t = plt.imshow(data_cpy, cmap=colormap, norm=norm)

            if "yticks" in kwargs:
                plt.yticks(kwargs["yticks"][0], kwargs["yticks"][1])
            else:
                plt.yticks([], [])

            if "xticks" in kwargs:
                plt.xticks(kwargs["xticks"][0], kwargs["xticks"][1])
            else:
                plt.xticks([], [])

            if "topography" in kwargs and kwargs["topography"] is not None:
                x = [y for y in range(len(kwargs["topography"]))]
                plt.gca().set_ylim(kwargs["yticks"][0][2], kwargs["yticks"][0][0])
                plt.plot(x, kwargs["topography"], "k-", linewidth=0.75)

        if not basic:
            cax = plt.axes([0.05, 0.1, 0.90, 0.02])
            cbar = plt.colorbar(t, cax=cax, orientation='horizontal', ticks=self.ticks)

            cbar.set_label(self.plot_cbar_label if isinstance(self.plot_cbar_label, str) else "")

            if not save:
                plt.show()
            else:
                plt.savefig(save)
        else:
            return t

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
