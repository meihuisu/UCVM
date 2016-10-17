/*  -*- C -*-  */
/*
 * ======================================================================
 *
 *                           Brad T. Aagaard
 *                        U.S. Geological Survey
 *
 * {LicenseText}
 *
 * ======================================================================
 */

/** @file libsrc/query/f77vmquery.h
 *
 * @brief Fortran 77 interface definitions for querying the USGS
 * central CA velocity model (USER INTERFACE).
 *
 * These are the routines that are used to query the USGS central CA
 * velocity model from Fortran 77. See cvmquery.h for the routines to
 * query the velocity model using C. If you are using C++, you should
 * use the VMQuery object directly.
 *
 * The current defaults for queries are:
 * @li return all values in a query,
 * @li query at the maximum resolution of the model
 * @li 128 MB cache for queries
 *
 * The default behavior can be modified by calling the appropriate
 * class method, e.g., cencalvm_querytype_f() and cencalvm_cachesize_f().
 *
 * The general order of use is:
 *
 * <ol>
 * <li> Create query object using cencalvm_createquery_f()
 * <li> Set filename of database using cencalvm_filename_f()
 * <li> Optionally, set cache size using cencalvm_cachesize_f()
 * <li> Open database using cencalvm_open_f()
 * <li> Query database using cencalvm_query_f()
 * <li> Close database using cencalvm_close_f()
 * <li> Destroy query object using cencalvm_destroyquery_f()
 * </ol>
 *
 * The values associated with the error status come from the status of
 * the error handler. See cencalvm::storage::ErrorHandler for what the
 * values mean.
 */

#if !defined(cencalvm_f77vmquery_h)
#define cencalvm_f77vmquery_h ///< include protection

// ----------------------------------------------------------------------
/** Fortran name mangling */
#define cencalvm_createquery_f \
  FC_FUNC_(cencalvm_createquery_f, CENCALVM_CREATEQUERY_F)
/** Create velocity model query object.
 *
 * Create handle to velocity model query object. If creation fails
 * address of handle is set to 0.
 *
 * @param handleAddr Address of handle to VMQuery object.
 */
extern "C"
void cencalvm_createquery_f(size_t* handleAddr);

// ----------------------------------------------------------------------
/** Fortran name mangling */
#define cencalvm_destroyquery_f \
  FC_FUNC_(cencalvm_destroyquery_f, CENCALVM_DESTROYQUERY_F)
/** Destroy velocity model query object.
 *
 * Calls ~VMQuery().
 *
 * @param handleAddr Address of handle to VMQuery object
 * @param err Set to status of error handler
 */
extern "C"
void cencalvm_destroyquery_f(size_t* handleAddr,
			     int* err);

// ----------------------------------------------------------------------
/** Fortran name mangling */
#define cencalvm_open_f \
  FC_FUNC_(cencalvm_open_f, CENCALVM_OPEN_F)
/** Open database for querying.
 *
 * @param handleAddr Address of handle to VMQuery object
 * @param err Set to status of error handler
 */
extern "C"
void cencalvm_open_f(size_t* handleAddr,
		     int* err);
  
// ----------------------------------------------------------------------
/** Fortran name mangling */
#define cencalvm_close_f \
  FC_FUNC_(cencalvm_close_f, CENCALVM_CLOSE_F)
/** Close the database.
 *
 * @param handleAddr Address of handle to VMQuery object
 * @param err Set to status of error handler
 */
extern "C"
void cencalvm_close_f(size_t* handleAddr,
		      int* err);

// ----------------------------------------------------------------------
/** Fortran name mangling */
#define cencalvm_querytype_f \
  FC_FUNC_(cencalvm_querytype_f, CENCALVM_QUERYTYPE_F)
/** Set query type.
 *
 * Queries other than maximum resolution require an etree database
 * that has been spatially averaged. This fills in parent octants with
 * the average of its children. 
 *
 * @param handleAddr Address of handle to VMQuery object
 * @param queryType Type of query
 *   @li =0  Query at maximum resolution
 *   @li =1  Query at fixed resolution
 *   @li =2  Query at resolution tuned to wavelength of shear waves
 * @param err Set to status of error handler
 */
extern "C"
void cencalvm_querytype_f(size_t* handleAddr,
			  const int* queryType,
			  int* err);

// ----------------------------------------------------------------------
/** Fortran name mangling */
#define cencalvm_queryres_f \
  FC_FUNC_(cencalvm_queryres_f, CENCALVM_QUERYRES_F)
/** Set query resolution.
 *
 * Meaning depends on type of query:
 *   @li MAXRES Resolution is not applicable
 *   @li FIXEDRES Query etree at level corresponding to resolution
 *   @li AVGRES Resolution corresponds to minimum period of waves;
 *     etree is queried at lavel corresponding to minimum wavelength
 *     for shear waves
 *
 * @param handleAddr Address of handle to VMQuery object
 * @param res Resolution of query.
 * @param err Set to status of error handler
 */
