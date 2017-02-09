.. _AvailableModels:

Available Models
================

UCVM can download and install multiple community models. These models are broadly categorized as "velocity",
"elevation", and "vs30". Each model represents a different area of scientific research.

Velocity
~~~~~~~~

UCVM delivers seismic material properties from multiple 3D velocity models. There are seven 3D velocity models and two
1D models that are included within UCVM. Each velocity model represents a part of California.

**CVM-S4.26**: This is a trilinearly interpolated version of the 26th iteration of Po Chen and En-Jui Lee's full 3D
tomographic inversions of the southern California region. The inversions were done on a 500m coarse mesh, however the
trilinear interpolation allows this model to be queried at arbitrary precision.

To call this model within UCVM, use the model code "*cvms426*".

Citation:

Lee, E.-J., P. Chen, T. H. Jordan, P. J. Maechling, M. A. M. Denolle, and G. C.Beroza (2014), Full 3-D
tomography for crustal structure in Southern California based on the scattering-integral and the adjoint-waveﬁeld
methods, J. Geophys. Res. Solid Earth, 119, doi:10.1002/2014JB011346.

**CCA06**: This is a trilinearly interpolated version of the 6th iteration of Po Chen and En-Jui Lee's full 3D
tomographic inversions of the central California region. The inversions were done on a 500m coarse mesh, however
the trilinearly interpolation allows this model to be queried at arbitrary precision.

To call this model within UCVM, use the model code "*cca06*".

**CVM-H 15.1.0**: The CVM-H is a velocity model of crust and upper mantle structure in southern California developed by the
SCEC community for use in fault systems analysis, strong ground motion prediction, and earthquake hazards assessment.
The model describes seismic P- and S-wave velocities and densities, and is comprised of basin structures embedded in
tomographic and teleseismic crust and upper mantle models.

The CVM-H consists of basin structures defined using high-quality industry seismic reflection profiles and tens of
thousands of direct velocity measurements from boreholes (Plesch et al., 2009; Süss and Shaw, 2003). The basin
structures are also compatible with the locations and displacements of major faults represented in the SCEC Community
Fault Model (CFM) (Plesch et al., 2007). These basin structures were used to develop travel time tomographic models of
the crust (after Hauksson, 2000) extending to a depth of 35 km, and upper mantle teleseismic and surface wave models
extending to a depth of 300 km (Prindle and Tanimoto, 2006). These various model components were integrated and used to
perform a series of 3D adjoint tomographic inversions that highlight areas of the model that were responsible for
mismatches between observed and synthetic waveforms (Tape et al, 2009). Sixteen tomographic iterations, requiring 6800
wavefield simulations, yielded perturbations to the starting model that have been incorporated in the latest model
release.

To call this model within UCVM, use the model "*cvmh1510*".

Citations:

Suess, M. P., and J. H. Shaw, 2003, P-wave seismic velocity structure derived from sonic logs and industry
reflection data in the Los Angeles basin, California, Journal of Geophysical Research, 108/B3.

Plesch, A., C. Tape, JR. Graves, P. Small, G. Ely and J. H. Shaw, 2011, Updates for the CVM-H including new
representations of the offshore Santa Maria and San Bernardino basin and a new Moho surface, in 2011 Southern
California Earthquake Center Annual Meeting, Proceedings and Abstracts, vol. 21.

Tape, C., Q. Liu, A. Maggi, and J. Tromp, 2009, Adjoint tomography of the southern California crust, Science, v. 325,
p. 988-992.

**CVM-S4**: CVM-SCEC version 4 (CVM-S4), also known as SCEC CVM-4, is a 3D seismic velocity model.The current
version is CVM-S4 was released in 2006 and was originally posted for download from the SCEC Data Center SCEC 3D
Velocity Models Site.The purpose of the Three-Dimensional Community Velocity Model for Southern California is to
provide a unified reference model for the several areas of research that depend of the subsurface velocity structure
in their analysis. These include strong motion modeling, seismicity location, and tomographic velocity modeling. It is
also hoped that the geologic community will find the basin models useful because they are based on structures and
interfaces that are largely derived from geologic structure models. The deeper sediment velocities themselves are
obtained from empirical relationships that take into account age of the formation and depth of burial. The coefficients
of these relationships are calibrated to sonic logs taken from boreholes in the region. Shallow sediment velocities are
taken from geotechnical borehole measurements. Hardrock velocities are based on tomographic studies.

