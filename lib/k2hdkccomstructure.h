/*
 * 
 * K2HDKC
 * 
 * Copyright 2016 Yahoo Japan Corporation.
 * 
 * K2HDKC is k2hash based distributed KVS cluster.
 * K2HDKC uses K2HASH, CHMPX, FULLOCK libraries. K2HDKC supports
 * distributed KVS cluster server program and client libraries.
 * 
 * For the full copyright and license information, please view
 * the license file that was distributed with this source code.
 *
 * AUTHOR:   Takeshi Nakatani
 * CREATE:   Fri Jul 15 2016
 * REVISION:
 * 
 */

#ifndef	K2HDKCCOMSTRUCTURE_H
#define	K2HDKCCOMSTRUCTURE_H

#include "k2hdkccommon.h"
#include "k2hdkctypes.h"
#include "k2hdkc.h"

DECL_EXTERN_C_START

//---------------------------------------------------------
// Common command/response type
//---------------------------------------------------------
//
// Command types
//
// [NOTE]
// following command is a collection of other commands
// 		DKC_COM_ADD_SUBKEY		This command does not have structure, and calls only other commands.
//		DKC_COM_DEL				This command has structure, and calls other commands
//		DKC_COM_DEL_SUBKEY		This command does not have structure, and calls only other commands.
//		DKC_COM_QDEL			This command does not have structure, and calls only DKC_COM_QPOP.
//		DKC_COM_STATE			This command has only response structure, and calls only DKC_COM_K2HSTATE.
//
#define	DKC_COM_MIN						(0L)
#define	DKC_COM_GET						(DKC_COM_MIN)
#define	DKC_COM_GET_DIRECT				(DKC_COM_GET			+ 1L)
#define	DKC_COM_GET_SUBKEYS				(DKC_COM_GET_DIRECT		+ 1L)			// get binary subkeys list formatted by K2HSubKeys(not get subkey's data)
#define	DKC_COM_GET_ATTRS				(DKC_COM_GET_SUBKEYS	+ 1L)			// get binary attrs list formatted by K2HAttrs(not get attr's data)
#define	DKC_COM_GET_ATTR				(DKC_COM_GET_ATTRS		+ 1L)
#define	DKC_COM_SET						(DKC_COM_GET_ATTR		+ 1L)
#define	DKC_COM_SET_DIRECT				(DKC_COM_SET			+ 1L)
#define	DKC_COM_SET_SUBKEYS				(DKC_COM_SET_DIRECT		+ 1L)			// replace key's subkey list by binary subkeys list formatted by K2HSubKeys(only change key's subkey list), can clear all subkey list
#define	DKC_COM_SET_ALL					(DKC_COM_SET_SUBKEYS	+ 1L)			// set all elements(value, subkeys list, attrs) in key by binary data
#define	DKC_COM_ADD_SUBKEYS				(DKC_COM_SET_ALL		+ 1L)			// insert subkey name into key's subkey list without update subkey
#define	DKC_COM_ADD_SUBKEY				(DKC_COM_ADD_SUBKEYS	+ 1L)			// insert subkey name into key's subkey list and set subkey with value
#define	DKC_COM_DEL						(DKC_COM_ADD_SUBKEY		+ 1L)
#define	DKC_COM_DEL_SUBKEYS				(DKC_COM_DEL			+ 1L)			// delete one subkey name from key's subkey list(not delete subkey)
#define	DKC_COM_DEL_SUBKEY				(DKC_COM_DEL_SUBKEYS	+ 1L)			// delete one subkey from key's subkey list and delete subkey
#define	DKC_COM_REN						(DKC_COM_DEL_SUBKEY		+ 1L)
#define	DKC_COM_QPUSH					(DKC_COM_REN			+ 1L)
#define	DKC_COM_QPOP					(DKC_COM_QPUSH			+ 1L)
#define	DKC_COM_QDEL					(DKC_COM_QPOP			+ 1L)
#define	DKC_COM_CAS_INIT				(DKC_COM_QDEL			+ 1L)
#define	DKC_COM_CAS_GET					(DKC_COM_CAS_INIT		+ 1L)
#define	DKC_COM_CAS_SET					(DKC_COM_CAS_GET		+ 1L)
#define	DKC_COM_CAS_INCDEC				(DKC_COM_CAS_SET		+ 1L)
#define	DKC_COM_REPL_KEY				(DKC_COM_CAS_INCDEC		+ 1L)
#define	DKC_COM_K2HSTATE				(DKC_COM_REPL_KEY		+ 1L)
#define	DKC_COM_STATE					(DKC_COM_K2HSTATE		+ 1L)
#define	DKC_COM_MAX						(DKC_COM_STATE)
#if defined(__cplusplus)
#define	DKC_COM_UNKNOWN					static_cast<dkccom_type_t>(-1)
#else
#define	DKC_COM_UNKNOWN					(dkccom_type_t)(-1L)
#endif
//
// Macros for Command type
//
#define	IS_SAFE_DKCCOM_TYPE(comtype)	(DKC_COM_MIN <= comtype && comtype <= DKC_COM_MAX)
#define	STR_DKCCOM_TYPE(comtype)		(	DKC_COM_GET				== comtype ? "DKC_COM_GET"			: \
											DKC_COM_GET_DIRECT		== comtype ? "DKC_COM_GET_DIRECT"	: \
											DKC_COM_GET_SUBKEYS		== comtype ? "DKC_COM_GET_SUBKEYS"	: \
											DKC_COM_GET_ATTRS		== comtype ? "DKC_COM_GET_ATTRS"	: \
											DKC_COM_GET_ATTR		== comtype ? "DKC_COM_GET_ATTR"		: \
											DKC_COM_SET				== comtype ? "DKC_COM_SET"			: \
											DKC_COM_SET_DIRECT		== comtype ? "DKC_COM_SET_DIRECT"	: \
											DKC_COM_SET_SUBKEYS		== comtype ? "DKC_COM_SET_SUBKEYS"	: \
											DKC_COM_ADD_SUBKEY		== comtype ? "DKC_COM_ADD_SUBKEY"	: \
											DKC_COM_SET_ALL			== comtype ? "DKC_COM_SET_ALL"		: \
											DKC_COM_ADD_SUBKEYS		== comtype ? "DKC_COM_ADD_SUBKEYS"	: \
											DKC_COM_DEL				== comtype ? "DKC_COM_DEL"			: \
											DKC_COM_DEL_SUBKEYS		== comtype ? "DKC_COM_DEL_SUBKEYS"	: \
											DKC_COM_DEL_SUBKEY		== comtype ? "DKC_COM_DEL_SUBKEY"	: \
											DKC_COM_REN				== comtype ? "DKC_COM_REN"			: \
											DKC_COM_QPUSH			== comtype ? "DKC_COM_QPUSH"		: \
											DKC_COM_QPOP			== comtype ? "DKC_COM_QPOP"			: \
											DKC_COM_QDEL			== comtype ? "DKC_COM_QDEL"			: \
											DKC_COM_CAS_INIT		== comtype ? "DKC_COM_CAS_INIT"		: \
											DKC_COM_CAS_GET			== comtype ? "DKC_COM_CAS_GET"		: \
											DKC_COM_CAS_SET			== comtype ? "DKC_COM_CAS_SET"		: \
											DKC_COM_CAS_INCDEC		== comtype ? "DKC_COM_CAS_INCDEC"	: \
											DKC_COM_REPL_KEY		== comtype ? "DKC_COM_REPL_KEY"		: \
											DKC_COM_K2HSTATE		== comtype ? "DKC_COM_K2HSTATE"		: \
											DKC_COM_STATE			== comtype ? "DKC_COM_STATE"		: \
											"UNKNOWN_DKC_COM_TYPE"									)

