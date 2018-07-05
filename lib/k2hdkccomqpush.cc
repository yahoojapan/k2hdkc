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

#include "k2hdkccomqpush.h"
#include "k2hdkccomset.h"
#include "k2hdkccomaddsubkeys.h"
#include "k2hdkccomutil.h"
#include "k2hdkcutil.h"
#include "k2hdkcdbg.h"

using namespace	std;

//---------------------------------------------------------
// Constructor/Destructor
//---------------------------------------------------------
K2hdkcComQPush::K2hdkcComQPush(K2HShm* pk2hash, ChmCntrl* pchmcntrl, uint64_t comnum, bool without_self, bool is_routing_on_server, bool is_wait_on_server) : K2hdkcCommand(pk2hash, pchmcntrl, comnum, without_self, is_routing_on_server, is_wait_on_server, DKC_COM_QPUSH)
{
	strClassName = "K2hdkcComQPush";
}

K2hdkcComQPush::~K2hdkcComQPush(void)
{
}

//---------------------------------------------------------
// Methods
//---------------------------------------------------------
PDKCCOM_ALL K2hdkcComQPush::MakeResponseOnlyHeadData(dkcres_type_t subcode, dkcres_type_t errcode)
{
	PDKCCOM_ALL	pComErr;
	if(NULL == (pComErr = reinterpret_cast<PDKCCOM_ALL>(malloc(sizeof(DKCRES_QPUSH))))){
		ERR_DKCPRN("Could not allocate memory.");
		return NULL;
	}
	PDKCRES_QPUSH	pComRes		= CVT_DKCRES_QPUSH(pComErr);
	pComRes->head.comtype		= DKC_COM_QPUSH;
	pComRes->head.restype		= COMPOSE_DKC_RES(errcode, subcode);
	pComRes->head.comnumber		= GetComNumber();
	pComRes->head.dispcomnumber	= GetDispComNumber();
	pComRes->head.length		= sizeof(DKCRES_QPUSH);

	return pComErr;
}

bool K2hdkcComQPush::SetSucceedResponseData(void)
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

