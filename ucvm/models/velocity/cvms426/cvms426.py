"""
Defines the CVM-S4.26 velocity model.

Copyright:
    Southern California Earthquake Center

Developer:
    David Gill <davidgil@usc.edu>
"""
# UCVM Imports
from ucvm.src.model.velocity.gridded_velocity_model import GriddedVelocityModel


class CVMS426VelocityModel(GriddedVelocityModel):
    """
    All of Po and En-Jui's models are handled perfectly by the superclass.
    """
    pass
