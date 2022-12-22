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

#include "k2hdkccomsetdirect.h"
#include "k2hdkcutil.h"
#include "k2hdkcdbg.h"

using namespace	std;

//---------------------------------------------------------
// Constructor/Destructor
//---------------------------------------------------------
K2hdkcComSetDirect::K2hdkcComSetDirect(K2HShm* pk2hash, ChmCntrl* pchmcntrl, uint64_t comnum, bool without_self, bool is_routing_on_server, bool is_wait_on_server) : K2hdkcCommand(pk2hash, pchmcntrl, comnum, without_self, is_routing_on_server, is_wait_on_server, DKC_COM_SET_DIRECT)
{
	strClassName = "K2hdkcComSetDirect";
}

K2hdkcComSetDirect::~K2hdkcComSetDirect(void)
{
}

//---------------------------------------------------------
// Methods
//---------------------------------------------------------
PDKCCOM_ALL K2hdkcComSetDirect::MakeResponseOnlyHeadData(dkcres_type_t subcode, dkcres_type_t errcode)
{
	PDKCCOM_ALL	pComErr;
	if(NULL == (pComErr = reinterpret_cast<PDKCCOM_ALL>(malloc(sizeof(DKCRES_SET_DIRECT))))){
		ERR_DKCPRN("Could not allocate memory.");
		return NULL;
	}
	PDKCRES_SET_DIRECT	pComRes	= CVT_DKCRES_SET_DIRECT(pComErr);
	pComRes->head.comtype		= DKC_COM_SET_DIRECT;
	pComRes->head.restype		= COMPOSE_DKC_RES(errcode, subcode);
	pComRes->head.comnumber		= GetComNumber();
	pComRes->head.dispcomnumber	= GetDispComNumber();
	pComRes->head.length		= sizeof(DKCRES_SET_DIRECT);

	return pComErr;
}

bool K2hdkcComSetDirect::SetSucceedResponseData(void)
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

bool K2hdkcComSetDirect::CommandProcessing(void)
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
		const PDKCCOM_SET_DIRECT	pCom	= CVT_DKCCOM_SET_DIRECT(pRcvComAll);
		const unsigned char*		pKey	= GET_BIN_DKC_COM_ALL(pRcvComAll, pCom->key_offset, pCom->key_length);
		const unsigned char*		pVal	= GET_BIN_DKC_COM_ALL(pRcvComAll, pCom->val_offset, pCom->val_length);		// allow empty value
		if(!pKey || !pVal){
			ERR_DKCPRN("Could not get safe key(%zd offset, %zu byte) and val(%zd offset, %zu byte) in DKCCOM_SET_DIRECT(%p).", pCom->key_offset, pCom->key_length, pCom->val_offset, pCom->val_length, pRcvComAll);
			SetErrorResponseData(DKC_RES_SUBCODE_INVAL);
			return false;
		}

		// attach direct object
		K2HDAccess*	pAccess;
		if(NULL == (pAccess = pK2hObj->GetDAccessObj(pKey, pCom->key_length, K2HDAccess::WRITE_ACCESS, pCom->val_pos))){
			MSG_DKCPRN("Could not get direct access object from k2hash object for key(%s)", bin_to_string(pKey, pCom->key_length).c_str());
			SetErrorResponseData(DKC_RES_SUBCODE_DIRECTOPEN);
			return false;
		}

		// do command
		if(!pAccess->Write(pVal, pCom->val_length)){
			MSG_DKCPRN("Failed to write val(%s) directly to key(%s)", bin_to_string(pVal, pCom->val_length).c_str(), bin_to_string(pKey, pCom->key_length).c_str());
			SetErrorResponseData(DKC_RES_SUBCODE_SETVAL);
			K2H_Delete(pAccess);
			return false;
		}
		K2H_Delete(pAccess);

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

