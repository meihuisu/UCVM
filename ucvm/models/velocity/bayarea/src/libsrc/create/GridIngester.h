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

/** @file libsrc/create/GridIngester.h
 *
 * @brief C++ object for reading input grids containing velocity model
 * data and inserting them into the database.
 */

#if !defined(cencalvm_create_gridingester_h)
#define cencalvm_create_gridingester_h

#include <string> // HASA std::string

namespace cencalvm {
  namespace create {
    class GridIngester;
    class VMCreator; // USES VMCreator
    class TestGridIngester; // friend
  } // namespace create
  namespace storage {
    class Geometry; // HOLDSA Geometry
  } // namespace storage
} // namespace cencalvm

/// C++ object for reading input grids containing velocity model data
/// and inserting them into the database.
class cencalvm::create::GridIngester
{ // GridIngester
  friend class TestGridIngester;

public :
  // PUBLIC METHODS /////////////////////////////////////////////////////

  /// Constructor
  GridIngester(void);

  /// Destructor
  ~GridIngester(void);

  /** Set filename for parameter file.
   *
   * @param filename Name of file
   */
  void filenameParams(const char* filename);

  /** Set filename for output file.
   *
   * @param filename Name of file
   */
  void filenameOut(const char* filename);

  /** Set filename for temporary (scratch) file.
   *
   * @param filename Name of file
   */
  void filenameTmp(const char* filename);

  /** Set database cache size.
   *
   * @param size Size of cache in MB
   */
  void cacheSize(const int size);

  /** Set description of database.
   *
   * @param text Description of database
   */
  void description(const char* text);

  /** Set geometry of velocity model.
   *
   * @param pGeom Pointer to velocity model geometry
   */
  void geometry(const storage::Geometry* pGeom);

  /// Create the database by ingesting the grids
  void run(void) const;

  /** Set flag indicating creation should be quiet (no progress reports).
   *
   * Default behavior is for to give progress reports.
   *
   * @param flag True for quiet operation, false to give progress reports
   */
  void quiet(const bool flag);

private :
  // PRIVATE METHODS ////////////////////////////////////////////////////

  /** Read parameter file.
   *
   * @param ppGridFilenames Pointer to array of input grid filenames
   * @param pNumGrids Pointer to number of input grids
   */
  void _readParams(std::string** pGridFilenames,
		   int* pNumGrids) const;

  /** Add gridded data to database.
   *
   * @param pCreator Pointer to database creator
   * @param filename Filename of gridded data
   */
  void _addGrid(VMCreator* pCreator,
		const char* filename) const;

private :
  // PRIVATE MEMBERS ////////////////////////////////////////////////////

  std::string _filenameParams; ///< Filename for parameter file
  std::string _filenameOut; ///< Filename for output file
  std::string _filenameTmp; ///< Filename for temporary file

  int _cacheSize; ///< Size of database cache in MB
  std::string _description; ///< Description of database

  storage::Geometry* _pGeom; ///< Pointer to velocity model geometry

  bool _quiet; ///< Flag to eliminate progress reports

}; // GridIngester

#include "GridIngester.icc" // inline methods

#endif // cencalvm_create_gridingester  

// version
// $Id: GridIngester.h 3059 2007-02-22 05:33:12Z brad $

// End of file 
