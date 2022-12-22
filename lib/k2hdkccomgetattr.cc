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

#include "k2hdkccomgetattr.h"
#include "k2hdkcutil.h"
#include "k2hdkcdbg.h"

using namespace	std;

//---------------------------------------------------------
// Constructor/Destructor
//---------------------------------------------------------
K2hdkcComGetAttr::K2hdkcComGetAttr(K2HShm* pk2hash, ChmCntrl* pchmcntrl, uint64_t comnum, bool without_self, bool is_routing_on_server, bool is_wait_on_server) : K2hdkcCommand(pk2hash, pchmcntrl, comnum, without_self, is_routing_on_server, is_wait_on_server, DKC_COM_GET_ATTR)
{
	strClassName = "K2hdkcComGetAttr";
}

K2hdkcComGetAttr::~K2hdkcComGetAttr(void)
{
}

//---------------------------------------------------------
// Methods
//---------------------------------------------------------
PDKCCOM_ALL K2hdkcComGetAttr::MakeResponseOnlyHeadData(dkcres_type_t subcode, dkcres_type_t errcode)
{
	PDKCCOM_ALL	pComErr;
	if(NULL == (pComErr = reinterpret_cast<PDKCCOM_ALL>(malloc(sizeof(DKCRES_GET_ATTR))))){
		ERR_DKCPRN("Could not allocate memory.");
		return NULL;
	}
	PDKCRES_GET_ATTR	pComRes	= CVT_DKCRES_GET_ATTR(pComErr);
	pComRes->head.comtype		= DKC_COM_GET_ATTR;
	pComRes->head.restype		= COMPOSE_DKC_RES(errcode, subcode);
	pComRes->head.comnumber		= GetComNumber();
	pComRes->head.dispcomnumber	= GetDispComNumber();
	pComRes->head.length		= sizeof(DKCRES_GET_ATTR);
	pComRes->attrval_offset		= sizeof(DKCRES_GET_ATTR);
	pComRes->attrval_length		= 0;

	return pComErr;
}

bool K2hdkcComGetAttr::SetResponseData(const unsigned char* pAttrVal, size_t attrvallength)
{
	// allocate response
	PDKCCOM_ALL	pComAll;
	if(NULL == (pComAll = reinterpret_cast<PDKCCOM_ALL>(malloc(sizeof(DKCRES_GET_ATTR) + (sizeof(unsigned char) * attrvallength))))){
		ERR_DKCPRN("Could not allocate memory.");
		SetErrorResponseData(DKC_RES_SUBCODE_NOMEM);		// could this set?
		return false;
	}
	PDKCRES_GET_ATTR	pComRes	= CVT_DKCRES_GET_ATTR(pComAll);
	pComRes->head.comtype		= DKC_COM_GET_ATTR;
	pComRes->head.restype		= COMPOSE_DKC_RES(DKC_RES_SUCCESS, DKC_RES_SUBCODE_NOTHING);
	pComRes->head.comnumber		= GetComNumber();
	pComRes->head.dispcomnumber	= GetDispComNumber();
	pComRes->head.length		= sizeof(DKCRES_GET_ATTR) + (sizeof(unsigned char) * attrvallength);
	pComRes->attrval_offset		= sizeof(DKCRES_GET_ATTR);
	pComRes->attrval_length		= attrvallength;

	if(0 < pComRes->attrval_length){
		unsigned char*	pdata	= CVT_BIN_DKC_COM_ALL(pComAll, pComRes->attrval_offset, pComRes->attrval_length);
		if(!pdata){
			ERR_DKCPRN("Could not convert respond data area.");
			DKC_FREE(pComAll);
			SetErrorResponseData(DKC_RES_SUBCODE_INTERNAL);
			return false;
		}else{
			memcpy(pdata, pAttrVal, pComRes->attrval_length);
		}
	}

	if(!SetResponseData(pComAll)){
		ERR_DKCPRN("Failed to set send data.");
		DKC_FREE(pComAll);
		SetErrorResponseData(DKC_RES_SUBCODE_INTERNAL);
		return false;
	}
	return true;
}

bool K2hdkcComGetAttr::CommandProcessing(void)
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
		const PDKCCOM_GET_ATTR	pCom	= CVT_DKCCOM_GET_ATTR(pRcvComAll);
		const unsigned char*	pKey	= GET_BIN_DKC_COM_ALL(pRcvComAll, pCom->key_offset, pCom->key_length);
		const unsigned char*	pAttr	= GET_BIN_DKC_COM_ALL(pRcvComAll, pCom->attr_offset, pCom->attr_length);
		if(!pKey || !pAttr){
			ERR_DKCPRN("Could not get safe key(%zd offset, %zu byte) and attr(%zd offset, %zu byte) in DKCCOM_GET_ATTR(%p).", pCom->key_offset, pCom->key_length, pCom->attr_offset, pCom->attr_length, pRcvComAll);
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

		// find attribute
		unsigned char*		byAttr		= NULL;
		size_t				attr_length	= 0;
		K2HAttrs::iterator	iter		= pAttrsObj->find(pAttr, pCom->attr_length);
		if(pAttrsObj->end() != iter){
			byAttr		= iter->pval;
			attr_length	= iter->vallength;
		}

		// set response data
		if(!byAttr || !SetResponseData(byAttr, attr_length)){
			MSG_DKCPRN("Failed to make response data for key(%s)", bin_to_string(pKey, pCom->key_length).c_str());
			// continue for responding
		}
		DKC_DELETE(pAttrsObj);

	}else{
		//
		// receive data is response
		//
		// nothing to do
		MSG_DKCPRN("Received data is response data, so nothing to do here.");
	}
	return true;
}

