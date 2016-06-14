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
#include <math.h>
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

/* Property definition */
typedef struct property_t 
{
    float Vp;
    float Vs;
    float density;
} property_t;


/* Stack definition */
typedef struct stack2_t
{
  etree_addr_t addr;
  property_t payload;
} stack2_t;
#define MAX_STACK_SIZE 4096


/* Default material property tolerances */
#define DEFAULT_VS_PERCENT 2.0
#define DEFAULT_RHO_PERCENT 5.0
#define DEFAULT_VP_PERCENT 2.0

/* Interval between status reports */
#define PROGRESS_INTERVAL 250000000


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


static int coalesce_stack(stack2_t *stack, int *stack_ptr)
{
  etree_addr_t *addr, ref_addr;
  int ref_vp, ref_vs, ref_rho;
  etree_tick_t edge_len, addr_diff[3];
  int i, j;
  float vp, vs, rho;

  assert( MAX_STACK_SIZE > *stack_ptr );
  assert( *stack_ptr >= 7 );

  /* Compute edge len */
  edge_len = eh_edge_len(stack[*stack_ptr].addr.level);

  /* Save the 8th previous octant as a reference */
  memcpy(&(ref_addr), &(stack[*stack_ptr - 7].addr), sizeof(etree_addr_t));
  ref_vp = (int)stack[*stack_ptr - 7].payload.Vp;
  ref_vs = (int)stack[*stack_ptr - 7].payload.Vs;
  ref_rho = (int)stack[*stack_ptr - 7].payload.density;

  /* Ensure reference x,y,z can be raised one level */
  if ((ref_addr.x % (edge_len * 2) != 0) || 
      (ref_addr.y % (edge_len * 2) != 0) || 
      (ref_addr.z % (edge_len * 2) != 0)) {
    return 0;
  }

  /* Init material properties */
  vp = 0.0;
  vs = 0.0;
  rho = 0.0;

  for (i = *stack_ptr - 7; i <= *stack_ptr; i++) {
    addr = &(stack[i].addr);

    /* Ensure all on same level as reference */
    if (addr->level != ref_addr.level) {
      return 0;
    }

    /* Ensure all are adjacent to reference */
    addr_diff[0] = addr->x - ref_addr.x;
    addr_diff[1] = addr->y - ref_addr.y;
    addr_diff[2] = addr->z - ref_addr.z;
    for (j = 0; j < 3; j++) {
      if ((addr_diff[j] != 0) && (addr_diff[j] != edge_len)) {
	return 0;
      }
    }

    /* Sum up material properties for 8 octants */
    vp = vp + stack[i].payload.Vp;
    vs = vs + stack[i].payload.Vs;
    rho = rho + stack[i].payload.density;
  }

  /* Compute material property averages */
  vp = vp / 8.0;
  vs = vs / 8.0;
  rho = rho / 8.0;

  /* Verify 8 octants are within tolerance for Vp, Vs, rho */
  for (i = *stack_ptr - 7; i <= *stack_ptr; i++) {
    if (((int)stack[i].payload.Vp != ref_vp) ||
	((int)stack[i].payload.Vs != ref_vs) ||
	((int)stack[i].payload.density != ref_rho)) {
      return 0;
    }
  }

  /* Delete top 8 octants and replace with single octant one level up */
  /* Address of new octant is same as first octant in the set since all 
     data is in Z-order */
  *stack_ptr = *stack_ptr - 7;
  stack[*stack_ptr].payload.Vp = vp;
  stack[*stack_ptr].payload.Vs = vs;
  stack[*stack_ptr].payload.density = rho;
  stack[*stack_ptr].addr.level = stack[*stack_ptr].addr.level - 1;

  return 1;
}


