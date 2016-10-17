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

/** @file libsrc/query/f77vmerror.h
 *
 * @brief Fortran 77 interface to error handler for the USGS central
 * CA velocity model (USER INTERFACE).
 *
 * These routines can be used to turn logging of errors on or off, set
 * the name of the log file, get information about errors, and reset
 * the status of the error handler. See cvmerror.h for the C bindings
 * of these routines. If you are using C++, you should use the
 * ErrorHandler object directly.
 *
 * The log file will generally contain coordinates of points for which
 * data was not found in the velocity model along with a short string
 * indicating the type of error.
 *
 * The default behavior is no log file is written (the log file is set
 * to /dev/null). To turn logging on, simply set the name of the log
 * file.
 */

#if !defined(cencalvm_f77vmerror_h)
#define cencalvm_f77vmerror_h ///< include protection

// ----------------------------------------------------------------------
/** Fortran name mangling */
#define cencalvm_error_logfilename_f \
  FC_FUNC_(cencalvm_error_logfilename_f, CENCALVM_ERROR_LOGFILENAME_F)
/** Set the filename for logging and enable logging.
 *
 * Calls logFilename().
 *
 * @param handleAddr Handle to error handler
 * @param filename Name of file
 * @param err set to 0 on success, 1 on error
 * @param len Length of filename string (IMPLICIT ARG)
 */
extern "C"
void cencalvm_error_logfilename_f(size_t* handleAddr,
				  const char* filename,
				  int* err,
				  const int len);

// ----------------------------------------------------------------------
/** Fortran name mangling */
#define cencalvm_error_loggingon_f \
  FC_FUNC_(cencalvm_error_loggingon_f, CENCALVM_ERROR_LOGGINGON_F)
/** Turn logging on or off.
 *
 * @pre The log filename must have been set for logging to work if
 * it is turned on.
 *
 * @note Turning logging on after it has been turned off will cause
 * subsequent messages to be appended to the log file.
 *
 * @param handleAddr Handle to error handler
 * @param turnOn =1 to turn on logging, =0 to turn logging off
 * @param err set to 0 on success, 1 on error
 */
extern "C"
void cencalvm_error_loggingon_f(size_t* handleAddr,
				int* turnOn,
				int* err);
  
// ----------------------------------------------------------------------
/** Fortran name mangling */
#define cencalvm_error_resetstatus_f					\
  FC_FUNC_(cencalvm_error_resetstatus_f, CENCALVM_ERROR_RESETSTATUS_F)
/** Reset error status and clear any error message.
 *
 * @param handleAddr Handle to error handler
 * @param err set to 0 on success, 1 on error
 */
extern "C"
void cencalvm_error_resetstatus_f(size_t* handleAddr,
				  int* err);

// ----------------------------------------------------------------------
/** Fortran name mangling */
#define cencalvm_error_status_f \
  FC_FUNC_(cencalvm_error_status_f, CENCALVM_ERROR_STATUS_F)
/** Get status of error handler.
 *
 * @param handleAddr Handle to error handler
 * @param err set to 0 if ok, 1 if warning, 2 if error
 */
extern "C"
void cencalvm_error_status_f(size_t* handleAddr,
			     int* err);

// ----------------------------------------------------------------------
/** Fortran name mangling */
#define cencalvm_error_message_f \
  FC_FUNC_(cencalvm_error_message_f, CENCALVM_ERROR_MESSAGE_F)
/** Get warning/error message.
 *
 * @param handleAddr Handle to error handler
 * @param message String for message
 * @param err set to 0 on success, 1 on error
 * @param len Length of filename string (IMPLICIT ARG)
 */
extern "C"
void cencalvm_error_message_f(size_t* handleAddr,
			      char* message,
			      int* err,
			      const int len);

#endif // cencalvm77vmerror_h

// version
// $Id: f77vmerror.h 3720 2008-02-06 19:53:51Z brad $

// End of file
