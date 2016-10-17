// -*- C++ -*-
//
// ======================================================================
//
//                           Brad T. Aagaard
//                        U.S. Geological Survey
//
// {LicenseText}
//
// ======================================================================
//

/** @file libsrc/storage/GeomCenCA.h
 *
 * @brief C++ manager for geometry of the USGS central CA velocity
 * model.
 */

#if !defined(cencalvm_storage_geomcenca_h)
#define cencalvm_storage_geomcenca_h

#include "etreefwd.h" // USES etree types

#include <math.h> // USES log()

namespace cencalvm {
  namespace storage {
    class GeomCenCA;
    class Geometry; // ISA Geometry
    class Projector; // HOLDSA Projector
    class TestGeomCenCA; // friend
  } // namespace storage
} // namespace cencalvm

/// C++ manager for geometry of the USGS central CA velocity
/// model.
class cencalvm::storage::GeomCenCA : public cencalvm::storage::Geometry
{ // GeomCenCA
  friend class TestGeomCenCA;

public :
  // PUBLIC METHODS /////////////////////////////////////////////////////

  /// Constructor.
  GeomCenCA(void);

  /// Destructor
  ~GeomCenCA(void);

  /** Clone geometry.
   *
   * @returns Copy of this
   */
  Geometry* clone(void) const;  

  /** Map global coordinates xyz to etree address. 
   *
   * @warning Level in etree must have been set in address.
   *
   * @param pAddr Pointer to etree address
   * @param lon Longitude of location in degrees
   * @param lat Latitude of location in degrees
   * @param elev Elevation of location wrt MSL in meters
   */
  void lonLatElevToAddr(etree_addr_t* pAddr,
			const double lon,
			const double lat,
			const double elev);
  
  /** Get global coordinates of octant centroid.
   *
   * @param pLon Pointer to longitude of location in degrees
   * @param pLat Pointer to latitude of location in degrees
   * @param pElev Pointer to elevation of location wrt MSL in meters
   * @param pAddr Pointer to etree address
   */
  void addrToLonLatElev(double* pLon,
			double* pLat,
			double* pElev,
			etree_addr_t* pAddr);
  
  /** Get length of octant edge (horizontal direction).
   *
   * @param level Level in etree
   */
  double edgeLen(const etree_tick_t level);

  /** Get level in etree corresponding to horizontal resolution.
   *
   * @param res Spatial resolution
   */
  int level(const double res);

  /** Get vertical exaggeration.
   *
   * @returns Vertical exaggeration
   */
  double vertExag(void);

  /** Get metadata describing model geometry.
   *
   * @returns String containing metadata
   */
  const char* metadata(void);

  /** Get projector associated with geometry.
   *
   * @returns Pointer to projector
   */
  Projector* projector(void);

 private :
  // PRIVATE METHODS ////////////////////////////////////////////////////

  GeomCenCA(const GeomCenCA& g); ///< Not implemented
  const GeomCenCA& operator=(const GeomCenCA& g); ///< Not implemented
  
private :
  // PRIVATE MEMBERS ////////////////////////////////////////////////////

  /// Projected X coordinate of NW corner of root octant
  const double _projXNWRoot;

  /// Projected Y coordinate of NW corner of root octant
  const double _projYNWRoot;

  /// Projector for converting from lon/lat/elev to projected coordinates
  Projector* _pProj;
  
  /// Length of root octant
  static const double _ROOTLEN;

  /// Azimuth of NW to NE edge of octants
  static const double _AZ;

  /// Vertical exaggeration of octants
  static const double _VERTEXAG;

  /// Projected X coordinate of NW corner of detailed region
  static const double _PROJXNW;

  /// Projected Y coordinate of NW corner of detailed region
  static const double _PROJYNW;

  /// Distant to NW from detailed region to NW edge of root octant
  static const double _BUFFERNW;

  /// Distant to SW from detailed region to SW edge of root octant
  static const double _BUFFERSW;

  /// Elevation (wrt MSL) of top of root octant
  static const double _MAXELEV;

  static const char* _METADATA; ///< String describing model geometry

}; // GeomCenCA

#include "GeomCenCA.icc" // inline methods

#endif // cencalvm_storage_geomcenca_h


// End of file 
