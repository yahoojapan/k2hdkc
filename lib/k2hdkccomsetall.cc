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

#include "k2hdkccomsetall.h"
#include "k2hdkcutil.h"
#include "k2hdkcdbg.h"

using namespace	std;

//---------------------------------------------------------
// Constructor/Destructor
//---------------------------------------------------------
K2hdkcComSetAll::K2hdkcComSetAll(K2HShm* pk2hash, ChmCntrl* pchmcntrl, uint64_t comnum, bool without_self, bool is_routing_on_server, bool is_wait_on_server) : K2hdkcCommand(pk2hash, pchmcntrl, comnum, without_self, is_routing_on_server, is_wait_on_server, DKC_COM_SET_ALL)
{
	strClassName = "K2hdkcComSetAll";
}

K2hdkcComSetAll::~K2hdkcComSetAll(void)
{
}

//---------------------------------------------------------
// Methods
//---------------------------------------------------------
PDKCCOM_ALL K2hdkcComSetAll::MakeResponseOnlyHeadData(dkcres_type_t subcode, dkcres_type_t errcode)
{
	PDKCCOM_ALL	pComErr;
	if(NULL == (pComErr = reinterpret_cast<PDKCCOM_ALL>(malloc(sizeof(DKCRES_SET_ALL))))){
		ERR_DKCPRN("Could not allocate memory.");
		return NULL;
	}
	PDKCRES_SET_ALL	pComRes		= CVT_DKCRES_SET_ALL(pComErr);
	pComRes->head.comtype		= DKC_COM_SET_ALL;
	pComRes->head.restype		= COMPOSE_DKC_RES(errcode, subcode);
	pComRes->head.comnumber		= GetComNumber();
	pComRes->head.dispcomnumber	= GetDispComNumber();
	pComRes->head.length		= sizeof(DKCRES_SET_ALL);

	return pComErr;
}

bool K2hdkcComSetAll::SetSucceedResponseData(void)
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

bool K2hdkcComSetAll::CommandProcessing(void)
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
		const PDKCCOM_SET_ALL	pCom	= CVT_DKCCOM_SET_ALL(pRcvComAll);
		const unsigned char*	pKey	= GET_BIN_DKC_COM_ALL(pRcvComAll, pCom->key_offset, pCom->key_length);
		const unsigned char*	pVal	= GET_BIN_DKC_COM_ALL(pRcvComAll, pCom->val_offset, pCom->val_length);			// allow empty value
		const unsigned char*	pSKeys	= GET_BIN_DKC_COM_ALL(pRcvComAll, pCom->subkeys_offset, pCom->subkeys_length);	// allow empty value
		const unsigned char*	pAttrs	= GET_BIN_DKC_COM_ALL(pRcvComAll, pCom->attrs_offset, pCom->attrs_length);		// allow empty value
		if(!pKey){
			ERR_DKCPRN("Could not get safe key(%zd offset, %zu byte) in DKCCOM_SET_ALL(%p).", pCom->key_offset, pCom->key_length, pRcvComAll);
			SetErrorResponseData(DKC_RES_SUBCODE_INVAL);
			return false;
		}

		// do command
		if(!pK2hObj->ReplaceAll(pKey, pCom->key_length, pVal, pCom->val_length, pSKeys, pCom->subkeys_length, pAttrs, pCom->attrs_length)){
			ERR_DKCPRN("Failed to replicate for key(%s), val(%s), subkeys(%s) and attrs(%s)", bin_to_string(pKey, pCom->key_length).c_str(), bin_to_string(pVal, pCom->val_length).c_str(), bin_to_string(pSKeys, pCom->subkeys_length).c_str(), bin_to_string(pAttrs, pCom->attrs_length).c_str());
			SetErrorResponseData(DKC_RES_SUBCODE_SETALL);
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
		MSG_DKCPRN("Received data is response data, so nothing to do here.");
	}
	return true;
}

