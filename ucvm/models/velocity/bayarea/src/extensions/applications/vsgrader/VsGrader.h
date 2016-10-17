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

/** @file VsGrader.h
 *
 * @brief C++ manager for limiting gradient in shear wave speed in
 * velocity model stored as an Etree database.
 */

#if !defined(cencalvm_vsgrader_vsgrader_h)
#define cencalvm_vsgrader_vsgrader_h

#include <string> // HASA std::string
#include "cencalvm/query/VMQuery.h" // HASA QueryEnum

#include "cencalvm/storage/etreefwd.h" // USES etree_t

namespace cencalvm {
  namespace vsgrader {
    class VsGrader;
    class TestVsGrader; // friend
  } // namespace vsgrader
  namespace storage {
    class Geometry; // HOLDSA Geometry
    class Projector; // HOLDSA Projector
    class PayloadStruct; // forward declaration
  } // namespace storage
} // namespace vsgrader

/// C++ manager for limiting gradient in shear wave speed in
/// velocity model stored as an Etree database.
class cencalvm::vsgrader::VsGrader
{ // VsGrader
  friend class TestVsGrader;

public :
  // PUBLIC METHODS /////////////////////////////////////////////////////

  /// Constructor.
  VsGrader(void);

  /// Destructor
  ~VsGrader(void);

  /** Set filename of input database.
   *
   * @param filename Name of file
   */
  void filenameIn(const char* filename);

  /** Set filename of output database.
   *
   * @param filename Name of file
   */
  void filenameOut(const char* filename);

  /** Set filename of temporary database.
   *
   * @param filename Name of file
   */
  void filenameTmp(const char* filename);

  /** Set filename for parameter file.
   *
   * @param filename Name of file
   */
  void filenameParams(const char* filename);

  /** Set cache size.
   *
   * @param size of cache in MB
   */
  void cacheSize(const int size);

  /** Set velocity model geometry.
   *
   * @param pGeometry Pointer to geometry
   */
  void geometry(const storage::Geometry* pGeometry);

  /** Set flag indicating operations should be quiet (no progress reports).
   *
   * Default behavior is for to give progress reports.
   *
   * @param flag True for quiet operation, false to give progress reports
   */
  void quiet(const bool flag);

  /// Create Etree database with maximum gradient.
  void run(void);

 private :
  // PRIVATE METHODS ////////////////////////////////////////////////////

  /// Read parameters from file.
  void _readParams(void);

  /// Initialize.
  void _initialize(void);

  /// Extract database.
  void _extract(void) const;

  /// Pack new database.
  void _pack(void) const;

  /// Limit gradient in database.
  void _grade(void) const;

  /** Compute elevation given height index.
   *
   * @param index Height index
   *
   * @returns Elevation in meters
   */
  double _indexToElev(const int index) const;

  /** Compute longitude and latitude given length and width
   * indices. Values are returned through the parameters.
   *
   * @param pLon Pointer to longitude
   * @param pLat Pointer to latitude
   * @param indexL Index in length direction
   * @param indexW Index in width direction
   */
  void _indexToLonLat(double* pLon,
		      double* pLat,
		      const int indexL,
		      const int indexW) const;

  /** Get data from database. Payload is returned through the parameters.
   *
   * @param pPayload Pointer to payload
   * @param lon Longitude of location
   * @param lat Latitude of location
   * @param elev Elevation of location
   * @param pDB Pointer to database
   */
  void _pullData(cencalvm::storage::PayloadStruct* pPayload,
		 const double lon,
		 const double lat,
		 const double elev,
		 etree_t* db) const;

  /** Update data in database.
   *
   * @param pDB Pointer to database to update
   * @param pPayload Pointer to payload
   * @param lon Longitude of location
   * @param lat Latitude of location
   * @param elev Elevation of location
   */
  void _pushData(etree_t* pDB,
		 const cencalvm::storage::PayloadStruct& payload,
		 const double lon,
		 const double lat,
		 const double elev) const;

  /** Limit maximum difference in Vs. Other material properties are
   * changed proportionally.
   *
   * @param ppData Pointer to array of payload values
   * @param ppIsChanged Pointer to array of flags indicating payload changed
   * @param dataLen Size of array
   * @param maxdiff Maximum difference
   */
  void _limitDiff(cencalvm::storage::PayloadStruct** ppData,
		  bool** ppIsChanged,
		  const int dataLen,
		  const double maxdiff) const;

 private :
  // PRIVATE METHODS ////////////////////////////////////////////////////

  VsGrader(const VsGrader& e); ///< Not implemented
  const VsGrader& operator=(const VsGrader& e); ///< Not implemented
  
private :
  // PRIVATE MEMBERS ////////////////////////////////////////////////////

  double _gradientMaxVs; ///< Maximum gradient in Vs
  double _minVs; ///< Minimum Vs

  double _swcornerLon; ///< Longitude of southwest top corner of domain
  double _swcornerLat; ///< Latitude of southwest top corner of domain
  double _swcornerElev; ///< Elevation of southwest top corner of domain
  double _domainLen; ///< Length of domain
  double _domainWidth; ///< Width of domain
  double _domainHt; ///< Height of domain
  double _resVert; ///< Resolution (vertical) of domain

  double _swcornerX; ///< Projected x coordinate of SW top corner
  double _swcornerY; ///< Projected y coordinate of SW top corner

  double _queryRes; ///< Resolution of extraction query

  std::string _filenameIn; ///< Filename for input database
  std::string _filenameOut; ///< Filename for output database
  std::string _filenameTmp; ///< Filename for temporary database
  std::string _filenameParams; ///< Filename for parameters

  int _cacheSize; ///< Database cache size in MB
  query::VMQuery::QueryEnum _queryType; ///< Type of query in extraction

  cencalvm::storage::Geometry* _pGeom; ///< Velocity model geometry

  bool _quiet; ///< Flag to eliminate progress reports

  static const double _AZIMUTHLEN; ///< Azimuth of edge of domain (length)

  /// Value for locations not in input database
  static const double _NODATAVAL;

  /// Fault block for locations not in input database
  static const int16_t _NODATABLOCK;

  /// Zone for locations not in input database
  static const int16_t _NODATAZONE;

}; // VsGrader

#include "VsGrader.icc" // inline methods

#endif // cencalvm_vsgrader_vsgrader_h

// version
// $Id: VsGrader.h 3066 2007-02-22 17:24:07Z brad $

// End of file 
