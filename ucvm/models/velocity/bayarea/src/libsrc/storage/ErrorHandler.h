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

/** @file libsrc/storage/ErrorHandler.h
 *
 * @brief C++ error handler associated with the USGS central CA velocity
 * model (USER INTERFACE).
 *
 * Use this object to turn logging of errors on or off, set the name
 * of the log file, get information about errors, and reset the status
 * of the error handler.
 *
 * The log file will generally contain coordinates of points for which
 * data was not found in the velocity model along with a short string
 * indicating the type of error.
 *
 * The default behavior is no log file is written (the log file is set
 * to /dev/null). To turn logging on, simply set the name of the log
 * file.
 */

#if !defined(cencalvm_storage_errorhandler_h)
#define cencalvm_storage_errorhandler_h

#include <string> // HASA std::string
#include <iosfwd> // HOLDSA std::ostream

namespace cencalvm {
  namespace storage {
    class ErrorHandler;
    class TestErrorHandler; // friend
  } // namespace storage
} // namespace cencalvm

/// C++ error handler associated with the USGS central CA velocity model.
class cencalvm::storage::ErrorHandler
{ // Geometry
  friend class TestErrorHandler;

public :
  // PUBLIC ENUMS ///////////////////////////////////////////////////////

  /// Enumerated type for error status.
  enum StatusEnum { 
    OK=0, ///< No errors
    WARNING=1, ///< Non-fatal error
    ERROR=2 ///< Fatal error
  };

public :
  // PUBLIC METHODS /////////////////////////////////////////////////////

  /// Constructor
  ErrorHandler(void);

  /// Destructor
  ~ErrorHandler(void);

  /** Set filename for logging and enable logging.
   *
   * @param filename Name of file
   */
  void logFilename(const char* filename);

  /** Turn logging on/off.
   *
   * @pre The log filename must have been set for logging to work if
   * it is turned on.
   *
   * @note Turning logging on after it has been turned off will cause
   * subsequent messages to be appended to the log file.
   *
   * @param turnOn True to turn on logging, false to turn logging off
   */
  void loggingOn(const bool turnOn =true);

  /// Reset error status and clear any error message.
  void resetStatus(void);

  /** Get status.
   *
   * @returns Status of errors
   */
  StatusEnum status(void) const;

  /** Get warning/error message.
   *
   * @returns Warning/error message
   */
  const char* message(void) const;

  /** Set status to error and store error message.
   *
   * @param msg Error message
   */
  void error(const char* msg);
  
  /** Set status to warning and store warning message.
   *
   * @param msg Warning message
   */
  void warning(const char* msg);
  
  /** Write message to log file.
   *
   * @param msg Message to write to log file
   */
  void log(const char* msg);
  
 private :
  // PRIVATE METHODS ////////////////////////////////////////////////////

  ErrorHandler(const ErrorHandler& h); ///< Not implemented
  const ErrorHandler& operator=(const ErrorHandler& h); ///< Not implemented
  
private :
  // PRIVATE MEMBERS ////////////////////////////////////////////////////

  std::string _message; ///< Message associated with error/warning
  std::string _logFilename; ///< Name of log file
  std::ofstream* _pLogFile; ///< Pointer to log file
  StatusEnum _status; ///< Error status

  static const char* _NULLFILE; ///< Name of null device

}; // ErrorHandler

#include "ErrorHandler.icc" // inline methods

#endif // cencalvm_storage_errorhandler_h

// version
// $Id: ErrorHandler.h 3059 2007-02-22 05:33:12Z brad $

// End of file 
