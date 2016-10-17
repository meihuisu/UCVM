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

/** @file libsrc/create/VMCreator.h
 *
 * @brief C++ manager for creating the velocity model
 * database.
 */

#if !defined(cencalvm_create_vmcreator_h)
#define cencalvm_create_vmcreator_h

#include <string> // HASA std::string

#include "cencalvm/storage/etreefwd.h" // HOLDSA etree_t

namespace cencalvm {
  namespace create {
    class VMCreator;
    class TestVMCreator; // friend
  } // namespace create
  namespace storage {
    class Geometry; // USES Geometry
    struct PayloadStruct; // USES PayloadStruct
  } // namespace storage
} // namespace cencalvm

/// C++ manager for creating the velocity model database.
class cencalvm::create::VMCreator
{ // VMCreator
  friend class TestVMCreator;

public :
  // PUBLIC METHODS /////////////////////////////////////////////////////

  /// Constructor
  VMCreator(void);

  /// Destructor
  ~VMCreator(void);

  /** Open a new database.
   *
   * Description of database is included in database metadata along
   * with the hostname where database was created and the time the
   * database was created.
   *
   * @param filename Name of file
   * @param cacheSize Size of cache in MB
   * @param description Description of database
   */
  void openDB(const char* filename,
	      const int cacheSize,
	      const char* description);

  /// Close the database
  void closeDB(void);

  /** Create packed etree database from unpacked etree database.
   *
   * @param filenamePacked Filename of packed database
   * @param filenameUnpacked Filename of unpacked database
   * @param cacheSize Size of cache in MB
   */
  void packDB(const char* filenamePacked,
	      const char* filenameUnpacked,
	      const int cacheSize) const;

  /** Insert data into database.
   *
   * @param payload Data to insert
   * @param lon Longitude of location
   * @param lat Latitude of location
   * @param elev Elevation of location
   * @param resHoriz Horizontal resolution associated with location
   * @param pGeom Pointer to velocity model geometry
   */
  void insert(const storage::PayloadStruct& payload,
	      const double lon,
	      const double lat,
	      const double elev,
	      const double resHoriz,
	      storage::Geometry* pGeom);

  /** Set flag indicating creation should be quiet (no progress reports).
   *
   * Default behavior is for to give progress reports.
   *
   * @param flag True for quiet operation, false to give progress reports
   */
  void quiet(const bool flag);

 private :
  // PRIVATE METHODS ////////////////////////////////////////////////////

  VMCreator(const VMCreator& c); ///< Not implemented
  const VMCreator& operator=(const VMCreator& c); ///< Not implemented
  
private :
  // PRIVATE MEMBERS ////////////////////////////////////////////////////

  std::string _filename; ///< Name of database file
  etree_t* _pDB; ///< Pointer to database
  bool _quiet; ///< Flag to eliminate progress reports

}; // VMCreator

#include "VMCreator.icc" // inline methods

#endif // cencalvm_create_vmcreator_h



// version
// $Id: VMCreator.h 3059 2007-02-22 05:33:12Z brad $

// End of file 
