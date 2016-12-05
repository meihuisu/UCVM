import math
import pyproj

points = {
    "bl": {
        "e": -122.95,
        "n": 36.60
    },
    "br": {
        "e": -120,
        "n": 33.399999999999999
    },
    "tl": {
        "e": -118.29620879302077,
        "n": 39.354833735842931
    },
    "tr": {
        "e": -115.44539372107891,
        "n": 36.040259683762166
    }
}

config = {
    "bl": {
        "e": 503240.52312058664,
        "n": 4051863.8294700049
    },
    "br": {
        "e": 779032.90147686121,
        "n": 3699450.9834490498
    },
    "tl": {
        "e": 906054.31248308613,
        "n": 4367099.1401498578
    },
    "tr": {
        "e": 1181846.6908393607,
        "n": 4014686.2941289032
    }
}

north_height_m = config["tl"]["n"] - config["bl"]["n"]
east_width_m = config["tl"]["e"] - config["bl"]["e"]

rotation_atan2 = math.degrees(math.atan2(config["br"]["n"] - config["bl"]["n"],
                                         config["br"]["e"] - config["bl"]["e"]))
rotation_atan = math.degrees(math.atan2(config["tl"]["n"] - config["bl"]["n"],
                                         config["tl"]["e"] - config["bl"]["e"]))

print(rotation_atan, rotation_atan2, rotation_atan + -1 * rotation_atan2)

total_height_m = math.sqrt(math.pow(config["tl"]["n"] - config["bl"]["n"], 2.0) +
                           math.pow(config["tl"]["e"] - config["bl"]["e"], 2.0))
print(total_height_m, total_height_m / 1023)
total_width_m = math.sqrt(math.pow(config["tr"]["n"] - config["tl"]["n"], 2.0) +
                          math.pow(config["tr"]["e"] - config["tl"]["e"], 2.0))
print(total_width_m, total_width_m / 895)

latlon = pyproj.Proj("+proj=latlong +datum=WGS84")
utmgeo = pyproj.Proj("+proj=utm +zone=10 +ellps=clrk66 +datum=NAD27 +units=m +no_defs")

new_points = pyproj.transform(latlon, utmgeo,
                              (points["bl"]["e"], points["tl"]["e"], points["tr"]["e"],
                               points["br"]["e"]),
                              (points["bl"]["n"], points["tl"]["n"], points["tr"]["n"],
                               points["br"]["n"]))
print(new_points[0][0], new_points[1][0])
print(new_points[0][1], new_points[1][1])
print(new_points[0][2], new_points[1][2])
print(new_points[0][3], new_points[1][3])

