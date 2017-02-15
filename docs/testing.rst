Testing
=======

UCVM is bundled with various tests to ensure scientific integrity within the models and also to ensure that the
platform itself is performing as expected on various computers. Each of these tests is detailed in this section.

Platform
--------

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
ucvm.tests.mesh.UCVMMeshTest.test_generate_simple_mesh_ijk12_unrotated