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

extern "C" {
#include "cvmquery.h"
}

#include "cencalvm/query/VMQuery.h" // USES VMQuery
#include "cencalvm/storage/ErrorHandler.h" // USES ErrorHandler

#include <stdexcept> // USES std::exception
#include <iostream> // USES std::cerr

// ----------------------------------------------------------------------
// Create velocity model query object.
void*
cencalvm_createQuery(void)
{ // createQuery
  void* handle = (void*) new cencalvm::query::VMQuery;
  return handle;
} // createQuery

// ----------------------------------------------------------------------
// Destroy velocity model query object.
int
cencalvm_destroyQuery(void* handle)
{ // destroyQuery
  cencalvm::query::VMQuery* pQuery = (cencalvm::query::VMQuery*) handle;

  delete pQuery; pQuery = 0;

  return cencalvm::storage::ErrorHandler::OK;
} // destroyQuery

// ----------------------------------------------------------------------
// Open database for querying.
int
cencalvm_open(void* handle)
{ // open
  if (0 == handle) {
    std::cerr << "Null handle for query manager in call to open()."
	      << std::endl;
    return cencalvm::storage::ErrorHandler::ERROR;
  } // if

  cencalvm::query::VMQuery* pQuery = (cencalvm::query::VMQuery*) handle;
  pQuery->open();

  const cencalvm::storage::ErrorHandler* pErrHandler = pQuery->errorHandler();
  return pErrHandler->status();
} // open
  
// ----------------------------------------------------------------------
// Close the database.
int
cencalvm_close(void* handle)
{ // close
  if (0 == handle) {
    std::cerr << "Null handle for query manager in call to close()."
	      << std::endl;
    return cencalvm::storage::ErrorHandler::ERROR;
  } // if

  cencalvm::query::VMQuery* pQuery = (cencalvm::query::VMQuery*) handle;
  pQuery->close();

  const cencalvm::storage::ErrorHandler* pErrHandler = pQuery->errorHandler();
  return pErrHandler->status();
} // close
  
// ----------------------------------------------------------------------
// Set query type.
int
cencalvm_queryType(void* handle,
		   const int queryType)
{ // queryType
  if (0 == handle) {
    std::cerr << "Null handle for query manager in call to queryType()."
	      << std::endl;
    return cencalvm::storage::ErrorHandler::ERROR;
  } // if

  cencalvm::query::VMQuery* pQuery = (cencalvm::query::VMQuery*) handle;
  cencalvm::query::VMQuery::QueryEnum queryEnum = 
    cencalvm::query::VMQuery::QueryEnum(queryType);
  pQuery->queryType(queryEnum);

  const cencalvm::storage::ErrorHandler* pErrHandler = pQuery->errorHandler();
  return pErrHandler->status();
} // queryType

// ----------------------------------------------------------------------
// Set query resolution.
int
cencalvm_queryRes(void* handle,
		  const double res)
{ // queryRes
  if (0 == handle) {
    std::cerr << "Null handle for query manager in call to queryRes()."
	      << std::endl;
    return cencalvm::storage::ErrorHandler::ERROR;
  } // if

  cencalvm::query::VMQuery* pQuery = (cencalvm::query::VMQuery*) handle;
  pQuery->queryRes(res);

  const cencalvm::storage::ErrorHandler* pErrHandler = pQuery->errorHandler();
  return pErrHandler->status();
} // queryRes

// ----------------------------------------------------------------------
// Set values to be returned by queries. Default is to return all
int 
cencalvm_queryVals(void* handle,
		   const char* names[],
		   const int numVals)
{ // queryVals
  if (0 == handle) {
    std::cerr << "Null handle for query manager in call to queryVals()."
	      << std::endl;
    return cencalvm::storage::ErrorHandler::ERROR;
  } // if

  cencalvm::query::VMQuery* pQuery = (cencalvm::query::VMQuery*) handle;
  pQuery->queryVals(names, numVals);

  const cencalvm::storage::ErrorHandler* pErrHandler = pQuery->errorHandler();
  return pErrHandler->status();
} // queryVals

