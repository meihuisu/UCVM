Data Structures Reference
=========================

UCVM uses multiple basic data structures for holding and querying data. The primary container for all properties is
the SeismicData object. This object represents a collection of known information (known meaning that a requested model
filled in information) about a point in the Earth. The basic premise behind UCVM is that you initialize a SeismicData
object and then pass it to different models. The CVM-S4 model, for example, will fill in the velocity properties. Then
the USGS/NOAA model will fill in the elevation data. This keeps going until all properties are filled or all models
are exhausted. Eventually the script running the query will get one or more of these SeismicData objects back. The
script then handles how to display and/or use the information contained within these objects.

So, in summary, a SeismicData object contains: a point on the Earth, velocity data, elevation data, and Vs30 data. This
object provides a standardized method for adding and retrieving material properties through the UCVM interface.

Projections are specified in Proj.4 format. `More information on Proj.4 can be found here. <http://proj4.org>`_

**Starter Examples**
::

    p = Point(-118, 34, 0)              # Create a point at (-118, 34) at 0m depth in WGS84 lat, long
                                        # projection.
    s = SeismicData(p)                  # Create a placeholder SeismicData object. This will have *no*
                                        # material properties in it.
    UCVM.query([s], "cvms4")            # Query CVM-S4 at this point for material properties.
    print(s.velocity_properties.vs)     # Print the Vs data at -118, 34 at depth 0m for model CVM-S4.

::

    p = Point(-118, 34, 0)              # Create a point at (-118, 34) at 0m depth in WGS84 lat, long
                                        # projection.
    p_utm = p.convert_to_projection(                        # Convert p into UTM projection, Clarke 1866
        "+proj=utm +ellps=clrk66 +datum=NAD27 +zone=11"     # ellipsoid and NAD27 datum. This must be a
    )                                                       # valid Proj.4 string.
    print(p_utm.x_value, p_utm.y_value) # Print the new X and Y UTM coordinates.

.. currentmodule:: ucvm.src.shared.constants

**Constants**

To use, import from ucvm.src.shared.constants.

.. autoattribute:: ucvm.src.shared.constants.UCVM_DEPTH
    :annotation: : Value is 0. Pass to a Point to specify it in depth.
.. autoattribute:: ucvm.src.shared.constants.UCVM_ELEVATION
    :annotation: : Value is 1. Pass to a Point to specify it in elevation.
.. autoattribute:: ucvm.src.shared.constants.UCVM_DEFAULT_PROJECTION
    :annotation: The default projection for all UCVM points unless otherwise specified. It is WGS84 lat, long.
        The exact Proj.4 string is "+proj=latlong +datum=WGS84".

**Model Property Containers**

These are properties that the following classes interact with (i.e. that models can provide and fill). To use,
import from ucvm.src.shared.properties.

.. autoattribute:: ucvm.src.shared.properties.VelocityProperties
    :annotation: A namedtuple that holds the following properties: vp, vs, density, qp, qs, vp_source, vs_source,
        density_source, qp_source, qs_source

.. autoattribute:: ucvm.src.shared.properties.ElevationProperties
    :annotation: A namedtuple that holds the following properties related to elevation values:
        elevation, elevation_source

.. autoattribute:: ucvm.src.shared.properties.Vs30Properties
    :annotation: A namedtuple that holds the following properties related to Vs30 values: vs30, vs30_source

.. autoattribute:: ucvm.src.shared.properties.ZProperties
    :annotation: A namedtuple that holds the following properties related to Z-depth values: z10, z25

**Classes**

To use, import from ucvm.src.shared.properties.

.. autoclass:: ucvm.src.shared.properties.Point

    **Methods**

    .. automethod:: ucvm.src.shared.properties.Point.convert_to_projection
    .. automethod:: ucvm.src.shared.properties.Point.get_depth_or_elevation

    **Properties**

    .. autoinstanceattribute:: ucvm.src.shared.properties.Point.x_value
    .. autoinstanceattribute:: ucvm.src.shared.properties.Point.y_value
    .. autoinstanceattribute:: ucvm.src.shared.properties.Point.z_value
    .. autoinstanceattribute:: ucvm.src.shared.properties.Point.projection
    .. autoinstanceattribute:: ucvm.src.shared.properties.Point.depth_elev

.. autoclass:: ucvm.src.shared.properties.SeismicData(point: Point=None, extras: dict=None)

    **Methods**

    .. automethod:: ucvm.src.shared.properties.SeismicData.set_velocity_data(velocity_properties: VelocityProperties)
    .. automethod:: ucvm.src.shared.properties.SeismicData.set_elevation_data(elevation_properties: ElevationProperties)
    .. automethod:: ucvm.src.shared.properties.SeismicData.set_vs30_data(vs30_properties: Vs30Properties)
    .. automethod:: ucvm.src.shared.properties.SeismicData.set_z_data(z_properties: ZProperties)
    .. automethod:: ucvm.src.shared.properties.SeismicData.set_model_string
    .. automethod:: ucvm.src.shared.properties.SeismicData.is_property_type_set
    .. automethod:: ucvm.src.shared.properties.SeismicData.convert_point_to_projection
    .. automethod:: ucvm.src.shared.properties.SeismicData.set_point_to_depth_or_elev

    **Properties**

    .. autoinstanceattribute:: ucvm.src.shared.properties.SeismicData.original_point
    .. autoinstanceattribute:: ucvm.src.shared.properties.SeismicData.converted_point
    .. autoinstanceattribute:: ucvm.src.shared.properties.SeismicData.velocity_properties
    .. autoinstanceattribute:: ucvm.src.shared.properties.SeismicData.elevation_properties
    .. autoinstanceattribute:: ucvm.src.shared.properties.SeismicData.vs30_properties
    .. autoinstanceattribute:: ucvm.src.shared.properties.SeismicData.z_properties
    .. autoinstanceattribute:: ucvm.src.shared.properties.SeismicData.model_string
    .. autoinstanceattribute:: ucvm.src.shared.properties.SeismicData.extras