//
// Response types
//
// Response type(64 bit) is built up by high and low 32 bit parts.
// High 32 bit:		means succeed or failed
// Low 32 bit:		means sub code for succeed or failed
//
#define	DKC_RES_MASK64_HIBIT				0xFFFFFFFF00000000
#define	DKC_RES_MASK64_LOWBIT				0x00000000FFFFFFFF

// high 32 bit
#define	DKC_RES_HIGH_SUCCESS				0x0
#define	DKC_RES_HIGH_ERROR					0x80000000
#define	DKC_RES_HIGH_NORESTYPE				0xFFFFFFFF
#define	DKC_RES_SUCCESS						(DKC_RES_HIGH_SUCCESS)
#define	DKC_RES_ERROR						(DKC_RES_HIGH_ERROR)
#define	DKC_RES_NORESTYPE					(DKC_RES_HIGH_NORESTYPE)

// low 32 bit
#define	DKC_RES_LOW_NOTHING					0x0
#define	DKC_RES_LOW_INTERNAL 				0x1							// Internal error
#define	DKC_RES_LOW_NOMEM 					0x2							// Not enough space
#define	DKC_RES_LOW_INVAL 					0x3							// Invalid variables(parameter)
#define	DKC_RES_LOW_DIRECTOPEN 				0x4							// Failed to open direct access
#define	DKC_RES_LOW_LOCK 					0x5							// Failed to lock key(by open direct access)
#define	DKC_RES_LOW_SETVAL 					0x6							// Failed to set value to key
#define	DKC_RES_LOW_GETVAL 					0x7							// Failed to get value to key
#define	DKC_RES_LOW_SETSKEY 				0x8							// Failed to set subkey and value to key
#define	DKC_RES_LOW_SETSKEYLIST 			0x9							// Failed to set subkey list to key
#define	DKC_RES_LOW_GETSKEYLIST 			0xa							// Failed to get subkey list to key
#define	DKC_RES_LOW_ADDSKEYLIST 			0xb							// Failed to add subkey to key
#define	DKC_RES_LOW_SETALL 					0xc							// Failed to set all to key
#define	DKC_RES_LOW_DELKEY 					0xd							// Failed to remove key
#define	DKC_RES_LOW_DELSKEYLIST 			0xe							// Failed to remove subkey name from subkeys list
#define	DKC_RES_LOW_REPL 					0xf							// Failed to replicate all
#define	DKC_RES_LOW_GETMARKER 				0x10						// Failed to get queue marker
#define	DKC_RES_LOW_SETMARKER 				0x11						// Failed to set queue marker
#define	DKC_RES_LOW_GETNEWQKEY 				0x12						// Failed to get new queue key name
#define	DKC_RES_LOW_GETQUEUE 				0x13						// Failed to get queue
#define	DKC_RES_LOW_PUSHQUEUE 				0x14						// Failed to push queue
#define	DKC_RES_LOW_POPQUEUE 				0x15						// Failed to pop queue
#define	DKC_RES_LOW_GETPOPQUEUENAME 		0x16						// Failed to planned pop queue name
#define	DKC_RES_LOW_NOTSAMEVAL 				0x17						// different values
#define	DKC_RES_LOW_NOTSAMEDATATYPE			0x18						// different value type(length)
#define	DKC_RES_LOW_GETSLEFSVRCHMPX			0x19						// Failed to get self chmpx information
#define	DKC_RES_LOW_NOTSAMECHMPXID			0x1a						// different chmpxid
#define	DKC_RES_LOW_GETK2HSTATE				0x1b						// Failed to get k2hash state
#define	DKC_RES_LOW_NOSERVERNODE			0x1c						// no server node
#define	DKC_RES_LOW_NODATA 					0x10000						// No data
#define	DKC_RES_LOW_NORESTYPE				0xFFFFFFFF
#define	DKC_RES_SUBCODE_NOTHING				(DKC_RES_LOW_NOTHING)
#define	DKC_RES_SUBCODE_INTERNAL			(DKC_RES_LOW_INTERNAL)
#define	DKC_RES_SUBCODE_NOMEM				(DKC_RES_LOW_NOMEM)
#define	DKC_RES_SUBCODE_INVAL				(DKC_RES_LOW_INVAL)
#define	DKC_RES_SUBCODE_DIRECTOPEN			(DKC_RES_LOW_DIRECTOPEN)
#define	DKC_RES_SUBCODE_LOCK				(DKC_RES_LOW_LOCK)
#define	DKC_RES_SUBCODE_SETVAL				(DKC_RES_LOW_SETVAL)
#define	DKC_RES_SUBCODE_GETVAL				(DKC_RES_LOW_GETVAL)
#define	DKC_RES_SUBCODE_SETSKEY				(DKC_RES_LOW_SETSKEY)
#define	DKC_RES_SUBCODE_SETSKEYLIST			(DKC_RES_LOW_SETSKEYLIST)
#define	DKC_RES_SUBCODE_GETSKEYLIST			(DKC_RES_LOW_GETSKEYLIST)
#define	DKC_RES_SUBCODE_ADDSKEYLIST			(DKC_RES_LOW_ADDSKEYLIST)
#define	DKC_RES_SUBCODE_SETALL				(DKC_RES_LOW_SETALL)
#define	DKC_RES_SUBCODE_DELKEY				(DKC_RES_LOW_DELKEY)
#define	DKC_RES_SUBCODE_DELSKEYLIST			(DKC_RES_LOW_DELSKEYLIST)
#define	DKC_RES_SUBCODE_REPL				(DKC_RES_LOW_REPL)
#define	DKC_RES_SUBCODE_GETMARKER			(DKC_RES_LOW_GETMARKER)
#define	DKC_RES_SUBCODE_SETMARKER			(DKC_RES_LOW_SETMARKER)
#define	DKC_RES_SUBCODE_GETNEWQKEY			(DKC_RES_LOW_GETNEWQKEY)
#define	DKC_RES_SUBCODE_GETQUEUE			(DKC_RES_LOW_GETQUEUE)
#define	DKC_RES_SUBCODE_PUSHQUEUE 			(DKC_RES_LOW_PUSHQUEUE)
#define	DKC_RES_SUBCODE_POPQUEUE 			(DKC_RES_LOW_POPQUEUE)
#define	DKC_RES_SUBCODE_GETPOPQUEUENAME 	(DKC_RES_LOW_GETPOPQUEUENAME)
#define	DKC_RES_SUBCODE_NOTSAMEVAL 			(DKC_RES_LOW_NOTSAMEVAL)
#define	DKC_RES_SUBCODE_NOTSAMEDATATYPE 	(DKC_RES_LOW_NOTSAMEDATATYPE)
#define	DKC_RES_SUBCODE_GETSLEFSVRCHMPX 	(DKC_RES_LOW_GETSLEFSVRCHMPX)
#define	DKC_RES_SUBCODE_NOTSAMECHMPXID	 	(DKC_RES_LOW_NOTSAMECHMPXID)
#define	DKC_RES_SUBCODE_GETK2HSTATE		 	(DKC_RES_LOW_GETK2HSTATE)
#define	DKC_RES_SUBCODE_NOSERVERNODE	 	(DKC_RES_LOW_NOSERVERNODE)
#define	DKC_RES_SUBCODE_NODATA				(DKC_RES_LOW_NODATA)
#define	DKC_RES_SUBCODE_NORESTYPE			(DKC_RES_LOW_NORESTYPE)

