// -*- C++ -*-
//
// ----------------------------------------------------------------------
//
// Brad T. Aagaard, U.S. Geological Survey
//
// ----------------------------------------------------------------------
//

// SWIG interface to C++ CenCalVMDB object.

namespace cencalvm {
  namespace extensions {
    namespace cencalvmdb {
    
      class CenCalVMDB : public spatialdata::spatialdb::SpatialDB
      { // CenCalVMDB
 
	// PUBLIC MEMBERS /////////////////////////////////////////////////
      public :
 
	/// Constructor
	CenCalVMDB(void);
	
	/** Constructor with label.
	 *
	 * @param label Label for database
	 */
	CenCalVMDB(const char* label);
  
	/// Destructor
	~CenCalVMDB(void);
      
	// PUBLIC METHODS /////////////////////////////////////////////////
      public :
      
	/// Open the database and prepare for querying.
	void open(void);
      
	/// Close the database.
	void close(void);
      
	/** Set query type.
	 *
	 * @param queryType Set type of query
	 */
	void queryType(const cencalvm::query::VMQuery::QueryEnum queryType);

	/** Set query resolution. Resolution is associated with vertical
	 * resolution, which is 4 times greater than the horizontal
	 * resolution.
	 *
	 * Meaning depends on type of query:
	 *   @li MAXRES Resolution is not applicable
	 *   @li FIXEDRES Query etree at level corresponding to resolution
	 *   @li WAVERES Resolution corresponds to minimum period of waves;
	 *     etree is queried at level corresponding to minimum wavelength
	 *     for shear waves.
	 *
	 * @param res Resolution of query.
	 */
	void queryRes(const double res);

	/** Set minimum shear-wave speed (clip values lower than this).
	 *
	 * @param vs Shear wave speed
	 */
	void minVs(const double vs);
	
	/** Set values to be returned by queries.
	 *
	 * @pre Must call open() before queryVals()
	 *
	 * @param names Names of values to be returned in queries
	 * @param numVals Number of values to be returned in queries
	 */
	void queryVals(const char* const* names,
		       const int numVals);
	
	/** Set the database filename.
	 *
	 * @param filename Name of database file
	 */
	void filename(const char* filename);
	
	/** Set size of cache during queries.
	 *
	 * @param size Size of cache in MB
	 */
	void cacheSize(const int size);
	
	/** Set the extended database filename.
	 *
	 * @param filename Name of extended database file
	 */
	void filenameExt(const char* filename);
	
	/** Set size of cache during queries of extended database
	 *
	 * @param size Size of cache in MB
	 */
	void cacheSizeExt(const int size);
	
	/** Set squashed topography/bathymetry flag and minimum elevation of
	 * squashing.
	 *
	 * @param flag True if squashing, false otherwise.
	 * @param limit Minimum elevation for squashing.
	 */
	void squash(const bool flag,
		    const double limit =-2000.0);
	
	/** Query the database.
	 *
	 * @pre Must call open() before query()
	 *
	 * @param pVals Array for computed values (output from query)
	 * @param numVals Number of values expected (size of pVals array)
	 * @param coords Coordinates of location for query
	 * @param numDims Number of dimensions for location
	 * @param pCSQuery Coordinate system of coordinates
	 *
	 * @returns 0 on success, 1 on failure (i.e., values not set)
	 */
	int query(double* pVals,
		  const int numVals,
		  const double* coords,
		  const int numDims,
		  const spatialdata::geocoords::CoordSys* pCSQuery);
	
      }; // CenCalVMDB
    } // cencalvmdb
  } // extensions
} // cencalvm


// End of file
