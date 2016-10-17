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

/** @file libsrc/storage/proj4fwd.h
 *
 * @brief Forward declarations for proj4 API.
 */

#if !defined(cencalvm_storage_proj4fwd_h)
#define cencalvm_storage_proj4fwd_h

/// forward declaration of projection
#if !defined(PROJECTS_H)
typedef void *projPJ;
#else
typedef PJ *projPJ;
#endif

#endif // cencalvm_storage_proj4fwd_h