static int coalesce_stack_percent(stack2_t *stack, int *stack_ptr)
{
  etree_addr_t *addr, ref_addr;
  etree_tick_t edge_len, addr_diff[3];
  int i, j;
  float vp, vs, rho;

  assert( MAX_STACK_SIZE > *stack_ptr );
  assert( *stack_ptr >= 7 );

  /* Compute edge len */
  edge_len = eh_edge_len(stack[*stack_ptr].addr.level);

  /* Save the 8th previous octant as a reference */
  memcpy(&(ref_addr), &(stack[*stack_ptr - 7].addr), sizeof(etree_addr_t));

  /* Ensure reference x,y,z can be raised one level */
  if ((ref_addr.x % (edge_len * 2) != 0) || 
      (ref_addr.y % (edge_len * 2) != 0) || 
      (ref_addr.z % (edge_len * 2) != 0)) {
    return 0;
  }

  /* Init material properties */
  vp = 0.0;
  vs = 0.0;
  rho = 0.0;

  for (i = *stack_ptr - 7; i <= *stack_ptr; i++) {
    addr = &(stack[i].addr);

    /* Ensure all on same level as reference */
    if (addr->level != ref_addr.level) {
      return 0;
    }

    /* Ensure all are adjacent to reference */
    addr_diff[0] = addr->x - ref_addr.x;
    addr_diff[1] = addr->y - ref_addr.y;
    addr_diff[2] = addr->z - ref_addr.z;
    for (j = 0; j < 3; j++) {
      if ((addr_diff[j] != 0) && (addr_diff[j] != edge_len)) {
	return 0;
      }
    }

    /* Sum up material properties for 8 octants */
    vp = vp + stack[i].payload.Vp;
    vs = vs + stack[i].payload.Vs;
    rho = rho + stack[i].payload.density;
  }

  /* Compute property averages */
  vp = vp / 8.0;
  vs = vs / 8.0;
  rho = rho / 8.0;

  /* Verify 8 octants are within tolerance for Vs, rho*/
  for (i = *stack_ptr - 7; i <= *stack_ptr; i++) {
    if (fabs(stack[i].payload.Vs/vs - 1.0) > DEFAULT_VS_PERCENT/100.0) {
      return 0;
    }
    if (fabs(stack[i].payload.density/rho - 1.0) > DEFAULT_RHO_PERCENT/100.0) {
      return 0;
    }
  }

  /* Verify 8 octants are within tolerance for Vp */
  for (i = *stack_ptr - 7; i <= *stack_ptr; i++) {
    if (fabs(stack[i].payload.Vp/vp - 1.0) > DEFAULT_VP_PERCENT/100.0) {
     break;
    }
  }

  /* If Vp check fails, verify all Vp within factor of 2 of Vs */
  if (i != *stack_ptr + 1) {
    for (i = *stack_ptr - 7; i <= *stack_ptr; i++) {
      if (stack[i].payload.Vp <= 2.0*vs || stack[i].payload.Vp >= 4.0*vs) {
	return 0;
      }
    }
  }

  /* Delete top 8 octants and replace with single octant one level up */
  /* Address of new octant is same as first octant in the set since all 
     data is in Z-order */
  *stack_ptr = *stack_ptr - 7;
  stack[*stack_ptr].payload.Vp = vp;
  stack[*stack_ptr].payload.Vs = vs;
  stack[*stack_ptr].payload.density = rho;
  stack[*stack_ptr].addr.level = stack[*stack_ptr].addr.level - 1;

  return 1;
}


int push_stack(stack2_t *stack, int *stack_ptr,
	       etree_addr_t *addr, property_t *payload)
{
  assert(*stack_ptr >= -1);
  assert(*stack_ptr < MAX_STACK_SIZE - 1);

  *stack_ptr = *stack_ptr + 1;

  memcpy(&(stack[*stack_ptr].addr), addr, sizeof(etree_addr_t));
  memcpy(&(stack[*stack_ptr].payload), payload, sizeof(property_t));

  return 0;
}


