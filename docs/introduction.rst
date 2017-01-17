Introduction
============

Thank you for downloading UCVM |version|, which was released in December, 2016. UCVM stands for "Unified Community
Velocity Model" and it is a framework through which various 3D seismic representations of the Earth's structure are
delivered. UCVM primarily has models of California although there is no geographical limitation to one state. It is
possible to register any velocity model within the UCVM framework.

What's New
----------

UCVM |version| includes key capabilities that are critical for scientists' ability to run large-scale 3D wave
propagation simulations. UCVM has been re-designed from scratch to support the needs of scientists who require meshes
that are in the terabytes. In addition to the old capabilities of UCVM, this version supports the following:

* Ability to extract partial meshes: Instead of running one large job for many hours, it is now possible to break-up\
the mesh extraction and extract part of a mesh. For example, say you have a mesh that has 500 slices. The mesh\
extraction can now be split into 100 slice increments. This helps ensure that progress can always be made on a mesh or\
e-tree without requiring a full restart of the job if it fails. More details can be found in our meshing guide.

TEST

.. toctree::
   :maxdepth: 1
