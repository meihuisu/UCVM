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

/** @file libsrc/storage//etreefwd.h
 *
 * @brief Forward declarations for etree API.
 */

#if !defined(cencalvm_storage_etreefwd_h)
#define cencalvm_storage_etreefwd_h

#include <inttypes.h> // USES uint32_t

typedef struct etree_addr_t etree_addr_t; ///< etree address
typedef struct etree_t etree_t; ///< etree database
typedef uint32_t etree_tick_t; ///< etree coordinate

#endif // cencalvm_storage_etreefwd_h

// version
// $Id: etreefwd.h 3059 2007-02-22 05:33:12Z brad $

// End of file 
