/* -*- C -*-
 *
 * Copyright (C) 2006-2007 Julio Lopez. All Rights Reserved.
 * For copyright and disclaimer details see the file COPYING
 * included in the package distribution.
 *
 * Description: Etree helper functions
 *
 * Package:     Etree tools
 * Name:        $RCSfile: ehelper.c,v $
 * Language:    C
 * Author:      Julio Lopez <jclopez at ece dot cmu dot edu >
 * Last mod:    $Date: 2007/05/24 03:57:20 $ $Author: jclopez $
 * Version      $Revision: 1.2 $
 */
#include "ehelper.h"

#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>


/**
 * Format a string representation of an etree address.
 */
char*
eh_etree_straddr (int dim, char* buf, const etree_addr_t* addr)
{
    if (dim == 3) { /* 3d case */
	sprintf (buf, "[%08X %08X %08X %02d-%c]",
		 addr->x, addr->y, addr->z, addr->level,
		 addr->type == ETREE_INTERIOR ? 'I' : 'L');
    }

    else { /* 4d case */
	sprintf (buf, "[%08X %08X %08X %08X %02d-%c]",
		 addr->x, addr->y, addr->z, addr->t, addr->level,
		 addr->type == ETREE_INTERIOR ? 'I' : 'L');
    }

    return buf;
}


int
eh_print_error(
	FILE*			stream,
	etree_t*		ep,
	const etree_addr_t*	addr,
	const char*		fmt,
	...
	)
{
    char	  buf[80];
    const char*   e_straddr = "not available";

    etree_error_t e_errno = ET_NOERROR;
    va_list	ap;

    if ( NULL != fmt ) {
	va_start ( ap, fmt );
	vfprintf ( stream, fmt, ap );
	va_end   ( ap );
    } else {
	fputs ( "ERROR ", stream );
    }

    if ( NULL != ep ) {
	e_errno = etree_errno (ep);
    }

    fprintf (stream, "\n  etree error (%d): ", e_errno);
    fputs   (etree_strerror (e_errno), stream);

    if (NULL != addr) {
	e_straddr = eh_etree_straddr (4, buf, addr);
    }

    fputs   (".\n  etree address:    ", stream);
    fputs   (e_straddr, stream);
    fprintf (stream, "\n  etree handle:     %p\n", ep);

    return 0;
}


/**
 * Wrapper helper function to open or create an etree file.  On failure
 * it prints error messages on stderr.
 *
 * \param filename name of the etree file in the file system.
 *
 * \param flags It specifies the mode in which to open an etree file.
 *      The mode can be one of \c O_RDONLY or \c O_RDWR.
 *      Additional flags may be bitwise-or'd to specify the creation of
 *      a new etree ( \c O_CREAT or \c O_TRUNC ).
 *
 * \param payload_size Size of the associated payload data (i.e.,
 *     record).  This parameter is only used to created a new etree
 *     database (i.e., when O_CREAT, O_TRUNC is specified), otherwise
 *     ignored.
 *
 * \param dim Dimensionality of a new etree database.
 *     This parameter is only used to created a new etree database (i.e.,
 *     when O_CREAT, O_TRUNC is specified), otherwise ignored.
 *
 * \param buf_size Size of the internal etree buffer cache.
 *      The size is specified in megabytes.
 *
 * \return a pointer to an etree_t handle on success, NULL on error.
 */
etree_t*
eh_open_etree_ex(
	const char* filename,
        int         flags,
	int         payload_size,
	int         dim,
	int         buf_size
	)
{
    etree_t* ep;

    assert (NULL != filename);

    ep = etree_open (filename, flags, buf_size, payload_size, dim);

    if (NULL == ep) {
	int errsv = errno;

	fputs ("  Could not open etree file \'", stderr);
	fputs (filename, stderr);
	fputs ("\'\n  Reason: ", stderr);
	fputs (strerror (errsv), stderr);
	fputc ('\n', stderr);
    }

    return ep;
}


/**
 * Wrapper helper function to close an etree file.  On failure
 * it prints error messages on stderr.
 *
 * \param ep handle to the etree to close.
 *
 * \return 0 on success, -1 on error.
 */
int
eh_close (etree_t* ep)
{
    int ret = 0;

    if (NULL != ep) {
        ret = etree_close (ep);

        if (0 != ret) {
	   perror ("etree_close()");
	   fprintf (stderr, "Could not close etree handle = %p\n", ep);
        }
    }

    return ret;
}


int
eh_search(
	etree_t*            ep,
        const etree_addr_t* addr,
	etree_addr_t*       hit_addr,
	const char*         field,
	void*               payload
	)
{
    int ret;

    assert (NULL != ep);
    assert (NULL != addr);
    assert (NULL != hit_addr);

    ret = etree_search (ep, *addr, hit_addr, field, payload);

    if (0 != ret) { /* not found? */
	eh_print_error (stderr, ep, addr,
			"Could not find sought record, etree_search() failed!\n");
    }

    return ret;
}


