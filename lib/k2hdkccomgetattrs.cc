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
 * CREATE:   Wed Jul 20 2016
 * REVISION:
 * 
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "k2hdkccomgetattrs.h"
#include "k2hdkcutil.h"
#include "k2hdkcdbg.h"

using namespace	std;

//---------------------------------------------------------
// Constructor/Destructor
//---------------------------------------------------------
K2hdkcComGetAttrs::K2hdkcComGetAttrs(K2HShm* pk2hash, ChmCntrl* pchmcntrl, uint64_t comnum, bool without_self, bool is_routing_on_server, bool is_wait_on_server) : K2hdkcCommand(pk2hash, pchmcntrl, comnum, without_self, is_routing_on_server, is_wait_on_server, DKC_COM_GET_ATTRS)
{
	strClassName = "K2hdkcComGetAttrs";
}

K2hdkcComGetAttrs::~K2hdkcComGetAttrs(void)
{
}

//---------------------------------------------------------
// Methods
//---------------------------------------------------------
PDKCCOM_ALL K2hdkcComGetAttrs::MakeResponseOnlyHeadData(dkcres_type_t subcode, dkcres_type_t errcode)
{
	PDKCCOM_ALL	pComErr;
	if(NULL == (pComErr = reinterpret_cast<PDKCCOM_ALL>(malloc(sizeof(DKCRES_GET_ATTRS))))){
		ERR_DKCPRN("Could not allocate memory.");
		return NULL;
	}
	PDKCRES_GET_ATTRS	pComRes	= CVT_DKCRES_GET_ATTRS(pComErr);
	pComRes->head.comtype		= DKC_COM_GET_ATTRS;
	pComRes->head.restype		= COMPOSE_DKC_RES(errcode, subcode);
	pComRes->head.comnumber		= GetComNumber();
	pComRes->head.dispcomnumber	= GetDispComNumber();
	pComRes->head.length		= sizeof(DKCRES_GET_ATTRS);
	pComRes->attrs_offset		= sizeof(DKCRES_GET_ATTRS);
	pComRes->attrs_length		= 0;

	return pComErr;
}

bool K2hdkcComGetAttrs::SetResponseData(const unsigned char* pAttrs, size_t attrslength)
{
	if((!pAttrs && 0 < attrslength) || (pAttrs && 0 == attrslength)){
		ERR_DKCPRN("Parameter are wrong.");
		SetErrorResponseData(DKC_RES_SUBCODE_INTERNAL);
		return false;
	}

	// allocate response
	PDKCCOM_ALL	pComAll;
	if(NULL == (pComAll = reinterpret_cast<PDKCCOM_ALL>(malloc(sizeof(DKCRES_GET_ATTRS) + (sizeof(unsigned char) * attrslength))))){
		ERR_DKCPRN("Could not allocate memory.");
		SetErrorResponseData(DKC_RES_SUBCODE_NOMEM);		// could this set?
		return false;
	}
	PDKCRES_GET_ATTRS	pComRes	= CVT_DKCRES_GET_ATTRS(pComAll);
	pComRes->head.comtype		= DKC_COM_GET_ATTRS;
	pComRes->head.restype		= COMPOSE_DKC_RES(DKC_RES_SUCCESS, DKC_RES_SUBCODE_NOTHING);
	pComRes->head.comnumber		= GetComNumber();
	pComRes->head.dispcomnumber	= GetDispComNumber();
	pComRes->head.length		= sizeof(DKCRES_GET_ATTRS) + (sizeof(unsigned char) * attrslength);
	pComRes->attrs_offset		= sizeof(DKCRES_GET_ATTRS);
	pComRes->attrs_length		= attrslength;

	if(0 < pComRes->attrs_length){
		unsigned char*	pdata	= CVT_BIN_DKC_COM_ALL(pComAll, pComRes->attrs_offset, pComRes->attrs_length);
		if(!pdata){
			ERR_DKCPRN("Could not convert respond data area.");
			DKC_FREE(pComAll);
			SetErrorResponseData(DKC_RES_SUBCODE_INTERNAL);
			return false;
		}
		memcpy(pdata, pAttrs, pComRes->attrs_length);
	}

	if(!SetResponseData(pComAll)){
		ERR_DKCPRN("Failed to set send data.");
		DKC_FREE(pComAll);
		SetErrorResponseData(DKC_RES_SUBCODE_INTERNAL);
		return false;
	}
	return true;
}