int flush_stack(etree_t* out_etree, stack2_t *stack, int *stack_ptr,
		int start_ptr, int end_ptr, uint64_t *count)
{
  int i;
  int req_count = 0;
  int write_count = 0;
  int a_ret;

  if (*stack_ptr == -1) {
    /* No work to do */
    return 0;
  }

  assert(*stack_ptr >= 0);
  assert(*stack_ptr < MAX_STACK_SIZE);
  assert(start_ptr <= end_ptr);
  assert(end_ptr <= *stack_ptr);

  for (i = start_ptr; i <= end_ptr; i++) {
    /* append octant to the output etree */
    a_ret = eh_append( out_etree, &(stack[i].addr), &(stack[i].payload) );
    if ( 0 != a_ret ) {    /* error, handle outside loop */
      break;
    }
    write_count = write_count + 1;
  }
  req_count = end_ptr - start_ptr + 1;
  *count = *count + write_count;

  /* Shift remainder of stack to start */
  if (*stack_ptr - end_ptr > 0) {
    memcpy(&(stack[start_ptr]), &(stack[end_ptr + 1]), 
	   sizeof(stack2_t) * (*stack_ptr - end_ptr));
  }
  *stack_ptr = *stack_ptr - req_count;

  if (req_count != write_count) {
    fprintf(stderr, " Flush failed! Req %d, wrote %d, start=%d, end=%d\n", 
	    req_count, write_count, start_ptr, end_ptr );
    return 1;
  }

  return 0;
}


static int
compact_etree( etree_t* in_etree, etree_t* out_etree )
{
  int a_ret = 0;
  int gc_ret = 0;
  int  ac_ret = 0;
  etree_addr_t addr;
  void*	 payload  = NULL;
  const char*  field = NULL;
  uint64_t     read_count = 0;      /* count number of octants */
  uint64_t     write_count = 0;      /* count number of octants */
  stack2_t stack[MAX_STACK_SIZE];
  int stack_ptr = -1;
  int retval;
  uint64_t progress_thres = PROGRESS_INTERVAL;
  
  assert( NULL != in_etree );
  assert( NULL != out_etree );
  
  memset(stack, 0, sizeof(stack2_t) * MAX_STACK_SIZE);
  
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
    
    read_count = read_count + 1;
    
    if (stack_ptr == (MAX_STACK_SIZE - 1)) {
      /* Stack is full and can't be coalesced further */
      /* Flush stack[0]->stack[stack_ptr-8] to output etree */
      a_ret = flush_stack(out_etree, 
			  &stack[0], &stack_ptr, 0, stack_ptr - 8,
			  &write_count);
      if (a_ret != 0) {
	break;
      }
    } else if ((stack_ptr >= 0) && 
	       (addr.level != stack[stack_ptr].addr.level)) {
      /* Flush stack to output etree */
      a_ret = flush_stack(out_etree,
			  &stack[0], &stack_ptr, 0, stack_ptr,
			  &write_count);
      if (a_ret != 0) {
	break;
      }
    }
    if (write_count > progress_thres) {
      fprintf( stdout, 
	       "Counts - Read: %" UINT64_FMT ", Wrote: %" UINT64_FMT "\n", 
	       read_count, write_count);
      progress_thres = progress_thres + PROGRESS_INTERVAL;
    }

    /* Push octant onto stack */
    push_stack(&stack[0], &stack_ptr, &addr, payload);

    /* Attempt to coalesce recursively */
    if (stack_ptr >= 7) {
      do {
    	retval = coalesce_stack(&stack[0], &stack_ptr);
     } while ((retval != 0) && (stack_ptr >= 7));
    }

  } while ( (ac_ret = eh_advance_cursor( in_etree )) == 0 );

  if (ac_ret != 0) {
    /* Flush remaining stack octants to output etree */
    fprintf( stdout, "Flushing remaining %d octants\n", stack_ptr);
    a_ret = flush_stack(out_etree, 
			&stack[0], &stack_ptr, 0, stack_ptr,
			&write_count);
  }

  assert(stack_ptr == -1);

  int ea_ret = eh_end_append( out_etree );
  
  /* check for errors during processing */
  if ( ea_ret != 0 || a_ret != 0 || ac_ret != 1 ) {
    fputs( __FUNCTION_NAME " failed!\n", stderr );
    return -1;
  }
  
  /* else */
  fprintf(stdout, 
	  "Success!!\nRead %" UINT64_FMT " octants, wrote %" UINT64_FMT "\n", 
	  read_count, write_count);
  
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
