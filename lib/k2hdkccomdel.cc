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
 * CREATE:   Thu Jul 28 2016
 * REVISION:
 * 
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "k2hdkccomdel.h"
#include "k2hdkccomgetsubkeys.h"
#include "k2hdkccomutil.h"
#include "k2hdkcutil.h"
#include "k2hdkcdbg.h"

using namespace	std;

//---------------------------------------------------------
// Constructor/Destructor
//---------------------------------------------------------
K2hdkcComDel::K2hdkcComDel(K2HShm* pk2hash, ChmCntrl* pchmcntrl, uint64_t comnum, bool without_self, bool is_routing_on_server, bool is_wait_on_server) : K2hdkcCommand(pk2hash, pchmcntrl, comnum, without_self, is_routing_on_server, is_wait_on_server, DKC_COM_DEL)
{
	strClassName = "K2hdkcComDel";
}

K2hdkcComDel::~K2hdkcComDel(void)
{
}

//---------------------------------------------------------
// Methods
//---------------------------------------------------------
PDKCCOM_ALL K2hdkcComDel::MakeResponseOnlyHeadData(dkcres_type_t subcode, dkcres_type_t errcode)
{
	PDKCCOM_ALL	pComErr;
	if(NULL == (pComErr = reinterpret_cast<PDKCCOM_ALL>(malloc(sizeof(DKCRES_DEL))))){
		ERR_DKCPRN("Could not allocate memory.");
		return NULL;
	}
	PDKCRES_DEL	pComRes			= CVT_DKCRES_DEL(pComErr);
	pComRes->head.comtype		= DKC_COM_DEL;
	pComRes->head.restype		= COMPOSE_DKC_RES(errcode, subcode);
	pComRes->head.comnumber		= GetComNumber();
	pComRes->head.dispcomnumber	= GetDispComNumber();
	pComRes->head.length		= sizeof(DKCRES_DEL);

	return pComErr;
}

bool K2hdkcComDel::SetSucceedResponseData(void)
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

