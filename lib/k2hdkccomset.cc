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

#include "k2hdkccomset.h"
#include "k2hdkcutil.h"
#include "k2hdkcdbg.h"

using namespace	std;

//---------------------------------------------------------
// Constructor/Destructor
//---------------------------------------------------------
K2hdkcComSet::K2hdkcComSet(K2HShm* pk2hash, ChmCntrl* pchmcntrl, uint64_t comnum, bool without_self, bool is_routing_on_server, bool is_wait_on_server) : K2hdkcCommand(pk2hash, pchmcntrl, comnum, without_self, is_routing_on_server, is_wait_on_server, DKC_COM_SET)
{
	strClassName = "K2hdkcComSet";
}

K2hdkcComSet::~K2hdkcComSet(void)
{
}

//---------------------------------------------------------
// Methods
//---------------------------------------------------------
PDKCCOM_ALL K2hdkcComSet::MakeResponseOnlyHeadData(dkcres_type_t subcode, dkcres_type_t errcode)
{
	PDKCCOM_ALL	pComErr;
	if(NULL == (pComErr = reinterpret_cast<PDKCCOM_ALL>(malloc(sizeof(DKCRES_SET))))){
		ERR_DKCPRN("Could not allocate memory.");
		return NULL;
	}
	PDKCRES_SET	pComRes			= CVT_DKCRES_SET(pComErr);
	pComRes->head.comtype		= DKC_COM_SET;
	pComRes->head.restype		= COMPOSE_DKC_RES(errcode, subcode);
	pComRes->head.comnumber		= GetComNumber();
	pComRes->head.dispcomnumber	= GetDispComNumber();
	pComRes->head.length		= sizeof(DKCRES_SET);

	return pComErr;
}

bool K2hdkcComSet::SetSucceedResponseData(void)
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

bool K2hdkcComSet::CommandProcessing(void)
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
		const PDKCCOM_SET		pCom	= CVT_DKCCOM_SET(pRcvComAll);
		const unsigned char*	pKey	= GET_BIN_DKC_COM_ALL(pRcvComAll, pCom->key_offset, pCom->key_length);
		const unsigned char*	pVal	= GET_BIN_DKC_COM_ALL(pRcvComAll, pCom->val_offset, pCom->val_length);		// allow empty value
		const unsigned char*	pAttrs	= GET_BIN_DKC_COM_ALL(pRcvComAll, pCom->attrs_offset, pCom->attrs_length);
		char*					pEncPass= NULL;
		if(0 < pCom->encpass_length){
			pEncPass					= reinterpret_cast<char*>(k2hbindup(GET_BIN_DKC_COM_ALL(pRcvComAll, pCom->encpass_offset, pCom->encpass_length), pCom->encpass_length));
		}
		if(!pKey){
			ERR_DKCPRN("Could not get safe key(%zd offset, %zu byte) and val(%zd offset, %zu byte) in DKCCOM_SET(%p).", pCom->key_offset, pCom->key_length, pCom->val_offset, pCom->val_length, pRcvComAll);
			SetErrorResponseData(DKC_RES_SUBCODE_INVAL);
			DKC_FREE(pEncPass);
			return false;
		}

		// subkeys, attributes, and attributes mask
		K2HSubKeys*					pSKeysObj= NULL;
		K2HAttrs*					pAttrObj = NULL;
		K2hAttrOpsMan::ATTRINITTYPE	attrtype =	(	DKC_QUEUE_TYPE_QUEUE	== pCom->queue_type ?	K2hAttrOpsMan::OPSMAN_MASK_QUEUEKEY		:
													DKC_QUEUE_TYPE_KEYQUEUE	== pCom->queue_type ?	K2hAttrOpsMan::OPSMAN_MASK_KEYQUEUEKEY	:
																									K2hAttrOpsMan::OPSMAN_MASK_NORMAL		);
		if(K2hAttrOpsMan::OPSMAN_MASK_NORMAL != attrtype && pAttrs && 0 < pCom->attrs_length){
			pAttrObj = new K2HAttrs();
			if(!pAttrObj->Serialize(pAttrs, pCom->attrs_length)){
				ERR_DKCPRN("Failed to convert binary data to attribute object for key(%s).", bin_to_string(pKey, pCom->key_length).c_str());
				SetErrorResponseData(DKC_RES_SUBCODE_SETVAL);
				DKC_DELETE(pAttrObj);
				DKC_FREE(pEncPass);
				return false;
			}
		}
		if(!pCom->rm_subkeylist){
			pSKeysObj = pK2hObj->GetSubKeys(pKey, pCom->key_length, false);											// not check attributes and no encript pass
		}

		// do command
		if(!pK2hObj->Set(pKey, pCom->key_length, pVal, pCom->val_length, pSKeysObj, false, pAttrObj, pEncPass, (pCom->enable_expire ? &(pCom->expire) : NULL), attrtype)){
			ERR_DKCPRN("Failed to set val(%s) to key(%s)", bin_to_string(pVal, pCom->val_length).c_str(), bin_to_string(pKey, pCom->key_length).c_str());
			SetErrorResponseData(DKC_RES_SUBCODE_SETVAL);
			DKC_DELETE(pAttrObj);
			DKC_DELETE(pSKeysObj);
			DKC_FREE(pEncPass);
			return false;
		}
		DKC_DELETE(pAttrObj);
		DKC_DELETE(pSKeysObj);
		DKC_FREE(pEncPass);

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

