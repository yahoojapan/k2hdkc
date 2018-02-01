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
 * CREATE:   Wed Jul 20 2016
 * REVISION:
 * 
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "k2hdkccomgetsubkeys.h"
#include "k2hdkcutil.h"
#include "k2hdkcdbg.h"

using namespace	std;

//---------------------------------------------------------
// Constructor/Destructor
//---------------------------------------------------------
K2hdkcComGetSubkeys::K2hdkcComGetSubkeys(K2HShm* pk2hash, ChmCntrl* pchmcntrl, uint64_t comnum, bool without_self, bool is_routing_on_server, bool is_wait_on_server) : K2hdkcCommand(pk2hash, pchmcntrl, comnum, without_self, is_routing_on_server, is_wait_on_server, DKC_COM_GET_SUBKEYS)
{
	strClassName = "K2hdkcComGetSubkeys";
}

K2hdkcComGetSubkeys::~K2hdkcComGetSubkeys(void)
{
}

//---------------------------------------------------------
// Methods
//---------------------------------------------------------
PDKCCOM_ALL K2hdkcComGetSubkeys::MakeResponseOnlyHeadData(dkcres_type_t subcode, dkcres_type_t errcode)
{
	PDKCCOM_ALL	pComErr;
	if(NULL == (pComErr = reinterpret_cast<PDKCCOM_ALL>(malloc(sizeof(DKCRES_GET_SUBKEYS))))){
		ERR_DKCPRN("Could not allocate memory.");
		return NULL;
	}
	PDKCRES_GET_SUBKEYS	pComRes	= CVT_DKCRES_GET_SUBKEYS(pComErr);
	pComRes->head.comtype		= DKC_COM_GET_SUBKEYS;
	pComRes->head.restype		= COMPOSE_DKC_RES(errcode, subcode);
	pComRes->head.comnumber		= GetComNumber();
	pComRes->head.dispcomnumber	= GetDispComNumber();
	pComRes->head.length		= sizeof(DKCRES_GET_SUBKEYS);
	pComRes->subkeys_offset		= sizeof(DKCRES_GET_SUBKEYS);
	pComRes->subkeys_length		= 0;

	return pComErr;
}

bool K2hdkcComGetSubkeys::SetResponseData(const unsigned char* pSubkeys, size_t subkeyslength)
{
	if((!pSubkeys && 0 < subkeyslength) || (pSubkeys && 0 == subkeyslength)){
		ERR_DKCPRN("Parameter are wrong.");
		SetErrorResponseData(DKC_RES_SUBCODE_INTERNAL);
		return false;
	}

	// allocate response
	PDKCCOM_ALL	pComAll;
	if(NULL == (pComAll = reinterpret_cast<PDKCCOM_ALL>(malloc(sizeof(DKCRES_GET_SUBKEYS) + (sizeof(unsigned char) * subkeyslength))))){
		ERR_DKCPRN("Could not allocate memory.");
		SetErrorResponseData(DKC_RES_SUBCODE_NOMEM);		// could this set?
		return false;
	}
	PDKCRES_GET_SUBKEYS	pComRes	= CVT_DKCRES_GET_SUBKEYS(pComAll);
	pComRes->head.comtype		= DKC_COM_GET_SUBKEYS;
	pComRes->head.restype		= COMPOSE_DKC_RES(DKC_RES_SUCCESS, DKC_RES_SUBCODE_NOTHING);
	pComRes->head.comnumber		= GetComNumber();
	pComRes->head.dispcomnumber	= GetDispComNumber();
	pComRes->head.length		= sizeof(DKCRES_GET_SUBKEYS) + (sizeof(unsigned char) * subkeyslength);
	pComRes->subkeys_offset		= sizeof(DKCRES_GET_SUBKEYS);
	pComRes->subkeys_length		= subkeyslength;

	if(0 < pComRes->subkeys_length){
		unsigned char*	pdata	= CVT_BIN_DKC_COM_ALL(pComAll, pComRes->subkeys_offset, pComRes->subkeys_length);
		if(!pdata){
			ERR_DKCPRN("Could not convert respond data area.");
			DKC_FREE(pComAll);
			SetErrorResponseData(DKC_RES_SUBCODE_INTERNAL);
			return false;
		}
		memcpy(pdata, pSubkeys, pComRes->subkeys_length);
	}

	if(!SetResponseData(pComAll)){
		ERR_DKCPRN("Failed to set send data.");
		DKC_FREE(pComAll);
		SetErrorResponseData(DKC_RES_SUBCODE_INTERNAL);
		return false;
	}
	return true;
}

