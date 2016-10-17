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

/** @file libsrc/query/cvmerror.h
 *
 * @brief C interface to error handler for the USGS central CA velocity
 * model (USER INTERFACE).
 *
 * These routines can be used to turn logging of errors on or off, set
 * the name of the log file, get information about errors, and reset
 * the status of the error handler. See f77vmerror.h for the Fortran
 * bindings of these routines. If you are using C++, you should use
 * the ErrorHandler object directly.
 *
 * The log file will generally contain coordinates of points for which
 * data was not found in the velocity model along with a short string
 * indicating the type of error.
 *
 * The default behavior is no log file is written (the log file is set
 * to /dev/null). To turn logging on, simply set the name of the log
 * file.
 */

#if !defined(cencalvm_cvmerror_h)
#define cencalvm_cvmerror_h

/** Set the filename for logging and enable logging.
 *
 * Calls logFilename().
 *
 * @param handle Handle to error handler
 * @param filename Name of file
 *
 * @returns 0 on success, 1 on error
 */
int cencalvm_error_logFilename(void* handle,
			       char* filename);

/** Turn logging on or off.
 *
 * @pre The log filename must have been set for logging to work if
 * it is turned on.
 *
 * @note Turning logging on after it has been turned off will cause
 * subsequent messages to be appended to the log file.
 *
 * @param handle Handle to error handler
 * @param turnOn =1 to turn on logging, =0 to turn logging off
 *
 * @returns 0 on success, 1 on error
 */
int cencalvm_error_loggingOn(void* handle,
			     int turnOn);

/** Reset error status and clear any error message.
 *
 * @param handle Handle to error handler
 *
 * @returns 0 on success, 1 on error
 */
int cencalvm_error_resetStatus(void* handle);
  
/** Get status of error handler.
 *
 * @param handle Handle to error handler
 *
 * @returns 0 if OK, 1 if warning, 2 if error
 */
int cencalvm_error_status(void* handle);
  
/** Get warning/error message.
 *
 * @param handle Handle to error handler
 *
 * @returns 0 if OK, 1 if warning, 2 if error
 */
char* cencalvm_error_message(void* handle);

#endif /* cencalvm_cvmerror_h */

/* version */
/* $Id: cvmerror.h 3059 2007-02-22 05:33:12Z brad $ */

/* End of file  */