// ----------------------------------------------------------------------
// Set the database filename.
int
cencalvm_filename(void* handle,
		   const char* filename)
{ // filename
  if (0 == handle) {
    std::cerr << "Null handle for query manager in call to filename()."
	      << std::endl;
    return cencalvm::storage::ErrorHandler::ERROR;
  } // if

  cencalvm::query::VMQuery* pQuery = (cencalvm::query::VMQuery*) handle;
  pQuery->filename(filename);

  const cencalvm::storage::ErrorHandler* pErrHandler = pQuery->errorHandler();
  return pErrHandler->status();
} // filename

// ----------------------------------------------------------------------
// Set size of cache during queries.
int
cencalvm_cacheSize(void* handle,
		   const int size)
{ // cacheSize
  if (0 == handle) {
    std::cerr << "Null handle for query manager in call to cacheSize()."
	      << std::endl;
    return cencalvm::storage::ErrorHandler::ERROR;
  } // if

  cencalvm::query::VMQuery* pQuery = (cencalvm::query::VMQuery*) handle;
  pQuery->cacheSize(size);

  const cencalvm::storage::ErrorHandler* pErrHandler = pQuery->errorHandler();
  return pErrHandler->status();
} // cacheSize

// ----------------------------------------------------------------------
// Set the extended database filename.
int
cencalvm_filenameExt(void* handle,
		     const char* filename)
{ // filenameExt
  if (0 == handle) {
    std::cerr << "Null handle for query manager in call to filename()."
	      << std::endl;
    return cencalvm::storage::ErrorHandler::ERROR;
  } // if

  cencalvm::query::VMQuery* pQuery = (cencalvm::query::VMQuery*) handle;
  pQuery->filenameExt(filename);

  const cencalvm::storage::ErrorHandler* pErrHandler = pQuery->errorHandler();
  return pErrHandler->status();
} // filenameExt

// ----------------------------------------------------------------------
// Set size of cache during queries of extended database
int
cencalvm_cacheSizeExt(void* handle,
		      const int size)
{ // cacheSizeExt
  if (0 == handle) {
    std::cerr << "Null handle for query manager in call to cacheSize()."
	      << std::endl;
    return cencalvm::storage::ErrorHandler::ERROR;
  } // if

  cencalvm::query::VMQuery* pQuery = (cencalvm::query::VMQuery*) handle;
  pQuery->cacheSizeExt(size);

  const cencalvm::storage::ErrorHandler* pErrHandler = pQuery->errorHandler();
  return pErrHandler->status();
} // cacheSizeExt

// ----------------------------------------------------------------------
// Set squashed topography/bathymetry flag and minimum elevation of
// squashing. Squashing is turned off by default.
int
cencalvm_squash(void* handle,
		const int flag,
		const double limit)
{ // squash
  if (0 == handle) {
    std::cerr << "Null handle for query manager in call to squash()."
	      << std::endl;
    return cencalvm::storage::ErrorHandler::ERROR;
  } // if

  cencalvm::query::VMQuery* pQuery = (cencalvm::query::VMQuery*) handle;
  pQuery->squash(flag, limit);

  const cencalvm::storage::ErrorHandler* pErrHandler = pQuery->errorHandler();
  return pErrHandler->status();
} // squash

// ----------------------------------------------------------------------
// Query the database.
int
cencalvm_query(void* handle,
	       double** ppVals,
	       const int numVals,
	       const double lon,
	       const double lat,
	       const double elev)
{ // query
  if (0 == handle) {
    std::cerr << "Null handle for query manager in call to query()."
	      << std::endl;
    return cencalvm::storage::ErrorHandler::ERROR;
  } // if

  cencalvm::query::VMQuery* pQuery = (cencalvm::query::VMQuery*) handle;
  pQuery->query(ppVals, numVals, lon, lat, elev);

  const cencalvm::storage::ErrorHandler* pErrHandler = pQuery->errorHandler();
  return pErrHandler->status();
} // query

// ----------------------------------------------------------------------
// Get handle to error handler.
void*
cencalvm_errorHandler(void* handle)
{ // errorHandler
  if (0 == handle) {
    std::cerr << "Null handle for query manager in call to errorHandler()."
	      << std::endl;
    return 0;
  } // if

  cencalvm::query::VMQuery* pQuery = (cencalvm::query::VMQuery*) handle;
  return pQuery->errorHandler();
} // errorHandler


// End of file
