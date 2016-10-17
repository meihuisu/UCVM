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

/** @file libsrc/query/cvmquery.h
 *
 * @brief C interface definitions for querying the USGS central CA velocity
 * model (USER INTERFACE).
 *
 * These are the routines that are used to query the USGS central CA
 * velocity model from C. See f77vmquery.h for the routines to query
 * the velocity model using Fortran via Fortran 77. If you are using
 * C++, you should use the VMQuery object directly.
 *
 * The current defaults for queries are:
 * @li return all values in a query,
 * @li query at the maximum resolution of the model
 * @li 128 MB cache for queries
 *
 * The default behavior can be modified by calling the appropriate
 * class method, e.g., cencalvm_queryType(), cencalvm_cacheSize(),
 * cencalvm_queryVals().
 *
 * The general order of use is:
 *
 * <ol>
 * <li> Create query object using cencalvm_createQuery()
 * <li> Set filename of database using cencalvm_filename()
 * <li> Optionally, set cache size using cencalvm_cacheSize()
 * <li> Optionally, set values to returnin query using cencalvm_queryVals()
 * <li> Open database using cencalvm_open()
 * <li> Query database using cencalvm_query()
 * <li> Close database using cencalvm_close()
 * <li> Destroy query object using cencalvm_destroyQuery()
 * </ol>
 *
 * The values associated with the error status come from the status
 * of the error handler. See cencalvm::storage::ErrorHandler for what
 * the values mean.
 */

#if !defined(cencalvm_cvmquery_h)
#define cencalvm_cvmquery_h

/** Create velocity model query object.
 *
 * Calls VMQuery().
 *
 * @returns pointer to VMQuery object (0 on failure)
 */
void* cencalvm_createQuery(void);

/** Destroy velocity model query object.
 *
 * Calls ~VMQuery().
 *
 * @param handle Pointer to query
 *
 * @returns Status of error handler
 */
int cencalvm_destroyQuery(void* handle);

/** Open database for querying.
 *
 * @param handle Pointer to query
 *
 * @returns Status of error handler
 */
int cencalvm_open(void* handle);
  
/** Close the database.
 *
 * @param handle Pointer to query
 *
 * @returns Status of error handler
 */
int cencalvm_close(void* handle);
  
/** Set query type.
 *
 * Queries other than maximum resolution require an etree database
 * that has been spatially averaged. This fills in parent octants with
 * the average of its children. 
 *
 * @param handle Pointer to query
 * @param queryType Type of query
 *   @li =0  Query at maximum resolution
 *   @li =1  Query at fixed resolution
 *   @li =2  Query at resolution tuned to wavelength of shear waves
 *
 * @returns Status of error handler
 */
int cencalvm_queryType(void* handle,
		       const int queryType);

/** Set query resolution.
 *
 * Meaning depends on type of query:
 *   @li MAXRES Resolution is not applicable
 *   @li FIXEDRES Query etree at level corresponding to resolution
 *   @li AVGRES Resolution corresponds to minimum period of waves;
 *     etree is queried at lavel corresponding to minimum wavelength
 *     for shear waves
 *
 * @param handle Pointer to query
 * @param res Resolution of query.
 *
 * @returns Status of error handler
 */
int cencalvm_queryRes(void* handle,
		      const double res);

/** Set values to be returned by queries. Default is to return all
 * values in order they appear in PayloadStruct plus the elevation of
 * topography/bathymetry (ground surface or ocean floor; the query
 * value is 'elevation').
 *
 * @param handle Pointer to query
 * @param names Names of values to be returned in queries
 * @param numVals Number of values to be returned in queries
 *
 * @returns Status of error handler
 */
int cencalvm_queryVals(void* handle,
		       const char* names[],
		       const int numVals);

/** Set the database filename.
 *
 * @param handle Pointer to query
 * @param filename Name of database file
 *
 * @returns Status of error handler
 */
int cencalvm_filename(void* handle,
		      const char* filename);

/** Set size of cache during queries.
 *
 * @param handle Pointer to query
 * @param size Size of cache in MB
 *
 * @returns Status of error handler
 */
int cencalvm_cacheSize(void* handle,
		       const int size);

/** Set the extended database filename.
 *
 * @param handle Pointer to query
 * @param filename Name of database file
 *
 * @returns Status of error handler
 */
int cencalvm_filenameExt(void* handle,
			 const char* filename);

/** Set size of cache during queries of extended database.
 *
 * @param handle Pointer to query
 * @param size Size of cache in MB
 *
 * @returns Status of error handler
 */
int cencalvm_cacheSizeExt(void* handle,
			  const int size);

/** Set squashed topography/bathymetry flag and minimum elevation of
 * squashing. Squashing is turned off by default.
 *
 * @param handle Pointer to query
 * @param flag True if squashing, false otherwise.
 * @param limit Minimum elevation for squashing.
 */
int cencalvm_squash(void* handle,
		    const int flag,
		    const double limit);

/** Query the database.
 *
 * @warning Array for values to be returned must be allocated BEFORE
 * query.
 *
 * @note Longitude and latitude are given in degrees in the WGS84 datum.
 *
 * @note Elevation is given in meters with respect to mean sea level.
 *
 * @note Values will be returned either of the order they are shown
 * in cencalvm::storage::PayloadStruct or as specified with a call
 * to queryVals().
 *
 * @param handle Pointer to query
 * @param ppVals Pointer to array holding computed values (output from query)
 * @param numVals Number of values expected (size of array (preallocated))
 * @param lon Longitude of location for query in degrees
 * @param lat Latitude of location for query in degrees
 * @param elev Elevation of location wrt MSL in meters
 *
 * @returns Status of error handler
 */
int cencalvm_query(void* handle,
		   double** ppVals,
		   const int numVals,
		   const double lon,
		   const double lat,
		   const double elev);

/** Get handle to error handler.
 *
 * @param handle Pointer to query
 *
 * @returns pointer to ErrorHandler object (0 on failure)
 */
void* cencalvm_errorHandler(void* handle);

#endif /* cencalvm_cvmquery_h */


/* End of file  */
