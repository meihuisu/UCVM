/* -*- C -*-
 *
 * Copyright (C) 2006-2007 Julio Lopez. All Rights Reserved.
 * For copyright and disclaimer details see the file COPYING
 * included in the package distribution.
 *
 * Author:    Julio Lopez (jclopez at ece dot cmu dot edu)
 * Project:   WaveDB
 *
 * File info:
 *   $RCSfile: ecompact.c,v $
 *   $Author: jclopez $
 *   $Revision: 1.2 $
 *   $Date: 2007/05/24 16:07:53 $
 */

/**
 * \file
 * \brief Etree compactation.
 *
 * \author Julio Lopez (jclopez at ece dot cmu dot edu)
 *
 * $Id: ecompact.c,v 1.2 2007/05/24 16:07:53 jclopez Exp $
 */

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include "ehelper.h"

// RCS_ID_DECL("$Id: ecompact.c,v 1.2 2007/05/24 16:07:53 jclopez Exp $");

/**
 * In case __FUNCTION_NAME has not been properly defined, let's define it as
 * an empty string.
 */
#ifndef __FUNCTION_NAME
# define __FUNCTION_NAME	""
#endif


/**
 * Macros with the format string for int64_t and uint64_t types.
 * For these to work, __WORDSIZE must be defined.
 */
#if defined __PRI64_PREFIX
#  define UINT64_FMT		__PRI64_PREFIX"u"
#  define INT64_FMT		__PRI64_PREFIX"d"
#elif defined __WORDSIZE
#  if (__WORDSIZE == 64)
#    define UINT64_FMT		"lu"
#    define INT64_FMT		"ld"
#  else		/* __WORDSIZE != 64, assume equals to 32 */
#    define UINT64_FMT		"llu"
#    define INT64_FMT		"lld"
#  endif
#elif defined __x86_64__	/* intel and amd EM64T */
#  define UINT64_FMT		"lu"
#  define INT64_FMT		"ld"
#else				/* everything else */
#  define UINT64_FMT		"llu"
#  define INT64_FMT		"lld"
#endif


/**
 * This function modifies (sets) the following global static variables:
 * - mesh_filename_
 * - input_mdb_filename_
 * - output_mdb_filename_
 * - verbosity_
 */
static int
parse_argv( int argc, char* argv[], const char* filenames[2] )
{
    const char* progname = argv[0]; /* before we loose it */

    if ( argc != 3 ) {
	fprintf( stderr, "Usage: %s <input_etree> <output_etree>\n", progname );
	exit( 2 );
    }

    filenames[0] = argv[1];
    filenames[1] = argv[2];
      
    return 0;
}


/**
 * Open two etree files needed for processing.
 *
 * \param filenames Array with three the filenames to open.
 *      -# Input mesh etree file.
 *      -# Output segment index etree.
 *
 * \param[out] eps Array with pointers to etree_t structs.  On return, this
 *      array contains pointers to the corresponding etree handles.
 *
 * \return 0 on success, \< 0 on error.
 */
