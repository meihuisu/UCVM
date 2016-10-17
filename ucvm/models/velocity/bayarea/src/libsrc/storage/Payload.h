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

/** @file libsrc/storage/Payload.h
 *
 * @brief C++ manager of data structures for velocity model.
 */

#if !defined(cencalvm_storage_payload_h)
#define cencalvm_storage_payload_h

#include <inttypes.h>

namespace cencalvm {
  namespace storage {
    struct PayloadStruct;
    class Payload;
  } // namespace storage
} // namespace cencalvm

/// Data stored in velocity model database.
struct cencalvm::storage::PayloadStruct {
  float Vp; ///< P wave speed in m/s
  float Vs; ///< S wave speed in m/s
  float Density; ///< Density in kg/m^3
  float Qp; ///< Q for P waves
  float Qs; ///< Q for S waves
  float DepthFreeSurf; ///< Depth wrt free surface (burial depth) in m

  /** Fault block identifier.
   *
   * @li Set to fault block for location in Earth Vision model
   * @li Set to 0 for locations outside the domain.
   * @li Set to INTERIORBLOCK for interior octants that contain values
   *   that are the average of its children.
   */
  int16_t FaultBlock;

  /** Zone identifier. 
   *
   * @li Set to zone for location in Earth Vision model
   * @li Set to 0 for locations outside the domain.
   * @li Set to INTERIORZONE for interior octants that contain values 
   *   that are the averages of its children.
   */
  int16_t Zone;
}; // struct PayloadStruct

/// C++ manager of data structures for velocity model.
class cencalvm::storage::Payload
{ // Payload
public :
  // PUBLIC MEMBERS /////////////////////////////////////////////////////

  static const char* SCHEMA; ///< Database schema

  static const float NODATAVAL; ///< Database flags for no data
  static const short NODATABLOCK; ///< Database flags for no data
  static const short NODATAZONE; ///< Database flags for no data
  
  static const short INTERIORBLOCK; ///< Database flags for interior octants
  static const short INTERIORZONE; ///< Database flags for interior octants

}; // Payload

#endif // cencalvm_storage_payload_h

// version
// $Id: Payload.h 5645 2011-05-26 19:50:37Z brad $

// End of file 
