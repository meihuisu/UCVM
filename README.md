#UCVM

UCVM 17.2.0 includes key capabilities that are critical for scientists’ ability to run large-scale 3D wave propagation simulations. UCVM has been re-designed from scratch to support the needs of scientists who require meshes that are in the terabytes. In addition to the old capabilities of UCVM, this version supports the following:

Ability to extract partial meshes: Instead of running one large job for many hours, it is now possible to break-up the mesh extraction and extract part of a mesh. For example, suppose you have a mesh that has 500 slices. The mesh extraction can now be split into 100 slice increments. This helps ensure that progress can always be made on a mesh or e-tree without requiring a full restart of the job if it fails.

Added mesh format: The RWG mesh format is now a supported output for UCVM. This helps for comparison studies between multiple forward wave propagation simulation codes.

Validation: UCVM has been tested and checked to ensure scientific accuracy. Any known issues or inconsistencies with the software have been addressed in this version.

Reproducible: Most utilities within UCVM generate XML configuration files which can be archived and re-run to ensure reproducible results.

Improved installation process: Taking advantage of some of the most recent Python enhancements has meant simplified installation on a wide variety of platforms - from Mac OS X to Linux to major supercomputers.

#Installing UCVM