//
// Macros for Response types
//
#if defined(__cplusplus)
#define	COMPOSE_DKC_RES(hi, low)			(((static_cast<dkcres_type_t>(hi) & DKC_RES_MASK64_LOWBIT) << 32) | (static_cast<dkcres_type_t>(low) & DKC_RES_MASK64_LOWBIT))
#define	PARSE_DKC_RES_HIGH(restype)			((static_cast<dkcres_type_t>(restype) & DKC_RES_MASK64_HIBIT) >> 32)
#define	PARSE_DKC_RES_LOW(restype)			(static_cast<dkcres_type_t>(restype) & DKC_RES_MASK64_LOWBIT)
#else
#define	COMPOSE_DKC_RES(hi, low)			((((dkccom_type_t)hi & DKC_RES_MASK64_LOWBIT) << 32) | ((dkccom_type_t)low & DKC_RES_MASK64_LOWBIT))
#define	PARSE_DKC_RES_HIGH(restype)			(((dkccom_type_t)restype & DKC_RES_MASK64_HIBIT) >> 32)
#define	PARSE_DKC_RES_LOW(restype)			((dkccom_type_t)restype & DKC_RES_MASK64_LOWBIT)
#endif

#define	GET_DKC_RES_RESULT(restype)			PARSE_DKC_RES_HIGH(restype)
#define	GET_DKC_RES_SUBCODE(restype)		PARSE_DKC_RES_LOW(restype)

#define	DKC_NORESTYPE						COMPOSE_DKC_RES(DKC_RES_NORESTYPE, DKC_RES_SUBCODE_NORESTYPE)
#define	DKC_INITRESTYPE						COMPOSE_DKC_RES(DKC_RES_SUCCESS, DKC_RES_SUBCODE_NOTHING)

#define	STR_DKCRES_RESULT_TYPE(restype)		(	DKC_RES_SUCCESS					== GET_DKC_RES_RESULT(restype) ? "DKC_RES_SUCCESS"					: \
												DKC_RES_ERROR					== GET_DKC_RES_RESULT(restype) ? "DKC_RES_ERROR"					: \
												DKC_RES_NORESTYPE				== GET_DKC_RES_RESULT(restype) ? "DKC_RES_NORESTYPE"				: \
												"UNKNOWN_DKC_RES_TYPE"																				)

