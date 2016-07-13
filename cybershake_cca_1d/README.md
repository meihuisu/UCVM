# CyberShake CCA 1D Model

The gen_bbp_format_no_water.py script was used to generate the 1D 
CyberShake Central California model. It queries all the grid points of 
the CCA inversion iteration 6 using the following constraints:

- Points must be on land (i.e. any point that UCVM deems to have an 
  elevation below 0m, we consider to be in water).
- Points must be within the Central California CyberShake box.

Instructions on how to run the script (and use the output) are included 
in the Python file.

Reference: https://scec.usc.edu/scecpedia/CyberShake_Central_California_1D_Model 