extern "C"
void cencalvm_queryres_f(size_t* handleAddr,
			 const double* res,
			 int* err);

// ----------------------------------------------------------------------
/** Fortran name mangling */
#define cencalvm_filename_f \
  FC_FUNC_(cencalvm_filename_f, CENCALVM_FILENAME_F)
/** Set the database filename.
 *
 * @param handleAddr Address of handle to VMQuery object
 * @param filename Name of database file
 * @param len Length of string (IMPLICIT IN FORTRAN)
 * @param err Set to status of error handler
 */
extern "C"
void cencalvm_filename_f(size_t* handleAddr,
			 const char* filename,
			 int* err,
			 const int len);

// ----------------------------------------------------------------------
/** Fortran name mangling */
#define cencalvm_cachesize_f \
  FC_FUNC_(cencalvm_cachesize_f, CENCALVM_CACHESIZE_F)
/** Set size of cache during queries.
 *
 * @param handleAddr Address of handle to VMQuery object
 * @param size Size of cache in MB
 * @param err Set to status of error handler
 */
extern "C"
void cencalvm_cachesize_f(size_t* handleAddr,
			  const int* size,
			  int* err);

// ----------------------------------------------------------------------
/** Fortran name mangling */
#define cencalvm_filenameext_f \
  FC_FUNC_(cencalvm_filenameext_f, CENCALVM_FILENAMEEXT_F)
/** Set the extended database filename.
 *
 * @param handleAddr Address of handle to VMQuery object
 * @param filename Name of database file
 * @param len Length of string (IMPLICIT IN FORTRAN)
 * @param err Set to status of error handler
 */
extern "C"
void cencalvm_filenameext_f(size_t* handleAddr,
			    const char* filename,
			    int* err,
			    const int len);

// ----------------------------------------------------------------------
/** Fortran name mangling */
#define cencalvm_cachesizeext_f \
  FC_FUNC_(cencalvm_cachesizeext_f, CENCALVM_CACHESIZEEXT_F)
/** Set size of cache during queries.
 *
 * @param handleAddr Address of handle to VMQuery object
 * @param size Size of cache in MB
 * @param err Set to status of error handler
 */
extern "C"
void cencalvm_cachesizeext_f(size_t* handleAddr,
			     const int* size,
			     int* err);

// ----------------------------------------------------------------------
/** Fortran name mangling */
#define cencalvm_squash_f \
  FC_FUNC_(cencalvm_squash_f, CENCALVM_SQUASH_F)
/** Set squashed topography/bathymetry flag and minimum elevation of
 * squashing. Squashing is turned off by default.
 *
 * @param handleAddr Address of handle to VMQuery object
 * @param flag True if squashing, false otherwise.
 * @param limit Minimum elevation for squashing.
 * @param err Set to status of error handler
 */
extern "C"
void cencalvm_squash_f(size_t* handleAddr,
		       const int* flag,
		       const double* limit,
		       int* err);

// ----------------------------------------------------------------------
/** Fortran name mangling */
#define cencalvm_query_f \
  FC_FUNC_(cencalvm_query_f, CENCALVM_QUERY_F)
/** Query the database.
 *
 * @warning Array for values to be returned must be allocated BEFORE
 * query.
 *
 * @note Longitude and latitude are given in degrees in the WGS84 datum.
 *
 * @note Elevation is given in meters with respect to mean sea level.
 *
 * @note Values returned include the values in
 * cencalvm::storage::PayloadStruct plus the elevation of
 * topography/bathymetry (ground surface or ocean floor; the query
 * value is 'elevation').
 *
 * @param handleAddr Address of handle to VMQuery object
 * @param pVals Pointer to computed values (output from query)
 * @param numVals Number of values expected (size of array (preallocated))
 * @param lon Longitude of location for query in degrees
 * @param lat Latitude of location for query in degrees
 * @param elev Elevation of location wrt MSL in meters
 * @param err Set to status of error handler
 */
extern "C"
void cencalvm_query_f(size_t* handleAddr,
		      double* pVals,
		      const int* numVals,
		      const double* lon,
		      const double* lat,
		      const double* elev,
		      int* err);

// ----------------------------------------------------------------------
/** Fortran name mangling */
#define cencalvm_errorhandler_f \
  FC_FUNC_(cencalvm_errorhandler_f, CENCALVM_ERRORHANDLER_F)
/** Get handle to error handler.
 *
 * Create handle to velocity model query object. If creation fails
 * address of handle is set to 0.
 *
 * @param handleAddr Address of handle to VMQuery object.
 * @param errAddr Address of handle to ErrorHandler object.
 */
extern "C"
void cencalvm_errorhandler_f(size_t* handleAddr,
			     size_t* errAddr);

#endif // cencalvm77vmquery_h


// End of file