#define	STR_DKCRES_SUBCODE_TYPE(restype)	(	DKC_RES_SUBCODE_NOTHING			== GET_DKC_RES_SUBCODE(restype) ? "DKC_RES_SUBCODE_NOTHING"			: \
												DKC_RES_SUBCODE_INTERNAL		== GET_DKC_RES_SUBCODE(restype) ? "DKC_RES_SUBCODE_INTERNAL"		: \
												DKC_RES_SUBCODE_NOMEM			== GET_DKC_RES_SUBCODE(restype) ? "DKC_RES_SUBCODE_NOMEM"			: \
												DKC_RES_SUBCODE_INVAL			== GET_DKC_RES_SUBCODE(restype) ? "DKC_RES_SUBCODE_INVAL"			: \
												DKC_RES_SUBCODE_DIRECTOPEN		== GET_DKC_RES_SUBCODE(restype) ? "DKC_RES_SUBCODE_DIRECTOPEN"		: \
												DKC_RES_SUBCODE_LOCK			== GET_DKC_RES_SUBCODE(restype) ? "DKC_RES_SUBCODE_LOCK"			: \
												DKC_RES_SUBCODE_SETVAL			== GET_DKC_RES_SUBCODE(restype) ? "DKC_RES_SUBCODE_SETVAL"			: \
												DKC_RES_SUBCODE_GETVAL			== GET_DKC_RES_SUBCODE(restype) ? "DKC_RES_SUBCODE_GETVAL"			: \
												DKC_RES_SUBCODE_SETSKEY			== GET_DKC_RES_SUBCODE(restype) ? "DKC_RES_SUBCODE_SETSKEY"			: \
												DKC_RES_SUBCODE_SETSKEYLIST		== GET_DKC_RES_SUBCODE(restype) ? "DKC_RES_SUBCODE_SETSKEYLIST"		: \
												DKC_RES_SUBCODE_GETSKEYLIST		== GET_DKC_RES_SUBCODE(restype) ? "DKC_RES_SUBCODE_GETSKEYLIST"		: \
												DKC_RES_SUBCODE_ADDSKEYLIST		== GET_DKC_RES_SUBCODE(restype) ? "DKC_RES_SUBCODE_ADDSKEYLIST"		: \
												DKC_RES_SUBCODE_SETALL			== GET_DKC_RES_SUBCODE(restype) ? "DKC_RES_SUBCODE_SETALL"			: \
												DKC_RES_SUBCODE_DELKEY			== GET_DKC_RES_SUBCODE(restype) ? "DKC_RES_SUBCODE_DELKEY"			: \
												DKC_RES_SUBCODE_DELSKEYLIST		== GET_DKC_RES_SUBCODE(restype) ? "DKC_RES_SUBCODE_DELSKEYLIST"		: \
												DKC_RES_SUBCODE_REPL			== GET_DKC_RES_SUBCODE(restype) ? "DKC_RES_SUBCODE_REPL"			: \
												DKC_RES_SUBCODE_GETMARKER		== GET_DKC_RES_SUBCODE(restype) ? "DKC_RES_SUBCODE_GETMARKER"		: \
												DKC_RES_SUBCODE_SETMARKER		== GET_DKC_RES_SUBCODE(restype) ? "DKC_RES_SUBCODE_SETMARKER"		: \
												DKC_RES_SUBCODE_GETNEWQKEY		== GET_DKC_RES_SUBCODE(restype) ? "DKC_RES_SUBCODE_GETNEWQKEY"		: \
												DKC_RES_SUBCODE_GETQUEUE		== GET_DKC_RES_SUBCODE(restype) ? "DKC_RES_SUBCODE_GETQUEUE"		: \
												DKC_RES_SUBCODE_PUSHQUEUE		== GET_DKC_RES_SUBCODE(restype) ? "DKC_RES_SUBCODE_PUSHQUEUE"		: \
												DKC_RES_SUBCODE_POPQUEUE		== GET_DKC_RES_SUBCODE(restype) ? "DKC_RES_SUBCODE_POPQUEUE"		: \
												DKC_RES_SUBCODE_GETPOPQUEUENAME	== GET_DKC_RES_SUBCODE(restype) ? "DKC_RES_SUBCODE_GETPOPQUEUENAME"	: \
												DKC_RES_SUBCODE_NOTSAMEVAL		== GET_DKC_RES_SUBCODE(restype) ? "DKC_RES_SUBCODE_NOTSAMEVAL"		: \
												DKC_RES_SUBCODE_NOTSAMEDATATYPE	== GET_DKC_RES_SUBCODE(restype) ? "DKC_RES_SUBCODE_NOTSAMEDATATYPE"	: \
												DKC_RES_SUBCODE_GETSLEFSVRCHMPX	== GET_DKC_RES_SUBCODE(restype) ? "DKC_RES_SUBCODE_GETSLEFSVRCHMPX"	: \
												DKC_RES_SUBCODE_NOTSAMECHMPXID	== GET_DKC_RES_SUBCODE(restype) ? "DKC_RES_SUBCODE_NOTSAMECHMPXID"	: \
												DKC_RES_SUBCODE_GETK2HSTATE		== GET_DKC_RES_SUBCODE(restype) ? "DKC_RES_SUBCODE_GETK2HSTATE"		: \
												DKC_RES_SUBCODE_NOSERVERNODE	== GET_DKC_RES_SUBCODE(restype) ? "DKC_RES_SUBCODE_NOSERVERNODE"	: \
												DKC_RES_SUBCODE_NODATA			== GET_DKC_RES_SUBCODE(restype) ? "DKC_RES_SUBCODE_NODATA"			: \
												DKC_RES_SUBCODE_NORESTYPE		== GET_DKC_RES_SUBCODE(restype) ? "DKC_RES_SUBCODE_NORESTYPE"		: \
												"UNKNOWN_DKC_RES_SUBCODE_TYPE"																		)

//
// For DKC_COM_SET Command(dkc_qtype_t)
//
#define	DKC_QUEUE_TYPE_MIN					0x0
#define	DKC_QUEUE_TYPE_NOTQUEUE				(DKC_QUEUE_TYPE_MIN)
#define	DKC_QUEUE_TYPE_QUEUE				(DKC_QUEUE_TYPE_NOTQUEUE	+ 1L)
#define	DKC_QUEUE_TYPE_KEYQUEUE				(DKC_QUEUE_TYPE_QUEUE		+ 1L)
#define	DKC_QUEUE_TYPE_MAX					(DKC_QUEUE_TYPE_KEYQUEUE)
#define IS_SAFE_DKC_QUEUE_TYPE(type)		(DKC_QUEUE_TYPE_MIN <= type && type <= DKC_QUEUE_TYPE_MAX)
#define IS_DKC_QUEUE_TYPE(type)				(DKC_QUEUE_TYPE_KEYQUEUE == type || DKC_QUEUE_TYPE_QUEUE == type)
#define	STR_DKC_QUEUE_TYPE(type)			(	DKC_QUEUE_TYPE_NOTQUEUE		== type ? "DKC_QUEUE_TYPE_NOTQUEUE"	: \
												DKC_QUEUE_TYPE_QUEUE		== type ? "DKC_QUEUE_TYPE_QUEUE"	: \
												DKC_QUEUE_TYPE_KEYQUEUE		== type ? "DKC_QUEUE_TYPE_KEYQUEUE"	: \
												"UNKNOWN_DKC_QUEUE_TYPE"										)

//---------------------------------------------------------
// Common command header
//---------------------------------------------------------
//
// Command header
//
typedef struct k2hdkc_com_head{
	dkccom_type_t	comtype;						// command type
	dkcres_type_t	restype;						// response type & code
	uint64_t		comnumber;						// ancestry communication number
	uint64_t		dispcomnumber;					// disposable communication number
	size_t			length;							// total length (including this structure)
}K2HDKC_ATTR_PACKED DKCCOM_HEAD, *PDKCCOM_HEAD;

