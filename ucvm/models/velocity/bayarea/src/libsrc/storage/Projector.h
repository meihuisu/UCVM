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

/** @file libsrc/storage/Projector.h
 *
 * @brief C++ manager for projecting to/from 3-D geologic
 * model.
 */

#if !defined(cencalvm_storage_projector_h)
#define cencalvm_storage_projector_h

#include "proj4fwd.h" // HOLDSA projPJ

namespace cencalvm {
  namespace storage {
    class Projector;
  } // namespace storage
} // namespace cencalvm

/// C++ manager for projecting to/from 3-D geologic model.
class cencalvm::storage::Projector
{ // Projector

public :
  // PUBLIC METHODS /////////////////////////////////////////////////////

  /// Constructor.
  Projector(void);

  /// Destructor
  ~Projector(void);

  /** Apply projection to geographic coordinates.
   *
   * @param pX Pointer to projected X coordinate
   * @param pY Pointer to projected Y coordinate
   * @param lon Longitude of location in degrees
   * @param lat Latitude of location in degrees
   */
  void project(double* pX,
	       double* pY,
	       const double lon,
	       const double lat) const;

  /** Apply inverse projection.
   *
   * @param pLon Pointer to longitude of location in degrees
   * @param pLat Pointer to latitude of location in degrees
   * @param x Projected X coordinate
   * @param y Projected Y coordinate
   */
  void invProject(double* pLon,
		  double* pLat,
		  const double x,
		  const double y) const;

 private :
  // PRIVATE METHODS ////////////////////////////////////////////////////

  Projector(const Projector& p); ///< Not implemented
  const Projector& operator=(const Projector& p); ///< Not implemented
  
private :
  // PRIVATE MEMBERS ////////////////////////////////////////////////////

  
  projPJ _pProj; ///< Handle to Proj4 projection

  static const double _MERIDIAN; ///< Longitude of central meridian for proj
  static const double _LAT; ///< Latitude of origin for projection
  static const double _SCALE; ///< Scale factor for projection
  static const char* _PROJECTION; ///< Name of projection
  static const char* _DATUM; ///< Name of horizontal datum
  static const char* _UNITS; ///< Name of units used in projection

}; // Projector

#endif // cencalvm_storage_projector_h

// version
// $Id: Projector.h 3059 2007-02-22 05:33:12Z brad $

// End of file 
