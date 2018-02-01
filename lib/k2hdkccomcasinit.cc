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
 * CREATE:   Tue Aug 2 2016
 * REVISION:
 * 
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "k2hdkccomcasinit.h"
#include "k2hdkcutil.h"
#include "k2hdkcdbg.h"

using namespace	std;

//---------------------------------------------------------
// Constructor/Destructor
//---------------------------------------------------------
K2hdkcComCasInit::K2hdkcComCasInit(K2HShm* pk2hash, ChmCntrl* pchmcntrl, uint64_t comnum, bool without_self, bool is_routing_on_server, bool is_wait_on_server) : K2hdkcCommand(pk2hash, pchmcntrl, comnum, without_self, is_routing_on_server, is_wait_on_server, DKC_COM_CAS_INIT)
{
	strClassName = "K2hdkcComCasInit";
}

K2hdkcComCasInit::~K2hdkcComCasInit(void)
{
}

//---------------------------------------------------------
// Methods
//---------------------------------------------------------
PDKCCOM_ALL K2hdkcComCasInit::MakeResponseOnlyHeadData(dkcres_type_t subcode, dkcres_type_t errcode)
{
	PDKCCOM_ALL	pComErr;
	if(NULL == (pComErr = reinterpret_cast<PDKCCOM_ALL>(malloc(sizeof(DKCRES_CAS_INIT))))){
		ERR_DKCPRN("Could not allocate memory.");
		return NULL;
	}
	PDKCRES_CAS_INIT	pComRes	= CVT_DKCRES_CAS_INIT(pComErr);
	pComRes->head.comtype		= DKC_COM_CAS_INIT;
	pComRes->head.restype		= COMPOSE_DKC_RES(errcode, subcode);
	pComRes->head.comnumber		= GetComNumber();
	pComRes->head.dispcomnumber	= GetDispComNumber();
	pComRes->head.length		= sizeof(DKCRES_CAS_INIT);

	return pComErr;
}

bool K2hdkcComCasInit::SetSucceedResponseData(void)
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

bool K2hdkcComCasInit::CommandProcessing(void)
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
		const PDKCCOM_CAS_INIT	pCom	= CVT_DKCCOM_CAS_INIT(pRcvComAll);
		const unsigned char*	pKey	= GET_BIN_DKC_COM_ALL(pRcvComAll, pCom->key_offset, pCom->key_length);
		const unsigned char*	pVal	= GET_BIN_DKC_COM_ALL(pRcvComAll, pCom->val_offset, pCom->val_length);
		char*					pEncPass= NULL;
		if(0 < pCom->encpass_length){
			pEncPass					= reinterpret_cast<char*>(k2hbindup(GET_BIN_DKC_COM_ALL(pRcvComAll, pCom->encpass_offset, pCom->encpass_length), pCom->encpass_length));
		}
		if(!pKey || !pVal){
			ERR_DKCPRN("Could not get safe key(%zd offset, %zu byte) and val(%zd offset, %zu byte) in DKCCOM_CAS_INIT(%p).", pCom->key_offset, pCom->key_length, pCom->val_offset, pCom->val_length, pRcvComAll);
			SetErrorResponseData(DKC_RES_SUBCODE_INVAL);
			DKC_FREE(pEncPass);
			return false;
		}

		// for locking data
		K2HDALock*	pLock = new K2HDALock(pK2hObj, false, pKey, pCom->key_length);

		// do command
		if(!pK2hObj->Set(pKey, pCom->key_length, pVal, pCom->val_length, pEncPass, (pCom->enable_expire ? &(pCom->expire) : NULL))){	// [NOTICE] keep subkeys list
			ERR_DKCPRN("Failed to set val(%s) to key(%s)", bin_to_string(pVal, pCom->val_length).c_str(), bin_to_string(pKey, pCom->key_length).c_str());
			SetErrorResponseData(DKC_RES_SUBCODE_SETVAL);
			DKC_FREE(pEncPass);
			DKC_DELETE(pLock);
			return false;
		}

		// unlock
		DKC_FREE(pEncPass);
		DKC_DELETE(pLock);

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

