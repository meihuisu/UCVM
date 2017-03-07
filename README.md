# UCVM

UCVM 17.3.0 represents the fifth major release of the Unified Community Velocity Model (UCVM) framework. UCVM is a
collection of software utilities that are designed to make querying velocity models, building meshes, and visualizing
velocity models easier to do through a uniform software interface. UCVM has been used extensively to generate meshes
and e-trees that are then used for 3D wave propagation simulations within California.

## Features

UCVM 17.3.0 includes key capabilities that are critical for scientists’ ability to run large-scale 3D wave propagation
simulations. UCVM has been re-designed from scratch to support the needs of scientists who require meshes that are in
the terabytes. In addition to the old capabilities of UCVM, this version supports the following:

Ability to extract partial meshes: Instead of running one large job for many hours, it is now possible to break-up the
mesh extraction and extract part of a mesh. For example, suppose you have a mesh that has 500 slices. The mesh
extraction can now be split into 100 slice increments. This helps ensure that progress can always be made on a mesh
or e-tree without requiring a full restart of the job if it fails.

Added mesh format: The RWG mesh format is now a supported output for UCVM. This helps for comparison studies between
multiple forward wave propagation simulation codes.

Validation: UCVM has been tested and checked to ensure scientific accuracy. Any known issues or inconsistencies with
the software have been addressed in this version.

Reproducible: Most utilities within UCVM generate XML configuration files which can be archived and re-run to ensure
reproducible results.

Improved installation process: Taking advantage of some of the most recent Python enhancements has meant simplified
installation on a wide variety of platforms - from Mac OS X to Linux to major supercomputers.

## Installing UCVM

UCVM supports Linux, Mac OS X, and two supercomputers. Details on how to install UCVM are available in our
[installation guide](https://github.com/SCECcode/UCVM/wiki/Installation).