//---------------------------------------------------------
// Command structures
//---------------------------------------------------------
//
// Get value
//
typedef struct k2hdkc_com_get{
	DKCCOM_HEAD		head;
	off_t			key_offset;
	size_t			key_length;
	off_t			encpass_offset;
	size_t			encpass_length;
	bool			check_attr;
}K2HDKC_ATTR_PACKED DKCCOM_GET, *PDKCCOM_GET;

//
// Get value by direct access
//
typedef struct k2hdkc_com_get_direct{
	DKCCOM_HEAD		head;
	off_t			key_offset;
	size_t			key_length;
	size_t			val_length;
	off_t			val_pos;
}K2HDKC_ATTR_PACKED DKCCOM_GET_DIRECT, *PDKCCOM_GET_DIRECT;

//
// Get subkey list
//
typedef struct k2hdkc_com_get_subkeys{
	DKCCOM_HEAD		head;
	off_t			key_offset;
	size_t			key_length;
	bool			check_attr;
}K2HDKC_ATTR_PACKED DKCCOM_GET_SUBKEYS, *PDKCCOM_GET_SUBKEYS;

//
// Get attribute list
//
typedef struct k2hdkc_com_get_attrs{
	DKCCOM_HEAD		head;
	off_t			key_offset;
	size_t			key_length;
}K2HDKC_ATTR_PACKED DKCCOM_GET_ATTRS, *PDKCCOM_GET_ATTRS;

//
// Get attribute
//
typedef struct k2hdkc_com_get_attr{
	DKCCOM_HEAD		head;
	off_t			key_offset;
	size_t			key_length;
	off_t			attr_offset;
	size_t			attr_length;
}K2HDKC_ATTR_PACKED DKCCOM_GET_ATTR, *PDKCCOM_GET_ATTR;

//
// Set value
//
typedef struct k2hdkc_com_set{
	DKCCOM_HEAD		head;
	off_t			key_offset;
	size_t			key_length;
	off_t			val_offset;
	size_t			val_length;
	off_t			attrs_offset;								// set when queue_type is not DKC_QUEUE_TYPE_NOTQUEUE
	size_t			attrs_length;								// set when queue_type is not DKC_QUEUE_TYPE_NOTQUEUE
	off_t			encpass_offset;
	size_t			encpass_length;
	time_t			expire;
	bool			enable_expire;
	bool			rm_subkeylist;
	dkc_qtype_t		queue_type;									// almost DKC_QUEUE_TYPE_NOTQUEUE
}K2HDKC_ATTR_PACKED DKCCOM_SET, *PDKCCOM_SET;

//
// Set value by direct access
//
typedef struct k2hdkc_com_set_direct{
	DKCCOM_HEAD		head;
	off_t			key_offset;
	size_t			key_length;
	off_t			val_offset;
	size_t			val_length;
	off_t			val_pos;
}K2HDKC_ATTR_PACKED DKCCOM_SET_DIRECT, *PDKCCOM_SET_DIRECT;

//
// Set subkey list to key
//
// [NOTE] This command should not be used.
//
typedef struct k2hdkc_com_set_subkeys{
	DKCCOM_HEAD		head;
	off_t			key_offset;
	size_t			key_length;
	off_t			subkeys_offset;								// serialized data by K2HSubKeys
	size_t			subkeys_length;								// serialized length by K2HSubKeys
}K2HDKC_ATTR_PACKED DKCCOM_SET_SUBKEYS, *PDKCCOM_SET_SUBKEYS;

//
// Set all
//
typedef struct k2hdkc_com_set_all{
	DKCCOM_HEAD		head;
	off_t			key_offset;
	size_t			key_length;
	off_t			val_offset;
	size_t			val_length;
	off_t			subkeys_offset;								// serialized data by K2HSubKeys
	size_t			subkeys_length;								// serialized length by K2HSubKeys
	off_t			attrs_offset;								// serialized data by K2HAttrs
	size_t			attrs_length;								// serialized length by K2HAttrs
}K2HDKC_ATTR_PACKED DKCCOM_SET_ALL, *PDKCCOM_SET_ALL;

//
// Add subkey name to key's subkey list
//
typedef struct k2hdkc_com_add_subkeys{
	DKCCOM_HEAD		head;
	off_t			key_offset;
	size_t			key_length;
	off_t			subkey_offset;
	size_t			subkey_length;
	bool			check_attr;
}K2HDKC_ATTR_PACKED DKCCOM_ADD_SUBKEYS, *PDKCCOM_ADD_SUBKEYS;

//
// Delete key
//
typedef struct k2hdkc_com_del{
	DKCCOM_HEAD		head;
	off_t			key_offset;
	size_t			key_length;
}K2HDKC_ATTR_PACKED DKCCOM_DEL, *PDKCCOM_DEL;

//
// Delete subkey name from key's subkey list without deleting subkey
//
typedef struct k2hdkc_com_del_subkeys{
	DKCCOM_HEAD		head;
	off_t			key_offset;
	size_t			key_length;
	off_t			subkey_offset;
	size_t			subkey_length;
	bool			check_attr;
}K2HDKC_ATTR_PACKED DKCCOM_DEL_SUBKEYS, *PDKCCOM_DEL_SUBKEYS;

//
// Rename key
//
typedef struct k2hdkc_com_ren{
	DKCCOM_HEAD		head;
	off_t			oldkey_offset;
	size_t			oldkey_length;
	off_t			newkey_offset;
	size_t			newkey_length;
	off_t			encpass_offset;
	size_t			encpass_length;
	bool			check_attr;
}K2HDKC_ATTR_PACKED DKCCOM_REN, *PDKCCOM_REN;

//
// Push to queue
//
typedef struct k2hdkc_com_queue_push{
	DKCCOM_HEAD		head;
	bool			is_fifo;
	off_t			prefix_offset;								// Allow empty
	size_t			prefix_length;								// Allow empty
	off_t			key_offset;									// empty when not keyqueue, this value is empty it means not keyqueue type
	size_t			key_length;									// empty when not keyqueue, this value is empty it means not keyqueue type
	off_t			val_offset;
	size_t			val_length;
	off_t			attrs_offset;
	size_t			attrs_length;
	off_t			encpass_offset;
	size_t			encpass_length;
	time_t			expire;
	bool			enable_expire;
	bool			check_attr;
	bool			fifo;
}K2HDKC_ATTR_PACKED DKCCOM_QPUSH, *PDKCCOM_QPUSH;

