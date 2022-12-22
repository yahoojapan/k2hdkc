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

#include "k2hdkccomsetsubkeys.h"
#include "k2hdkcutil.h"
#include "k2hdkcdbg.h"

using namespace	std;

//---------------------------------------------------------
// Constructor/Destructor
//---------------------------------------------------------
K2hdkcComSetSubkeys::K2hdkcComSetSubkeys(K2HShm* pk2hash, ChmCntrl* pchmcntrl, uint64_t comnum, bool without_self, bool is_routing_on_server, bool is_wait_on_server) : K2hdkcCommand(pk2hash, pchmcntrl, comnum, without_self, is_routing_on_server, is_wait_on_server, DKC_COM_SET_SUBKEYS)
{
	strClassName = "K2hdkcComSetSubkeys";
}

K2hdkcComSetSubkeys::~K2hdkcComSetSubkeys(void)
{
}

//---------------------------------------------------------
// Methods
//---------------------------------------------------------
PDKCCOM_ALL K2hdkcComSetSubkeys::MakeResponseOnlyHeadData(dkcres_type_t subcode, dkcres_type_t errcode)
{
	PDKCCOM_ALL	pComErr;
	if(NULL == (pComErr = reinterpret_cast<PDKCCOM_ALL>(malloc(sizeof(DKCRES_SET_SUBKEYS))))){
		ERR_DKCPRN("Could not allocate memory.");
		return NULL;
	}
	PDKCRES_SET_SUBKEYS	pComRes	= CVT_DKCRES_SET_SUBKEYS(pComErr);
	pComRes->head.comtype		= DKC_COM_SET_SUBKEYS;
	pComRes->head.restype		= COMPOSE_DKC_RES(errcode, subcode);
	pComRes->head.comnumber		= GetComNumber();
	pComRes->head.dispcomnumber	= GetDispComNumber();
	pComRes->head.length		= sizeof(DKCRES_SET_SUBKEYS);

	return pComErr;
}

bool K2hdkcComSetSubkeys::SetSucceedResponseData(void)
{
	PDKCCOM_ALL	pComAll = MakeResponseOnlyHeadData(DKC_RES_SUBCODE_NOTHING, DKC_RES_SUCCESS);

	if(!SetResponseData(pComAll)){
		ERR_DKCPRN("Failed to set send data.");
		DKC_FREE(pComAll);
		SetErrorResponseData(DKC_RES_SUBCODE_INTERNAL);
		return false;
	}
	return true;
}

bool K2hdkcComSetSubkeys::CommandProcessing(void)
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
		const PDKCCOM_SET_SUBKEYS	pCom	= CVT_DKCCOM_SET_SUBKEYS(pRcvComAll);
		const unsigned char*		pKey	= GET_BIN_DKC_COM_ALL(pRcvComAll, pCom->key_offset, pCom->key_length);
		const unsigned char*		pSKeys	= GET_BIN_DKC_COM_ALL(pRcvComAll, pCom->subkeys_offset, pCom->subkeys_length);		// allow empty subkeys
		if(!pKey){
			ERR_DKCPRN("Could not get safe key(%zd offset, %zu byte) and subkeys(%zd offset, %zu byte) in DKCCOM_SET_SUBKEYS(%p).", pCom->key_offset, pCom->key_length, pCom->subkeys_offset, pCom->subkeys_length, pRcvComAll);
			SetErrorResponseData(DKC_RES_SUBCODE_INVAL);
			return false;
		}

		// do command
		if(!pK2hObj->ReplaceSubkeys(pKey, pCom->key_length, pSKeys, pCom->subkeys_length)){
			ERR_DKCPRN("Failed to set subkey list(%s) to key(%s)", bin_to_string(pSKeys, pCom->subkeys_length).c_str(), bin_to_string(pKey, pCom->key_length).c_str());
			SetErrorResponseData(DKC_RES_SUBCODE_SETSKEYLIST);
			return false;
		}

		// get time at setting.
		struct timespec	ts;
		INIT_DKC_TIMESPEC(ts);

		// set response data
		SetSucceedResponseData();

		// do response(reply assap)
		if(!ReplyResponse()){
			ERR_DKCPRN("Failed sending response for key(%s)", bin_to_string(pKey, pCom->key_length).c_str());
			return false;
		}

		// replicate key and datas to other servers
		//
		// [NOTE] reason of sending other server after replying result.
		// because the result of sending replication data to other server on server node can not be caught
		// this event loop, it is caught another session after this event handling.
		// (this result is caught in event loop on server node)
		//
		if(!CommandReplicate(pKey, pCom->key_length, ts)){
			ERR_DKCPRN("Failed to replicate key(%s) to other servers, but continue...", bin_to_string(pKey, pCom->key_length).c_str());
		}

	}else{
		//
		// receive data is response
		//
		// nothing to do
		MSG_DKCPRN("Received data is response data, so nothing to do here.");
	}
	return true;
}

