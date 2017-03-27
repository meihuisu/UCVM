Query API Reference
===================

The core capability of UCVM is the ability to query one or more models (of seismic velocity data, elevation data, Vs30
data, etc.) through a standardized query interface. This version of UCVM supports that standardized query interface
primarily through one main method: UCVM.query. While this method has some advanced capabilities to it, at its core, it
takes SeismicData data structures and a model string (such as "cvms4.vs30-calc") and fills that SeismicData data
structure with material properties. The properties that are filled and what the script that requested them does with
that information is entirely up to the user.

**Please note**: UCVM automatically adjusts the library paths for the user. Therefore, you must call from
ucvm.src.framework.ucvm import UCVM as quickly as possible as it will relaunch the process with the correct library
paths if they are not added correctly before the process starts.

.. automethod:: ucvm.src.framework.ucvm.UCVM.query
.. automethod:: ucvm.src.framework.ucvm.UCVM.get_list_of_installed_models