bool K2hdkcComGetSubkeys::CommandProcessing(void)
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
		const PDKCCOM_GET_SUBKEYS	pCom = CVT_DKCCOM_GET_SUBKEYS(pRcvComAll);
		const unsigned char*		pKey = GET_BIN_DKC_COM_ALL(pRcvComAll, pCom->key_offset, pCom->key_length);
		if(!pKey){
			ERR_DKCPRN("Could not get safe key data(%zd offset, %zu byte) in DKCCOM_GET_SUBKEYS(%p).", pCom->key_offset, pCom->key_length, pRcvComAll);
			SetErrorResponseData(DKC_RES_SUBCODE_INVAL);
			return false;
		}

		// do command
		K2HSubKeys*	pSubKeys;
		if(NULL == (pSubKeys = pK2hObj->GetSubKeys(pKey, pCom->key_length, pCom->check_attr))){
			MSG_DKCPRN("Error or No subkeys for key(%s)", bin_to_string(pKey, pCom->key_length).c_str());
			SetErrorResponseData(DKC_RES_SUBCODE_NODATA, DKC_RES_SUCCESS);
			return true;															// [NOTE] result is success
		}

		// serialize subkeys
		unsigned char*	bySubkeys		= NULL;
		size_t			subkeys_length	= 0;
		if(!pSubKeys->Serialize(&bySubkeys, subkeys_length)){
			ERR_DKCPRN("Failed to serialize subkeys to binary data for key data(%zd offset, %zu byte) in DKCCOM_GET_SUBKEYS(%p).", pCom->key_offset, pCom->key_length, pRcvComAll);
			SetErrorResponseData(DKC_RES_SUBCODE_INTERNAL);
			DKC_DELETE(pSubKeys);
			return false;
		}
		DKC_DELETE(pSubKeys);

		// set response data
		if(!SetResponseData(bySubkeys, subkeys_length)){
			MSG_DKCPRN("Failed to make response data for key(%s)", bin_to_string(pKey, pCom->key_length).c_str());
			// continue for responsing
		}
		DKC_FREE(bySubkeys);

	}else{
		//
		// receive data is response
		//
		// nothing to do
		MSG_DKCPRN("Received data is response data, so nothing to do here.");
	}
	return true;
}

