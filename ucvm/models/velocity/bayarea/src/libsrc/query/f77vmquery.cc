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

#include <portinfo>

#include <sys/types.h> // USES size_t

#include "f77vmquery.h"

extern "C" {
#include "cvmquery.h"
}

#include <sstream> // USES std::istringstream
#include <string.h> // USE strncpy
#include <assert.h> // USES assert()

// ----------------------------------------------------------------------
// Create velocity model query object.
void
cencalvm_createquery_f(size_t* handle)
{ // createquery
  assert(0 != handle);

  *handle = (size_t) cencalvm_createQuery();
} // createquery

// ----------------------------------------------------------------------
// Destroy velocity model query object.
void
cencalvm_destroyquery_f(size_t* handleAddr,
			int* err)
{ // destroyQuery
  assert(0 != err);

  *err = cencalvm_destroyQuery((void*) *handleAddr);
} // destroyQuery

// ----------------------------------------------------------------------
// Open database for querying.
void
cencalvm_open_f(size_t* handleAddr,
		int* err)
{ // open
  assert(0 != err);

  *err = cencalvm_open((void*) *handleAddr);
} // open
  
// ----------------------------------------------------------------------
// Close the database.
void
cencalvm_close_f(size_t* handleAddr,
		 int* err)
{ // close
  assert(0 != err);

  *err = cencalvm_close((void*) *handleAddr);
} // close
  
// ----------------------------------------------------------------------
// Set query type.
void
cencalvm_querytype_f(size_t* handleAddr,
		     const int* queryType,
		     int* err)
{ // queryType
  assert(0 != err);

  *err = cencalvm_queryType((void*) *handleAddr, *queryType);
} // queryType

// ----------------------------------------------------------------------
// Set query resolution.
void
cencalvm_queryres_f(size_t* handleAddr,
		    const double* res,
		    int* err)
{ // queryRes
  assert(0 != err);

  *err = cencalvm_queryRes((void*) *handleAddr, *res);
} // queryRes

// ----------------------------------------------------------------------
// Set the database filename.
void
cencalvm_filename_f(size_t* handleAddr,
		    const char* filename,
		    int* err,
		    const int len)
{ // filename
  assert(0 != err);
  assert(0 != filename);
  assert(len > 0);

  std::istringstream sin(filename);
  std::string cfilename;
  sin >> cfilename;
  *err = cencalvm_filename((void*) *handleAddr, cfilename.c_str());
} // filename

// ----------------------------------------------------------------------
// Set size of cache during queries.
void
cencalvm_cachesize_f(size_t* handleAddr,
		     const int* size,
		     int* err)
{ // cacheSize
  assert(0 != err);

  *err = cencalvm_cacheSize((void*) *handleAddr, *size);
} // cacheSize

// ----------------------------------------------------------------------
// Set the extended database filename.
void
cencalvm_filenameext_f(size_t* handleAddr,
		       const char* filename,
		       int* err,
		       const int len)
{ // filenameExt
  assert(0 != err);
  assert(0 != filename);
  assert(len > 0);

  std::istringstream sin(filename);
  std::string cfilename;
  sin >> cfilename;
  *err = cencalvm_filenameExt((void*) *handleAddr, cfilename.c_str());
} // filenameExt

// ----------------------------------------------------------------------
// Set size of cache during queries.
void
cencalvm_cachesizeext_f(size_t* handleAddr,
			const int* size,
			int* err)
{ // cacheSizeExt
  assert(0 != err);

  *err = cencalvm_cacheSizeExt((void*) *handleAddr, *size);
} // cacheSizeExt

// ----------------------------------------------------------------------
// Set squashed topography/bathymetry flag and minimum elevation of
// squashing. Squashing is turned off by default.
void
cencalvm_squash_f(size_t* handleAddr,
		  const int* flag,
		  const double* limit,
		  int* err)
{ // squash
  assert(0 != err);

  *err = cencalvm_squash((void*) *handleAddr, *flag, *limit);
} // squash

// ----------------------------------------------------------------------
// Query the database.
void
cencalvm_query_f(size_t* handleAddr,
		 double* pVals,
		 const int* numVals,
		 const double* lon,
		 const double* lat,
		 const double* elev,
		 int *err)
{ // query
  assert(0 != err);

  *err = cencalvm_query((void*) *handleAddr, &pVals, *numVals, 
			*lon, *lat, *elev);
} // query

// ----------------------------------------------------------------------
// Get handle to error handler.
void
cencalvm_errorhandler_f(size_t* handleAddr,
			size_t* errAddr)
{ // errorHandler
  assert(0 != errAddr);

  *errAddr = (size_t) cencalvm_errorHandler((void*) *handleAddr);
} // errorHandler


// End of file
