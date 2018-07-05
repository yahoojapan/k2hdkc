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

#include "k2hdkccomreplkey.h"
#include "k2hdkcutil.h"
#include "k2hdkcdbg.h"

using namespace	std;

//---------------------------------------------------------
// Constructor/Destructor
//---------------------------------------------------------
K2hdkcComReplKey::K2hdkcComReplKey(K2HShm* pk2hash, ChmCntrl* pchmcntrl, uint64_t comnum, bool without_self, bool is_routing_on_server, bool is_wait_on_server) : K2hdkcCommand(pk2hash, pchmcntrl, comnum, without_self, is_routing_on_server, is_wait_on_server, DKC_COM_REPL_KEY)
{
	strClassName = "K2hdkcComReplKey";

	// [NOTE]
	// This command must be replicate mode for all case.
	// (RoutingOnSlave does not care, but set here.)
	//
	SetReplicateSend();
	UnsetRoutingOnSlave();
}

K2hdkcComReplKey::~K2hdkcComReplKey(void)
{
}

//---------------------------------------------------------
// Methods
//---------------------------------------------------------
PDKCCOM_ALL K2hdkcComReplKey::MakeResponseOnlyHeadData(dkcres_type_t subcode, dkcres_type_t errcode)
{
	PDKCCOM_ALL	pComErr;
	if(NULL == (pComErr = reinterpret_cast<PDKCCOM_ALL>(malloc(sizeof(DKCRES_REPL_KEY))))){
		ERR_DKCPRN("Could not allocate memory.");
		return NULL;
	}
	PDKCRES_REPL_KEY	pComRes	= CVT_DKCRES_REPL_KEY(pComErr);
	pComRes->head.comtype		= DKC_COM_REPL_KEY;
	pComRes->head.restype		= COMPOSE_DKC_RES(errcode, subcode);
	pComRes->head.comnumber		= GetComNumber();
	pComRes->head.dispcomnumber	= GetDispComNumber();
	pComRes->head.length		= sizeof(DKCRES_REPL_KEY);

	return pComErr;
}

bool K2hdkcComReplKey::SetSucceedResponseData(void)
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

bool K2hdkcComReplKey::CommandProcessing(void)
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
		const PDKCCOM_REPL_KEY	pCom	= CVT_DKCCOM_REPL_KEY(pRcvComAll);
		const unsigned char*	pKey	= GET_BIN_DKC_COM_ALL(pRcvComAll, pCom->key_offset, pCom->key_length);
		const unsigned char*	pVal	= GET_BIN_DKC_COM_ALL(pRcvComAll, pCom->val_offset, pCom->val_length);			// allow empty value
		const unsigned char*	pSKeys	= GET_BIN_DKC_COM_ALL(pRcvComAll, pCom->subkeys_offset, pCom->subkeys_length);	// allow empty value
		const unsigned char*	pAttrs	= GET_BIN_DKC_COM_ALL(pRcvComAll, pCom->attrs_offset, pCom->attrs_length);		// allow empty value
		if(!pKey){
			ERR_DKCPRN("Could not get safe key(%zd offset, %zu byte) in DKCCOM_REPL_KEY(%p).", pCom->key_offset, pCom->key_length, pRcvComAll);
			SetErrorResponseData(DKC_RES_SUBCODE_INVAL);
			return false;
		}

		// make RALLEDATA buffer & set
		PRALLEDATA	pRAllData;
		if(NULL == (pRAllData = reinterpret_cast<PRALLEDATA>(malloc(sizeof(RALLEDATA) + pCom->key_length + pCom->val_length + pCom->subkeys_length + pCom->attrs_length)))){
			ERR_DKCPRN("Could not allocate memory.");
			return false;
		}
		pRAllData->hash			= pCom->hash;
		pRAllData->subhash		= pCom->subhash;
		pRAllData->key_length	= pCom->key_length;
		pRAllData->val_length	= pCom->val_length;
		pRAllData->skey_length	= pCom->subkeys_length;
		pRAllData->attrs_length	= pCom->attrs_length;
		pRAllData->key_pos		= sizeof(RALLEDATA);
		pRAllData->val_pos		= sizeof(RALLEDATA) + pCom->key_length;
		pRAllData->skey_pos		= sizeof(RALLEDATA) + pCom->key_length + pCom->val_length;
		pRAllData->attrs_pos	= sizeof(RALLEDATA) + pCom->key_length + pCom->val_length + pCom->subkeys_length;
		if(0 < pCom->key_length){
			memcpy((reinterpret_cast<unsigned char*>(pRAllData) + pRAllData->key_pos), pKey, pCom->key_length);
		}
		if(0 < pCom->val_length){
			memcpy((reinterpret_cast<unsigned char*>(pRAllData) + pRAllData->val_pos), pVal, pCom->val_length);
		}
		if(0 < pCom->subkeys_length){
			memcpy((reinterpret_cast<unsigned char*>(pRAllData) + pRAllData->skey_pos), pSKeys, pCom->subkeys_length);
		}
		if(0 < pCom->attrs_length){
			memcpy((reinterpret_cast<unsigned char*>(pRAllData) + pRAllData->attrs_pos), pAttrs, pCom->attrs_length);
		}

		// do command
		//
		// [NOTE]
		// SetElementByBinArray method does not trigger transaction.
		//
		if(!pK2hObj->SetElementByBinArray(pRAllData, &(pCom->ts))){
			ERR_DKCPRN("Failed to replicate for key(%s)", bin_to_string(pKey, pCom->key_length).c_str());
			SetErrorResponseData(DKC_RES_SUBCODE_REPL);
			DKC_FREE(pRAllData);
			return false;
		}
		DKC_FREE(pRAllData);

		// set response data
		SetSucceedResponseData();

	}else{
		//
		// receive data is response
		//
		// get receive data
		const PDKCRES_REPL_KEY	pComRes = CVT_DKCRES_REPL_KEY(pRcvComAll);

		// print result
		DKC_RES_PRINT(pComRes->head.comtype, pComRes->head.restype);
	}
	return true;
}

