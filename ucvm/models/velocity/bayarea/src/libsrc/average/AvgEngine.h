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

/** @file libsrc/average/AvgEngine.h
 *
 * @brief C++ engine for doing averaging over an etree database.
 *
 * This C++ code is based on the C convertdb application written by
 * Julio Lopez (Carnegie Mellon University).
 */

#include <sys/types.h> // USES size_t
#include <iosfwd> // USES std::ostream

#if !defined(cencalvm_average_avgengine_h)
#define cencalvm_average_avgengine_h

#include "cencalvm/storage/etreefwd.h" // HOLDSA etree_t

namespace cencalvm {
  namespace average {
    class AvgEngine;
  } // namespace average
  namespace storage {
    struct PayloadStruct; // USES PayloadStruct
  } // namespace storage
} // namespace cencalvm

/// C++ enginer for doing averaging over an etree database
class cencalvm::average::AvgEngine
{ // Projector

public :
  // PUBLIC METHODS /////////////////////////////////////////////////////

  /** Constructor
   *
   * @param dbOut Output database
   * @param dbIn Input database
   */
  AvgEngine(etree_t* dbOut,
	    etree_t* dbIn);

  /// Destructor
  ~AvgEngine(void);

  /// Fill in octants with averages of their children
  void fillOctants(void);

  /// Print octant counting information to stream.
  void printOctantInfo(void) const;

private :
  // PRIVATE METHODS ////////////////////////////////////////////////////

  AvgEngine(const AvgEngine& e); ///< Not implemented
  const AvgEngine& operator=(const AvgEngine& e); ///< Not implemented
  
  // KLUDGE TO WORK ON SUN
public :
  // PRIVATE STRUCTS ////////////////////////////////////////////////////

  /// Object to hold pending octant data
  struct PendingDataStruct {
    cencalvm::storage::PayloadStruct* pSum; ///< Sum values
    size_t numChildren; ///< Number of children in sum
  }; // PendingDataStruct

private :
  // PRIVATE STRUCTS ////////////////////////////////////////////////////

  struct OctantPendingStruct {
    PendingDataStruct data;
    etree_addr_t* pAddr;
    unsigned char processedChildren;
    bool isValid;
  }; // OctantPendingStruct

  struct CounterStruct {
    uint64_t input;
    uint64_t output;
    uint64_t interior;
    uint64_t inc_x;
    uint64_t inc_y;
    uint64_t inc_z;
    uint64_t inc_xy;
    uint64_t inc_yz;
    uint64_t inc_xz;
    uint64_t inc_xyz;
    uint64_t inc_invalid;
  }; // CounterStruct

private :
  // PRIVATE METHODS ////////////////////////////////////////////////////

  /** Do average processing on octant.
   *
   * @param pAddr Address of octant to process
   * @param payload Payload of octant
   */
  void _averageOctant(etree_addr_t* pAddr,
		      const storage::PayloadStruct& payload);

  /** Add contribution of octant to parent.
   *
   * @param pPendingParent Pointer to parent pending octant
   * @param pAddrChild Pointer to address of child octant
   * @param childPayload Payload of child octant
   */
  void _addToParent(OctantPendingStruct* pPendingParent,
		    etree_addr_t* pAddrChild,
		    const storage::PayloadStruct& childPayload);

  /** Update pending octant.
   *
   * @param pAddr Pointer to octant address
   * @param payload Octant payload
   */
  void _updateOctant(etree_addr_t* pAddr,
		     const storage::PayloadStruct& childPayload);

  /** Process pending octant.
   *
   * @param pendingIndex Index in array of pending octants
   */
  void _processOctant(const int pendingIndex);

  /** Find parent octant.
   *
   * @param pAddr Pointer to octant
   *
   * @returns Index of parent in array of pending octants 
   */
  int _findParent(etree_addr_t* pAddr);

  /// Process any remaining pending octants.
  void _finishProcessing(void);

  /** Create a new octant in the average database and append it to the
   *  array of pending octants.
   *
   * @param pAddr Pointer to address of octant to create
   */
  void _createOctant(etree_addr_t* pAddr);

  /** Get bit associated with child's location relative to parent.
   *
   * @param pAddr Pointer to address of child.
   *
   * @returns Bit of child
   */
  unsigned char _childOctantBit(etree_addr_t* pAddr) const;

  /** Are octant addresses the same.
   *
   * @param pA Pointer to address A
   * @param pB Pointer to address B
   * 
   * @returns True if addresses are the same, false otherwise
   */
  bool _sameAddr(etree_addr_t* pA,
		 etree_addr_t* pB) const;
  
  /** Check consistency of pending octants structure.
   *
   * @param addr Address of current octant
   */
  void _checkPending(etree_addr_t& addr);

private :
  // PRIVATE MEMBERS ////////////////////////////////////////////////////

  etree_t* _dbAvg; ///< Database with averaging
  etree_t* _dbIn; ///< Input database

  OctantPendingStruct* _pPendingOctants; ///< Array of pending octants
  int _pendingSize; ///< Number of pending octants
  int _pendingCursor;

  CounterStruct _octantCounter;

  static const etree_tick_t _LEFTMOSTONE; ///< first bit is 1, others 0

}; // AvgEngine

#endif // cencalvm_average_averager_h

// version
// $Id: AvgEngine.h 3059 2007-02-22 05:33:12Z brad $

// End of file 