bool K2hdkcComGetAttr::CommandSend(const unsigned char* pkey, size_t keylength, const unsigned char* pattr, size_t attrlength)
{
	if(!pkey || 0 == keylength || !pattr || 0 == attrlength){
		ERR_DKCPRN("Parameter are wrong.");
		return false;
	}

	// make send data
	PDKCCOM_ALL	pComAll;
	if(NULL == (pComAll = reinterpret_cast<PDKCCOM_ALL>(malloc(sizeof(DKCCOM_GET_ATTR) + keylength + attrlength)))){
		ERR_DKCPRN("Could not allocate memory.");
		return false;
	}
	PDKCCOM_GET_ATTR	pComGetAttr	= CVT_DKCCOM_GET_ATTR(pComAll);
	pComGetAttr->head.comtype		= DKC_COM_GET_ATTR;
	pComGetAttr->head.restype		= DKC_NORESTYPE;
	pComGetAttr->head.comnumber		= GetComNumber();
	pComGetAttr->head.dispcomnumber	= GetDispComNumber();
	pComGetAttr->head.length		= sizeof(DKCCOM_GET_ATTR) + keylength + attrlength;
	pComGetAttr->key_offset			= sizeof(DKCCOM_GET_ATTR);
	pComGetAttr->key_length			= keylength;
	pComGetAttr->attr_offset		= sizeof(DKCCOM_GET_ATTR) + keylength;
	pComGetAttr->attr_length		= attrlength;

	unsigned char*	pdata			= reinterpret_cast<unsigned char*>(pComGetAttr) + pComGetAttr->key_offset;
	memcpy(pdata, pkey, keylength);
	pdata							= reinterpret_cast<unsigned char*>(pComGetAttr) + pComGetAttr->attr_offset;
	memcpy(pdata, pattr, attrlength);

	if(!SetSendData(pComAll, K2hdkcCommand::MakeChmpxHash((reinterpret_cast<unsigned char*>(pComGetAttr) + pComGetAttr->key_offset), keylength))){
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

bool K2hdkcComGetAttr::CommandSend(const unsigned char* pkey, size_t keylength, const unsigned char* pattr, size_t attrlength, const unsigned char** ppattrval, size_t* pattrvallength, dkcres_type_t* prescode)
{
	if(!pChmObj){
		ERR_DKCPRN("Chmpx object is NULL, so could not send command.");
		return false;
	}

	if(!CommandSend(pkey, keylength, pattr, attrlength)){
		ERR_DKCPRN("Failed to send command.");
		return false;
	}
	return GetResponseData(ppattrval, pattrvallength, prescode);
}

bool K2hdkcComGetAttr::GetResponseData(const unsigned char** ppattrval, size_t* pattrvallength, dkcres_type_t* prescode) const
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
	PDKCRES_GET_ATTR	pResGetAttr = CVT_DKCRES_GET_ATTR(pcomall);
	if(DKC_COM_GET_ATTR != pResGetAttr->head.comtype || DKC_NORESTYPE == pResGetAttr->head.restype){
		ERR_DKCPRN("Response(received) data is something wrong(internal error: data is invalid).");
		return false;
	}
	if(ppattrval && pattrvallength){
		*ppattrval		= reinterpret_cast<const unsigned char*>(pResGetAttr) + pResGetAttr->attrval_offset;
		*pattrvallength	= pResGetAttr->attrval_length;
	}
	return true;
}

//---------------------------------------------------------
// Methods - Dump
//---------------------------------------------------------
void K2hdkcComGetAttr::RawDumpComAll(const PDKCCOM_ALL pComAll) const
{
	assert(DKC_COM_GET_ATTR == pComAll->com_head.comtype);

	if(DKC_NORESTYPE == pComAll->com_head.restype){
		PDKCCOM_GET_ATTR	pCom = CVT_DKCCOM_GET_ATTR(pComAll);

		RAW_DKCPRN("  DKCCOM_GET_ATTR = {");
		RAW_DKCPRN("    key_offset      = (%016" PRIx64 ") %s",	pCom->key_offset, bin_to_string(GET_BIN_DKC_COM_ALL(pComAll, pCom->key_offset, pCom->key_length), pCom->key_length).c_str());
		RAW_DKCPRN("    key_length      = %zu",					pCom->key_length);
		RAW_DKCPRN("    attr_offset     = (%016" PRIx64 ") %s",	pCom->attr_offset, bin_to_string(GET_BIN_DKC_COM_ALL(pComAll, pCom->attr_offset, pCom->attr_length), pCom->attr_length).c_str());
		RAW_DKCPRN("    attr_length     = %zu",					pCom->attr_length);
		RAW_DKCPRN("  }");

	}else{
		PDKCRES_GET_ATTR	pCom = CVT_DKCRES_GET_ATTR(pComAll);

		RAW_DKCPRN("  DKCRES_GET_ATTR = {");
		RAW_DKCPRN("    attrval_offset  = (%016" PRIx64 ") %s",	pCom->attrval_offset, bin_to_string(GET_BIN_DKC_COM_ALL(pComAll, pCom->attrval_offset, pCom->attrval_length), pCom->attrval_length).c_str());
		RAW_DKCPRN("    attrval_length  = %zu",					pCom->attrval_length);
		RAW_DKCPRN("  }");
	}
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noexpandtab sw=4 ts=4 fdm=marker
 * vim<600: noexpandtab sw=4 ts=4
 */