bool K2hdkcComGetAttrs::CommandProcessing(void)
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
		const PDKCCOM_GET_ATTRS	pCom = CVT_DKCCOM_GET_ATTRS(pRcvComAll);
		const unsigned char*	pKey = GET_BIN_DKC_COM_ALL(pRcvComAll, pCom->key_offset, pCom->key_length);
		if(!pKey){
			ERR_DKCPRN("Could not get safe key data(%zd offset, %zu byte) in DKCCOM_GET_ATTRS(%p).", pCom->key_offset, pCom->key_length, pRcvComAll);
			SetErrorResponseData(DKC_RES_SUBCODE_INVAL);
			return false;
		}

		// do command
		K2HAttrs*	pAttrsObj;
		if(NULL == (pAttrsObj = pK2hObj->GetAttrs(pKey, pCom->key_length))){
			MSG_DKCPRN("Error or No attributes for key(%s)", bin_to_string(pKey, pCom->key_length).c_str());
			SetErrorResponseData(DKC_RES_SUBCODE_NODATA, DKC_RES_SUCCESS);
			return true;															// [NOTE] result is success
		}

		// serialize attributes
		unsigned char*	byAttrs		= NULL;
		size_t			attrs_length= 0;
		if(!pAttrsObj->Serialize(&byAttrs, attrs_length)){
			ERR_DKCPRN("Failed to serialize attributes to binary data for key data(%zd offset, %zu byte) in DKCCOM_GET_ATTRS(%p).", pCom->key_offset, pCom->key_length, pRcvComAll);
			SetErrorResponseData(DKC_RES_SUBCODE_INTERNAL);
			DKC_DELETE(pAttrsObj);
			return false;
		}
		DKC_DELETE(pAttrsObj);

		// set response data
		if(!SetResponseData(byAttrs, attrs_length)){
			MSG_DKCPRN("Failed to make response data for key(%s)", bin_to_string(pKey, pCom->key_length).c_str());
			// continue for responding
		}
		DKC_FREE(byAttrs);

	}else{
		//
		// receive data is response
		//
		// nothing to do
		MSG_DKCPRN("Received data is response data, so nothing to do here.");
	}
	return true;
}

