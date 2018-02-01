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

#include "k2hdkccomqpop.h"
#include "k2hdkccomget.h"
#include "k2hdkccomdel.h"
#include "k2hdkccomgetsubkeys.h"
#include "k2hdkccomutil.h"
#include "k2hdkcutil.h"
#include "k2hdkcdbg.h"

using namespace	std;

//---------------------------------------------------------
// Constructor/Destructor
//---------------------------------------------------------
K2hdkcComQPop::K2hdkcComQPop(K2HShm* pk2hash, ChmCntrl* pchmcntrl, uint64_t comnum, bool without_self, bool is_routing_on_server, bool is_wait_on_server) : K2hdkcCommand(pk2hash, pchmcntrl, comnum, without_self, is_routing_on_server, is_wait_on_server, DKC_COM_QPOP)
{
	strClassName = "K2hdkcComQPop";
}

K2hdkcComQPop::~K2hdkcComQPop(void)
{
}

//---------------------------------------------------------
// Methods
//---------------------------------------------------------
PDKCCOM_ALL K2hdkcComQPop::MakeResponseOnlyHeadData(dkcres_type_t subcode, dkcres_type_t errcode)
{
	PDKCCOM_ALL	pComErr;
	if(NULL == (pComErr = reinterpret_cast<PDKCCOM_ALL>(malloc(sizeof(DKCRES_QPOP))))){
		ERR_DKCPRN("Could not allocate memory.");
		return NULL;
	}
	PDKCRES_QPOP	pComRes		= CVT_DKCRES_QPOP(pComErr);
	pComRes->head.comtype		= DKC_COM_QPOP;
	pComRes->head.restype		= COMPOSE_DKC_RES(errcode, subcode);
	pComRes->head.comnumber		= GetComNumber();
	pComRes->head.dispcomnumber	= GetDispComNumber();
	pComRes->head.length		= sizeof(DKCRES_QPOP);
	pComRes->key_offset			= sizeof(DKCRES_QPOP);
	pComRes->key_length			= 0;
	pComRes->val_offset			= sizeof(DKCRES_QPOP);
	pComRes->val_length			= 0;

	return pComErr;
}

bool K2hdkcComQPop::SetResponseData(const unsigned char* pKey, size_t KeyLen, const unsigned char* pValue, size_t ValLen)
{
	PDKCCOM_ALL	pComAll;
	if(NULL == (pComAll = reinterpret_cast<PDKCCOM_ALL>(malloc(sizeof(DKCRES_QPOP) + KeyLen + ValLen)))){
		ERR_DKCPRN("Could not allocate memory.");
		SetErrorResponseData(DKC_RES_SUBCODE_NOMEM);		// could this set?
		return false;
	}
	PDKCRES_QPOP	pComRes		= CVT_DKCRES_QPOP(pComAll);
	pComRes->head.comtype		= DKC_COM_QPOP;
	pComRes->head.restype		= COMPOSE_DKC_RES(DKC_RES_SUCCESS, DKC_RES_SUBCODE_NOTHING);
	pComRes->head.comnumber		= GetComNumber();
	pComRes->head.dispcomnumber	= GetDispComNumber();
	pComRes->head.length		= sizeof(DKCRES_QPOP) + KeyLen + ValLen;
	pComRes->key_offset			= sizeof(DKCRES_QPOP);
	pComRes->key_length			= KeyLen;
	pComRes->val_offset			= sizeof(DKCRES_QPOP) + KeyLen;
	pComRes->val_length			= ValLen;

	unsigned char*	pdata		= CVT_BIN_DKC_COM_ALL(pComAll, pComRes->val_offset, pComRes->val_length);
	if(!pdata){
		ERR_DKCPRN("Could not convert respond data area.");
		DKC_FREE(pComAll);
		SetErrorResponseData(DKC_RES_SUBCODE_INTERNAL);
		return false;
	}
	memcpy(pdata, pValue, ValLen);

	if(0 < KeyLen){
		pdata	= CVT_BIN_DKC_COM_ALL(pComAll, pComRes->key_offset, pComRes->key_length);
		if(!pdata){
			ERR_DKCPRN("Could not convert respond data area.");
			DKC_FREE(pComAll);
			SetErrorResponseData(DKC_RES_SUBCODE_INTERNAL);
			return false;
		}
		memcpy(pdata, pKey, KeyLen);
	}

	if(!SetResponseData(pComAll)){
		ERR_DKCPRN("Failed to set send data.");
		DKC_FREE(pComAll);
		SetErrorResponseData(DKC_RES_SUBCODE_INTERNAL);
		return false;
	}
	return true;
}