bool K2hdkcComSetAll::CommandSend(const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, const unsigned char* psubkeys, size_t subkeyslength, const unsigned char* pattrs, size_t attrslength)
{
	if(!pkey || 0 == keylength){
		ERR_DKCPRN("Parameter are wrong.");
		return false;
	}

	// make send data
	PDKCCOM_ALL	pComAll;
	if(NULL == (pComAll = reinterpret_cast<PDKCCOM_ALL>(malloc(sizeof(DKCCOM_SET_ALL) + keylength + vallength + subkeyslength + attrslength)))){
		ERR_DKCPRN("Could not allocate memory.");
		return false;
	}
	PDKCCOM_SET_ALL	pComSetAll		= CVT_DKCCOM_SET_ALL(pComAll);
	pComSetAll->head.comtype		= DKC_COM_SET_ALL;
	pComSetAll->head.restype		= DKC_NORESTYPE;
	pComSetAll->head.comnumber		= GetComNumber();
	pComSetAll->head.dispcomnumber	= GetDispComNumber();
	pComSetAll->head.length			= sizeof(DKCCOM_SET_ALL) + keylength + vallength + subkeyslength + attrslength;
	pComSetAll->key_offset			= sizeof(DKCCOM_SET_ALL);
	pComSetAll->key_length			= keylength;
	pComSetAll->val_offset			= sizeof(DKCCOM_SET_ALL) + keylength;
	pComSetAll->val_length			= vallength;
	pComSetAll->subkeys_offset		= sizeof(DKCCOM_SET_ALL) + keylength + vallength;
	pComSetAll->subkeys_length		= subkeyslength;
	pComSetAll->attrs_offset		= sizeof(DKCCOM_SET_ALL) + keylength + vallength + subkeyslength;
	pComSetAll->attrs_length		= attrslength;

	unsigned char*	pdata			= reinterpret_cast<unsigned char*>(pComSetAll) + pComSetAll->key_offset;
	memcpy(pdata, pkey, keylength);
	if(pval && 0 < vallength){
		pdata						= reinterpret_cast<unsigned char*>(pComSetAll) + pComSetAll->val_offset;
		memcpy(pdata, pval, vallength);
	}
	if(psubkeys && 0 < subkeyslength){
		pdata						= reinterpret_cast<unsigned char*>(pComSetAll) + pComSetAll->subkeys_offset;
		memcpy(pdata, psubkeys, subkeyslength);
	}
	if(pattrs && 0 < attrslength){
		pdata						= reinterpret_cast<unsigned char*>(pComSetAll) + pComSetAll->attrs_offset;
		memcpy(pdata, pattrs, attrslength);
	}

	if(!SetSendData(pComAll, K2hdkcCommand::MakeChmpxHash((reinterpret_cast<unsigned char*>(pComSetAll) + pComSetAll->key_offset), keylength))){
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

bool K2hdkcComSetAll::CommandSend(const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, const unsigned char* psubkeys, size_t subkeyslength, const unsigned char* pattrs, size_t attrslength, dkcres_type_t* prescode)
{
	if(!pChmObj){
		ERR_DKCPRN("Chmpx object is NULL, so could not send command.");
		return false;
	}

	if(!CommandSend(pkey, keylength, pval, vallength, psubkeys, subkeyslength, pattrs, attrslength)){
		ERR_DKCPRN("Failed to send command.");
		return false;
	}
	return GetResponseData(prescode);
}

bool K2hdkcComSetAll::GetResponseData(dkcres_type_t* prescode) const
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
void K2hdkcComSetAll::RawDumpComAll(const PDKCCOM_ALL pComAll) const
{
	assert(DKC_COM_SET_ALL == pComAll->com_head.comtype);

	if(DKC_NORESTYPE == pComAll->com_head.restype){
		PDKCCOM_SET_ALL	pCom = CVT_DKCCOM_SET_ALL(pComAll);

		RAW_DKCPRN("  DKCCOM_SET_ALL = {");
		RAW_DKCPRN("    key_offset      = (%016" PRIx64 ") %s",	pCom->key_offset, bin_to_string(GET_BIN_DKC_COM_ALL(pComAll, pCom->key_offset, pCom->key_length), pCom->key_length).c_str());
		RAW_DKCPRN("    key_length      = %zu",					pCom->key_length);
		RAW_DKCPRN("    val_offset      = (%016" PRIx64 ") %s",	pCom->val_offset, bin_to_string(GET_BIN_DKC_COM_ALL(pComAll, pCom->val_offset, pCom->val_length), pCom->val_length).c_str());
		RAW_DKCPRN("    val_length      = %zu",					pCom->val_length);
		RAW_DKCPRN("    subkeys_offset  = (%016" PRIx64 ") %s",	pCom->subkeys_offset, bin_to_string(GET_BIN_DKC_COM_ALL(pComAll, pCom->subkeys_offset, pCom->subkeys_length), pCom->subkeys_length).c_str());
		RAW_DKCPRN("    subkeys_length  = %zu",					pCom->subkeys_length);
		RAW_DKCPRN("    attrs_offset    = (%016" PRIx64 ") %s",	pCom->attrs_offset, bin_to_string(GET_BIN_DKC_COM_ALL(pComAll, pCom->attrs_offset, pCom->attrs_length), pCom->attrs_length).c_str());
		RAW_DKCPRN("    attrs_length    = %zu",					pCom->attrs_length);
		RAW_DKCPRN("  }");

	}else{
		//PDKCRES_SET_ALL	pCom = CVT_DKCRES_SET_ALL(pComAll);
		RAW_DKCPRN("  DKCRES_SET_ALL = {");
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