bool K2hdkcComCasInit::CommandSend(const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, const char* encpass, const time_t* expire)
{
	if(!pkey || 0 == keylength || !pval || 0 == vallength){
		ERR_DKCPRN("Parameter are wrong.");
		return false;
	}

	// make send data
	PDKCCOM_ALL	pComAll;
	if(NULL == (pComAll = reinterpret_cast<PDKCCOM_ALL>(malloc(sizeof(DKCCOM_CAS_INIT) + keylength + vallength + (encpass ? (strlen(encpass) + 1) : 0))))){
		ERR_DKCPRN("Could not allocate memory.");
		return false;
	}
	PDKCCOM_CAS_INIT	pComCasInit	= CVT_DKCCOM_CAS_INIT(pComAll);
	pComCasInit->head.comtype		= DKC_COM_CAS_INIT;
	pComCasInit->head.restype		= DKC_NORESTYPE;
	pComCasInit->head.comnumber		= GetComNumber();
	pComCasInit->head.dispcomnumber	= GetDispComNumber();
	pComCasInit->head.length		= sizeof(DKCCOM_CAS_INIT) + keylength + vallength + (encpass ? (strlen(encpass) + 1) : 0);
	pComCasInit->key_offset			= sizeof(DKCCOM_CAS_INIT);
	pComCasInit->key_length			= keylength;
	pComCasInit->val_offset			= sizeof(DKCCOM_CAS_INIT) + keylength;
	pComCasInit->val_length			= vallength;
	pComCasInit->encpass_offset		= sizeof(DKCCOM_CAS_INIT) + keylength + vallength;
	pComCasInit->encpass_length		= encpass ? (strlen(encpass) + 1) : 0;
	pComCasInit->expire				= expire ? (*expire) : 0L;
	pComCasInit->enable_expire		= expire ? true : false;

	unsigned char*	pdata			= reinterpret_cast<unsigned char*>(pComCasInit) + pComCasInit->key_offset;
	memcpy(pdata, pkey, keylength);
	pdata							= reinterpret_cast<unsigned char*>(pComCasInit) + pComCasInit->val_offset;
	memcpy(pdata, pval, vallength);
	if(encpass){
		pdata						= reinterpret_cast<unsigned char*>(pComCasInit) + pComCasInit->encpass_offset;
		memcpy(pdata, encpass, strlen(encpass) + 1);
	}

	if(!SetSendData(pComAll, K2hdkcCommand::MakeChmpxHash((reinterpret_cast<unsigned char*>(pComCasInit) + pComCasInit->key_offset), keylength))){
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

bool K2hdkcComCasInit::CommandSend(const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, const char* encpass, const time_t* expire, dkcres_type_t* prescode)
{
	if(!pChmObj){
		ERR_DKCPRN("Chmpx object is NULL, so could not send command.");
		return false;
	}

	if(!CommandSend(pkey, keylength, pval, vallength, encpass, expire)){
		ERR_DKCPRN("Failed to send command.");
		return false;
	}
	return GetResponseData(prescode);
}

bool K2hdkcComCasInit::GetResponseData(dkcres_type_t* prescode) const
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
void K2hdkcComCasInit::RawDumpComAll(const PDKCCOM_ALL pComAll) const
{
	assert(DKC_COM_CAS_INIT == pComAll->com_head.comtype);

	if(DKC_NORESTYPE == pComAll->com_head.restype){
		PDKCCOM_CAS_INIT	pCom = CVT_DKCCOM_CAS_INIT(pComAll);

		RAW_DKCPRN("  DKCCOM_CAS_INIT = {");
		RAW_DKCPRN("    key_offset      = (%016" PRIx64 ") %s",	pCom->key_offset, bin_to_string(GET_BIN_DKC_COM_ALL(pComAll, pCom->key_offset, pCom->key_length), pCom->key_length).c_str());
		RAW_DKCPRN("    key_length      = %zu",					pCom->key_length);

		if(sizeof(uint8_t) == pCom->val_length){
			RAW_DKCPRN("    val             = (%016" PRIx64 ") 0x%02" PRIx8 "(uint8_t)",	pCom->val_offset, *(reinterpret_cast<const uint8_t*>(GET_BIN_DKC_COM_ALL(pComAll, pCom->val_offset, pCom->val_length))));
			RAW_DKCPRN("    val_length      = %zu byte(uint8_t)",							pCom->val_length);
		}else if(sizeof(uint16_t) == pCom->val_length){
			RAW_DKCPRN("    val             = (%016" PRIx64 ") 0x%04" PRIx16 "(uint16_t)",	pCom->val_offset, *(reinterpret_cast<const uint16_t*>(GET_BIN_DKC_COM_ALL(pComAll, pCom->val_offset, pCom->val_length))));
			RAW_DKCPRN("    val_length      = %zu byte(uint16_t)",							pCom->val_length);
		}else if(sizeof(uint32_t) == pCom->val_length){
			RAW_DKCPRN("    val             = (%016" PRIx64 ") 0x%04" PRIx32 "(uint32_t)",	pCom->val_offset, *(reinterpret_cast<const uint32_t*>(GET_BIN_DKC_COM_ALL(pComAll, pCom->val_offset, pCom->val_length))));
			RAW_DKCPRN("    val_length      = %zu byte(uint32_t)",							pCom->val_length);
		}else if(sizeof(uint64_t) == pCom->val_length){
			RAW_DKCPRN("    val             = (%016" PRIx64 ") 0x%04" PRIx64 "(uint64_t)",	pCom->val_offset, *(reinterpret_cast<const uint64_t*>(GET_BIN_DKC_COM_ALL(pComAll, pCom->val_offset, pCom->val_length))));
			RAW_DKCPRN("    val_length      = %zu byte(uint64_t)",							pCom->val_length);
		}else if(0 < pCom->val_length){
			RAW_DKCPRN("    val             = (%016" PRIx64 ") %s",							pCom->val_offset, bin_to_string(GET_BIN_DKC_COM_ALL(pComAll, pCom->val_offset, pCom->val_length), pCom->val_length).c_str());
			RAW_DKCPRN("    val_length      = %zu byte(binary)",							pCom->val_length);
		}else{
			RAW_DKCPRN("    val             = (%016" PRIx64 ")",							pCom->val_offset);
			RAW_DKCPRN("    val_length      = %zu byte(wrong val)",							pCom->val_length);
		}

		RAW_DKCPRN("    encpass_offset  = (%016" PRIx64 ") %s",	pCom->encpass_offset, bin_to_string(GET_BIN_DKC_COM_ALL(pComAll, pCom->encpass_offset, pCom->encpass_length), pCom->encpass_length).c_str());
		RAW_DKCPRN("    encpass_length  = %zu",					pCom->encpass_length);
		RAW_DKCPRN("    expire          = %zu",					pCom->expire);
		RAW_DKCPRN("    enable_expire   = %s",					pCom->enable_expire ? "yes" : "no");
		RAW_DKCPRN("  }");

	}else{
		//PDKCRES_CAS_INIT	pCom = CVT_DKCRES_CAS_INIT(pComAll);
		RAW_DKCPRN("  DKCRES_CAS_INIT = {");
			// nothing to print member
		RAW_DKCPRN("  }");
	}
}

/*
 * VIM modelines
 *
 * vim:set ts=4 fenc=utf-8:
 */