bool K2hdkcComGetSubkeys::CommandSend(const unsigned char* pkey, size_t keylength, bool checkattr)
{
	if(!pkey || 0 == keylength){
		ERR_DKCPRN("Parameter are wrong.");
		return false;
	}

	// make send data
	PDKCCOM_ALL	pComAll;
	if(NULL == (pComAll = reinterpret_cast<PDKCCOM_ALL>(malloc(sizeof(DKCCOM_GET_SUBKEYS) + keylength)))){
		ERR_DKCPRN("Could not allocate memory.");
		return false;
	}
	PDKCCOM_GET_SUBKEYS	pComGetSubkeys	= CVT_DKCCOM_GET_SUBKEYS(pComAll);
	pComGetSubkeys->head.comtype		= DKC_COM_GET_SUBKEYS;
	pComGetSubkeys->head.restype		= DKC_NORESTYPE;
	pComGetSubkeys->head.comnumber		= GetComNumber();
	pComGetSubkeys->head.dispcomnumber	= GetDispComNumber();
	pComGetSubkeys->head.length			= sizeof(DKCCOM_GET_SUBKEYS) + keylength;
	pComGetSubkeys->key_offset			= sizeof(DKCCOM_GET_SUBKEYS);
	pComGetSubkeys->key_length			= keylength;
	pComGetSubkeys->check_attr			= checkattr;

	unsigned char*	pdata				= reinterpret_cast<unsigned char*>(pComGetSubkeys) + pComGetSubkeys->key_offset;
	memcpy(pdata, pkey, keylength);

	if(!SetSendData(pComAll, K2hdkcCommand::MakeChmpxHash((reinterpret_cast<unsigned char*>(pComGetSubkeys) + pComGetSubkeys->key_offset), keylength))){
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

bool K2hdkcComGetSubkeys::CommandSend(const unsigned char* pkey, size_t keylength, bool checkattr, K2HSubKeys** ppSubKeys, dkcres_type_t* prescode)
{
	if(!pChmObj){
		ERR_DKCPRN("Chmpx object is NULL, so could not send command.");
		return false;
	}

	if(!CommandSend(pkey, keylength, checkattr)){
		ERR_DKCPRN("Failed to send command.");
		return false;
	}
	return GetResponseData(ppSubKeys, prescode);
}

bool K2hdkcComGetSubkeys::GetResponseData(K2HSubKeys** ppSubKeys, dkcres_type_t* prescode) const
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
	PDKCRES_GET_SUBKEYS	pResGetSubkeys = CVT_DKCRES_GET_SUBKEYS(pcomall);
	if(DKC_COM_GET_SUBKEYS != pResGetSubkeys->head.comtype || DKC_NORESTYPE == pResGetSubkeys->head.restype){
		ERR_DKCPRN("Response(received) data is somthing wrong(internal error: data is invalid).");
		return false;
	}
	if(ppSubKeys){
		*ppSubKeys = new K2HSubKeys();
		if(0 < pResGetSubkeys->subkeys_length){
			unsigned char*	pSKey = reinterpret_cast<unsigned char*>(pResGetSubkeys) + pResGetSubkeys->subkeys_offset;	// to absolute address
			if(!(*ppSubKeys)->Serialize(pSKey, pResGetSubkeys->subkeys_length)){
				ERR_DKCPRN("Response(received) data is somthing wrong(internal error: data is invalid).");
				DKC_DELETE((*ppSubKeys));
				return false;
			}
		}
	}
	return true;
}

//---------------------------------------------------------
// Methods - Dump
//---------------------------------------------------------
void K2hdkcComGetSubkeys::RawDumpComAll(const PDKCCOM_ALL pComAll) const
{
	assert(DKC_COM_GET_SUBKEYS == pComAll->com_head.comtype);

	if(DKC_NORESTYPE == pComAll->com_head.restype){
		PDKCCOM_GET_SUBKEYS	pCom = CVT_DKCCOM_GET_SUBKEYS(pComAll);

		RAW_DKCPRN("  DKCCOM_GET_SUBKEYS = {");
		RAW_DKCPRN("    key_offset      = (%016" PRIx64 ") %s",	pCom->key_offset, bin_to_string(GET_BIN_DKC_COM_ALL(pComAll, pCom->key_offset, pCom->key_length), pCom->key_length).c_str());
		RAW_DKCPRN("    key_length      = %zu",					pCom->key_length);
		RAW_DKCPRN("    check_attr      = %s",					pCom->check_attr ? "yes" : "no");
		RAW_DKCPRN("  }");

	}else{
		PDKCRES_GET_SUBKEYS	pCom = CVT_DKCRES_GET_SUBKEYS(pComAll);

		RAW_DKCPRN("  DKCRES_GET_SUBKEYS = {");
		RAW_DKCPRN("    subkeys_offset  = (%016" PRIx64 ") %s",	pCom->subkeys_offset, bin_to_string(GET_BIN_DKC_COM_ALL(pComAll, pCom->subkeys_offset, pCom->subkeys_length), pCom->subkeys_length).c_str());
		RAW_DKCPRN("    subkeys_length  = %zu",					pCom->subkeys_length);
		RAW_DKCPRN("  }");
	}
}

/*
 * VIM modelines
 *
 * vim:set ts=4 fenc=utf-8:
 */
