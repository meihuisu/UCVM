Tests
=====

UCVM is bundled with various tests to ensure scientific integrity within the models and also to ensure that the
platform itself is performing as expected on various computers. Each of these tests is detailed in this section.

.. toctree::

    tests_release_verification

**Commands**

UCVM has a test for many of the command-line utilities. These include:

- ucvm_query regular query by depth
- ucvm_query regular query by elevation
- ucvm_query errors on non-existant model
- ucvm_query SCEC 1D model returning interpolated values by default (UCVMC Test)
- ucvm_query SCEC BBP model returning interpolated values by default (UCVMC Test)
- ucvm_query SCEC BBP model returning non-interpolated values when interpolation flag is none (UCVMC Test)
- ucvm_query CVM-S4 returning interpolated 1D background model outside defined region (UCVMC Test)
- ucvm_query CVM-S4.26 model having no background model (UCVMC Test)
- ucvm_query CVM-S4.26.M01 model returning interpolated 1D outside region (UCVMC Test)
- ucvm_query all metadata working and displayed to the user if -a selected
- ucvm_plot_horizontal_slice known good path working
- ucvm_plot_depth_profile known good path working
- ucvm_plot_cross_section known good path working
- ucvm_plot_comparison known good path working

The details of these tests can be viewed by opening the ucvm/tests/data/commands.db file with any SQLite viewer. The
input and expected output, as well as the parameters, are included in the TestCase table.

**Framework**

.. automethod:: ucvm.tests.framework.UCVMFrameworkTest.test_ucvm_parse_model_string
.. automethod:: ucvm.tests.framework.UCVMFrameworkTest.test_ucvm_load_models
.. automethod:: ucvm.tests.framework.UCVMFrameworkTest.test_ucvm_query_with_test_velocity_model
.. automethod:: ucvm.tests.framework.UCVMFrameworkTest.test_ucvm_parse_string

**Meshing**

.. automethod:: ucvm.tests.mesh.UCVMMeshTest.test_internal_mesh_basics
.. automethod:: ucvm.tests.mesh.UCVMMeshTest.test_internal_mesh_iterator
.. automethod:: ucvm.tests.mesh.UCVMMeshTest.test_awp_rwg_equivalent
.. automethod:: ucvm.tests.mesh.UCVMMeshTest.test_generate_simple_mesh_ijk12_unrotated
.. automethod:: ucvm.tests.mesh.UCVMMeshTest.test_generate_simple_mesh_ijk12_rotated
.. automethod:: ucvm.tests.mesh.UCVMMeshTest.test_generate_simple_utm_mesh_ijk12_rotated
.. automethod:: ucvm.tests.mesh.UCVMMeshTest.test_generate_simple_utm_mesh_rwg_rotated