To call this model within UCVM, use the model code "*cvmh1510*".

Citation:

Kohler, M., H. Magistrale, and R. Clayton, 2003, Mantle heterogeneities and the SCEC three-dimensional seismic velocity
model version 3, Bulletin Seismological Society of America 93, 757-774.

**CVM-S4.26.M01**: The CVM-S4.26.M01 model is an effort to integrate Po Chen's perturbations (the CVM-S4.26 model) while
still honoring the CVM-S4 GTL. The desired effect is that we recover the CVM-S GTL while smoothly adding in positive
and negative perturbations.

To call this model within UCVM, use the model code "*cvms426m01*".

**USGS Bay Area Model**: The USGS 3-D Geologic and Seismic Velocity Models of the San Francisco Bay region provide a
three-dimensional view of the geologic structure and physical properties of the region down to a depth of 45 km
(28 miles). The 3-D models combine 100 years of surface geologic mapping by the USGS, the California Geological Survey,
and many other institutions together with decades of research into the seismic properties of the rocks in the Bay Area.
They also include information from boreholes and variations in Earth's gravity and magnetic fields. Traditional
two-dimensional geologic maps show only the distribution of rock units at Earth's surface. The geologic model is a
fault block model - that is, the upper 45 km (28 miles) of Earth's crust has been broken up into irregular shaped
blocks, bounded by faults. The model also includes the subsurface shape of basins that underlie the Santa Clara Valley,
Livermore Valley, and Santa Rosa Plain. The soft sediments in these basins trap seismic energy and greatly enhance
shaking levels relative to surrounding regions.

To call this model within UCVM, use the model code "*bayarea*".

**Lin-Thurber Model**: The Lin-Thurber model is a seismic velocity model of the California crust and uppermost
mantle using a regional-scale double-difference tomography algorithm. The model is the first 3D seismic velocity
model for the entire state of California based on local and regional arrival time data that has ever been developed.
It has improved areal coverage compared to the previous northern and southern California models, and extends to greater
depth due to the inclusion of substantial data at large epicentral distances.

To call this model within UCVM, use the model code "*linthurber*".

Citation:

Lin, G., C. H. Thurber, H. Zhang, E. Hauksson, P. Shearer, F. Waldhauser, T. M. Brocher, and J. Hardebeck (2010), A
California statewide three-dimensional seismic velocity model from both absolute and differential Times, Bull. Seism.
Soc. Am., 100, in press.

Elevation
~~~~~~~~~

Elevation data comes from one of two sources:

**ETOPO1** is a 1 arc-minute global relief model of Earth's surface that integrates land topography and ocean
bathymetry. This data is used if we are querying outside the state of California.

**USGS National Map** data is used within the state of California. This data is 1 arc-second and, as such, provides
for higher precision than the ETOPO1 data.

These two sources make the *usgs_noaa* digital elevation model. There is no other elevation model currently registered
within UCVM.

Vs30
~~~~

There are two Vs30 models included within UCVM:

**Wills-Wald 2006**: This dataset uses Wills and Clahan 2006 data within the state of California and falls back to the
Wald 2007 data outside of the California boundary. This is the default method of retrieving Vs30 data.

To call this model within UCVM, use the model code "*wills-wald-2006*".

Citations:

Wald, D. J., and T. I. Allen (2007), Topographic slope as a proxy for seismic site conditions and amplification,
Bull. Seism. Soc. Am., 97 (5), 1379-1395, doi:10.1785/0120060267.

Wills, C. J., and K. B. Clahan (2006), Developing a map of geologically defined site-condition categories for
California, Bull. Seism. Soc. Am., 96 (4A), 1483-1501, doi:10.1785/0120050179.

**Vs30 Calculated**: This calculates the Vs30 from the model directly. It samples the top 30m of the velocity model
and calculates the average of its slowness.

To call this model within UCVM, use the model code "*vs30-calc*".