//
// Pop from queue
//
typedef struct k2hdkc_com_queue_pop{
	DKCCOM_HEAD		head;
	off_t			prefix_offset;
	size_t			prefix_length;
	off_t			encpass_offset;
	size_t			encpass_length;
	bool			check_attr;
	bool			fifo;
	bool			keyqueue;
}K2HDKC_ATTR_PACKED DKCCOM_QPOP, *PDKCCOM_QPOP;

//
// Initialize compare and swap
//
typedef struct k2hdkc_com_cas_init{
	DKCCOM_HEAD		head;
	off_t			key_offset;
	size_t			key_length;
	off_t			val_offset;								// [TODO] care for endian
	size_t			val_length;
	off_t			encpass_offset;
	size_t			encpass_length;
	time_t			expire;
	bool			enable_expire;
}K2HDKC_ATTR_PACKED DKCCOM_CAS_INIT, *PDKCCOM_CAS_INIT;

//
// Get compare and swap
//
typedef struct k2hdkc_com_cas_get{
	DKCCOM_HEAD		head;
	off_t			key_offset;
	size_t			key_length;
	off_t			encpass_offset;
	size_t			encpass_length;
	bool			check_attr;
}K2HDKC_ATTR_PACKED DKCCOM_CAS_GET, *PDKCCOM_CAS_GET;

//
// Set compare and swap
//
typedef struct k2hdkc_com_cas_set{
	DKCCOM_HEAD		head;
	off_t			key_offset;
	size_t			key_length;
	off_t			oldval_offset;								// [TODO] care for endian
	size_t			oldval_length;
	off_t			newval_offset;								// [TODO] care for endian
	size_t			newval_length;
	off_t			encpass_offset;
	size_t			encpass_length;
	time_t			expire;
	bool			enable_expire;
	bool			check_attr;
}K2HDKC_ATTR_PACKED DKCCOM_CAS_SET, *PDKCCOM_CAS_SET;

//
// Increment/Decrement compare and swap
//
typedef struct k2hdkc_com_cas_incdec{
	DKCCOM_HEAD		head;
	off_t			key_offset;
	size_t			key_length;
	off_t			encpass_offset;
	size_t			encpass_length;
	time_t			expire;
	bool			enable_expire;
	bool			check_attr;
	bool			increment;
}K2HDKC_ATTR_PACKED DKCCOM_CAS_INCDEC, *PDKCCOM_CAS_INCDEC;

//
// Replicate key
//
// [NOTE]
// This command is sent only clients on server node.
//
typedef struct k2hdkc_com_replicate_key{
	DKCCOM_HEAD		head;
	k2h_hash_t		hash;
	k2h_hash_t		subhash;
	off_t			key_offset;
	size_t			key_length;
	off_t			val_offset;
	size_t			val_length;
	off_t			subkeys_offset;
	size_t			subkeys_length;
	off_t			attrs_offset;
	size_t			attrs_length;
	struct timespec	ts;
}K2HDKC_ATTR_PACKED DKCCOM_REPL_KEY, *PDKCCOM_REPL_KEY;

//
// K2hash State
//
typedef struct k2hdkc_com_k2hstate{
	DKCCOM_HEAD		head;
	chmpxid_t		chmpxid;
	chmhash_t		base_hash;
}K2HDKC_ATTR_PACKED DKCCOM_K2HSTATE, *PDKCCOM_K2HSTATE;

//---------------------------------------------------------
// Command response structures
//---------------------------------------------------------
//
// Response Get value
//
typedef struct k2hdkc_res_get{
	DKCCOM_HEAD		head;
	off_t			val_offset;
	size_t			val_length;
}K2HDKC_ATTR_PACKED DKCRES_GET, *PDKCRES_GET;

//
// Response Get value by direct access
//
typedef struct k2hdkc_res_get_direct{
	DKCCOM_HEAD		head;
	off_t			val_offset;
	size_t			val_length;
}K2HDKC_ATTR_PACKED DKCRES_GET_DIRECT, *PDKCRES_GET_DIRECT;

//
// Response Get subkey list
//
typedef struct k2hdkc_res_get_subkeys{
	DKCCOM_HEAD		head;
	off_t			subkeys_offset;								// serialized data by K2HSubKeys
	size_t			subkeys_length;								// serialized length by K2HSubKeys
}K2HDKC_ATTR_PACKED DKCRES_GET_SUBKEYS, *PDKCRES_GET_SUBKEYS;

//
// Response Get attribute list
//
typedef struct k2hdkc_res_get_attrs{
	DKCCOM_HEAD		head;
	off_t			attrs_offset;								// serialized data by K2HAttrs
	size_t			attrs_length;								// serialized length by K2HAttrs
}K2HDKC_ATTR_PACKED DKCRES_GET_ATTRS, *PDKCRES_GET_ATTRS;

//
// Response Get attribute
//
typedef struct k2hdkc_res_get_attr{
	DKCCOM_HEAD		head;
	off_t			attrval_offset;
	size_t			attrval_length;
}K2HDKC_ATTR_PACKED DKCRES_GET_ATTR, *PDKCRES_GET_ATTR;

//
// Response Set value
//
typedef struct k2hdkc_res_set{
	DKCCOM_HEAD		head;
}K2HDKC_ATTR_PACKED DKCRES_SET, *PDKCRES_SET;

//
// Response Set value by direct access
//
typedef struct k2hdkc_res_set_direct{
	DKCCOM_HEAD		head;
}K2HDKC_ATTR_PACKED DKCRES_SET_DIRECT, *PDKCRES_SET_DIRECT;

//
// Response Set subkey list to key
//
typedef struct k2hdkc_res_set_subkeys{
	DKCCOM_HEAD		head;
}K2HDKC_ATTR_PACKED DKCRES_SET_SUBKEYS, *PDKCRES_SET_SUBKEYS;

//
// Response Set all
//
typedef struct k2hdkc_res_set_all{
	DKCCOM_HEAD		head;
}K2HDKC_ATTR_PACKED DKCRES_SET_ALL, *PDKCRES_SET_ALL;

//
// Response Add subkey name to key's subkey list
//
typedef struct k2hdkc_res_add_subkeys{
	DKCCOM_HEAD		head;
}K2HDKC_ATTR_PACKED DKCRES_ADD_SUBKEYS, *PDKCRES_ADD_SUBKEYS;

//
// Response Add subkey(and value) to parent key
//
// [NOTE]
// This command is composed of a plurality of other commands.
// And this response is not the response of the communication,
// it reflects the results of other commands.
//
typedef struct k2hdkc_res_add_subkey{
	DKCCOM_HEAD		head;
}K2HDKC_ATTR_PACKED DKCRES_ADD_SUBKEY, *PDKCRES_ADD_SUBKEY;