bool K2hdkcComSet::CommandSendEx(const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, bool rm_subkeylist, dkc_qtype_t queue_type, const unsigned char* pattrs, size_t attrslength, const char* encpass, const time_t* expire)
{
	if(!pkey || 0 == keylength || !IS_SAFE_DKC_QUEUE_TYPE(queue_type)){
		ERR_DKCPRN("Parameter are wrong.");
		return false;
	}
	if(DKC_QUEUE_TYPE_NOTQUEUE == queue_type && (pattrs || 0 < attrslength)){
		// if attibutes mask type is normal, pattrs must be empty.
		ERR_DKCPRN("Parameter are wrong.");
		return false;
	}

	// make send data
	PDKCCOM_ALL	pComAll;
	if(NULL == (pComAll = reinterpret_cast<PDKCCOM_ALL>(malloc(sizeof(DKCCOM_SET) + keylength + vallength + attrslength + (encpass ? (strlen(encpass) + 1) : 0))))){
		ERR_DKCPRN("Could not allocate memory.");
		return false;
	}
	PDKCCOM_SET	pComSet			= CVT_DKCCOM_SET(pComAll);
	pComSet->head.comtype		= DKC_COM_SET;
	pComSet->head.restype		= DKC_NORESTYPE;
	pComSet->head.comnumber		= GetComNumber();
	pComSet->head.dispcomnumber	= GetDispComNumber();
	pComSet->head.length		= sizeof(DKCCOM_SET) + keylength + vallength + attrslength + (encpass ? (strlen(encpass) + 1) : 0);
	pComSet->key_offset			= sizeof(DKCCOM_SET);
	pComSet->key_length			= keylength;
	pComSet->val_offset			= sizeof(DKCCOM_SET) + keylength;
	pComSet->val_length			= vallength;
	pComSet->attrs_offset		= sizeof(DKCCOM_SET) + keylength + vallength;
	pComSet->attrs_length		= attrslength;
	pComSet->encpass_offset		= sizeof(DKCCOM_SET) + keylength + vallength + attrslength;
	pComSet->encpass_length		= encpass ? (strlen(encpass) + 1) : 0;
	pComSet->expire				= expire ? (*expire) : 0L;
	pComSet->enable_expire		= expire ? true : false;
	pComSet->rm_subkeylist		= rm_subkeylist;
	pComSet->queue_type			= queue_type;

	unsigned char*	pdata		= reinterpret_cast<unsigned char*>(pComSet) + pComSet->key_offset;
	memcpy(pdata, pkey, keylength);
	if(pval && 0 < vallength){
		pdata					= reinterpret_cast<unsigned char*>(pComSet) + pComSet->val_offset;
		memcpy(pdata, pval, vallength);
	}
	if(pattrs && 0 < attrslength){
		pdata					= reinterpret_cast<unsigned char*>(pComSet) + pComSet->attrs_offset;
		memcpy(pdata, pattrs, attrslength);
	}
	if(encpass){
		pdata					= reinterpret_cast<unsigned char*>(pComSet) + pComSet->encpass_offset;
		memcpy(pdata, encpass, strlen(encpass) + 1);
	}

	if(!SetSendData(pComAll, K2hdkcCommand::MakeChmpxHash((reinterpret_cast<unsigned char*>(pComSet) + pComSet->key_offset), keylength))){
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

bool K2hdkcComSet::CommandSendEx(const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, bool rm_subkeylist, dkc_qtype_t queue_type, const unsigned char* pattrs, size_t attrslength, const char* encpass, const time_t* expire, dkcres_type_t* prescode)
{
	if(!pChmObj){
		ERR_DKCPRN("Chmpx object is NULL, so could not send command.");
		return false;
	}

	if(!CommandSendEx(pkey, keylength, pval, vallength, rm_subkeylist, queue_type, pattrs, attrslength, encpass, expire)){
		ERR_DKCPRN("Failed to send command.");
		return false;
	}
	return GetResponseData(prescode);
}

bool K2hdkcComSet::GetResponseData(dkcres_type_t* prescode) const
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
void K2hdkcComSet::RawDumpComAll(const PDKCCOM_ALL pComAll) const
{
	assert(DKC_COM_SET == pComAll->com_head.comtype);

	if(DKC_NORESTYPE == pComAll->com_head.restype){
		PDKCCOM_SET	pCom = CVT_DKCCOM_SET(pComAll);

		RAW_DKCPRN("  DKCCOM_SET = {");
		RAW_DKCPRN("    key_offset      = (%016" PRIx64 ") %s",	pCom->key_offset, bin_to_string(GET_BIN_DKC_COM_ALL(pComAll, pCom->key_offset, pCom->key_length), pCom->key_length).c_str());
		RAW_DKCPRN("    key_length      = %zu",					pCom->key_length);
		RAW_DKCPRN("    val_offset      = (%016" PRIx64 ") %s",	pCom->val_offset, bin_to_string(GET_BIN_DKC_COM_ALL(pComAll, pCom->val_offset, pCom->val_length), pCom->val_length).c_str());
		RAW_DKCPRN("    val_length      = %zu",					pCom->val_length);
		RAW_DKCPRN("    attrs_offset    = (%016" PRIx64 ") %s",	pCom->attrs_offset, bin_to_string(GET_BIN_DKC_COM_ALL(pComAll, pCom->attrs_offset, pCom->attrs_length), pCom->attrs_length).c_str());
		RAW_DKCPRN("    attrs_length    = %zu",					pCom->attrs_length);
		RAW_DKCPRN("    encpass_offset  = (%016" PRIx64 ") %s",	pCom->encpass_offset, bin_to_string(GET_BIN_DKC_COM_ALL(pComAll, pCom->encpass_offset, pCom->encpass_length), pCom->encpass_length).c_str());
		RAW_DKCPRN("    encpass_length  = %zu",					pCom->encpass_length);
		RAW_DKCPRN("    expire          = %zu",					pCom->expire);
		RAW_DKCPRN("    enable_expire   = %s",					pCom->enable_expire ? "yes" : "no");
		RAW_DKCPRN("    rm_subkeylist   = %s",					pCom->rm_subkeylist ? "yes" : "no");
		RAW_DKCPRN("    queue_type      = %s",					STR_DKC_QUEUE_TYPE(pCom->queue_type));
		RAW_DKCPRN("  }");

	}else{
		//PDKCRES_SET	pCom = CVT_DKCRES_SET(pComAll);
		RAW_DKCPRN("  DKCRES_SET = {");
			// nothing to print member
		RAW_DKCPRN("  }");
	}
}

/*
 * VIM modelines
 *
 * vim:set ts=4 fenc=utf-8:
 */