bool K2hdkcComSetSubkeys::CommandSend(const unsigned char* pkey, size_t keylength, const unsigned char* psubkeys, size_t subkeyslength)
{
	if(!pkey || 0 == keylength){
		ERR_DKCPRN("Parameter are wrong.");
		return false;
	}

	// make send data
	PDKCCOM_ALL	pComAll;
	if(NULL == (pComAll = reinterpret_cast<PDKCCOM_ALL>(malloc(sizeof(DKCCOM_SET_SUBKEYS) + keylength + subkeyslength)))){
		ERR_DKCPRN("Could not allocate memory.");
		return false;
	}
	PDKCCOM_SET_SUBKEYS	pComSetSubkeys	= CVT_DKCCOM_SET_SUBKEYS(pComAll);
	pComSetSubkeys->head.comtype		= DKC_COM_SET_SUBKEYS;
	pComSetSubkeys->head.restype		= DKC_NORESTYPE;
	pComSetSubkeys->head.comnumber		= GetComNumber();
	pComSetSubkeys->head.dispcomnumber	= GetDispComNumber();
	pComSetSubkeys->head.length			= sizeof(DKCCOM_SET_SUBKEYS) + keylength + subkeyslength;
	pComSetSubkeys->key_offset			= sizeof(DKCCOM_SET_SUBKEYS);
	pComSetSubkeys->key_length			= keylength;
	pComSetSubkeys->subkeys_offset		= sizeof(DKCCOM_SET_SUBKEYS) + keylength;
	pComSetSubkeys->subkeys_length		= subkeyslength;

	unsigned char*	pdata				= reinterpret_cast<unsigned char*>(pComSetSubkeys) + pComSetSubkeys->key_offset;
	memcpy(pdata, pkey, keylength);
	if(psubkeys && 0 < subkeyslength){
		pdata							= reinterpret_cast<unsigned char*>(pComSetSubkeys) + pComSetSubkeys->subkeys_offset;
		memcpy(pdata, psubkeys, subkeyslength);
	}

	if(!SetSendData(pComAll, K2hdkcCommand::MakeChmpxHash((reinterpret_cast<unsigned char*>(pComSetSubkeys) + pComSetSubkeys->key_offset), keylength))){
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

bool K2hdkcComSetSubkeys::CommandSend(const unsigned char* pkey, size_t keylength, const unsigned char* psubkeys, size_t subkeyslength, dkcres_type_t* prescode)
{
	if(!pChmObj){
		ERR_DKCPRN("Chmpx object is NULL, so could not send command.");
		return false;
	}

	if(!CommandSend(pkey, keylength, psubkeys, subkeyslength)){
		ERR_DKCPRN("Failed to send command.");
		return false;
	}
	return GetResponseData(prescode);
}

bool K2hdkcComSetSubkeys::GetResponseData(dkcres_type_t* prescode) const
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
	return true;
}

//---------------------------------------------------------
// Methods - Dump
//---------------------------------------------------------
void K2hdkcComSetSubkeys::RawDumpComAll(const PDKCCOM_ALL pComAll) const
{
	assert(DKC_COM_SET_SUBKEYS == pComAll->com_head.comtype);

	if(DKC_NORESTYPE == pComAll->com_head.restype){
		PDKCCOM_SET_SUBKEYS	pCom = CVT_DKCCOM_SET_SUBKEYS(pComAll);

		RAW_DKCPRN("  DKCCOM_SET_SUBKEYS = {");
		RAW_DKCPRN("    key_offset      = (%016" PRIx64 ") %s",	pCom->key_offset, bin_to_string(GET_BIN_DKC_COM_ALL(pComAll, pCom->key_offset, pCom->key_length), pCom->key_length).c_str());
		RAW_DKCPRN("    key_length      = %zu",					pCom->key_length);
		RAW_DKCPRN("    subkeys_offset  = (%016" PRIx64 ") %s",	pCom->subkeys_offset, bin_to_string(GET_BIN_DKC_COM_ALL(pComAll, pCom->subkeys_offset, pCom->subkeys_length), pCom->subkeys_length).c_str());
		RAW_DKCPRN("    subkeys_length  = %zu",					pCom->subkeys_length);
		RAW_DKCPRN("  }");

	}else{
		//PDKCRES_SET_SUBKEYS	pCom = CVT_DKCRES_SET_SUBKEYS(pComAll);
		RAW_DKCPRN("  DKCRES_SET_SUBKEYS = {");
			// nothing to print member
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