int
eh_begin_append (etree_t* ep, float fill_ratio)
{
    int ret;

    assert (NULL != ep);

    ret = etree_beginappend (ep, fill_ratio);

    if (0 != ret) {
	eh_print_error (stderr, ep, NULL, "Begin append operation failed!\n");
    }

    return ret;
}


int
eh_end_append (etree_t* ep)
{
    int ret;

    assert (NULL != ep);

    ret = etree_endappend (ep);

    if (0 != ret) {
	eh_print_error (stderr, ep, NULL, "End append operation failed!\n");
    }

    return ret;
}


/**
 * Wrapper around \c etree_append().
 */
int
eh_append (etree_t* ep, const etree_addr_t* addr, void* payload)
{
    int ret;

    assert (NULL != addr);
    assert (NULL != ep);

    ret = etree_append (ep, *addr, payload);

    if (0 != ret) {
	eh_print_error (stderr, ep, addr, "etree_append() failed!\n");
    }

    return ret;
}


/**
 * Allocate a chunk of memory to hold an etree's record payload.
 * This function performs appropriate error checks and on error prints out
 * corresponding diagnostic messages to standard error (\c stderr).
 *
 * \note This function should be part of the etree helper library.
 */
void*
eh_allocate_payload_buffer (etree_t* ep)
{
    int    payload_size;
    void*  payload = NULL;

    assert (NULL != ep);

    /* get the input db payload size and schema */
    payload_size = etree_getpayloadsize (ep);

    if (payload_size < 0) {
	eh_print_error (stderr, ep, NULL,
			"eh_allocate_payload (): etree_getpayload failed!\n");
	return NULL;
    }

    payload = malloc (payload_size);

    if (NULL == payload) {
	fprintf (stderr, "eh_allocate_payload (): payload memory allocation "
		"failed!, payload size = %d\n", payload_size);
    }

    return payload;
}


int
eh_get_payload_size (etree_t* ep)
{
    int    payload_size;

    assert (NULL != ep);

    /* get the input db payload size and schema */
    payload_size = etree_getpayloadsize (ep);

    if (payload_size < 0) {
	eh_print_error (stderr, ep, NULL,
			"eh_allocate_payload (): etree_getpayload failed!\n");
	return -1;
    }

    return payload_size;
}


char*
eh_get_schema (etree_t* ep)
{
    char* schema;

    assert (NULL != ep);
    schema = etree_getschema (ep);

    if (NULL == schema) {
        eh_print_error (stderr, ep, NULL, "eh_get_schema() got NULL schema!\n");
    }

    return schema;
}


int
eh_register_schema (etree_t* ep, const char* schema)
{
    int ret;

    assert (NULL != ep);
    ret = etree_registerschema (ep, schema);

    if (0 != ret){
        eh_print_error (stderr, ep, NULL,
			"etree_registerschema() failed\n\tschema = \"%s\"",
			schema);
    }

    return ret;
}


char*
eh_get_app_metadata (etree_t* ep)
{
    char* meta;

    assert (NULL != ep);

    meta = etree_getappmeta (ep);

    if (NULL == meta) {
	eh_print_error (stderr, ep, NULL, "Got NULL application metadata\n");
    }

    return meta;
}


int
eh_set_app_metadata (etree_t* ep, const char* metadata)
{
    int ret;

    assert (NULL != ep);
    ret = etree_setappmeta (ep, metadata);

    if (0 != ret) {
	eh_print_error (stderr, ep, NULL,
			"Could not set application metadata = \"%s\"",
			metadata);
    }

    return ret;
}


const char*
eh_get_all_field_spec (etree_t* ep)
{
    /* get the input mesh's payload size and schema */
    char* schema;

    assert (NULL != ep);

    if ( (schema = etree_getschema( ep ) ) != NULL ) {
	free( schema );
	return "*";
    }

    return NULL;
}

int
eh_init_cursor (etree_t* ep, etree_addr_t* addr)
{
    int ret;

    assert (NULL != ep);
    assert (NULL != addr);

    ret = etree_initcursor (ep, *addr);

    if (0 != ret) {
	eh_print_error (stderr, ep, addr, "etree_initcursor() failed!\n");
    }

    return ret;
}


int
eh_get_cursor (etree_t* ep, etree_addr_t* addr, const char* field, void* pl)
{
    int ret;

    assert (NULL != ep);
    assert (NULL != addr);
    assert (NULL != pl);

    ret = etree_getcursor (ep, addr, field, pl);

    if (0 != ret) {
	eh_print_error (stderr, ep, addr,
			"etree_getcursor() failed!, field=\"%s\"\n", field);
    }

    return ret;
}


/**
 * \return 0 on success, 1 on EOF, -1 on error.
 */
int
eh_advance_cursor( etree_t* ep )
{
    int ret;

    assert (NULL != ep);

    ret = etree_advcursor (ep);

    if (0 != ret) {
	etree_error_t err = etree_errno (ep);

        /* make sure that the cursor terminates correctly */
	 /* etree_advcursor returned non-zero but it could be just EOF */
	if (err != ET_END_OF_TREE) {
	    eh_print_error (stderr, ep, NULL, "etree_advcursor() failed!\n");
	    return -1;
	}

	return 1; // on EOF.
    }

    return ret; // should be 0
}
