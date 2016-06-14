/* -*- C -*-
 *
 * Copyright (C) 2007 Julio Lopez. All Rights Reserved.
 * For copyright and disclaimer details see the file COPYING
 * included in the package distribution.
 *
 * Description: Etree helper functions
 *
 * Package:     Etree tools
 * Name:        $RCSfile: ehelper.h,v $
 * Language:    C
 * Author:      Julio Lopez
 * Last mod:    $Date: 2007/05/24 03:57:20 $ $Author: jclopez $
 * Version      $Revision: 1.2 $
 *
 * Notes:       These functions belong to the etree helper library,
 *              this is only a subset plus minor additions.
 */
#ifndef BEM_EHELPER_H
#define BEM_EHELPER_H

#include <etree.h>


#ifdef __cplusplus
#include <cstdio>
#else
#include <stdio.h>
#endif


/** use 64 MB by default for each etree buffer size */
#ifndef DEF_ETREE_BUF_SIZE
#define DEF_ETREE_BUF_SIZE        64
#endif


#ifdef __cplusplus
extern "C" { 
#endif


enum coord_name_t { X_COORD, Y_COORD, Z_COORD, T_COORD };


/**
 * Address manipulation struct
 */
struct eh_addr_t {
    etree_tick_t a[4];      /**<  x, y, z, t coordinates */
    unsigned int level;     /**< address level */
    etree_type_t type;      /**< octant type: ETREE_INTERIOR or ETREE_LEAF */
};

typedef struct eh_addr_t eh_addr_t;


static inline eh_addr_t
eh_etree2addr (const etree_addr_t* ea)
{
    eh_addr_t addr;

    addr.a[X_COORD] = ea->x;
    addr.a[Y_COORD] = ea->y;
    addr.a[Z_COORD] = ea->z;
    addr.a[T_COORD] = ea->t;
    addr.level      = ea->level;
    addr.type       = ea->type;

    return addr;
}


static inline etree_addr_t
eh_addr2etree (const eh_addr_t* addr)
{
    etree_addr_t ea;

    ea.x     = addr->a[X_COORD];
    ea.y     = addr->a[Y_COORD];
    ea.z     = addr->a[Z_COORD];
    ea.t     = addr->a[T_COORD];
    ea.level = addr->level;
    ea.type  = addr->type;

    return ea;
}


char* eh_etree_straddr (int dim, char* buf, const etree_addr_t* addr);

int eh_print_error (FILE* stream, etree_t* ep, const etree_addr_t* addr,
		    const char* fmt, ...);

etree_t* eh_open_etree_ex (const char* filename, int flags, int payload_size,
			   int dim, int buf_size);


int eh_close (etree_t* ep);

int eh_begin_append (etree_t* ep, float fill_ratio);

int eh_end_append (etree_t* ep);

int eh_append (etree_t* ep, const etree_addr_t* addr, void* payload);

int eh_search (etree_t* ep, const etree_addr_t* addr, etree_addr_t* hit_addr,
	       const char* field, void* payload);

int eh_get_payload_size (etree_t* ep);

void* eh_allocate_payload_buffer (etree_t* ep);

char* eh_get_schema (etree_t* ep);

int eh_register_schema (etree_t* ep, const char* schema);

char* eh_get_app_metadata (etree_t* ep);

int eh_set_app_metadata (etree_t* ep, const char* metadata);

int eh_init_cursor (etree_t* ep, etree_addr_t* addr);

int eh_get_cursor(etree_t* ep, etree_addr_t* addr, const char* field, void*pl);

int eh_advance_cursor( etree_t* ep );

const char* eh_get_all_field_spec (etree_t* ep);


static inline etree_t*
eh_open_etree (const char* filename, int flags)
{
    return eh_open_etree_ex (filename, flags, 0, 3, DEF_ETREE_BUF_SIZE);
}


static inline etree_t*
eh_create_etree (const char* filename, int flags, int payload_size, int dim)
{
    return eh_open_etree_ex (filename, flags, payload_size, dim,
			     DEF_ETREE_BUF_SIZE);
}


/**
 * This function assumes etree_tick_t are 32 bits long and that the
 * maximum etree level (ETREE_MAXLEVEL) is 31
 */
static inline etree_tick_t
eh_edge_len (int level)
{
    return 0x80000000 >> level;
}


static inline etree_tick_t
eh_edge_len_addr (const etree_addr_t* addr)
{
    return eh_edge_len (addr->level);
}


static inline etree_addr_t
eh_octant_center (const etree_addr_t* addr)
{
    unsigned int edgelen = eh_edge_len_addr (addr);

    edgelen /= 2;
    etree_addr_t ret_addr;

    ret_addr.x     = addr->x + edgelen;
    ret_addr.y     = addr->y + edgelen;
    ret_addr.z     = addr->z + edgelen;
    ret_addr.t     = addr->t + edgelen;
    ret_addr.level = ETREE_MAXLEVEL;
    ret_addr.type  = ETREE_LEAF;

    return ret_addr;
}


static inline int
eh_addr_same_corner_3d (const etree_addr_t* a1, const etree_addr_t* a2)
{
    return (a1->x == a2->x) && (a1->y == a2->y) && (a1->z == a2->z);
}

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

/**
 * C++ Wrapper
 */
static inline etree_t*
eh_open_etree_cxx(
	const char* filename,
	int         flags,
	int         payload_size = 0,
	int         dim = 3,
	int         buf_size = DEF_ETREE_BUF_SIZE
	)
{
    return eh_open_etree_ex (filename, flags,  payload_size, dim, buf_size );
}


static inline etree_tick_t
eh_edge_len( const etree_addr_t& addr )
{
    return eh_edge_len_addr( &addr );
}


static inline int
eh_addr_same_corner_3d( const etree_addr_t& a1, const etree_addr_t& a2 )
{
    return eh_addr_same_corner_3d( &a1, &a2 );
}


/**
 * Address manipulation struct
 */
class ehAddr : public eh_addr_t {
public:
    ehAddr();
    ehAddr( const ehAddr& oa );
    ehAddr( const etree_addr_t& ea );
    void move( coord_name_t axis, etree_tick_t dist );
    operator etree_addr_t();
};


ehAddr::ehAddr()
{
    this->a[X_COORD] = 0;
    this->a[Y_COORD] = 0;
    this->a[Z_COORD] = 0;
    this->a[T_COORD] = 0;
    this->level = 0;
    this->type = ETREE_LEAF;
}


ehAddr::ehAddr( const ehAddr& oa )
{
    this->a[X_COORD] = oa.a[X_COORD];
    this->a[Y_COORD] = oa.a[Y_COORD];
    this->a[Z_COORD] = oa.a[Z_COORD];
    this->a[T_COORD] = oa.a[T_COORD];
    this->level      = oa.level;
    this->type       = oa.type;
}


ehAddr::ehAddr( const etree_addr_t& ea )
{
    this->a[X_COORD] = ea.x;
    this->a[Y_COORD] = ea.y;
    this->a[Z_COORD] = ea.z;
    this->a[T_COORD] = ea.t;
    this->level      = ea.level;
    this->type       = ea.type;
}



void
ehAddr::move( coord_name_t axis, etree_tick_t dist )
{
    this->a[axis] += dist;
}


/**
 * Conversion to etree_addr_t operator for ehAddr.
 */
ehAddr::operator etree_addr_t()
{
    etree_addr_t ea;

    ea.x     = this->a[X_COORD];
    ea.y     = this->a[Y_COORD];
    ea.z     = this->a[Z_COORD];
    ea.t     = this->a[T_COORD];
    ea.level = this->level;
    ea.type  = this->type;

    return ea;
}
#endif /* __cplusplus */

#endif /* WDB_BUFFER_H */