bool K2hdkcComQPop::CommandProcessing(void)
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
		const PDKCCOM_QPOP		pCom	= CVT_DKCCOM_QPOP(pRcvComAll);
		const unsigned char*	pPrefix	= GET_BIN_DKC_COM_ALL(pRcvComAll, pCom->prefix_offset, pCom->prefix_length);
		char*					pEncPass= NULL;
		if(0 < pCom->encpass_length){
			pEncPass					= reinterpret_cast<char*>(k2hbindup(GET_BIN_DKC_COM_ALL(pRcvComAll, pCom->encpass_offset, pCom->encpass_length), pCom->encpass_length));
		}

		// do command
		//
		// [NOTICE] This command is very procedure
		//
		// For that the queue marker key and the queue key are located on different machine,
		// we need to separate the processing of updating marker key and updating queue key.
		// But if those keys are located on same machine, we must not keep to lock marker
		// key during processing for deadlock.
		// Thus we lock marker key only during updating this key, and do not lock it during
		// other processing.
		// Then there is a possibility that this command processing is collision. So that
		// we check collision just before updating marker key, and we retry to process by
		// this loop if collision is occurred.
		// 

		// [0] get marker key name
		size_t			marker_length	= 0;
		unsigned char*	pMarker			= K2HLowOpsQueue::GetMarkerName(pPrefix, pCom->prefix_length, marker_length);
		if(!pMarker){
			ERR_DKCPRN("Failed to get marker key name for prefix(%s) in DKCCOM_QPOP(%p)", bin_to_string(pPrefix, pCom->prefix_length).c_str(), pRcvComAll);
			SetErrorResponseData(DKC_RES_SUBCODE_GETMARKER);
			DKC_FREE(pEncPass);
			return false;
		}

		// [1] get queue object
		K2HLowOpsQueue*	pQueue			= pK2hObj->GetLowOpsQueueObj(pCom->fifo, pPrefix, pCom->prefix_length);
		if(!pQueue){
			ERR_DKCPRN("Failed to get queue prefix(%s) in DKCCOM_QPOP(%p)", bin_to_string(pPrefix, pCom->prefix_length).c_str(), pRcvComAll);
			SetErrorResponseData(DKC_RES_SUBCODE_GETQUQUE);
			DKC_FREE(pEncPass);
			DKC_FREE(pMarker);
			return false;
		}

		// [2] get planned pop queue key name
		size_t			qkey_length		= 0;
		unsigned char*	pQKey			= pQueue->GetPlannedPopQueueKey(qkey_length);
		if(!pQKey){
			// there is no stacked queue data
			MSG_DKCPRN("Failed to get planned queue key name for queue prefix(%s) in DKCCOM_QPOP(%p)", bin_to_string(pPrefix, pCom->prefix_length).c_str(), pRcvComAll);
			SetErrorResponseData(DKC_RES_SUBCODE_GETPOPQUQUENAME);
			DKC_FREE(pEncPass);
			DKC_FREE(pMarker);
			DKC_DELETE(pQueue);
			return false;
		}

		// [3] get planned pop queue key's subkeys
		size_t			nextkey_length	= 0;
		unsigned char*	pNextKey		= NULL;
		{
			K2hdkcComGetSubkeys*	pComGetSubkeys	= GetCommonK2hdkcComGetSubkeys(pK2hObj, pChmObj, SendMsgid, GetDispComNumber(), false, true, true);	// [NOTICE] wait result
			K2HSubKeys*				pSubKeys		= NULL;
			dkcres_type_t			rescode			= DKC_INITRESTYPE;
			if(!pComGetSubkeys->CommandSend(pQKey, qkey_length, pCom->check_attr, &pSubKeys, &rescode) || !pSubKeys || 0 == pSubKeys->size()){			// [NOTE] probabry not check attr
				MSG_DKCPRN("Failed to get subkeys in planned queue key(%s) name for queue prefix(%s) in DKCCOM_QPOP(%p)", bin_to_string(pQKey, qkey_length).c_str(), bin_to_string(pPrefix, pCom->prefix_length).c_str(), pRcvComAll);
			}else{
				// get one subkey
				if(1 < pSubKeys->size()){
					WAN_DKCPRN("Too many subkeys in planned queue key(%s) name for queue prefix(%s) in DKCCOM_QPOP(%p)", bin_to_string(pQKey, qkey_length).c_str(), bin_to_string(pPrefix, pCom->prefix_length).c_str(), pRcvComAll);
				}
				K2HSubKeys::iterator	iter = pSubKeys->begin();
				if(0UL == iter->length){
					ERR_DKCPRN("wrong subkey length in planned queue key(%s) name for queue prefix(%s) in DKCCOM_QPOP(%p)", bin_to_string(pQKey, qkey_length).c_str(), bin_to_string(pPrefix, pCom->prefix_length).c_str(), pRcvComAll);
					SetErrorResponseData(DKC_RES_SUBCODE_GETSKEYLIST);
					DKC_FREE(pEncPass);
					DKC_FREE(pMarker);
					DKC_FREE(pQKey);
					DKC_DELETE(pSubKeys);
					DKC_DELETE(pQueue);
					return false;
				}
				// copy(bup)
				nextkey_length	= iter->length;
				pNextKey		= k2hbindup(iter->pSubKey, nextkey_length);
				DKC_DELETE(pSubKeys);
			}
			DKC_DELETE(pComGetSubkeys);
		}

		// [4] pop marker key
		if(!pQueue->Pop(pNextKey, nextkey_length)){
			ERR_DKCPRN("Failed to set planned new queue key(%s) name for queue prefix(%s) in DKCCOM_QPOP(%p)", bin_to_string(pQKey, qkey_length).c_str(), bin_to_string(pPrefix, pCom->prefix_length).c_str(), pRcvComAll);
			SetErrorResponseData(DKC_RES_SUBCODE_POPQUQUE);
			DKC_FREE(pEncPass);
			DKC_FREE(pMarker);
			DKC_FREE(pQKey);
			DKC_FREE(pNextKey);
			DKC_DELETE(pQueue);
			return false;
		}
		DKC_FREE(pNextKey);

		// result buffer
		unsigned char*	pResKey			= NULL;
		size_t			ResKeyLength	= 0L;
		unsigned char*	pResData		= NULL;
		size_t			ResDataLength	= 0;

		// [NOTE]
		// This loop is dummy for post processing as replicating marker
		//
		bool	result = false;
		do{
			// [5] get queue key's value(= poped queue value)
			{
				// [5-1] get value
				K2hdkcComGet*			pComGetObj	= GetCommonK2hdkcComGet(pK2hObj, pChmObj, SendMsgid, GetDispComNumber(), false, true, true);	// [NOTICE] wait result
				dkcres_type_t			rescode		= DKC_INITRESTYPE;
				const unsigned char*	pTmpResData	= NULL;
				if(!pComGetObj->CommandSend(pQKey, qkey_length, true, pEncPass, &pTmpResData, &ResDataLength, &rescode)){							// [NOTE] always check attr
					ERR_DKCPRN("Failed to get poped queue key(%s) value in DKCCOM_QPOP(%p).", bin_to_string(pQKey, qkey_length).c_str(), pRcvComAll);
					SetErrorResponseData(DKC_RES_SUBCODE_GETVAL);
					DKC_FREE(pEncPass);
					DKC_FREE(pMarker);
					DKC_FREE(pQKey);
					DKC_DELETE(pQueue);
					DKC_DELETE(pComGetObj);
					return false;
				}
				if(IS_DKC_RES_NOTSUCCESS(rescode)){
					ERR_DKCPRN("Failed to get poped queue key(%s) value in DKCCOM_QPOP(%p) by subcude(%s).", bin_to_string(pQKey, qkey_length).c_str(), pRcvComAll, STR_DKCRES_SUBCODE_TYPE(rescode));
					SetErrorResponseData(DKC_RES_SUBCODE_GETVAL);
					DKC_FREE(pEncPass);
					DKC_DELETE(pComGetObj);
					DKC_FREE(pMarker);
					DKC_FREE(pQKey);
					DKC_DELETE(pQueue);
					return false;
				}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
					MSG_DKCPRN("Get result subcode(%s) for getting poped queue key(%s) value in DKCCOM_QPOP(%p).", STR_DKCRES_SUBCODE_TYPE(rescode), bin_to_string(pQKey, qkey_length).c_str(), pRcvComAll);
				}

				// copy resdata
				pResData = k2hbindup(pTmpResData, ResDataLength);
				DKC_DELETE(pComGetObj);

				// [5-2] remove queue key
				K2hdkcComDel*	pComDelObj		= GetCommonK2hdkcComDel(pK2hObj, pChmObj, SendMsgid, GetDispComNumber(), false, true, true);	// [NOTICE] wait result
				rescode							= DKC_INITRESTYPE;
				if(!pComDelObj->CommandSend(pQKey, qkey_length, false, true, &rescode)){														// [NOTE] always check attr
					ERR_DKCPRN("Failed to delete poped queue key(%s) value in DKCCOM_QPOP(%p).", bin_to_string(pQKey, qkey_length).c_str(), pRcvComAll);
					SetErrorResponseData(DKC_RES_SUBCODE_DELKEY);
					DKC_FREE(pEncPass);
					DKC_FREE(pMarker);
					DKC_FREE(pQKey);
					DKC_FREE(pResData);
					DKC_DELETE(pQueue);
					DKC_DELETE(pComDelObj);
					return false;
				}
				DKC_DELETE(pComDelObj);

				if(IS_DKC_RES_NOTSUCCESS(rescode)){
					ERR_DKCPRN("Failed to delete poped queue key(%s) value in DKCCOM_QPOP(%p) by subcude(%s).", bin_to_string(pQKey, qkey_length).c_str(), pRcvComAll, STR_DKCRES_SUBCODE_TYPE(rescode));
					SetErrorResponseData(DKC_RES_SUBCODE_DELKEY);
					DKC_FREE(pEncPass);
					DKC_FREE(pMarker);
					DKC_FREE(pQKey);
					DKC_FREE(pResData);
					DKC_DELETE(pQueue);
					return false;
				}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
					ERR_DKCPRN("Get result subcode(%s) for deleting poped queue key(%s) value in DKCCOM_QPOP(%p)", STR_DKCRES_SUBCODE_TYPE(rescode), bin_to_string(pQKey, qkey_length).c_str(), pRcvComAll);
				}
			}
			DKC_FREE(pQKey);

			// [6] KEYQUEUE - read value from key(= poped queue key's value)
			if(pCom->keyqueue){
				// [6-1] read value
				pResKey								= pResData;			// swap
				ResKeyLength						= ResDataLength;	// swap
				pResData							= NULL;				// set null
				ResDataLength						= 0L;				// set 0
				K2hdkcComGet*			pComGetObj	= GetCommonK2hdkcComGet(pK2hObj, pChmObj, SendMsgid, GetDispComNumber(), false, true, true);	// [NOTICE] wait result
				dkcres_type_t			rescode		= DKC_INITRESTYPE;
				const unsigned char*	pTmpResData	= NULL;
				if(!pComGetObj->CommandSend(pResKey, ResKeyLength, true, pEncPass, &pTmpResData, &ResDataLength, &rescode)){						// [NOTE] always check attr
					ERR_DKCPRN("Failed to get poped key queue key(%s) value in DKCCOM_QPOP(%p).", bin_to_string(pResKey, ResKeyLength).c_str(), pRcvComAll);
					SetErrorResponseData(DKC_RES_SUBCODE_GETVAL);
					DKC_FREE(pEncPass);
					DKC_FREE(pMarker);
					DKC_FREE(pResKey);
					DKC_DELETE(pQueue);
					DKC_DELETE(pComGetObj);
					return false;
				}
				if(IS_DKC_RES_NOTSUCCESS(rescode)){
					ERR_DKCPRN("Failed to get poped key queue key(%s) value in DKCCOM_QPOP(%p) by subcude(%s).", bin_to_string(pResKey, ResKeyLength).c_str(), pRcvComAll, STR_DKCRES_SUBCODE_TYPE(rescode));
					SetErrorResponseData(DKC_RES_SUBCODE_GETVAL);
					DKC_FREE(pEncPass);
					DKC_DELETE(pComGetObj);
					DKC_FREE(pMarker);
					DKC_FREE(pResKey);
					DKC_DELETE(pQueue);
					return false;
				}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
					MSG_DKCPRN("Get result subcode(%s) for getting poped key queue key(%s) value in DKCCOM_QPOP(%p).", STR_DKCRES_SUBCODE_TYPE(rescode), bin_to_string(pResKey, ResKeyLength).c_str(), pRcvComAll);
				}

				// copy resdata
				pResData = k2hbindup(pTmpResData, ResDataLength);
				DKC_DELETE(pComGetObj);

				// [6-2] remove key
				K2hdkcComDel*	pComDelObj		= GetCommonK2hdkcComDel(pK2hObj, pChmObj, SendMsgid, GetDispComNumber(), false, true, true);	// [NOTICE] wait result
				rescode							= DKC_INITRESTYPE;
				if(!pComDelObj->CommandSend(pResKey, ResKeyLength, false, true, &rescode)){														// [NOTE] always check attr
					ERR_DKCPRN("Failed to delete poped queue key(%s) value in DKCCOM_QPOP(%p).", bin_to_string(pResKey, ResKeyLength).c_str(), pRcvComAll);
					SetErrorResponseData(DKC_RES_SUBCODE_DELKEY);
					DKC_FREE(pEncPass);
					DKC_FREE(pMarker);
					DKC_FREE(pResData);
					DKC_FREE(pResKey);
					DKC_DELETE(pQueue);
					DKC_DELETE(pComDelObj);
					return false;
				}
				DKC_DELETE(pComDelObj);

				if(IS_DKC_RES_NOTSUCCESS(rescode)){
					ERR_DKCPRN("Failed to delete poped queue key(%s) value in DKCCOM_QPOP(%p) by subcude(%s).", bin_to_string(pResKey, ResKeyLength).c_str(), pRcvComAll, STR_DKCRES_SUBCODE_TYPE(rescode));
					SetErrorResponseData(DKC_RES_SUBCODE_DELKEY);
					DKC_FREE(pEncPass);
					DKC_FREE(pMarker);
					DKC_FREE(pResData);
					DKC_FREE(pResKey);
					DKC_DELETE(pQueue);
					return false;
				}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
					ERR_DKCPRN("Get result subcode(%s) for deleting poped queue key(%s) value in DKCCOM_QPOP(%p)", STR_DKCRES_SUBCODE_TYPE(rescode), bin_to_string(pResKey, ResKeyLength).c_str(), pRcvComAll);
				}
			}
			result = true;

		}while(!result);

		// get time at setting.
		struct timespec	ts;
		INIT_DKC_TIMESPEC(ts);

		// [7] replicate merker key and datas to other servers assap
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
			if(!SetResponseData(pResKey, ResKeyLength, pResData, ResDataLength)){
				MSG_DKCPRN("Failed to make response data for marker prefix(%s)", bin_to_string(pPrefix, pCom->prefix_length).c_str());
				// continue for responsing
			}
		}
		DKC_FREE(pEncPass);
		DKC_FREE(pResKey);
		DKC_FREE(pResData);
		DKC_FREE(pMarker);
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