bool K2hdkcComDel::CommandProcessing(void)
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
		const PDKCCOM_DEL		pCom	= CVT_DKCCOM_DEL(pRcvComAll);
		const unsigned char*	pKey	= GET_BIN_DKC_COM_ALL(pRcvComAll, pCom->key_offset, pCom->key_length);
		if(!pKey){
			ERR_DKCPRN("Could not get safe key(%zd offset, %zu byte) in DKCCOM_DEL(%p).", pCom->key_offset, pCom->key_length, pRcvComAll);
			SetErrorResponseData(DKC_RES_SUBCODE_INVAL);
			return false;
		}

		// do command
		// 
		// [NOTE]
		// Here, we delete only key, because we already remove subkeys in key before
		// receiving command(slave node side).
		//
		if(!pK2hObj->Remove(pKey, pCom->key_length, false)){							// [NOTICE] does not remove subkeys list always
			ERR_DKCPRN("Failed to remove key(%s)", bin_to_string(pKey, pCom->key_length).c_str());
			SetErrorResponseData(DKC_RES_SUBCODE_DELKEY);
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

bool K2hdkcComDel::CommandSend(const unsigned char* pkey, size_t keylength, bool is_subkeys, bool checkattr)
{
	if(!pkey || 0 == keylength){
		ERR_DKCPRN("Parameter are wrong.");
		return false;
	}

	// do command
	// 
	// [NOTICE]
	// This command is a collection of other command which is delete subkey.
	// The if some operation is occurred to key and subkey by other process during
	// from modifying(creating) subkey to adding subkey name to key's subkey list,
	// maybe there is a possibility that the inconvenience will occur.
	//

	// [1] get subkey list before removing key
	K2HSubKeys*		pSubKeys	= NULL;
	dkcres_type_t	rescode		= DKC_INITRESTYPE;
	if(is_subkeys){
		K2hdkcComGetSubkeys*	pComGetSubkeys	= GetCommonK2hdkcComGetSubkeys(pK2hObj, pChmObj, SendMsgid, GetDispComNumber(), IsSendWithoutSelf(), false, true);	// [NOTICE] wait result
		if(!pComGetSubkeys->CommandSend(pkey, keylength, checkattr, &pSubKeys, &rescode) || !pSubKeys){
			MSG_DKCPRN("Could not get subkey in key(%s) in DKCCOM_DEL(%p), continue...", bin_to_string(pkey, keylength).c_str(), pRcvComAll);
		}
	}

	// [2] delete key(send command)
	PDKCCOM_ALL	pComAll;
	if(NULL == (pComAll = reinterpret_cast<PDKCCOM_ALL>(malloc(sizeof(DKCCOM_DEL) + keylength)))){
		ERR_DKCPRN("Could not allocate memory.");
		DKC_DELETE(pSubKeys);
		return false;
	}
	PDKCCOM_DEL	pComDel			= CVT_DKCCOM_DEL(pComAll);
	pComDel->head.comtype		= DKC_COM_DEL;
	pComDel->head.restype		= DKC_NORESTYPE;
	pComDel->head.comnumber		= GetComNumber();
	pComDel->head.dispcomnumber	= GetDispComNumber();
	pComDel->head.length		= sizeof(DKCCOM_DEL) + keylength;
	pComDel->key_offset			= sizeof(DKCCOM_DEL);
	pComDel->key_length			= keylength;

	unsigned char*	pdata		= reinterpret_cast<unsigned char*>(pComDel) + pComDel->key_offset;
	memcpy(pdata, pkey, keylength);

	if(!SetSendData(pComAll, K2hdkcCommand::MakeChmpxHash((reinterpret_cast<unsigned char*>(pComDel) + pComDel->key_offset), keylength))){
		ERR_DKCPRN("Failed to set command data to internal buffer.");
		DKC_FREE(pComAll);
		DKC_DELETE(pSubKeys);
		return false;
	}

	// do command & receive response(on slave node)
	if(!CommandSend()){
		ERR_DKCPRN("Failed to send command.");
		DKC_DELETE(pSubKeys);
		return false;
	}

	// [3] remove subkeys if it exists
	//
	// [NOTE]
	// Even if an error occurs, output only the message and continue processing.
	//
	if(is_subkeys && pSubKeys){
		// cppcheck-suppress postfixOperator
		for(K2HSubKeys::iterator iter = pSubKeys->begin(); iter != pSubKeys->end(); iter++){
			if(0UL == iter->length){
				WAN_DKCPRN("Subkey is empty in key(%s).", bin_to_string(pkey, keylength).c_str());
				continue;
			}
			// remove subkey
			K2hdkcComDel*	pComDel	= GetCommonK2hdkcComDel(pK2hObj, pChmObj, SendMsgid, GetDispComNumber(), false, true, false);
			rescode					= DKC_INITRESTYPE;
			if(!pComDel->CommandSend(iter->pSubKey, iter->length, is_subkeys, checkattr, &rescode)){
				ERR_DKCPRN("Failed to remove subkey(%s) in key(%s) in DKCCOM_DEL(%p), but continue...", bin_to_string(iter->pSubKey, iter->length).c_str(), bin_to_string(pkey, keylength).c_str(), pRcvComAll);
			}
			DKC_DELETE(pComDel);
		}
	}
	DKC_DELETE(pSubKeys);

	return true;
}

bool K2hdkcComDel::CommandSend(const unsigned char* pkey, size_t keylength, bool is_subkeys, bool checkattr, dkcres_type_t* prescode)
{
	if(!pChmObj){
		ERR_DKCPRN("Chmpx object is NULL, so could not send command.");
		return false;
	}

	if(!CommandSend(pkey, keylength, is_subkeys, checkattr)){
		ERR_DKCPRN("Failed to send command.");
		return false;
	}
	return GetResponseData(prescode);
}

bool K2hdkcComDel::GetResponseData(dkcres_type_t* prescode) const
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
void K2hdkcComDel::RawDumpComAll(const PDKCCOM_ALL pComAll) const
{
	assert(DKC_COM_DEL == pComAll->com_head.comtype);

	if(DKC_NORESTYPE == pComAll->com_head.restype){
		PDKCCOM_DEL	pCom = CVT_DKCCOM_DEL(pComAll);

		RAW_DKCPRN("  DKCCOM_DEL = {");
		RAW_DKCPRN("    key_offset      = (%016" PRIx64 ") %s",	pCom->key_offset, bin_to_string(GET_BIN_DKC_COM_ALL(pComAll, pCom->key_offset, pCom->key_length), pCom->key_length).c_str());
		RAW_DKCPRN("    key_length      = %zu",					pCom->key_length);
		RAW_DKCPRN("  }");

	}else{
		//PDKCRES_DEL	pCom = CVT_DKCRES_DEL(pComAll);
		RAW_DKCPRN("  DKCRES_DEL = {");
			// nothing to print member
		RAW_DKCPRN("  }");
	}
}

/*
 * VIM modelines
 *
 * vim:set ts=4 fenc=utf-8:
 */
