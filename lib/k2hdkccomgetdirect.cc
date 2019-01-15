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
 * CREATE:   Fri Jul 22 2016
 * REVISION:
 * 
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "k2hdkccomgetdirect.h"
#include "k2hdkcutil.h"
#include "k2hdkcdbg.h"

using namespace	std;

//---------------------------------------------------------
// Constructor/Destructor
//---------------------------------------------------------
K2hdkcComGetDirect::K2hdkcComGetDirect(K2HShm* pk2hash, ChmCntrl* pchmcntrl, uint64_t comnum, bool without_self, bool is_routing_on_server, bool is_wait_on_server) : K2hdkcCommand(pk2hash, pchmcntrl, comnum, without_self, is_routing_on_server, is_wait_on_server, DKC_COM_GET_DIRECT)
{
	strClassName = "K2hdkcComGetDirect";
}

K2hdkcComGetDirect::~K2hdkcComGetDirect(void)
{
}

//---------------------------------------------------------
// Methods
//---------------------------------------------------------
PDKCCOM_ALL K2hdkcComGetDirect::MakeResponseOnlyHeadData(dkcres_type_t subcode, dkcres_type_t errcode)
{
	PDKCCOM_ALL	pComErr;
	if(NULL == (pComErr = reinterpret_cast<PDKCCOM_ALL>(malloc(sizeof(DKCRES_GET_DIRECT))))){
		ERR_DKCPRN("Could not allocate memory.");
		return NULL;
	}
	PDKCRES_GET_DIRECT	pComRes	= CVT_DKCRES_GET_DIRECT(pComErr);
	pComRes->head.comtype		= DKC_COM_GET_DIRECT;
	pComRes->head.restype		= COMPOSE_DKC_RES(errcode, subcode);
	pComRes->head.comnumber		= GetComNumber();
	pComRes->head.dispcomnumber	= GetDispComNumber();
	pComRes->head.length		= sizeof(DKCRES_GET_DIRECT);
	pComRes->val_offset			= sizeof(DKCRES_GET_DIRECT);
	pComRes->val_length			= 0;

	return pComErr;
}

bool K2hdkcComGetDirect::SetResponseData(const unsigned char* pValue, size_t ValLen)
{
	if(!pValue || 0 == ValLen){
		ERR_DKCPRN("Parameter are wrong.");
		SetErrorResponseData(DKC_RES_SUBCODE_INTERNAL);
		return false;
	}

	PDKCCOM_ALL	pComAll;
	if(NULL == (pComAll = reinterpret_cast<PDKCCOM_ALL>(malloc(sizeof(DKCRES_GET_DIRECT) + ValLen)))){
		ERR_DKCPRN("Could not allocate memory.");
		SetErrorResponseData(DKC_RES_SUBCODE_NOMEM);		// could this set?
		return false;
	}
	PDKCRES_GET_DIRECT	pComRes	= CVT_DKCRES_GET_DIRECT(pComAll);
	pComRes->head.comtype		= DKC_COM_GET_DIRECT;
	pComRes->head.restype		= COMPOSE_DKC_RES(DKC_RES_SUCCESS, DKC_RES_SUBCODE_NOTHING);
	pComRes->head.comnumber		= GetComNumber();
	pComRes->head.dispcomnumber	= GetDispComNumber();
	pComRes->head.length		= sizeof(DKCRES_GET_DIRECT) + ValLen;
	pComRes->val_offset			= sizeof(DKCRES_GET_DIRECT);
	pComRes->val_length			= ValLen;

	unsigned char*	pdata		= CVT_BIN_DKC_COM_ALL(pComAll, pComRes->val_offset, pComRes->val_length);
	if(!pdata){
		ERR_DKCPRN("Could not convert respond data area.");
		DKC_FREE(pComAll);
		SetErrorResponseData(DKC_RES_SUBCODE_INTERNAL);
		return false;
	}
	memcpy(pdata, pValue, ValLen);

	if(!SetResponseData(pComAll)){
		ERR_DKCPRN("Failed to set send data.");
		DKC_FREE(pComAll);
		SetErrorResponseData(DKC_RES_SUBCODE_INTERNAL);
		return false;
	}
	return true;
}

