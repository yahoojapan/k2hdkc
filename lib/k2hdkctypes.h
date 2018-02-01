/*
 * 
 * K2HDKC
 * 
 * Copyright 2016 Yahoo! JAPAN corporation.
 * 
 * K2HDKC is k2hash based distributed KVS cluster.
 * K2HDKC uses K2HASH, CHMPX, FULLOCK libraries. K2HDKC supports
 * distributed KVS cluster server program and client libraries.
 * 
 * For the full copyright and license information, please view
 * the LICENSE file that was distributed with this source code.
 *
 * AUTHOR:   Takeshi Nakatani
 * CREATE:   Tue Sep 6 2016
 * REVISION:
 * 
 */

#ifndef	K2HDKCTYPES_H
#define	K2HDKCTYPES_H

#include <k2hash/k2hash.h>
#include <chmpx/chmpx.h>

#include "k2hdkccommon.h"

DECL_EXTERN_C_START

// [NOTE]
// This types header file is for compile error as 'does not name a type'.
//
//---------------------------------------------------------
// Typedefs
//---------------------------------------------------------
typedef	uint64_t	k2hdkc_chmpx_h;					// chmpx object handle presented by k2hdkc
typedef uint64_t	dkccom_type_t; 					// type for packet
typedef uint64_t	dkcres_type_t; 					// response type for packet
typedef uint64_t	dkc_qtype_t; 					// Queue type for using attributes mask type in DKC_COM_SET

//---------------------------------------------------------
// Structure
//---------------------------------------------------------
//
// Sub structure for K2hash State & State
//
typedef struct k2hdkc_nodestate{
	chmpxid_t		chmpxid;						// See: CHMPX in chmpx/chmstructure.h
	char			name[NI_MAXHOST];				//
	chmhash_t		base_hash;						//
	chmhash_t		pending_hash;					//
	K2HSTATE		k2hstate;						// See: k2hash/k2hash.h
}K2HDKC_ATTR_PACKED DKC_NODESTATE, *PDKC_NODESTATE;

DECL_EXTERN_C_END

#endif	// K2HDKCTYPES_H

/*
 * VIM modelines
 *
 * vim:set ts=4 fenc=utf-8:
 */