bool K2hdkcComQPop::CommandSend(const unsigned char* pprefix, size_t prefixlength, bool is_fifo, bool is_keyqueue, bool checkattr, const char* encpass)
{
	// do command
	// 
	// [NOTICE]
	// If command is keyueue type, at first we pop queue which is key name.
	// After that, we get value by key name and remove the key, and remake response
	// which is received by QPOP command.
	//

	// [1] make send data
	PDKCCOM_ALL	pComAll;
	if(NULL == (pComAll = reinterpret_cast<PDKCCOM_ALL>(malloc(sizeof(DKCCOM_QPOP) + prefixlength + (encpass ? (strlen(encpass) + 1) : 0))))){
		ERR_DKCPRN("Could not allocate memory.");
		return false;
	}
	PDKCCOM_QPOP	pComQPop		= CVT_DKCCOM_QPOP(pComAll);
	pComQPop->head.comtype			= DKC_COM_QPOP;
	pComQPop->head.restype			= DKC_NORESTYPE;
	pComQPop->head.comnumber		= GetComNumber();
	pComQPop->head.dispcomnumber	= GetDispComNumber();
	pComQPop->head.length			= sizeof(DKCCOM_QPOP) + prefixlength + (encpass ? (strlen(encpass) + 1) : 0);
	pComQPop->prefix_offset			= sizeof(DKCCOM_QPOP);
	pComQPop->prefix_length			= prefixlength;
	pComQPop->encpass_offset		= sizeof(DKCCOM_QPOP) + prefixlength;
	pComQPop->encpass_length		= encpass ? (strlen(encpass) + 1) : 0;
	pComQPop->fifo					= is_fifo;
	pComQPop->keyqueue				= is_keyqueue;

	unsigned char*	pdata		= NULL;
	if(pprefix && 0 < prefixlength){
		pdata					= reinterpret_cast<unsigned char*>(pComQPop) + pComQPop->prefix_offset;
		memcpy(pdata, pprefix, prefixlength);
	}
	if(encpass){
		pdata					= reinterpret_cast<unsigned char*>(pComQPop) + pComQPop->encpass_offset;
		memcpy(pdata, encpass, strlen(encpass) + 1);
	}

	if(!SetSendData(pComAll, K2hdkcCommand::MakeChmpxHash((reinterpret_cast<unsigned char*>(pComQPop) + pComQPop->prefix_offset), prefixlength))){
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

bool K2hdkcComQPop::CommandSend(const unsigned char* pprefix, size_t prefixlength, bool is_fifo, bool is_keyqueue, bool checkattr, const char* encpass, const unsigned char** ppkey, size_t* pkeylength, const unsigned char** ppval, size_t* pvallength, dkcres_type_t* prescode)
{
	if(!pChmObj){
		ERR_DKCPRN("Chmpx object is NULL, so could not send command.");
		return false;
	}

	if(!CommandSend(pprefix, prefixlength, is_fifo, is_keyqueue, checkattr, encpass)){
		ERR_DKCPRN("Failed to send command.");
		return false;
	}
	return GetResponseData(ppkey, pkeylength, ppval, pvallength, prescode);
}

bool K2hdkcComQPop::GetResponseData(const unsigned char** ppkey, size_t* pkeylength, const unsigned char** ppval, size_t* pvallength, dkcres_type_t* prescode) const
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
	PDKCRES_QPOP	pResQPop = CVT_DKCRES_QPOP(pcomall);
	if(DKC_COM_QPOP != pResQPop->head.comtype || DKC_NORESTYPE == pResQPop->head.restype || RcvComLength != pResQPop->head.length){
		ERR_DKCPRN("Response(received) data is somthing wrong(internal error: data is invalid).");
		return false;
	}
	if(ppkey && pkeylength){
		*ppkey		= (0 == pResQPop->key_length ? NULL : reinterpret_cast<const unsigned char*>(pResQPop) + pResQPop->key_offset);
		*pkeylength	= pResQPop->key_length;
	}
	if(ppval && pvallength){
		*ppval		= (0 == pResQPop->val_length ? NULL : reinterpret_cast<const unsigned char*>(pResQPop) + pResQPop->val_offset);
		*pvallength	= pResQPop->val_length;
	}
	return true;
}

bool K2hdkcComQPop::IsSuccessResponse(void) const
{
	PDKCCOM_ALL		pcomall = NULL;
	if(!K2hdkcCommand::GetResponseData(&pcomall, NULL, NULL) || !pcomall){
		return false;
	}
	if(DKC_NORESTYPE == pcomall->com_head.restype){
		return false;
	}
	return IS_DKC_RES_SUCCESS(pcomall->com_head.restype);
}

//---------------------------------------------------------
// Methods - Dump
//---------------------------------------------------------
void K2hdkcComQPop::RawDumpComAll(const PDKCCOM_ALL pComAll) const
{
	assert(DKC_COM_QPOP == pComAll->com_head.comtype);

	if(DKC_NORESTYPE == pComAll->com_head.restype){
		PDKCCOM_QPOP	pCom = CVT_DKCCOM_QPOP(pComAll);

		RAW_DKCPRN("  DKCCOM_QPOP = {");
		RAW_DKCPRN("    prefix_offset   = (%016" PRIx64 ") %s",	pCom->prefix_offset, bin_to_string(GET_BIN_DKC_COM_ALL(pComAll, pCom->prefix_offset, pCom->prefix_length), pCom->prefix_length).c_str());
		RAW_DKCPRN("    prefix_length   = %zu",					pCom->prefix_length);
		RAW_DKCPRN("    encpass_offset  = (%016" PRIx64 ") %s",	pCom->encpass_offset, bin_to_string(GET_BIN_DKC_COM_ALL(pComAll, pCom->encpass_offset, pCom->encpass_length), pCom->encpass_length).c_str());
		RAW_DKCPRN("    encpass_length  = %zu",					pCom->encpass_length);
		RAW_DKCPRN("    fifo            = %s",					pCom->fifo ? "yes" : "no");
		RAW_DKCPRN("    keyqueue        = %s",					pCom->keyqueue ? "yes" : "no");
		RAW_DKCPRN("  }");
	}else{
		//PDKCRES_QPOP	pCom = CVT_DKCRES_QPOP(pComAll);
		RAW_DKCPRN("  DKCRES_QPOP = {");
			// nothing to print member
		RAW_DKCPRN("  }");
	}
}

/*
 * VIM modelines
 *
 * vim:set ts=4 fenc=utf-8:
 */