bool K2hdkcComGetAttrs::CommandSend(const unsigned char* pkey, size_t keylength)
{
	if(!pkey || 0 == keylength){
		ERR_DKCPRN("Parameter are wrong.");
		return false;
	}

	// make send data
	PDKCCOM_ALL	pComAll;
	if(NULL == (pComAll = reinterpret_cast<PDKCCOM_ALL>(malloc(sizeof(DKCCOM_GET_ATTRS) + keylength)))){
		ERR_DKCPRN("Could not allocate memory.");
		return false;
	}
	PDKCCOM_GET_ATTRS	pComGetAttrs= CVT_DKCCOM_GET_ATTRS(pComAll);
	pComGetAttrs->head.comtype		= DKC_COM_GET_ATTRS;
	pComGetAttrs->head.restype		= DKC_NORESTYPE;
	pComGetAttrs->head.comnumber	= GetComNumber();
	pComGetAttrs->head.dispcomnumber= GetDispComNumber();
	pComGetAttrs->head.length		= sizeof(DKCCOM_GET_ATTRS) + keylength;
	pComGetAttrs->key_offset		= sizeof(DKCCOM_GET_ATTRS);
	pComGetAttrs->key_length		= keylength;

	unsigned char*	pdata			= reinterpret_cast<unsigned char*>(pComGetAttrs) + pComGetAttrs->key_offset;
	memcpy(pdata, pkey, keylength);

	if(!SetSendData(pComAll, K2hdkcCommand::MakeChmpxHash((reinterpret_cast<unsigned char*>(pComGetAttrs) + pComGetAttrs->key_offset), keylength))){
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

bool K2hdkcComGetAttrs::CommandSend(const unsigned char* pkey, size_t keylength, K2HAttrs** ppAttrsObj, dkcres_type_t* prescode)
{
	if(!pChmObj){
		ERR_DKCPRN("Chmpx object is NULL, so could not send command.");
		return false;
	}

	if(!CommandSend(pkey, keylength)){
		ERR_DKCPRN("Failed to send command.");
		return false;
	}
	return GetResponseData(ppAttrsObj, prescode);
}

bool K2hdkcComGetAttrs::GetResponseData(K2HAttrs** ppAttrsObj, dkcres_type_t* prescode) const
{
	PDKCCOM_ALL	pcomall = NULL;
	if(!K2hdkcCommand::GetResponseData(&pcomall, NULL, prescode) || !pcomall){
		ERR_DKCPRN("Failed to get result.");
		return false;
	}
	if(DKC_NORESTYPE == pcomall->com_head.restype){
		ERR_DKCPRN("Response(received) data is not received data type.");
		return false;
	}
	PDKCRES_GET_ATTRS	pResGetAttrs = CVT_DKCRES_GET_ATTRS(pcomall);
	if(DKC_COM_GET_ATTRS != pResGetAttrs->head.comtype || DKC_NORESTYPE == pResGetAttrs->head.restype){
		ERR_DKCPRN("Response(received) data is something wrong(internal error: data is invalid).");
		return false;
	}
	if(ppAttrsObj){
		*ppAttrsObj = new K2HAttrs();
		if(0 < pResGetAttrs->attrs_length){
			unsigned char*	pAttr = reinterpret_cast<unsigned char*>(pResGetAttrs) + pResGetAttrs->attrs_offset;	// to absolute address
			if(!(*ppAttrsObj)->Serialize(pAttr, pResGetAttrs->attrs_length)){
				ERR_DKCPRN("Response(received) data is something wrong(internal error: data is invalid).");
				DKC_DELETE((*ppAttrsObj));
				return false;
			}
		}
	}
	return true;
}

//---------------------------------------------------------
// Methods - Dump
//---------------------------------------------------------
void K2hdkcComGetAttrs::RawDumpComAll(const PDKCCOM_ALL pComAll) const
{
	assert(DKC_COM_GET_ATTRS == pComAll->com_head.comtype);

	if(DKC_NORESTYPE == pComAll->com_head.restype){
		PDKCCOM_GET_ATTRS	pCom = CVT_DKCCOM_GET_ATTRS(pComAll);

		RAW_DKCPRN("  DKCCOM_GET_ATTRS = {");
		RAW_DKCPRN("    key_offset      = (%016" PRIx64 ") %s",	pCom->key_offset, bin_to_string(GET_BIN_DKC_COM_ALL(pComAll, pCom->key_offset, pCom->key_length), pCom->key_length).c_str());
		RAW_DKCPRN("    key_length      = %zu",					pCom->key_length);
		RAW_DKCPRN("  }");

	}else{
		PDKCRES_GET_ATTRS	pCom = CVT_DKCRES_GET_ATTRS(pComAll);

		RAW_DKCPRN("  DKCRES_GET_ATTRS = {");
		RAW_DKCPRN("    attrs_offset    = (%016" PRIx64 ") %s",	pCom->attrs_offset, bin_to_string(GET_BIN_DKC_COM_ALL(pComAll, pCom->attrs_offset, pCom->attrs_length), pCom->attrs_length).c_str());
		RAW_DKCPRN("    attrs_length    = %zu",					pCom->attrs_length);
		RAW_DKCPRN("  }");
	}
}

/*
 * VIM modelines
 *
 * vim:set ts=4 fenc=utf-8:
 */