bool K2hdkcComSetDirect::CommandSend(const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, const off_t valpos)
{
	if(!pkey || 0 == keylength || !pval || 0 == vallength || 0 > valpos){
		ERR_DKCPRN("Parameter are wrong.");
		return false;
	}

	// make send data
	PDKCCOM_ALL	pComAll;
	if(NULL == (pComAll = reinterpret_cast<PDKCCOM_ALL>(malloc(sizeof(DKCCOM_SET_DIRECT) + keylength + vallength)))){
		ERR_DKCPRN("Could not allocate memory.");
		return false;
	}
	PDKCCOM_SET_DIRECT	pComSetDirect	= CVT_DKCCOM_SET_DIRECT(pComAll);
	pComSetDirect->head.comtype			= DKC_COM_SET_DIRECT;
	pComSetDirect->head.restype			= DKC_NORESTYPE;
	pComSetDirect->head.comnumber		= GetComNumber();
	pComSetDirect->head.dispcomnumber	= GetDispComNumber();
	pComSetDirect->head.length			= sizeof(DKCCOM_SET_DIRECT) + keylength + vallength;
	pComSetDirect->key_offset			= sizeof(DKCCOM_SET_DIRECT);
	pComSetDirect->key_length			= keylength;
	pComSetDirect->val_offset			= sizeof(DKCCOM_SET_DIRECT) + keylength;
	pComSetDirect->val_length			= vallength;
	pComSetDirect->val_pos				= valpos;

	unsigned char*	pdata				= reinterpret_cast<unsigned char*>(pComSetDirect) + pComSetDirect->key_offset;
	memcpy(pdata, pkey, keylength);
	pdata								= reinterpret_cast<unsigned char*>(pComSetDirect) + pComSetDirect->val_offset;
	memcpy(pdata, pval, vallength);

	if(!SetSendData(pComAll, K2hdkcCommand::MakeChmpxHash((reinterpret_cast<unsigned char*>(pComSetDirect) + pComSetDirect->key_offset), keylength))){
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

bool K2hdkcComSetDirect::CommandSend(const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, const off_t valpos, dkcres_type_t* prescode)
{
	if(!pChmObj){
		ERR_DKCPRN("Chmpx object is NULL, so could not send command.");
		return false;
	}

	if(!CommandSend(pkey, keylength, pval, vallength, valpos)){
		ERR_DKCPRN("Failed to send command.");
		return false;
	}
	return GetResponseData(prescode);
}

bool K2hdkcComSetDirect::GetResponseData(dkcres_type_t* prescode) const
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
void K2hdkcComSetDirect::RawDumpComAll(const PDKCCOM_ALL pComAll) const
{
	assert(DKC_COM_SET_DIRECT == pComAll->com_head.comtype);

	if(DKC_NORESTYPE == pComAll->com_head.restype){
		PDKCCOM_SET_DIRECT	pCom = CVT_DKCCOM_SET_DIRECT(pComAll);

		RAW_DKCPRN("  DKCCOM_SET_DIRECT = {");
		RAW_DKCPRN("    key_offset      = (%016" PRIx64 ") %s",	pCom->key_offset, bin_to_string(GET_BIN_DKC_COM_ALL(pComAll, pCom->key_offset, pCom->key_length), pCom->key_length).c_str());
		RAW_DKCPRN("    key_length      = %zu",					pCom->key_length);
		RAW_DKCPRN("    val_offset      = (%016" PRIx64 ") %s",	pCom->val_offset, bin_to_string(GET_BIN_DKC_COM_ALL(pComAll, pCom->val_offset, pCom->val_length), pCom->val_length).c_str());
		RAW_DKCPRN("    val_length      = %zu",					pCom->val_length);
		RAW_DKCPRN("    val_pos         = %zd",					pCom->val_pos);
		RAW_DKCPRN("  }");

	}else{
		//PDKCRES_SET_DIRECT	pCom = CVT_DKCRES_SET_DIRECT(pComAll);
		RAW_DKCPRN("  DKCRES_SET_DIRECT = {");
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
