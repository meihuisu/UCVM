"""
UCVM Common C Routines

These routines are used very frequently either within the UCVM platform or within some of the models. As such, they
have been written in Cython for optimal efficiency.

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
# Cython Imports
from libc.math cimport atan, pow, sin, sqrt, tan, cos, floor, fmod
from libc.stdint cimport uintptr_t, int32_t, uint32_t

cdef struct ucvm_epayload_t:
    float Vp
    float Vs
    float density

cdef extern from "etree.h":
    struct etree_t:
        int error
        pass
    struct etree_addr_t:
        uint32_t x
        uint32_t y
        uint32_t z
        uint32_t t
        int level
        int tree_type

    etree_t *etree_open(const char *, int32_t, int32_t, int32_t, int32_t)
    int etree_search(etree_t *, etree_addr_t, etree_addr_t *, const char *, void *)
    int etree_registerschema(etree_t *, const char *)
    char* etree_getappmeta(etree_t*)
    int etree_setappmeta(etree_t *, const char *)
    int etree_insert(etree_t *, etree_addr_t, const void *);
    int etree_close(etree_t *)

class UCVMCCommon:

    @staticmethod
    def c_etree_open(char *path, int mode):
        cdef int32_t imode = mode
        cdef int32_t bufsize = 64
        cdef int32_t payloadsize = 0
        cdef int32_t dimensions = 3
        return <uintptr_t>etree_open(path, imode, bufsize, payloadsize, dimensions)

    @staticmethod
    def c_etree_setappmeta(uintptr_t opened_etree, char *metadata) -> int:
        return etree_setappmeta(<etree_t *>opened_etree, metadata)

    @staticmethod
    def c_etree_getappmeta(uintptr_t opened_etree) -> dict:
        cdef char *appmeta = etree_getappmeta(<etree_t *>opened_etree)
        appmeta_dict = bytes(appmeta).decode("utf-8").split()

        return {
            "title": appmeta_dict[0].split(":"),
            "author": appmeta_dict[1].split(":"),
            "date": appmeta_dict[2].split(":"),
            "numfields": appmeta_dict[3],
            "payload": appmeta_dict[4].split(";"),
            "origin": [float(appmeta_dict[6]), float(appmeta_dict[5]), float(appmeta_dict[9])],
            "dims": [float(appmeta_dict[7]), float(appmeta_dict[8]), float(appmeta_dict[10])],
            "ticks": [int(appmeta_dict[11]), int(appmeta_dict[12]), int(appmeta_dict[13])]
        }

    @staticmethod
    def c_etree_registerschema(uintptr_t opened_etree, char *defstring) -> int:
        return etree_registerschema(<etree_t *>opened_etree, defstring)

    @staticmethod
    def c_etree_close(uintptr_t opened_etree):
        etree_close(<etree_t *>opened_etree)

    @staticmethod
    def c_etree_insert(uintptr_t opened_etree, long addr_x, long addr_y, long addr_z, int level,
                       float vp, float vs, float density):
        cdef etree_addr_t addr
        cdef ucvm_epayload_t payload

        addr.x = addr_x
        addr.y = addr_y
        addr.z = addr_z
        addr.level = level

        payload.Vp = vp
        payload.Vs = vs
        payload.density = density

        return etree_insert(<etree_t *>opened_etree, addr, &payload)

    @staticmethod
    def c_etree_query(uintptr_t opened_etree, float lon, float lat, float depth, corners: tuple,
                      dims: tuple, ticks: tuple) -> (float, float, float):
        cdef etree_addr_t addr
        cdef ucvm_epayload_t payload
        cdef uint32_t i = 0
        x_coord, y_coord = UCVMCCommon.c_etree_bilinear_geo2xy(lon, lat, corners, dims)
        z_coord = depth

        x_coord = x_coord / dims[0] * ticks[0]
        y_coord = y_coord / dims[1] * ticks[1]
        z_coord = z_coord / dims[2] * ticks[2]

        addr.x = <long>x_coord
        addr.y = <long>y_coord
        addr.z = <long>z_coord
        addr.level = 31

        if etree_search(<etree_t *>opened_etree, addr, NULL, "*", &payload) == 0:
            return (payload.Vp, payload.Vs, payload.density)
        else:
            return None

    @staticmethod
    def c_etree_bilinear_geo2xy(float lon, float lat, corners: tuple, dims: tuple) -> (float, float):
        cdef int i = 0, k = 0
        cdef double x = 0, y = 0, x0 = 0, y0 = 0, dx = 0, dy = 0
        cdef double j[4]
        cdef double j1[4]
        cdef double j2[4]
        cdef double jinv[4]
        cdef double xce = 0, yce = 0
        cdef double res = 1, d = 0, p = 0, q = 0

        cdef double csii[4]
        cdef double ethai[4]

        csii[0] = -1.0
        csii[1] = -1.0
        csii[2] = 1.0
        csii[3] = 1.0

        ethai[0] = -1.0
        ethai[1] = 1.0
        ethai[2] = 1.0
        ethai[3] = -1.0

        j1[0] = 0
        j1[1] = 0
        j1[2] = 0
        j1[3] = 0

        for i in range(4):
            j1[0] += corners[i][0] * csii[i]
            j1[1] += corners[i][0] * ethai[i]
            j1[2] += corners[i][1] * csii[i]
            j1[3] += corners[i][1] * ethai[i]
            xce += corners[i][0] * csii[i] * ethai[i]
            yce += corners[i][1] * csii[i] * ethai[i]

        while res > 0.000000000001 and k <= 10:
            k += 1
            j2[0] = y * xce
            j2[1] = x * xce
            j2[2] = y * yce
            j2[3] = x * yce

            j[0] = 0.25 * (j1[0] + j2[0])
            j[1] = 0.25 * (j1[1] + j2[1])
            j[2] = 0.25 * (j1[2] + j2[2])
            j[3] = 0.25 * (j1[3] + j2[3])

            d = (j[0] * j[3]) - (j[2] * j[1])
            jinv[0] = j[3] / d
            jinv[1] = -j[1] / d
            jinv[2] = -j[2] / d
            jinv[3] = j[0] / d

            x0 = 0
            y0 = 0

            for i in range(4):
                x0 += corners[i][0] * (.25 * (1 + (csii[i]  * x)) * (1 + (ethai[i] * y)))
                y0 += corners[i][1] * (.25 * (1 + (csii[i]  * x)) * (1 + (ethai[i] * y)))

            p = lon - x0
            q = lat - y0

            dx = (jinv[0] * p) + (jinv[1] * q)
            dy = (jinv[2] * p) + (jinv[3] * q)

            x += dx
            y += dy

            res = dx * dx + dy * dy

        if k > 10:
            raise Exception("Could not convert lon, lat to XY.")

        x = (x + 1) * dims[0] / 2.0
        y = (y + 1) * dims[1] / 2.0

        return (x, y)

    @staticmethod
    def c_etree_bilinear_xy2geo(float x, float y, corners: tuple, dims: tuple) -> (float, float):
        x_val = UCVMCCommon.c_etree_bilinear_interpolate(
            x, y, 0, 0, dims[0], dims[1], corners[0][1], corners[3][1], corners[1][1], corners[2][1]
        )
        y_val = UCVMCCommon.c_etree_bilinear_interpolate(
            x, y, 0, 0, dims[0], dims[1], corners[0][0], corners[3][0], corners[1][0], corners[2][0]
        )
        return x_val, y_val

    @staticmethod
    def c_etree_bilinear_interpolate(float x, float y, float x1, float y1, float x2, float y2,
                                     float q11, float q21, float q12, float q22) -> float:
        cdef float p = (x2 - x1) * (y2 - y1)
        cdef float f1 = (q11 / p) * (x2 - x) * (y2 - y)
        cdef float f2 = (q21 / p) * (x - x1) * (y2 - y)
        cdef float f3 = (q12 / p) * (x2 - x) * (y - y1)
        cdef float f4 = (q22 / p) * (x - x1) * (y - y1)
        return f1 + f2 + f3 + f4

    @staticmethod
    def calculate_grid_point(double width, double height, double depth, double x_value, double y_value,
                             double z_value, int dim_x, int dim_y, int z_interval) -> (dict, dict):

        cdef int x_c = (int)(floor(x_value / width * (dim_x - 1)))
        cdef int y_c = (int)(floor(y_value / height * (dim_y - 1)))
        cdef int z_c = (int)(floor(z_value / z_interval))

        cdef double x_p = fmod(x_value, (width / (dim_x - 1))) / (width / (dim_x - 1))
        cdef double y_p = fmod(y_value, (height / (dim_y - 1))) / (height / (dim_y - 1))
        cdef double z_p = fmod(z_value, z_interval) / z_interval

        return {"x": x_c, "y": y_c, "z": z_c}, {"x": x_p, "y": y_p, "z": z_p}

    @staticmethod
    def trilinear_interpolate(float t1, float t2, float t3, float t4, float b1, float b2, float b3,
                              float b4, float x_percent, float y_percent, float z_percent) -> float:
        cdef float tx1 = (1 - x_percent) * t1 + x_percent * t2
        cdef float tx2 = (1 - x_percent) * t3 + x_percent * t4
        cdef float bx1 = (1 - x_percent) * b1 + x_percent * b2
        cdef float bx2 = (1 - x_percent) * b3 + x_percent * b4

        cdef float ty = (1 - y_percent) * tx1 + y_percent * tx2
        cdef float by = (1 - y_percent) * bx1 + y_percent * bx2

        return (1 - z_percent) * ty + z_percent * by

    @staticmethod
    def bilinear_interpolate(float v1, float v2, float v3, float v4, float x_percent,
                             float y_percent) -> float:
        cdef float vx1 = (1 - x_percent) * v1 + x_percent * v2
        cdef float vx2 = (1 - x_percent) * v3 + x_percent * v4

        return (1 - y_percent) * vx1 + y_percent * vx2

    @staticmethod
    def fortran_convert_ll_utm(float longitude, float latitude, int zone) -> (float, float):
        """
        Converts longitude and latitude to UTM in the same manor as the Fortran code that Po
        and En-Jui used in their conversions does.

        Args:
            longitude (double): The longitude in decimal degrees.
            latitude (double): The latitude in decimal degrees.
            zone (int): The UTM zone to which the conversions should be done.

        Returns:
            A tuple containing two floats (easting and northing) in UTM projection.
        """
        cdef double semimaj = 6378206.40
        cdef double semimin = 6356583.80
        cdef double scfa = 0.9996
        cdef double north = 0.0
        cdef double east = 500000.0

        cdef double pi = 4.0 * atan(1.0)
        cdef double degrad = pi / 180.0

        cdef double e2, e4, e6, ep2, cm, rm, rn, new_lat, delam
        cdef double f1, f2, f3, f4

        cdef double xx, yy

        e2 = 1.0 - pow((semimin / semimaj), 2)
        e4 = e2 * e2
        e6 = e2 * e4
        ep2 = e2 / (1.0 - e2)

        cm = zone * 6.0 - 183.0
        new_lat = degrad * latitude

        delam = longitude - cm
        if delam < -180:
            delam += 360
        elif delam > 180:
            delam -= 360
        delam *= degrad

        f1 = (1.0 - e2 / 4.0 - 3.0 * e4 / 64.0 - 5.0 * e6 / 256.0) * new_lat
        f2 = 3.0 * e2 / 8.0 + 3.0 * e4 / 32.0 + 45.0 * e6 / 1024.0
        f2 *= sin(2.0 * new_lat)
        f3 = 15.0 * e4 / 256.0 * 45.0 * e6 / 1024.0
        f3 *= sin(4.0 * new_lat)
        f4 = 35.0 * e6 / 3072.0
        f4 *= sin(6.0 * new_lat)
        rm = semimaj * (f1 - f2 + f3 - f4)

        if latitude == 90 or latitude == -90:
            xx = 0.0
            yy = scfa * rm
        else:
            rn = semimaj / sqrt(1.0 - e2 * sin(new_lat) ** 2)
            t = tan(new_lat) ** 2
            c = ep2 * cos(new_lat) ** 2
            a = cos(new_lat) * delam

            f1 = (1.0 - t + c) * a ** 3 / 6.0
            f2 = 5.0 - 18.0 * t + t ** 2 + 72.0 * c - 58.0 * ep2
            f2 *= a ** 5 / 120.0
            xx = scfa * rn * (a + f1 + f2)
            f1 = a ** 2 / 2.0
            f2 = 5.0 - t + 9.0 * c + 4.0 * c ** 2
            f2 *= a ** 4 / 24.0
            f3 = 61.0 - 58.0 * t + t ** 2 + 600.0 * c - 330.0 * ep2
            f3 *= a ** 6 / 720.0
            yy = scfa * (rm + rn * tan(new_lat) * (f1 + f2 + f3))

        xx += east
        yy += north

        return xx, yy