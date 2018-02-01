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
 * CREATE:   Mon Jul 11 2016
 * REVISION:
 * 
 */

#ifndef	K2HDKCCOMMON_H
#define	K2HDKCCOMMON_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

//---------------------------------------------------------
// Macros for compiler
//---------------------------------------------------------
#ifndef	K2HDKC_NOWEAK
#define	K2HDKC_ATTR_WEAK				__attribute__ ((weak,unused))
#else
#define	K2HDKC_ATTR_WEAK
#endif

#ifndef	K2HDKC_NOPADDING
#define	K2HDKC_ATTR_PACKED				__attribute__ ((packed))
#else
#define	K2HDKC_ATTR_PACKED
#endif

#if defined(__cplusplus)
#define	DECL_EXTERN_C_START			extern "C" {
#define	DECL_EXTERN_C_END			}
#else	// __cplusplus
#define	DECL_EXTERN_C_START
#define	DECL_EXTERN_C_END
#endif	// __cplusplus

//---------------------------------------------------------
// Templates & macros
//---------------------------------------------------------
#if defined(__cplusplus)
template<typename T> inline bool DKCEMPTYSTR(const T& pstr)
{
	return (NULL == (pstr) || '\0' == *(pstr)) ? true : false;
}
#else	// __cplusplus
#define	DKCEMPTYSTR(pstr)			(NULL == (pstr) || '\0' == *(pstr))
#endif	// __cplusplus

#define DKCSTRJOIN(first, second)	first ## second

//---------------------------------------------------------
// Symbols
//---------------------------------------------------------
#define	DKC_INVALID_HANDLE			(-1)

#if defined(__cplusplus)
#define	DKC_INVALID_K2HDKCHANDLE	static_cast<uint64_t>(DKC_INVALID_HANDLE)
#else	// __cplusplus
#define	DKC_INVALID_K2HDKCHANDLE	(uint64_t)(DKC_INVALID_HANDLE)
#endif	// __cplusplus

// Environment
#define	K2HDKC_CONFFILE_ENV_NAME	"K2HDKCCONFFILE"
#define	K2HDKC_JSONCONF_ENV_NAME	"K2HDKCJSONCONF"

//---------------------------------------------------------
// For endian
//---------------------------------------------------------
#ifndef	_BSD_SOURCE
#define _BSD_SOURCE
#define	SET_LOCAL_BSD_SOURCE		1
#endif

#ifdef	HAVE_ENDIAN_H
#include <endian.h>
#else
#ifdef	HAVE_SYS_ENDIAN_H
#include <sys/endian.h>
#endif
#endif

#ifdef	SET_LOCAL_BSD_SOURCE
#undef _BSD_SOURCE
#endif

//---------------------------------------------------------
// types
//---------------------------------------------------------
#define	__STDC_FORMAT_MACROS
#include <inttypes.h>

#endif	// K2HDKCCOMMON_H

/*
 * VIM modelines
 *
 * vim:set ts=4 fenc=utf-8:
 */