//
// Response Delete key
//
typedef struct k2hdkc_res_del{
	DKCCOM_HEAD		head;
}K2HDKC_ATTR_PACKED DKCRES_DEL, *PDKCRES_DEL;

//
// Response Delete subkey name from key's subkey list without deleting subkey
//
typedef struct k2hdkc_res_del_subkeys{
	DKCCOM_HEAD		head;
}K2HDKC_ATTR_PACKED DKCRES_DEL_SUBKEYS, *PDKCRES_DEL_SUBKEYS;

//
// Response Delete subkey(and value) to parent key
//
// [NOTE]
// This command is composed of a plurality of other commands.
// And this response is not the response of the communication,
// it reflects the results of other commands.
//
typedef struct k2hdkc_res_del_subkey{
	DKCCOM_HEAD		head;
}K2HDKC_ATTR_PACKED DKCRES_DEL_SUBKEY, *PDKCRES_DEL_SUBKEY;

//
// Response Rename key
//
typedef struct k2hdkc_res_ren{
	DKCCOM_HEAD		head;
}K2HDKC_ATTR_PACKED DKCRES_REN, *PDKCRES_REN;

//
// Response Push to queue
//
typedef struct k2hdkc_res_queue_push{
	DKCCOM_HEAD		head;
}K2HDKC_ATTR_PACKED DKCRES_QPUSH, *PDKCRES_QPUSH;

//
// Response Pop from queue
//
typedef struct k2hdkc_res_queue_pop{
	DKCCOM_HEAD		head;
	off_t			key_offset;									// If normal queue, this value is always empty
	size_t			key_length;									// If normal queue, this value is always empty
	off_t			val_offset;
	size_t			val_length;
}K2HDKC_ATTR_PACKED DKCRES_QPOP, *PDKCRES_QPOP;

//
// Response Delete from queue
//
// [NOTE]
// This command is composed of a plurality of other commands.
// And this response is not the response of the communication,
// it reflects the results of other commands.
//
typedef struct k2hdkc_res_queue_del{
	DKCCOM_HEAD		head;
}K2HDKC_ATTR_PACKED DKCRES_QDEL, *PDKCRES_QDEL;

//
// Response Initialize compare and swap
//
typedef struct k2hdkc_res_cas_init{
	DKCCOM_HEAD		head;
}K2HDKC_ATTR_PACKED DKCRES_CAS_INIT, *PDKCRES_CAS_INIT;

//
// Response Get compare and swap
//
typedef struct k2hdkc_res_cas_get{
	DKCCOM_HEAD		head;
	off_t			val_offset;
	size_t			val_length;
}K2HDKC_ATTR_PACKED DKCRES_CAS_GET, *PDKCRES_CAS_GET;

//
// Response Set compare and swap
//
typedef struct k2hdkc_res_cas_set{
	DKCCOM_HEAD		head;
}K2HDKC_ATTR_PACKED DKCRES_CAS_SET, *PDKCRES_CAS_SET;

//
// Response Increment compare and swap
//
typedef struct k2hdkc_res_cas_incdec{
	DKCCOM_HEAD		head;
}K2HDKC_ATTR_PACKED DKCRES_CAS_INCDEC, *PDKCRES_CAS_INCDEC;


//
// Response Replicate key
//
typedef struct k2hdkc_res_replicate_key{
	DKCCOM_HEAD		head;
}K2HDKC_ATTR_PACKED DKCRES_REPL_KEY, *PDKCRES_REPL_KEY;

//
// Response K2hash State
//
typedef struct k2hdkc_res_k2hstate{
	DKCCOM_HEAD		head;
	DKC_NODESTATE	nodestate;
}K2HDKC_ATTR_PACKED DKCRES_K2HSTATE, *PDKCRES_K2HSTATE;

//
// Response State
//
// [NOTE]
// This command is composed of a plurality of other commands.
// And this response is not the response of the communication,
// it reflects the results of other commands.
//
typedef struct k2hdkc_res_state{
	DKCCOM_HEAD		head;
	off_t			states_offset;					// offset to PDKC_NODESTATE
	size_t			states_count;
}K2HDKC_ATTR_PACKED DKCRES_STATE, *PDKCRES_STATE;

//---------------------------------------------------------
// All command structure
//---------------------------------------------------------
//
// All command union
//
typedef union k2hdkc_com_all{
	DKCCOM_HEAD				com_head;
	DKCCOM_GET				com_get;
	DKCCOM_GET_DIRECT		com_get_direct;
	DKCCOM_GET_SUBKEYS		com_get_subkeys;
	DKCCOM_GET_ATTRS		com_atters;
	DKCCOM_GET_ATTR			com_attr;
	DKCCOM_SET				com_set;
	DKCCOM_SET_DIRECT		com_set_direct;
	DKCCOM_SET_SUBKEYS		com_set_subkeys;
	DKCCOM_SET_ALL			com_set_all;
	DKCCOM_ADD_SUBKEYS		com_add_subkeys;
	DKCCOM_DEL				com_del;
	DKCCOM_DEL_SUBKEYS		com_del_subkeys;
	DKCCOM_REN				com_ren;
	DKCCOM_QPUSH			com_qpush;
	DKCCOM_QPOP				com_qpop;
	DKCCOM_CAS_INIT			com_cas_init;
	DKCCOM_CAS_GET			com_cas_get;
	DKCCOM_CAS_SET			com_cas_set;
	DKCCOM_CAS_INCDEC		com_cas_incdec;
	DKCCOM_REPL_KEY			com_repl_key;
	DKCCOM_K2HSTATE			com_k2hstate;

	DKCRES_GET				res_get;
	DKCRES_GET_DIRECT		res_get_direct;
	DKCRES_GET_SUBKEYS		res_get_subkeys;
	DKCRES_GET_ATTRS		res_attrs;
	DKCRES_GET_ATTR			res_attr;
	DKCRES_SET				res_set;
	DKCRES_SET_DIRECT		res_set_direct;
	DKCRES_SET_SUBKEYS		res_set_subkeys;
	DKCRES_SET_ALL			res_set_all;
	DKCRES_ADD_SUBKEYS		res_add_subkeys;
	DKCRES_ADD_SUBKEY		res_add_subkey;				// [NOTE] there is no corresponding command packet
	DKCRES_DEL				res_del;
	DKCRES_DEL_SUBKEYS		res_del_subkeys;
	DKCRES_DEL_SUBKEY		res_del_subkey;				// [NOTE] there is no corresponding command packet
	DKCRES_REN				res_ren;
	DKCRES_QPUSH			res_qpush;
	DKCRES_QPOP				res_qpop;
	DKCRES_QDEL				res_qdel;					// [NOTE] there is no corresponding command packet
	DKCRES_CAS_INIT			res_cas_init;
	DKCRES_CAS_GET			res_cas_get;
	DKCRES_CAS_SET			res_cas_set;
	DKCRES_CAS_INCDEC		res_cas_incdec;
	DKCRES_REPL_KEY			res_repl_key;
	DKCRES_K2HSTATE			res_k2hstate;
	DKCRES_STATE			res_state;
}K2HDKC_ATTR_PACKED DKCCOM_ALL, *PDKCCOM_ALL;