bool K2hdkcComGetDirect::CommandProcessing(void)
{
	if(!pChmObj || !pRcvComPkt || !pRcvComAll || 0 == RcvComLength){
		ERR_DKCPRN("This object does not initialize yet.");
		SetErrorResponseData(DKC_RES_SUBCODE_INTERNAL);
		return false;
	}

	if(DKC_NORESTYPE == pRcvComAll->com_head.restype){
		//
		// receive data is command
		//
		if(!pK2hObj || !IsServerNode){
			ERR_DKCPRN("This object does not initialize yet.");
			SetErrorResponseData(DKC_RES_SUBCODE_INTERNAL);
			return false;
		}

		// get command data
		const PDKCCOM_GET_DIRECT	pCom = CVT_DKCCOM_GET_DIRECT(pRcvComAll);
		const unsigned char*		pKey = GET_BIN_DKC_COM_ALL(pRcvComAll, pCom->key_offset, pCom->key_length);
		if(!pKey){
			ERR_DKCPRN("Could not get safe key data(%zd offset, %zu byte) in DKCCOM_GET_DIRECT(%p).", pCom->key_offset, pCom->key_length, pRcvComAll);
			SetErrorResponseData(DKC_RES_SUBCODE_INVAL);
			return false;
		}
		if(pCom->val_pos < 0 || 0 == pCom->val_length){
			ERR_DKCPRN("Specified wrong value position(%zd) or length(%zu) in DKCCOM_GET_DIRECT(%p).", pCom->val_pos, pCom->val_length, pRcvComAll);
			SetErrorResponseData(DKC_RES_SUBCODE_INVAL);
			return false;
		}

		// attach direct object
		K2HDAccess*	pAccess;
		if(NULL == (pAccess = pK2hObj->GetDAccessObj(pKey, pCom->key_length, K2HDAccess::READ_ACCESS, pCom->val_pos))){
			ERR_DKCPRN("Could not get direct access object from k2hash object for key(%s)", bin_to_string(pKey, pCom->key_length).c_str());
			SetErrorResponseData(DKC_RES_SUBCODE_DIRECTOPEN);
			return false;
		}

		// do command
		unsigned char*	pValue = NULL;
		size_t			ValLen = pCom->val_length;
		if(!pAccess->Read(&pValue, ValLen) || !pValue || 0 == ValLen){
			MSG_DKCPRN("Error or No data for key(%s)", bin_to_string(pKey, pCom->key_length).c_str());
			SetErrorResponseData(DKC_RES_SUBCODE_NODATA, DKC_RES_SUCCESS);
			K2H_Delete(pAccess);
			return true;															// [NOTE] result is success
		}
		K2H_Delete(pAccess);

		// set response data
		if(!SetResponseData(pValue, ValLen)){
			MSG_DKCPRN("Failed to make response data for key(%s)", bin_to_string(pKey, pCom->key_length).c_str());
			// continue for responding
		}
		DKC_FREE(pValue);

	}else{
		//
		// receive data is response
		//
		// nothing to do
		MSG_DKCPRN("Received data is response data, so nothing to do here.");
	}
	return true;
}

bool K2hdkcComGetDirect::CommandSend(const unsigned char* pkey, size_t keylength, off_t val_pos, size_t val_length)
{
	if(!pkey || 0 == keylength || val_pos < 0 || 0 == val_length){
		ERR_DKCPRN("Parameter are wrong.");
		return false;
	}

	// make send data
	PDKCCOM_ALL	pComAll;
	if(NULL == (pComAll = reinterpret_cast<PDKCCOM_ALL>(malloc(sizeof(DKCCOM_GET_DIRECT) + keylength)))){
		ERR_DKCPRN("Could not allocate memory.");
		return false;
	}
	PDKCCOM_GET_DIRECT	pComGetDirect	= CVT_DKCCOM_GET_DIRECT(pComAll);
	pComGetDirect->head.comtype			= DKC_COM_GET_DIRECT;
	pComGetDirect->head.restype			= DKC_NORESTYPE;
	pComGetDirect->head.comnumber		= GetComNumber();
	pComGetDirect->head.dispcomnumber	= GetDispComNumber();
	pComGetDirect->head.length			= sizeof(DKCCOM_GET_DIRECT) + keylength;
	pComGetDirect->key_offset			= sizeof(DKCCOM_GET_DIRECT);
	pComGetDirect->key_length			= keylength;
	pComGetDirect->val_pos				= val_pos;
	pComGetDirect->val_length			= val_length;

	unsigned char*	pdata				= reinterpret_cast<unsigned char*>(pComGetDirect) + pComGetDirect->key_offset;
	memcpy(pdata, pkey, keylength);

	if(!SetSendData(pComAll, K2hdkcCommand::MakeChmpxHash((reinterpret_cast<unsigned char*>(pComGetDirect) + pComGetDirect->key_offset), keylength))){
		ERR_DKCPRN("Failed to set command data to internal buffer.");
		DKC_FREE(pComAll);
		return false;
	}

	// do command & receive response(on slave node)
	if(!CommandSend()){
		ERR_DKCPRN("Failed to send command.");
		return false;
	}
	return true;
}

