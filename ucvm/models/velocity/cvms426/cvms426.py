"""
Defines the CVM-S4.26 velocity model.

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
# UCVM Imports
from ucvm.src.model.velocity.gridded_velocity_model import GriddedVelocityModel


class CVMS426VelocityModel(GriddedVelocityModel):
    """
    All of Po and En-Jui's models are handled perfectly by the superclass.
    """
    pass
