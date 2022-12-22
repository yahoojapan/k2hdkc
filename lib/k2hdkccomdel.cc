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

#include <k2hash/k2hashfunc.h>

#include "k2hdkccomdel.h"
#include "k2hdkccomgetsubkeys.h"
#include "k2hdkccomrepldel.h"
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
			K2hdkcComDel*	pComDelSub	= GetCommonK2hdkcComDel(pK2hObj, pChmObj, SendMsgid, GetDispComNumber(), false, true, false);
			rescode						= DKC_INITRESTYPE;
			if(!pComDelSub->CommandSend(iter->pSubKey, iter->length, is_subkeys, checkattr, &rescode)){
				ERR_DKCPRN("Failed to remove subkey(%s) in key(%s) in DKCCOM_DEL(%p), but continue...", bin_to_string(iter->pSubKey, iter->length).c_str(), bin_to_string(pkey, keylength).c_str(), pRcvComAll);
			}
			DKC_DELETE(pComDelSub);
		}
	}
	DKC_DELETE(pSubKeys);

	// [*] special case
	// 
	// If the key is completely deleted on this node, but the key is holded in
	// another node and that node is up after deleting, the key might be restored
	// when data is merged.
	// If the server on the CHMPX RING is down or in operation(additions, deletions,
	// and their pending states), remember the key as a placeholder.
	// The placeholder writes the key expiration date as 0 seconds.
	//
	if(!IsAllNodesSafe()){
		// make placeholder
		K2hdkcComSet*	pComSetObj	= GetCommonK2hdkcComSet(pK2hObj, pChmObj, SendMsgid, GetDispComNumber(), false, true, false);
		time_t			expire		= 0;																	// always expired
		unsigned char	dummyval	= 0x0;																	// dummy value for clearing
		rescode						= DKC_INITRESTYPE;
		if(!pComSetObj->CommandSend(pkey, keylength, &dummyval, 1, false, NULL, &(expire), &rescode)){		// [NOTE] only key name with expire 0 sec for placeholder
			ERR_DKCPRN("Failed to make placeholder for key(%s) in DKCCOM_DEL(%p), but continue...", bin_to_string(pkey, keylength).c_str(), pRcvComAll);
		}
		DKC_DELETE(pComSetObj);
	}

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

// [NOTE]
// Replication of the delete command does not need to check the value of the key
// to be deleted, the subkey list, and the attribute, and overrides in this class.
//
bool K2hdkcComDel::CommandReplicate(const unsigned char* pkey, size_t keylength, const struct timespec ts)
{
	if(!pK2hObj || !pChmObj){
		ERR_DKCPRN("K2hash and Chmpx object are NULL, so could not send replicate command.");
		return false;
	}
	if(!IsServerNode){
		MSG_DKCPRN("Called replicate command from client on slave node, this method must be called from on server node.");
		return false;
	}

	// hash value made by same logic in k2hash
	k2h_hash_t	hash	= K2H_HASH_FUNC(reinterpret_cast<const void*>(pkey), keylength);
	k2h_hash_t	subhash	= K2H_2ND_HASH_FUNC(reinterpret_cast<const void*>(pkey), keylength);

	// Replicate Command object
	K2hdkcComReplDel*	pReplDel = GetCommonK2hdkcComReplDel(pK2hObj, pChmObj, SendMsgid, GetDispComNumber(), true, true, false);	// [NOTICE] always without self! and not wait response
	bool				result;
	if(false == (result = pReplDel->CommandSend(hash, subhash, pkey, keylength, ts))){
		ERR_DKCPRN("Failed to replicate command for key(%s).", bin_to_string(pkey, keylength).c_str());
	}

	// [NOTE]
	// This replicate command is sent on server node, so this response is received in event loop.
	// Then we do not get the response here.
	//
	return result;
}

// [NOTE]
// This method checks the status of all CHMPX server nodes and returns true
// if there may be a mismatch in the deleting key.
// Deleting key must be stored as a placeholder when a server on the RING is
// down/adding/deleting, thus this method is called.
//
// This method is a special CHMPX status check function used only by this class.
// However, if you need the whole K2HDKC eventually, move to a common function.
//
bool K2hdkcComDel::IsAllNodesSafe(void)
{
	if(!pChmObj){
		ERR_DKCPRN("Chmpx object are NULL, so could not check all nodes.");
		return false;
	}

	// get all chmpx information on server nodes
	bool		result		= true;
	PCHMINFOEX	pChmpxInfos	= pChmObj->DupAllChmInfo();
	if(!pChmpxInfos){
		WAN_DKCPRN("Failed to get all chmpx information, but return true...");
		return result;
	}

	// check all server node status
	if(pChmpxInfos->pchminfo && 0 < pChmpxInfos->pchminfo->chmpx_man.chmpx_server_count){
		for(PCHMPXLIST pchmpxlist = pChmpxInfos->pchminfo->chmpx_man.chmpx_servers; pchmpxlist; pchmpxlist = pchmpxlist->next){
			if(IS_CHMPXSTS_SRVIN(pchmpxlist->chmpx.status)){
				if(IS_CHMPXSTS_DOWN(pchmpxlist->chmpx.status)){
					// found SERVICE IN & DOWN node
					MSG_DKCPRN("Found SERVICEIN & DOWN node.");
					result = false;
					break;
				}else if(!IS_CHMPXSTS_NOACT(pchmpxlist->chmpx.status)){
					// found SERVICE IN & UP & ADD/DELETE action node --> This means that some node will be adding/deleting
					MSG_DKCPRN("Found SERVICEIN & UP & ADD/DELETE(pending/doing/done) node.");
					result = false;
					break;
				}
			}else{
				if(IS_CHMPXSTS_UP(pchmpxlist->chmpx.status) && !IS_CHMPXSTS_NOACT(pchmpxlist->chmpx.status)){
					// found SERVICE OUT & UP & ADD/DELETE action node --> This node will be adding
					MSG_DKCPRN("Found SERVICEOUT & UP & ADD(pending/doing/done) node.");
					result = false;
					break;
				}
			}
		}
	}
	ChmCntrl::FreeDupAllChmInfo(pChmpxInfos);

	return result;
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
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noexpandtab sw=4 ts=4 fdm=marker
 * vim<600: noexpandtab sw=4 ts=4
 */
