"""
Imports UCVM to calculate the following for each CyberShake site:

"Station name/code” “Long” “Lat” “Vmod Vs30” “Wills Vs30” “Z1.0” “Z2.5"

This utilizes the UCVM 17.3.0 framework.

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
# Python Imports
import sys
import csv
from typing import List

# Package Imports
import pymysql

# UCVM Imports
from ucvm.src.framework.ucvm import UCVM
from ucvm.src.shared.properties import Point, SeismicData


def all_cybershake_seismic_data() -> List[SeismicData]:
    """
    Function to get all the CyberShake sites as Points which are then turned into a list of SeismicData objects
    which contain the CyberShake site points.

    Returns:
         A list of SeismicData objects which correspond to the CyberShake sites.
    """
    ret_sds = []

    connection = pymysql.connect(host="moment.usc.edu", user="cybershk_ro", password="CyberShake2007", db="CyberShake",
                                 cursorclass=pymysql.cursors.DictCursor)
    with connection.cursor() as cursor:
        sql = "SELECT * FROM CyberShake_Sites WHERE CS_Site_ID IN (" \
              "SELECT Site_ID FROM CyberShake_Runs WHERE ERF_ID=36 AND SGT_Variation_ID=8 AND Rup_Var_Scenario_ID=6" \
              ")"
        cursor.execute(sql)
        results = cursor.fetchall()

    for result in results:
        sd = SeismicData(Point(result["CS_Site_Lon"], result["CS_Site_Lat"], 0))
        sd.cybershake_site_id = result["CS_Short_Name"]
        sd.cybershake_site_name = result["CS_Site_Name"]
        ret_sds.append(sd)

    return ret_sds


def main() -> int:
    """
    The main method which calculates the CSV containing the Wills-Wald Vs30, Vs30 from model, and the Z values.

    Returns:
        0 if successful. Raises an error if not.
    """

    # Initialize UCVM and get the CyberShake sites.
    u = UCVM()
    cs_sites = all_cybershake_seismic_data()

    # Get Wills for each site.
    model = "wills-wald-2006"
    u.query(cs_sites, model, ["vs30"])

    for site in cs_sites:
        site.vs30_wills = site.vs30_properties.vs30

    # Get the Vs30 from the model itself (CVM-S4.26.M01 in this case).
    model = "cvms426m01.vs30-calc.z-calc"
    u.query(cs_sites, model)

    for site in cs_sites:
        site.vs30_cvms426m01 = site.vs30_properties.vs30

    # Write everything to CSV.
    with open('cybershake_15.4_site_data_cvms426m01.csv', 'w') as csvfile:
        cwriter = csv.writer(csvfile)
        cwriter.writerow(["Site ID", "Longitude", "Latitude", "Vs30 (Wills)", "Vs30 (Model)",
                          "Difference (Model - Wills)", "Z1.0", "Z2.5"])
        for site in cs_sites:
            cwriter.writerow([site.cybershake_site_id, site.original_point.x_value, site.original_point.y_value,
                              site.vs30_wills, site.vs30_cvms426m01, site.vs30_cvms426m01 - site.vs30_wills,
                              site.z_properties.z10, site.z_properties.z25])

    return 0


if __name__ == "__main__":
    sys.exit(main())