bool K2hdkcComGetDirect::CommandSend(const unsigned char* pkey, size_t keylength, off_t val_pos, size_t val_length, const unsigned char** ppval, size_t* pvallength, dkcres_type_t* prescode)
{
	if(!pChmObj){
		ERR_DKCPRN("Chmpx object is NULL, so could not send command.");
		return false;
	}

	if(!CommandSend(pkey, keylength, val_pos, val_length)){
		ERR_DKCPRN("Failed to send command.");
		return false;
	}
	return GetResponseData(ppval, pvallength, prescode);
}

bool K2hdkcComGetDirect::GetResponseData(const unsigned char** ppdata, size_t* plength, dkcres_type_t* prescode) const
{
	PDKCCOM_ALL	pcomall = NULL;
	if(!K2hdkcCommand::GetResponseData(&pcomall, NULL, prescode) || !pcomall){
		ERR_DKCPRN("Failed to get result.");
		return false;
	}
	if(DKC_NORESTYPE == pRcvComAll->com_head.restype){
		ERR_DKCPRN("Response(received) data is not received data type.");
		return false;
	}

	PDKCRES_GET_DIRECT	pResGet = CVT_DKCRES_GET_DIRECT(pcomall);
	if(DKC_COM_GET_DIRECT != pResGet->head.comtype || DKC_NORESTYPE == pResGet->head.restype || RcvComLength != pResGet->head.length){
		ERR_DKCPRN("Response(received) data is something wrong(internal error: data is invalid).");
		return false;
	}

	if(ppdata && plength){
		*ppdata		= reinterpret_cast<const unsigned char*>(pResGet) + pResGet->val_offset;
		*plength	= pResGet->val_length;
	}
	return true;
}

//---------------------------------------------------------
// Methods - Dump
//---------------------------------------------------------
void K2hdkcComGetDirect::RawDumpComAll(const PDKCCOM_ALL pComAll) const
{
	assert(DKC_COM_GET_DIRECT == pComAll->com_head.comtype);

	if(DKC_NORESTYPE == pComAll->com_head.restype){
		PDKCCOM_GET_DIRECT	pCom = CVT_DKCCOM_GET_DIRECT(pComAll);

		RAW_DKCPRN("  DKCCOM_GET_DIRECT = {");
		RAW_DKCPRN("    key_offset      = (%016" PRIx64 ") %s",	pCom->key_offset, bin_to_string(GET_BIN_DKC_COM_ALL(pComAll, pCom->key_offset, pCom->key_length), pCom->key_length).c_str());
		RAW_DKCPRN("    key_length      = %zu",					pCom->key_length);
		RAW_DKCPRN("    val_length      = %zu",					pCom->val_length);
		RAW_DKCPRN("    val_pos         = %zu",					pCom->val_pos);
		RAW_DKCPRN("  }");

	}else{
		PDKCRES_GET_DIRECT	pCom = CVT_DKCRES_GET_DIRECT(pComAll);

		RAW_DKCPRN("  DKCRES_GET_DIRECT = {");
		RAW_DKCPRN("    val_offset      = (%016" PRIx64 ") %s",	pCom->val_offset, bin_to_string(GET_BIN_DKC_COM_ALL(pComAll, pCom->val_offset, pCom->val_length), pCom->val_length).c_str());
		RAW_DKCPRN("    val_length      = %zu",					pCom->val_length);
		RAW_DKCPRN("  }");
	}
}

/*
 * VIM modelines
 *
 * vim:set ts=4 fenc=utf-8:
 */
