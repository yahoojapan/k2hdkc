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
 * CREATE:   Wed Feb 14 2018
 * REVISION:
 * 
 */

#ifndef	K2HDKCCOMDBG_H
#define	K2HDKCCOMDBG_H

#include "k2hdkccommon.h"
#include "k2hdkccomstructure.h"
#include "k2hdkcutil.h"
#include "k2hdkcdbg.h"

//---------------------------------------------------------
// Debug Utility for Communication
//---------------------------------------------------------
inline bool IS_SAFE_DKC_COM_OFFSET(const DKCCOM_ALL* pComAll, off_t offset, size_t size)
{
	if(!pComAll){
		ERR_DKCPRN("Parameter is wrong.");
		return false;
	}
	if(static_cast<int64_t>(pComAll->com_head.comtype) < DKC_COM_MIN || DKC_COM_MAX < pComAll->com_head.comtype){
		ERR_DKCPRN("DKCCOM_ALL command type(0x%016" PRIx64 ") is wrong.", pComAll->com_head.comtype);
		return false;
	}

	if(	(DKC_COM_GET 			== pComAll->com_head.comtype && pComAll->com_head.length < (size + offset)) ||
		(DKC_COM_GET_DIRECT 	== pComAll->com_head.comtype && pComAll->com_head.length < (size + offset)) ||
		(DKC_COM_GET_SUBKEYS 	== pComAll->com_head.comtype && pComAll->com_head.length < (size + offset)) ||
		(DKC_COM_GET_ATTRS 		== pComAll->com_head.comtype && pComAll->com_head.length < (size + offset)) ||
		(DKC_COM_GET_ATTR 		== pComAll->com_head.comtype && pComAll->com_head.length < (size + offset)) ||
		(DKC_COM_SET 			== pComAll->com_head.comtype && pComAll->com_head.length < (size + offset)) ||
		(DKC_COM_SET_DIRECT 	== pComAll->com_head.comtype && pComAll->com_head.length < (size + offset)) ||
		(DKC_COM_SET_SUBKEYS 	== pComAll->com_head.comtype && pComAll->com_head.length < (size + offset)) ||
		(DKC_COM_SET_ALL 		== pComAll->com_head.comtype && pComAll->com_head.length < (size + offset)) ||
		(DKC_COM_ADD_SUBKEYS 	== pComAll->com_head.comtype && pComAll->com_head.length < (size + offset)) ||
		(DKC_COM_DEL 			== pComAll->com_head.comtype && pComAll->com_head.length < (size + offset)) ||
		(DKC_COM_DEL_SUBKEYS 	== pComAll->com_head.comtype && pComAll->com_head.length < (size + offset)) ||
		(DKC_COM_REN 			== pComAll->com_head.comtype && pComAll->com_head.length < (size + offset)) ||
		(DKC_COM_QPUSH 			== pComAll->com_head.comtype && pComAll->com_head.length < (size + offset)) ||
		(DKC_COM_QPOP 			== pComAll->com_head.comtype && pComAll->com_head.length < (size + offset)) ||
		(DKC_COM_CAS_INIT 		== pComAll->com_head.comtype && pComAll->com_head.length < (size + offset)) ||
		(DKC_COM_CAS_GET 		== pComAll->com_head.comtype && pComAll->com_head.length < (size + offset)) ||
		(DKC_COM_CAS_SET 		== pComAll->com_head.comtype && pComAll->com_head.length < (size + offset)) ||
		(DKC_COM_CAS_INCDEC 	== pComAll->com_head.comtype && pComAll->com_head.length < (size + offset)) ||
		(DKC_COM_REPL_KEY 		== pComAll->com_head.comtype && pComAll->com_head.length < (size + offset)) ||
		(DKC_COM_K2HSTATE 		== pComAll->com_head.comtype && pComAll->com_head.length < (size + offset)) ||
		(DKC_COM_STATE 			== pComAll->com_head.comtype && pComAll->com_head.length < (size + offset)) )
	{
		ERR_DKCPRN("DKCCOM_ALL command type(0x%016" PRIx64 ") length(%zu byte) is too small( + %zu length byte + %zd offset byte ).", pComAll->com_head.comtype, pComAll->com_head.length, size, offset);
		return false;
	}
	return true;
}

inline bool IS_SAFE_DKC_COM_ALL(const DKCCOM_ALL* pComAll)
{
	return IS_SAFE_DKC_COM_OFFSET(pComAll, 0, 0);
}

inline const unsigned char* GET_BIN_DKC_COM_ALL(const DKCCOM_ALL* pComAll, off_t offset, size_t size)
{
	if(!pComAll){
		ERR_DKCPRN("pComAll is NULL.");
		return NULL;
	}
	if(!IS_SAFE_DKC_COM_OFFSET(pComAll, offset, size)){
		return NULL;
	}
	if(0 == size){
		return NULL;
	}
	return (reinterpret_cast<const unsigned char*>(pComAll) + offset);
}

inline unsigned char* CVT_BIN_DKC_COM_ALL(PDKCCOM_ALL pComAll, off_t offset, size_t size)
{
	if(!pComAll){
		ERR_DKCPRN("pComAll is NULL.");
		return NULL;
	}
	if(!IS_SAFE_DKC_COM_OFFSET(pComAll, offset, size)){
		return NULL;
	}
	return (reinterpret_cast<unsigned char*>(pComAll) + offset);
}

inline bool IS_DKC_RES_SUCCESS(dkcres_type_t restype)
{
	return (DKC_RES_HIGH_SUCCESS == PARSE_DKC_RES_HIGH(restype));
}

inline bool IS_DKC_RES_NOTSUCCESS(dkcres_type_t restype)
{
	return (DKC_RES_HIGH_SUCCESS != PARSE_DKC_RES_HIGH(restype));
}

inline void DKC_RES_PRINT(dkccom_type_t comtype, dkcres_type_t restype)
{
	if(IS_DKC_RES_SUCCESS(restype)){
		if(DKC_RES_SUBCODE_NOTHING == GET_DKC_RES_SUBCODE(restype)){
			// success without any message is printed on dump mode.
			DMP_DKCPRN("%s Command Response: DKC_RES_SUCCESS", STR_DKCCOM_TYPE(comtype));
		}else{
			// success with some message is printed on message mode.
			MSG_DKCPRN("%s Command Response: DKC_RES_SUCCESS with %s", STR_DKCCOM_TYPE(comtype), STR_DKCRES_SUBCODE_TYPE(restype));
		}
	}else{
		// error is printed on warning mode.
		WAN_DKCPRN("%s Command Response: DKC_RES_ERROR with %s", STR_DKCCOM_TYPE(comtype), STR_DKCRES_SUBCODE_TYPE(restype));
	}
}

#endif	// K2HDKCCOMDBG_H

/*
 * VIM modelines
 *
 * vim:set ts=4 fenc=utf-8:
 */
