from libc.math cimport atan, pow, sin, sqrt, tan, cos, floor, fmod

class UCVMCCommon:

    @staticmethod
    def calculate_grid_point(float width, float height, float depth, float x_value, float y_value,
                             float z_value, int dim_x, int dim_y, int z_interval) -> (dict, dict):

        cdef int x_c = (int)(floor(x_value / width * (x_value - 1)))
        cdef int y_c = (int)(floor(y_value / height * (y_value - 1)))
        cdef int z_c = (int)(floor(depth / (z_interval - 1)) - floor(z_value / z_interval))

        cdef float x_p = fmod(x_value, (width / (dim_x - 1))) / (dim_x - 1)
        cdef float y_p = fmod(y_value, (height / (dim_y - 1))) / (dim_y - 1)
        cdef float z_p = fmod(z_value, z_interval) / z_interval

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
            longitude (float): The longitude in decimal degrees.
            latitude (float): The latitude in decimal degrees.
            zone (int): The UTM zone to which the conversions should be done.

        Returns:
            A tuple containing two floats (easting and northing) in UTM projection.
        """
        cdef float semimaj = 6378206.40
        cdef float semimin = 6356583.80
        cdef float scfa = 0.9996
        cdef float north = 0.0
        cdef float east = 500000.0

        cdef float pi = 4.0 * atan(1.0)
        cdef float degrad = pi / 180.0

        cdef float e2, e4, e6, ep2, cm, rm, rn, new_lat, delam
        cdef float f1, f2, f3, f4

        cdef float xx, yy

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