bool K2hdkcComQPush::CommandProcessing(void)
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
		const PDKCCOM_QPUSH		pCom	= CVT_DKCCOM_QPUSH(pRcvComAll);
		const unsigned char*	pPrefix	= GET_BIN_DKC_COM_ALL(pRcvComAll, pCom->prefix_offset, pCom->prefix_length);
		const unsigned char*	pKey	= GET_BIN_DKC_COM_ALL(pRcvComAll, pCom->key_offset, pCom->key_length);		// allow empty value
		const unsigned char*	pVal	= GET_BIN_DKC_COM_ALL(pRcvComAll, pCom->val_offset, pCom->val_length);
		const unsigned char*	pAttrs	= GET_BIN_DKC_COM_ALL(pRcvComAll, pCom->attrs_offset, pCom->attrs_length);	// allow empty value
		char*					pEncPass= NULL;
		if(0 < pCom->encpass_length){
			pEncPass					= reinterpret_cast<char*>(k2hbindup(GET_BIN_DKC_COM_ALL(pRcvComAll, pCom->encpass_offset, pCom->encpass_length), pCom->encpass_length));
		}

		if(!pVal){
			ERR_DKCPRN("Could not get safe val(%zd offset, %zu byte) in DKCCOM_QPUSH(%p).", pCom->val_offset, pCom->val_length, pRcvComAll);
			SetErrorResponseData(DKC_RES_SUBCODE_INVAL);
			DKC_FREE(pEncPass);
			return false;
		}
		bool	is_keyqueue = (NULL != pKey);

		// do command
		//
		// [NOTICE] This command is very procedure
		//

		// [0] get marker key name & check marker exists
		size_t			marker_length	= 0;
		unsigned char*	pMarker			= K2HLowOpsQueue::GetMarkerName(pPrefix, pCom->prefix_length, marker_length);
		unsigned char*	pmkval			= NULL;
		bool			is_marker_exist	= false;
		if(!pMarker){
			ERR_DKCPRN("Failed to get marker key name for prefix(%s) in DKCCOM_QPUSH(%p)", bin_to_string(pPrefix, pCom->prefix_length).c_str(), pRcvComAll);
			SetErrorResponseData(DKC_RES_SUBCODE_GETMARKER);
			DKC_FREE(pEncPass);
			return false;
		}
		if(-1 != pK2hObj->Get(pMarker, marker_length, &pmkval, false) && pmkval){
			// Marker exists
			DKC_FREE(pmkval);
			is_marker_exist = true;
		}

		// [1] get uniq queue key name for new queue key
		K2HLowOpsQueue*	pQueue			= pK2hObj->GetLowOpsQueueObj(pCom->fifo, pPrefix, pCom->prefix_length);
		if(!pQueue){
			ERR_DKCPRN("Failed to get queue prefix(%s) in DKCCOM_QPUSH(%p)", bin_to_string(pPrefix, pCom->prefix_length).c_str(), pRcvComAll);
			SetErrorResponseData(DKC_RES_SUBCODE_GETQUQUE);
			DKC_FREE(pEncPass);
			DKC_FREE(pMarker);
			return false;
		}
		size_t			qkey_length		= 0;
		unsigned char*	pQKey			= pQueue->GetUniqKey(qkey_length);
		if(!pQKey){
			ERR_DKCPRN("Failed to make new queue key name for queue prefix(%s) in DKCCOM_QPUSH(%p)", bin_to_string(pPrefix, pCom->prefix_length).c_str(), pRcvComAll);
			SetErrorResponseData(DKC_RES_SUBCODE_GETNEWQKEY);
			DKC_FREE(pEncPass);
			DKC_FREE(pMarker);
			DKC_DELETE(pQueue);
			return false;
		}

		// [2] KEYQUEUE - make new queue key and value
		if(is_keyqueue){
			K2hdkcComSet*	pComSetObj	= GetCommonK2hdkcComSet(pK2hObj, pChmObj, SendMsgid, GetDispComNumber(), false, true, true);		// [NOTICE] send all and wait result
			dkcres_type_t	rescode		= DKC_INITRESTYPE;
			if(!pComSetObj->CommandSend(pKey, pCom->key_length, pVal, pCom->val_length, false, pEncPass, (pCom->enable_expire ? &(pCom->expire) : NULL), &rescode)){	// [NOTE] keep subkeys list
				ERR_DKCPRN("Failed to set key(%s) and val(%s) in DKCCOM_QPUSH(%p).", bin_to_string(pKey, pCom->key_length).c_str(), bin_to_string(pVal, pCom->val_length).c_str(), pRcvComAll);
				SetErrorResponseData(DKC_RES_SUBCODE_SETVAL);
				DKC_FREE(pEncPass);
				DKC_FREE(pMarker);
				DKC_FREE(pQKey);
				DKC_DELETE(pQueue);
				DKC_DELETE(pComSetObj);
				return false;
			}
			DKC_DELETE(pComSetObj);

			if(IS_DKC_RES_NOTSUCCESS(rescode)){
				ERR_DKCPRN("Failed to set key(%s) and val(%s) in DKCCOM_QPUSH(%p) by subcude(%s).", bin_to_string(pKey, pCom->key_length).c_str(), bin_to_string(pVal, pCom->val_length).c_str(), pRcvComAll, STR_DKCRES_SUBCODE_TYPE(rescode));
				SetErrorResponseData(DKC_RES_SUBCODE_SETVAL);
				DKC_FREE(pEncPass);
				DKC_FREE(pMarker);
				DKC_FREE(pQKey);
				DKC_DELETE(pQueue);
				return false;
			}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
				MSG_DKCPRN("Get result subcode(%s) for setting key(%s) and val(%s) in DKCCOM_QPUSH(%p).", STR_DKCRES_SUBCODE_TYPE(rescode), bin_to_string(pKey, pCom->key_length).c_str(), bin_to_string(pVal, pCom->val_length).c_str(), pRcvComAll);
			}
		}

		// [3] make new queue key without subkey
		{
			K2hdkcComSet*			pComSetObj	= GetCommonK2hdkcComSet(pK2hObj, pChmObj, SendMsgid, GetDispComNumber(), false, true, true);		// [NOTICE] send all and wait result
			dkcres_type_t			rescode		= DKC_INITRESTYPE;
			const unsigned char*	pTmpVal		= is_keyqueue ? pKey : pVal;
			size_t					tmpvallen	= is_keyqueue ? pCom->key_length : pCom->val_length;
			if(!pComSetObj->CommandSend(pQKey, qkey_length, pTmpVal, tmpvallen, (is_keyqueue ? DKC_QUEUE_TYPE_KEYQUEUE : DKC_QUEUE_TYPE_QUEUE), pAttrs, pCom->attrs_length, pEncPass, (pCom->enable_expire ? &(pCom->expire) : NULL), &rescode)){
				ERR_DKCPRN("Failed to set new queue key(%s) and val(%s) in DKCCOM_QPUSH(%p).", bin_to_string(pQKey, qkey_length).c_str(), bin_to_string(pTmpVal, tmpvallen).c_str(), pRcvComAll);
				SetErrorResponseData(DKC_RES_SUBCODE_SETVAL);
				DKC_FREE(pEncPass);
				DKC_FREE(pMarker);
				DKC_FREE(pQKey);
				DKC_DELETE(pQueue);
				DKC_DELETE(pComSetObj);
				return false;
			}
			DKC_DELETE(pComSetObj);

			if(IS_DKC_RES_NOTSUCCESS(rescode)){
				ERR_DKCPRN("Failed to set new queue key(%s) and val(%s) in DKCCOM_QPUSH(%p) by subcude(%s).", bin_to_string(pQKey, qkey_length).c_str(), bin_to_string(pTmpVal, tmpvallen).c_str(), pRcvComAll, STR_DKCRES_SUBCODE_TYPE(rescode));
				SetErrorResponseData(DKC_RES_SUBCODE_SETVAL);
				DKC_FREE(pEncPass);
				DKC_FREE(pMarker);
				DKC_FREE(pQKey);
				DKC_DELETE(pQueue);
				return false;
			}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
				MSG_DKCPRN("Get result subcode(%s) for setting new queue key(%s) and val(%s) in DKCCOM_QPUSH(%p).", STR_DKCRES_SUBCODE_TYPE(rescode), bin_to_string(pQKey, qkey_length).c_str(), bin_to_string(pTmpVal, tmpvallen).c_str(), pRcvComAll);
			}
		}
		DKC_FREE(pEncPass);

		// [4] get top/bottom queue key name from marker
		size_t			exkey_length	= 0;
		unsigned char*	pExKey			= NULL;
		if(is_marker_exist){
			if(pCom->fifo){
				// [5B-1A] FIFO - get bottom(deepest) queue key
				pExKey = pQueue->GetBottomQueueKey(exkey_length);
			}else{
				// [5B-1B] LIFO - get top queue key
				pExKey = pQueue->GetTopQueueKey(exkey_length);
			}
		}

		// [5] update marker key
		if(!pQueue->Push(pQKey, qkey_length)){
			ERR_DKCPRN("Failed to initiate marker for queue prefix(%s) in DKCCOM_QPUSH(%p)", bin_to_string(pPrefix, pCom->prefix_length).c_str(), pRcvComAll);
			SetErrorResponseData(DKC_RES_SUBCODE_SETMARKER);
			DKC_FREE(pMarker);
			DKC_FREE(pQKey);
			DKC_FREE(pExKey);
			DKC_DELETE(pQueue);
			return false;
		}

		// [NOTE]
		// This loop is dummy for post processing as replicating marker
		//
		bool	result = false;
		do{
			// [6] rechain queue key and subkeys
			if(is_marker_exist && pExKey && pCom->fifo){
				// [6-1] FIFO - insert existing queue key into new queue key's subkey
				K2hdkcComAddSubkeys*	pComAddSubkeys	= GetCommonK2hdkcComAddSubkeys(pK2hObj, pChmObj, SendMsgid, GetDispComNumber(), false, true, true);		// [NOTICE] send all and wait result
				dkcres_type_t			rescode			= DKC_INITRESTYPE;
				if(!pComAddSubkeys->CommandSend(pExKey, exkey_length, pQKey, qkey_length, pCom->check_attr, &rescode)){
					ERR_DKCPRN("Failed to add new queue key(%s) into subkey of key(%s) in DKCCOM_QPUSH(%p).", bin_to_string(pQKey, qkey_length).c_str(), bin_to_string(pExKey, exkey_length).c_str(), pRcvComAll);
					SetErrorResponseData(DKC_RES_SUBCODE_ADDSKEYLIST);
					DKC_FREE(pMarker);
					DKC_FREE(pQKey);
					DKC_FREE(pExKey);
					DKC_DELETE(pQueue);
					DKC_DELETE(pComAddSubkeys);
					break;
				}
				DKC_DELETE(pComAddSubkeys);

				if(IS_DKC_RES_NOTSUCCESS(rescode)){
					ERR_DKCPRN("Failed to add new queue key(%s) into subkey of key(%s) in DKCCOM_QPUSH(%p)  by subcude(%s).", bin_to_string(pQKey, qkey_length).c_str(), bin_to_string(pExKey, exkey_length).c_str(), pRcvComAll, STR_DKCRES_SUBCODE_TYPE(rescode));
					SetErrorResponseData(DKC_RES_SUBCODE_ADDSKEYLIST);
					DKC_FREE(pMarker);
					DKC_FREE(pQKey);
					DKC_FREE(pExKey);
					DKC_DELETE(pQueue);
					break;
				}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
					ERR_DKCPRN("Get result subcode(%s) for adding new queue key(%s) into subkey of key(%s) in DKCCOM_QPUSH(%p).", STR_DKCRES_SUBCODE_TYPE(rescode), bin_to_string(pQKey, qkey_length).c_str(), bin_to_string(pExKey, exkey_length).c_str(), pRcvComAll);
				}

			}else if(is_marker_exist && pExKey && !pCom->fifo){
				// [6-2] LIFO - insert new queue key into existing queue key's subkey
				K2hdkcComAddSubkeys*	pComAddSubkeys	= GetCommonK2hdkcComAddSubkeys(pK2hObj, pChmObj, SendMsgid, GetDispComNumber(), false, true, true);		// [NOTICE] send all and wait result
				dkcres_type_t			rescode			= DKC_INITRESTYPE;
				if(!pComAddSubkeys->CommandSend(pQKey, qkey_length, pExKey, exkey_length, pCom->check_attr, &rescode)){
					ERR_DKCPRN("Failed to add existed key(%s) into new queue key(%s) in DKCCOM_QPUSH(%p).", bin_to_string(pExKey, exkey_length).c_str(), bin_to_string(pQKey, qkey_length).c_str(), pRcvComAll);
					SetErrorResponseData(DKC_RES_SUBCODE_ADDSKEYLIST);
					DKC_FREE(pMarker);
					DKC_FREE(pQKey);
					DKC_FREE(pExKey);
					DKC_DELETE(pQueue);
					break;
				}
				DKC_DELETE(pComAddSubkeys);

				if(IS_DKC_RES_NOTSUCCESS(rescode)){
					ERR_DKCPRN("Failed to add existed key(%s) into new queue key(%s) in DKCCOM_QPUSH(%p) by subcude(%s).", bin_to_string(pExKey, exkey_length).c_str(), bin_to_string(pQKey, qkey_length).c_str(), pRcvComAll, STR_DKCRES_SUBCODE_TYPE(rescode));
					SetErrorResponseData(DKC_RES_SUBCODE_ADDSKEYLIST);
					DKC_FREE(pMarker);
					DKC_FREE(pQKey);
					DKC_FREE(pExKey);
					DKC_DELETE(pQueue);
					break;
				}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
					ERR_DKCPRN("Get result subcode(%s) for adding existed key(%s) into new queue key(%s) in DKCCOM_QPUSH(%p).", STR_DKCRES_SUBCODE_TYPE(rescode), bin_to_string(pExKey, exkey_length).c_str(), bin_to_string(pQKey, qkey_length).c_str(), pRcvComAll);
				}
			}
			result = true;

		}while(!result);

		// get time at setting.
		struct timespec	ts;
		INIT_DKC_TIMESPEC(ts);

		// [7] replicate key and datas to other servers assap
		//
		// [NOTE] reason of sending other server after replying result.
		// because the result of sending replication data to other server on server node can not be caught
		// this event loop, it is caught another session after this event handling.
		// (this result is caught in event loop on server node)
		//
		if(!CommandReplicate(pMarker, marker_length, ts)){
			ERR_DKCPRN("Failed to replicate queue marker prefix key(%s) to other servers, but continue...", bin_to_string(pPrefix, pCom->prefix_length).c_str());
		}

		// set response data
		if(result){
			SetSucceedResponseData();
		}
		DKC_FREE(pMarker);
		DKC_FREE(pQKey);
		DKC_FREE(pExKey);
		DKC_DELETE(pQueue);

		// do response(reply assap)
		if(!ReplyResponse()){
			ERR_DKCPRN("Failed sending response for queue prefix(%s)", bin_to_string(pPrefix, pCom->prefix_length).c_str());
			return false;
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

bool K2hdkcComQPush::CommandSend(const unsigned char* pprefix, size_t prefixlength, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, bool is_fifo, const unsigned char* pattrs, size_t attrslength, bool checkattr, const char* encpass, const time_t* expire)
{
	if(!pval || 0 == vallength){
		ERR_DKCPRN("Parameter are wrong.");
		return false;
	}

	// make send data
	PDKCCOM_ALL	pComAll;
	if(NULL == (pComAll = reinterpret_cast<PDKCCOM_ALL>(malloc(sizeof(DKCCOM_QPUSH) + prefixlength + keylength + vallength + attrslength + (encpass ? (strlen(encpass) + 1) : 0))))){
		ERR_DKCPRN("Could not allocate memory.");
		return false;
	}
	PDKCCOM_QPUSH	pComQPush		= CVT_DKCCOM_QPUSH(pComAll);
	pComQPush->head.comtype			= DKC_COM_QPUSH;
	pComQPush->head.restype			= DKC_NORESTYPE;
	pComQPush->head.comnumber		= GetComNumber();
	pComQPush->head.dispcomnumber	= GetDispComNumber();
	pComQPush->head.length			= sizeof(DKCCOM_QPUSH) + prefixlength + keylength + vallength + attrslength + (encpass ? (strlen(encpass) + 1) : 0);
	pComQPush->prefix_offset		= sizeof(DKCCOM_QPUSH);
	pComQPush->prefix_length		= prefixlength;
	pComQPush->key_offset			= sizeof(DKCCOM_QPUSH) + prefixlength;
	pComQPush->key_length			= keylength;
	pComQPush->val_offset			= sizeof(DKCCOM_QPUSH) + prefixlength + keylength;
	pComQPush->val_length			= vallength;
	pComQPush->attrs_offset			= sizeof(DKCCOM_QPUSH) + prefixlength + keylength + vallength;
	pComQPush->attrs_length			= attrslength;
	pComQPush->encpass_offset		= sizeof(DKCCOM_QPUSH) + prefixlength + keylength + vallength + attrslength;
	pComQPush->encpass_length		= encpass ? (strlen(encpass) + 1) : 0;
	pComQPush->expire				= expire ? (*expire) : 0L;
	pComQPush->enable_expire		= expire ? true : false;
	pComQPush->check_attr			= checkattr ? true : false;
	pComQPush->fifo					= is_fifo;

	unsigned char*	pdata			= reinterpret_cast<unsigned char*>(pComQPush) + pComQPush->val_offset;
	memcpy(pdata, pval, vallength);
	if(pprefix && 0 < prefixlength){
		pdata						= reinterpret_cast<unsigned char*>(pComQPush) + pComQPush->prefix_offset;
		memcpy(pdata, pprefix, prefixlength);
	}
	if(pkey && 0 < keylength){
		pdata						= reinterpret_cast<unsigned char*>(pComQPush) + pComQPush->key_offset;
		memcpy(pdata, pkey, keylength);
	}
	if(pattrs && 0 < attrslength){
		pdata						= reinterpret_cast<unsigned char*>(pComQPush) + pComQPush->attrs_offset;
		memcpy(pdata, pattrs, attrslength);
	}
	if(encpass){
		pdata						= reinterpret_cast<unsigned char*>(pComQPush) + pComQPush->encpass_offset;
		memcpy(pdata, encpass, strlen(encpass) + 1);
	}

	if(!SetSendData(pComAll, K2hdkcCommand::MakeChmpxHash((reinterpret_cast<unsigned char*>(pComQPush) + pComQPush->prefix_offset), prefixlength))){
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

bool K2hdkcComQPush::CommandSend(const unsigned char* pprefix, size_t prefixlength, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, bool is_fifo, const unsigned char* pattrs, size_t attrslength, bool checkattr, const char* encpass, const time_t* expire, dkcres_type_t* prescode)
{
	if(!pChmObj){
		ERR_DKCPRN("Chmpx object is NULL, so could not send command.");
		return false;
	}

	if(!CommandSend(pprefix, prefixlength, pkey, keylength, pval, vallength, is_fifo, pattrs, attrslength, checkattr, encpass, expire)){
		ERR_DKCPRN("Failed to send command.");
		return false;
	}
	return GetResponseData(prescode);
}

bool K2hdkcComQPush::GetResponseData(dkcres_type_t* prescode) const
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
void K2hdkcComQPush::RawDumpComAll(const PDKCCOM_ALL pComAll) const
{
	assert(DKC_COM_QPUSH == pComAll->com_head.comtype);

	if(DKC_NORESTYPE == pComAll->com_head.restype){
		PDKCCOM_QPUSH	pCom = CVT_DKCCOM_QPUSH(pComAll);

		RAW_DKCPRN("  DKCCOM_QPUSH = {");
		RAW_DKCPRN("    prefix_offset   = (%016" PRIx64 ") %s",	pCom->prefix_offset, bin_to_string(GET_BIN_DKC_COM_ALL(pComAll, pCom->prefix_offset, pCom->prefix_length), pCom->prefix_length).c_str());
		RAW_DKCPRN("    prefix_length   = %zu",					pCom->prefix_length);
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
		RAW_DKCPRN("    check_attr      = %s",					pCom->check_attr ? "yes" : "no");
		RAW_DKCPRN("    fifo            = %s",					pCom->is_fifo ? "yes" : "no");
		RAW_DKCPRN("  }");
	}else{
		//PDKCRES_QPUSH	pCom = CVT_DKCRES_QPUSH(pComAll);
		RAW_DKCPRN("  DKCRES_QPUSH = {");
			// nothing to print member
		RAW_DKCPRN("  }");
	}
}

/*
 * VIM modelines
 *
 * vim:set ts=4 fenc=utf-8:
 */