static int
open_etrees( const char* filenames[2], etree_t* eps[2] )
{
    unsigned int dim   = -1;
    int   ret	       = -1;
    int   payload_size = 0;
    char* schema       = NULL;
    char* metadata     = NULL;


    assert( NULL != filenames );
    assert( NULL != filenames[0] );
    assert( NULL != filenames[1] );
    assert( NULL != eps );

    memset( eps, 0, sizeof( etree_t* ) * 2 );

    eps[0] = eh_open_etree( filenames[0], O_RDONLY );

    if ( NULL == eps[0] ) {
	return -2;
    }

    /* get the input etree payload size and schema */
    payload_size = etree_getpayloadsize( eps[0] );
    dim = eps[0]->dimensions;

    eps[1] = eh_open_etree_ex( filenames[1], O_RDWR | O_CREAT | O_EXCL,
			       payload_size, dim, DEF_ETREE_BUF_SIZE );

    if ( NULL == eps[1] ) {
	goto open_etree_error;
    }

    ret = 0;
    schema = eh_get_schema( eps[0] );

    if ( NULL != schema ) {
	puts("Found schema on input etree, registering it in the output etree");
	ret = eh_register_schema( eps[1], schema );
	free (schema);

	if ( 0 != ret ) {
	    fprintf( stderr, "Could not register schema for new material "
			     "database etree (%s)\n", filenames[1] );
	    goto open_etree_error;
	}
	puts( "Schema registration succeeded" );
    }

    metadata = eh_get_app_metadata( eps[0] );

    if ( NULL != metadata ) {
	puts( "Found application metadata on input etree, registering it in "
	      "the output etree" );
	ret = eh_set_app_metadata( eps[1], metadata );
	free( metadata );


	if ( 0 != ret ) {
	    fprintf( stderr, "Could not register app metadata new material "
			     "database etree (%s)\n", filenames[1] );
	    	goto open_etree_error;
	}
	puts( "Metadata registration succeeded" );
    }

    return ret;

    /* function error handling */
 open_etree_error:
    eh_close( eps[0] );
    eh_close( eps[1] );

    return -1;
}


/**
 * \return 0 on success, \<0 on error.
 */
static int
close_etrees( etree_t* eps[2] )
{
    int ret = eh_close( eps[0] )
            + eh_close( eps[1] );

    return ret;
}


static int
compact_etree( etree_t* in_etree, etree_t* out_etree )
{
    int          a_ret, gc_ret, ac_ret;
    etree_addr_t addr;
    void*	 payload  = NULL;
    const char*  field = NULL;
    uint64_t     count = 0;      /* count number of elements */


    assert( NULL != in_etree );
    assert( NULL != out_etree );


    /* get initial cursor */
    addr.x = addr.y = addr.z = addr.t = addr.level = 0;
    if ( eh_init_cursor( in_etree, &addr ) != 0 ) {
	return -2;
    }

    if ( eh_begin_append( out_etree, 1.0 ) != 0 ) {
	return -5;
    }

    payload = eh_allocate_payload_buffer( in_etree );

    if( NULL == payload ) {
	return -6;
    }


    field = eh_get_all_field_spec( in_etree );

    /* iteratively traverse the input etree using the cursor */
    do {
	gc_ret = eh_get_cursor( in_etree, &addr, field, payload );

	if ( 0 != gc_ret ) {   /* error, handle outside loop */
	    fputs( __FUNCTION_NAME ": could not get current etree "
		    "record\nbailing out!\n", stderr );
	    break;
	}

	/* append octant to the output etree */
	a_ret = eh_append( out_etree, &addr, payload );
	if ( 0 != a_ret ) {    /* error, handle outside loop */
	    break;
	}

	count++;
    } while ( (ac_ret = eh_advance_cursor( in_etree )) == 0 );

    int ea_ret = eh_end_append( out_etree );

    /* check for errors during processing */
    if ( ea_ret != 0 || a_ret != 0 || ac_ret != 1 ) {
	fputs( __FUNCTION_NAME " failed!\n", stderr );
	return -1;
    }

    /* else */
    fprintf( stderr, "Success!!!\nProcessed %" UINT64_FMT " octants\n", count );

    return 0;
}


int
main( int argc, char *argv[] )
{
    const char* filenames[2];
    etree_t*    eps[2];

    int ret = parse_argv( argc, argv, filenames );

    if ( 0 != ret ) {
	return 2;
    }

    fprintf( stderr, "Input etree  = %s\n", filenames[0] );
    fprintf( stderr, "Output etree = %s\n", filenames[1] );

    ret = open_etrees( filenames, eps );

    if (0 != ret) {
	return 3;
    }

    ret = compact_etree( eps[0], eps[1] );

    int close_ret = close_etrees( eps );

    return (ret == 0 && close_ret == 0) ? 0 : 4;
}