//---------------------------------------------------------
// Utility Macros
//---------------------------------------------------------
#define	CVT_DKCCOM_HEAD(pComAll)			(&(pComAll->com_head))
#define	CVT_DKCCOM_GET(pComAll)				(&(pComAll->com_get))
#define	CVT_DKCCOM_GET_DIRECT(pComAll)		(&(pComAll->com_get_direct))
#define	CVT_DKCCOM_GET_SUBKEYS(pComAll)		(&(pComAll->com_get_subkeys))
#define	CVT_DKCCOM_GET_ATTRS(pComAll)		(&(pComAll->com_atters))
#define	CVT_DKCCOM_GET_ATTR(pComAll)		(&(pComAll->com_attr))
#define	CVT_DKCCOM_SET(pComAll)				(&(pComAll->com_set))
#define	CVT_DKCCOM_SET_DIRECT(pComAll)		(&(pComAll->com_set_direct))
#define	CVT_DKCCOM_SET_SUBKEYS(pComAll)		(&(pComAll->com_set_subkeys))
#define	CVT_DKCCOM_SET_ALL(pComAll)			(&(pComAll->com_set_all))
#define	CVT_DKCCOM_ADD_SUBKEYS(pComAll)		(&(pComAll->com_add_subkeys))
#define	CVT_DKCCOM_DEL(pComAll)				(&(pComAll->com_del))
#define	CVT_DKCCOM_DEL_SUBKEYS(pComAll)		(&(pComAll->com_del_subkeys))
#define	CVT_DKCCOM_REN(pComAll)				(&(pComAll->com_ren))
#define	CVT_DKCCOM_QPUSH(pComAll)			(&(pComAll->com_qpush))
#define	CVT_DKCCOM_QPOP(pComAll)			(&(pComAll->com_qpop))
#define	CVT_DKCCOM_CAS_INIT(pComAll)		(&(pComAll->com_cas_init))
#define	CVT_DKCCOM_CAS_GET(pComAll)			(&(pComAll->com_cas_get))
#define	CVT_DKCCOM_CAS_SET(pComAll)			(&(pComAll->com_cas_set))
#define	CVT_DKCCOM_CAS_INCDEC(pComAll)		(&(pComAll->com_cas_incdec))
#define	CVT_DKCCOM_REPL_KEY(pComAll)		(&(pComAll->com_repl_key))
#define	CVT_DKCCOM_K2HSTATE(pComAll)		(&(pComAll->com_k2hstate))

#define	CVT_DKCRES_GET(pComAll)				(&(pComAll->res_get))
#define	CVT_DKCRES_GET_DIRECT(pComAll)		(&(pComAll->res_get_direct))
#define	CVT_DKCRES_GET_SUBKEYS(pComAll)		(&(pComAll->res_get_subkeys))
#define	CVT_DKCRES_GET_ATTRS(pComAll)		(&(pComAll->res_attrs))
#define	CVT_DKCRES_GET_ATTR(pComAll)		(&(pComAll->res_attr))
#define	CVT_DKCRES_SET(pComAll)				(&(pComAll->res_set))
#define	CVT_DKCRES_SET_DIRECT(pComAll)		(&(pComAll->res_set_direct))
#define	CVT_DKCRES_SET_SUBKEYS(pComAll)		(&(pComAll->res_set_subkeys))
#define	CVT_DKCRES_SET_ALL(pComAll)			(&(pComAll->res_set_all))
#define	CVT_DKCRES_ADD_SUBKEYS(pComAll)		(&(pComAll->res_add_subkeys))
#define	CVT_DKCRES_ADD_SUBKEY(pComAll)		(&(pComAll->res_add_subkey))
#define	CVT_DKCRES_DEL(pComAll)				(&(pComAll->res_del))
#define	CVT_DKCRES_DEL_SUBKEYS(pComAll)		(&(pComAll->res_del_subkeys))
#define	CVT_DKCRES_DEL_SUBKEY(pComAll)		(&(pComAll->res_del_subkey))
#define	CVT_DKCRES_REN(pComAll)				(&(pComAll->res_ren))
#define	CVT_DKCRES_QPUSH(pComAll)			(&(pComAll->res_qpush))
#define	CVT_DKCRES_QPOP(pComAll)			(&(pComAll->res_qpop))
#define	CVT_DKCRES_QDEL(pComAll)			(&(pComAll->res_qdel))
#define	CVT_DKCRES_CAS_INIT(pComAll)		(&(pComAll->res_cas_init))
#define	CVT_DKCRES_CAS_GET(pComAll)			(&(pComAll->res_cas_get))
#define	CVT_DKCRES_CAS_SET(pComAll)			(&(pComAll->res_cas_set))
#define	CVT_DKCRES_CAS_INCDEC(pComAll)		(&(pComAll->res_cas_incdec))
#define	CVT_DKCRES_REPL_KEY(pComAll)		(&(pComAll->res_repl_key))
#define	CVT_DKCRES_K2HSTATE(pComAll)		(&(pComAll->res_k2hstate))
#define	CVT_DKCRES_STATE(pComAll)			(&(pComAll->res_state))

DECL_EXTERN_C_END

//---------------------------------------------------------
// Debug Utility for Communication
//---------------------------------------------------------
#if defined(__cplusplus)
#include "k2hdkccomdbg.h"
#endif

#endif	// K2HDKCCOMSTRUCTURE_H

/*
 * VIM modelines
 *
 * vim:set ts=4 fenc=utf-8:
 */