bool K2hdkcComReplKey::CommandSend(k2h_hash_t hash, k2h_hash_t subhash, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, const unsigned char* psubkeys, size_t subkeyslength, const unsigned char* pattrs, size_t attrslength, const struct timespec ts)
{
	if(!pkey || 0 == keylength){
		ERR_DKCPRN("Parameter are wrong.");
		return false;
	}
	if(!IsServerNode){
		MSG_DKCPRN("This DKC_COM_REPL_KEY command must be sent on server node.");
		return false;
	}

	// make send data
	PDKCCOM_ALL	pComAll;
	if(NULL == (pComAll = reinterpret_cast<PDKCCOM_ALL>(malloc(sizeof(DKCCOM_REPL_KEY) + keylength + vallength + subkeyslength + attrslength)))){
		ERR_DKCPRN("Could not allocate memory.");
		return false;
	}
	PDKCCOM_REPL_KEY	pComReplKey	= CVT_DKCCOM_REPL_KEY(pComAll);
	pComReplKey->head.comtype		= DKC_COM_REPL_KEY;
	pComReplKey->head.restype		= DKC_NORESTYPE;
	pComReplKey->head.comnumber		= GetComNumber();
	pComReplKey->head.dispcomnumber	= GetDispComNumber();
	pComReplKey->head.length		= sizeof(DKCCOM_REPL_KEY) + keylength + vallength + subkeyslength + attrslength;
	pComReplKey->hash				= hash;
	pComReplKey->subhash			= subhash;
	pComReplKey->key_offset			= sizeof(DKCCOM_REPL_KEY);
	pComReplKey->key_length			= keylength;
	pComReplKey->val_offset			= sizeof(DKCCOM_REPL_KEY) + keylength;
	pComReplKey->val_length			= vallength;
	pComReplKey->subkeys_offset		= sizeof(DKCCOM_REPL_KEY) + keylength + vallength;
	pComReplKey->subkeys_length		= subkeyslength;
	pComReplKey->attrs_offset		= sizeof(DKCCOM_REPL_KEY) + keylength + vallength + subkeyslength;
	pComReplKey->attrs_length		= attrslength;
	pComReplKey->ts					= ts;

	unsigned char*	pdata			= reinterpret_cast<unsigned char*>(pComReplKey) + pComReplKey->key_offset;
	memcpy(pdata, pkey, keylength);
	if(pval && 0 < vallength){
		pdata						= reinterpret_cast<unsigned char*>(pComReplKey) + pComReplKey->val_offset;
		memcpy(pdata, pval, vallength);
	}
	if(psubkeys && 0 < subkeyslength){
		pdata						= reinterpret_cast<unsigned char*>(pComReplKey) + pComReplKey->subkeys_offset;
		memcpy(pdata, psubkeys, subkeyslength);
	}
	if(pattrs && 0 < attrslength){
		pdata						= reinterpret_cast<unsigned char*>(pComReplKey) + pComReplKey->attrs_offset;
		memcpy(pdata, pattrs, attrslength);
	}

	if(!SetSendData(pComAll, K2hdkcCommand::MakeChmpxHash((reinterpret_cast<unsigned char*>(pComReplKey) + pComReplKey->key_offset), keylength))){
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

bool K2hdkcComReplKey::CommandSend(k2h_hash_t hash, k2h_hash_t subhash, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, const unsigned char* psubkeys, size_t subkeyslength, const unsigned char* pattrs, size_t attrslength, const struct timespec ts, dkcres_type_t* prescode)
{
	if(!pChmObj){
		ERR_DKCPRN("Chmpx object is NULL, so could not send command.");
		return false;
	}

	if(!CommandSend(hash, subhash, pkey, keylength, pval, vallength, psubkeys, subkeyslength, pattrs, attrslength, ts)){
		ERR_DKCPRN("Failed to send command.");
		return false;
	}
	return GetResponseData(prescode);
}

bool K2hdkcComReplKey::GetResponseData(dkcres_type_t* prescode) const
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
void K2hdkcComReplKey::RawDumpComAll(const PDKCCOM_ALL pComAll) const
{
	assert(DKC_COM_REPL_KEY == pComAll->com_head.comtype);

	if(DKC_NORESTYPE == pComAll->com_head.restype){
		PDKCCOM_REPL_KEY	pCom = CVT_DKCCOM_REPL_KEY(pComAll);

		RAW_DKCPRN("  DKCCOM_REPL_KEY = {");
		RAW_DKCPRN("    hash            = 0x%016" PRIx64 ,		pCom->hash);
		RAW_DKCPRN("    subhash         = 0x%016" PRIx64 ,		pCom->subhash);
		RAW_DKCPRN("    key_offset      = (%016" PRIx64 ") %s",	pCom->key_offset, bin_to_string(GET_BIN_DKC_COM_ALL(pComAll, pCom->key_offset, pCom->key_length), pCom->key_length).c_str());
		RAW_DKCPRN("    key_length      = %zu",					pCom->key_length);
		RAW_DKCPRN("    val_offset      = (%016" PRIx64 ") %s",	pCom->val_offset, bin_to_string(GET_BIN_DKC_COM_ALL(pComAll, pCom->val_offset, pCom->val_length), pCom->val_length).c_str());
		RAW_DKCPRN("    val_length      = %zu",					pCom->val_length);
		RAW_DKCPRN("    subkeys_offset  = (%016" PRIx64 ") %s",	pCom->subkeys_offset, bin_to_string(GET_BIN_DKC_COM_ALL(pComAll, pCom->subkeys_offset, pCom->subkeys_length), pCom->subkeys_length).c_str());
		RAW_DKCPRN("    subkeys_length  = %zu",					pCom->subkeys_length);
		RAW_DKCPRN("    attrs_offset    = (%016" PRIx64 ") %s",	pCom->attrs_offset, bin_to_string(GET_BIN_DKC_COM_ALL(pComAll, pCom->attrs_offset, pCom->attrs_length), pCom->attrs_length).c_str());
		RAW_DKCPRN("    attrs_length    = %zu",					pCom->attrs_length);
		RAW_DKCPRN("    ts              = %s (%s)",				STR_DKC_TIMESPEC(&(pCom->ts)).c_str(), STR_DKC_TIMESPEC_NS(&(pCom->ts)).c_str());
		RAW_DKCPRN("  }");

	}else{
		//PDKCRES_REPL_KEY	pCom = CVT_DKCRES_REPL_KEY(pComAll);
		RAW_DKCPRN("  DKCRES_REPL_KEY = {");
			// nothing to print member
		RAW_DKCPRN("  }");
	}
}

/*
 * VIM modelines
 *
 * vim:set ts=4 fenc=utf-8:
 */
