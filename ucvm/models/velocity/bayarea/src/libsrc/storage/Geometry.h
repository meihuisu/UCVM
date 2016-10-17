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

/** @file libsrc/storage/Geometry.h
 *
 * @brief C++ manager for geometry of the USGS central CA velocity
 * model.
 */

#if !defined(cencalvm_storage_geometry_h)
#define cencalvm_storage_geometry_h

#include "etreefwd.h" // USES etree types

namespace cencalvm {
  namespace storage {
    class Geometry;
    class Projector; // USES Projector
  } // namespace storage
} // namespace cencalvm

/// C++ manager for geometry of the USGS central CA velocity
/// model.
class cencalvm::storage::Geometry
{ // Geometry
public :
  // PUBLIC METHODS /////////////////////////////////////////////////////

  /// Constructor.
  Geometry(void);

  /// Destructor
  virtual ~Geometry(void);

  /** Clone geometry.
   *
   * @returns Copy of this
   */
  virtual Geometry* clone(void) const = 0;

  /** Map global coordinates xyz to etree address. 
   *
   * @warning Level in etree must have been set in address.
   *
   * @param pAddr Pointer to etree address
   * @param lon Longitude of location in degrees
   * @param lat Latitude of location in degrees
   * @param elev Elevation of location wrt MSL in meters
   */
  virtual void lonLatElevToAddr(etree_addr_t* pAddr,
				const double lon,
				const double lat,
				const double elev) = 0;
  
  /** Get global coordinates of octant centroid.
   *
   * @param pLon Pointer to longitude of location in degrees
   * @param pLat Pointer to latitude of location in degrees
   * @param pElev Pointer to elevation of location wrt MSL in meters
   * @param pAddr Pointer to etree address
   */
  virtual void addrToLonLatElev(double* pLon,
				double* pLat,
				double* pElev,
				etree_addr_t* pAddr) = 0;
  
  /** Get length of octant edge (horizontal direction).
   *
   * @param level Level in etree
   */
  virtual double edgeLen(const etree_tick_t level) = 0;

  /** Get level in etree corresponding to horizontal resolution.
   *
   * @param res Spatial resolution
   */
  virtual int level(const double res) = 0;

  /** Get vertical exaggeration.
   *
   * @returns Vertical exaggeration
   */
  virtual double vertExag(void) = 0;

  /** Get projector associated with geometry.
   *
   * @returns Pointer to projector
   */
  virtual Projector* projector(void) = 0;

  /** Get metadata describing model geometry.
   *
   * @returns String containing metadata
   */
  virtual const char* metadata(void) = 0;

  /** Compute address of ancestor at specified level of given octant.
   *
   * @param pAncestorAddr Pointer to address of ancestor octant
   * @param childAddr Address of current octant
   * @param ancestorLevel Level of ancestor octant
   */
  static void findAncestor(etree_addr_t* pAncestorAddr,
			   const etree_addr_t& childAddr,
			   const int ancestorLevel);

 private :
  // PRIVATE METHODS ////////////////////////////////////////////////////

  Geometry(const Geometry& g); ///< Not implemented
  const Geometry& operator=(const Geometry& g); ///< Not implemented
  
protected :
  // PROTECTED MEMBERS //////////////////////////////////////////////////

  static const etree_tick_t _LEFTMOSTONE; ///< first bit is 1, others 0

}; // Geometry

#endif // cencalvm_storage_geometry_h

// version
// $Id: Geometry.h 3059 2007-02-22 05:33:12Z brad $

// End of file 
