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

#include "k2hdkccomren.h"
#include "k2hdkccomsetall.h"
#include "k2hdkccomdelsubkeys.h"
#include "k2hdkccomaddsubkeys.h"
#include "k2hdkccomutil.h"
#include "k2hdkcutil.h"
#include "k2hdkcdbg.h"

using namespace	std;

//---------------------------------------------------------
// Constructor/Destructor
//---------------------------------------------------------
K2hdkcComRen::K2hdkcComRen(K2HShm* pk2hash, ChmCntrl* pchmcntrl, uint64_t comnum, bool without_self, bool is_routing_on_server, bool is_wait_on_server) : K2hdkcCommand(pk2hash, pchmcntrl, comnum, without_self, is_routing_on_server, is_wait_on_server, DKC_COM_REN)
{
	strClassName = "K2hdkcComRen";
}

K2hdkcComRen::~K2hdkcComRen(void)
{
}

//---------------------------------------------------------
// Methods
//---------------------------------------------------------
PDKCCOM_ALL K2hdkcComRen::MakeResponseOnlyHeadData(dkcres_type_t subcode, dkcres_type_t errcode)
{
	PDKCCOM_ALL	pComErr;
	if(NULL == (pComErr = reinterpret_cast<PDKCCOM_ALL>(malloc(sizeof(DKCRES_REN))))){
		ERR_DKCPRN("Could not allocate memory.");
		return NULL;
	}
	PDKCRES_REN	pComRes			= CVT_DKCRES_REN(pComErr);
	pComRes->head.comtype		= DKC_COM_REN;
	pComRes->head.restype		= COMPOSE_DKC_RES(errcode, subcode);
	pComRes->head.comnumber		= GetComNumber();
	pComRes->head.dispcomnumber	= GetDispComNumber();
	pComRes->head.length		= sizeof(DKCRES_REN);

	return pComErr;
}

bool K2hdkcComRen::SetSucceedResponseData(void)
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

bool K2hdkcComRen::CommandProcessing(void)
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
		const PDKCCOM_REN		pCom	= CVT_DKCCOM_REN(pRcvComAll);
		const unsigned char*	pOldKey	= GET_BIN_DKC_COM_ALL(pRcvComAll, pCom->oldkey_offset, pCom->oldkey_length);
		const unsigned char*	pNewKey	= GET_BIN_DKC_COM_ALL(pRcvComAll, pCom->newkey_offset, pCom->newkey_length);
		char*					pEncPass= NULL;
		if(0 < pCom->encpass_length){
			pEncPass					= reinterpret_cast<char*>(k2hbindup(GET_BIN_DKC_COM_ALL(pRcvComAll, pCom->encpass_offset, pCom->encpass_length), pCom->encpass_length));
		}
		if(!pOldKey || !pNewKey){
			ERR_DKCPRN("Could not get safe old key(%zd offset, %zu byte) and new key(%zd offset, %zu byte) in DKCCOM_REN(%p).", pCom->oldkey_offset, pCom->oldkey_length, pCom->newkey_offset, pCom->newkey_length, pRcvComAll);
			SetErrorResponseData(DKC_RES_SUBCODE_INVAL);
			DKC_FREE(pEncPass);
			return false;
		}
		// check key name
		if(pCom->oldkey_length == pCom->newkey_length && 0 == memcmp(pOldKey, pNewKey, pCom->oldkey_length)){
			MSG_DKCPRN("old key name(%s) is as same as old key name(%s), so do nothing.", bin_to_string(pOldKey, pCom->oldkey_length).c_str(), bin_to_string(pNewKey, pCom->newkey_length).c_str());
			SetSucceedResponseData();
			DKC_FREE(pEncPass);
			return true;						// [NOTICE] returns success
		}

		// do command
		// 
		// [NOTE]
		// at first, gets all data(value, subkeys, attrs) from old key. after that,
		// set new key with all data and remove old key.
		//

		// for locking data
		K2HDALock*	pLock = new K2HDALock(pK2hObj, false, pOldKey, pCom->oldkey_length);

		// [1] get all data
		// get value	[NOTE] : We read value twice when pEncPass is specified.
		//
		unsigned char*	pValue = NULL;
		ssize_t			ValLen;
		if(0 >= (ValLen = pK2hObj->Get(pOldKey, pCom->oldkey_length, &pValue, pCom->check_attr, pEncPass)) || !pValue){
			MSG_DKCPRN("Error or No data for key(%s)", bin_to_string(pOldKey, pCom->oldkey_length).c_str());
			SetErrorResponseData(DKC_RES_SUBCODE_NODATA);
			DKC_FREE(pEncPass);
			DKC_DELETE(pLock);
			return false;

		}else if(pEncPass){
			// [NOTE]
			// if pEncPass is not NULL, we need raw value which is encrypted.
			// because we use SetAll function, it reqiures raw value.
			// thus we read value twice, but it is not high costs because reading value from local.
			//
			DKC_FREE(pValue);
			if(0 >= (ValLen = pK2hObj->Get(pOldKey, pCom->oldkey_length, &pValue, false, NULL)) || !pValue){
				MSG_DKCPRN("Error or No data for key(%s)", bin_to_string(pOldKey, pCom->oldkey_length).c_str());
				SetErrorResponseData(DKC_RES_SUBCODE_NODATA);
				DKC_FREE(pEncPass);
				DKC_DELETE(pLock);
				return false;
			}
		}
		DKC_FREE(pEncPass);

		// get subkeys
		K2HSubKeys*		pSubKeys;
		unsigned char*	bySubkeys		= NULL;
		size_t			subkeys_length	= 0;
		if(NULL == (pSubKeys = pK2hObj->GetSubKeys(pOldKey, pCom->oldkey_length, pCom->check_attr))){
			// continue...
			MSG_DKCPRN("Error or No subkeys for key(%s)", bin_to_string(pOldKey, pCom->oldkey_length).c_str());
		}else{
			// serialize
			if(!pSubKeys->Serialize(&bySubkeys, subkeys_length)){
				ERR_DKCPRN("Failed to serialize subkeys to binary data for key(%s) in DKCCOM_REN(%p).", bin_to_string(pOldKey, pCom->oldkey_length).c_str(), pRcvComAll);
				SetErrorResponseData(DKC_RES_SUBCODE_INTERNAL);
				DKC_DELETE(pSubKeys);
				DKC_FREE(pValue);
				DKC_DELETE(pLock);
				return false;
			}
			DKC_DELETE(pSubKeys);
		}

		// get attributes
		K2HAttrs*		pAttrsObj;
		unsigned char*	byAttrs		= NULL;
		size_t			attrs_length= 0;
		if(NULL == (pAttrsObj = pK2hObj->GetAttrs(pOldKey, pCom->oldkey_length))){
			// continue...
			MSG_DKCPRN("Error or No attributes for key(%s)", bin_to_string(pOldKey, pCom->oldkey_length).c_str());
		}else{
			// serialize
			if(!pAttrsObj->Serialize(&byAttrs, attrs_length)){
				ERR_DKCPRN("Failed to serialize attributes to binary data for key data(%s) in DKCCOM_REN(%p).", bin_to_string(pOldKey, pCom->oldkey_length).c_str(), pRcvComAll);
				SetErrorResponseData(DKC_RES_SUBCODE_INTERNAL);
				DKC_DELETE(pAttrsObj);
				DKC_FREE(bySubkeys);
				DKC_FREE(pValue);
				DKC_DELETE(pLock);
				return false;
			}
			DKC_DELETE(pAttrsObj);
		}

		// unlock
		DKC_DELETE(pLock);

		// [2] set new key(over write) by another command
		K2hdkcComSetAll*	pComSetAllObj	= GetCommonK2hdkcComSetAll(pK2hObj, pChmObj, SendMsgid, GetDispComNumber(), false, true, true);	// [NOTICE] send all server node & wait result
		dkcres_type_t		rescode			= DKC_INITRESTYPE;
		if(!pComSetAllObj->CommandSend(pNewKey, pCom->newkey_length, pValue, ValLen, bySubkeys, subkeys_length, byAttrs, attrs_length, &rescode)){
			ERR_DKCPRN("Failed to set new key(%s) with val(%s), subkeys(%s), attrs(%s) in DKCCOM_REN(%p).", bin_to_string(pNewKey, pCom->newkey_length).c_str(), bin_to_string(pValue, ValLen).c_str(), bin_to_string(bySubkeys, subkeys_length).c_str(), bin_to_string(byAttrs, attrs_length).c_str(), pRcvComAll);
			SetErrorResponseData(DKC_RES_SUBCODE_SETALL);
			DKC_DELETE(pComSetAllObj);
			DKC_FREE(byAttrs);
			DKC_FREE(bySubkeys);
			DKC_FREE(pValue);
			return false;
		}
		DKC_DELETE(pComSetAllObj);
		DKC_FREE(byAttrs);
		DKC_FREE(bySubkeys);
		DKC_FREE(pValue);

		if(IS_DKC_RES_NOTSUCCESS(rescode)){
			ERR_DKCPRN("Failed to set new key(%s) with val(%s), subkeys(%s), attrs(%s) in DKCCOM_REN(%p) by subcude(%s).", bin_to_string(pNewKey, pCom->newkey_length).c_str(), bin_to_string(pValue, ValLen).c_str(), bin_to_string(bySubkeys, subkeys_length).c_str(), bin_to_string(byAttrs, attrs_length).c_str(), pRcvComAll, STR_DKCRES_SUBCODE_TYPE(rescode));
			SetErrorResponseData(GET_DKC_RES_SUBCODE(rescode));
			return false;
		}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
			ERR_DKCPRN("Get result subcode(%s) for setting new key(%s) with val(%s), subkeys(%s), attrs(%s) in DKCCOM_REN(%p).", STR_DKCRES_SUBCODE_TYPE(rescode), bin_to_string(pNewKey, pCom->newkey_length).c_str(), bin_to_string(pValue, ValLen).c_str(), bin_to_string(bySubkeys, subkeys_length).c_str(), bin_to_string(byAttrs, attrs_length).c_str(), pRcvComAll);
		}

		// [3] remove old key on localhost and replicate it
		// remove old key without subkeys
		if(!pK2hObj->Remove(pOldKey, pCom->oldkey_length, false)){																					// [NOTICE] does not remove subkeys
			ERR_DKCPRN("Failed to remove old key(%s)", bin_to_string(pOldKey, pCom->oldkey_length).c_str());
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
			ERR_DKCPRN("Failed sending response for key(%s)", bin_to_string(pOldKey, pCom->oldkey_length).c_str());
			return false;
		}

		// replicate key and datas to other servers
		//
		// [NOTE] reason of sending other server after replying result.
		// because the result of sending replication data to other server on server node can not be caught
		// this event loop, it is caught another session after this event handling.
		// (this result is caught in event loop on server node)
		//
		if(!CommandReplicate(pOldKey, pCom->oldkey_length, ts)){
			ERR_DKCPRN("Failed to replicate new key(%s) to other servers, but continue...", bin_to_string(pOldKey, pCom->oldkey_length).c_str());
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

bool K2hdkcComRen::CommandSend(const unsigned char* poldkey, size_t oldkeylength, const unsigned char* pnewkey, size_t newkeylength, const unsigned char* pparentkey, size_t parentkeylength, bool checkattr, const char* encpass, const time_t* expire)
{
	if(!poldkey || 0 == oldkeylength || !pnewkey || 0 == newkeylength){
		ERR_DKCPRN("Parameter are wrong.");
		return false;
	}

	// do command
	// 
	// [NOTICE] Case of specifying parent key
	// This command is a collection of other 2 commands which are remoing/adding subkey
	// after renaming commands.
	// The if some operation is occuured to key and subkey by other process during
	// from modifying(creating) subkey to adding/removing subkey name to key's subkey
	// list, maybe there is a possibility that the inconvenience will occur.
	//

	// [0] make send data
	PDKCCOM_ALL	pComAll;
	if(NULL == (pComAll = reinterpret_cast<PDKCCOM_ALL>(malloc(sizeof(DKCCOM_REN) + oldkeylength + newkeylength + (encpass ? (strlen(encpass) + 1) : 0))))){
		ERR_DKCPRN("Could not allocate memory.");
		return false;
	}
	PDKCCOM_REN	pComRen			= CVT_DKCCOM_REN(pComAll);
	pComRen->head.comtype		= DKC_COM_REN;
	pComRen->head.restype		= DKC_NORESTYPE;
	pComRen->head.comnumber		= GetComNumber();
	pComRen->head.dispcomnumber	= GetDispComNumber();
	pComRen->head.length		= sizeof(DKCCOM_REN) + oldkeylength + newkeylength + (encpass ? (strlen(encpass) + 1) : 0);
	pComRen->oldkey_offset		= sizeof(DKCCOM_REN);
	pComRen->oldkey_length		= oldkeylength;
	pComRen->newkey_offset		= sizeof(DKCCOM_REN) + oldkeylength;
	pComRen->newkey_length		= newkeylength;
	pComRen->encpass_offset		= sizeof(DKCCOM_REN) + oldkeylength + newkeylength;
	pComRen->encpass_length		= encpass ? (strlen(encpass) + 1) : 0;
	pComRen->check_attr			= checkattr;

	unsigned char*	pdata		= reinterpret_cast<unsigned char*>(pComRen) + pComRen->oldkey_offset;
	memcpy(pdata, poldkey, oldkeylength);
	pdata						= reinterpret_cast<unsigned char*>(pComRen) + pComRen->newkey_offset;
	memcpy(pdata, pnewkey, newkeylength);
	if(encpass){
		pdata					= reinterpret_cast<unsigned char*>(pComRen) + pComRen->encpass_offset;
		memcpy(pdata, encpass, strlen(encpass) + 1);
	}
	if(!SetSendData(pComAll, K2hdkcCommand::MakeChmpxHash((reinterpret_cast<unsigned char*>(pComRen) + pComRen->oldkey_offset), oldkeylength))){
		ERR_DKCPRN("Failed to set command data to internal buffer.");
		DKC_FREE(pComAll);
		return false;
	}

	// [1] do command & receive response(on slave node)
	if(!CommandSend()){
		ERR_DKCPRN("Failed to send command.");
		return false;
	}

	// parent key is specified
	if(pparentkey && 0 != parentkeylength){
		// check result
		dkcres_type_t	rescode = GetResponseCode();
		if(IS_DKC_RES_NOTSUCCESS(rescode)){
			ERR_DKCPRN("Failed to rename key(%s) by subcude(%s).", bin_to_string(poldkey, oldkeylength).c_str(), STR_DKCRES_SUBCODE_TYPE(rescode));
			return false;
		}

		// [2] remove subkey(oldkey) in parent key
		K2hdkcComDelSubkeys*	pComDelSKeyObj	= GetCommonK2hdkcComDelSubkeys(pK2hObj, pChmObj, SendMsgid, GetDispComNumber(), IsSendWithoutSelf(), true, false);
		rescode									= DKC_INITRESTYPE;
		if(!pComDelSKeyObj->CommandSend(pparentkey, parentkeylength, poldkey, oldkeylength, checkattr, &rescode)){
			ERR_DKCPRN("Failed to remove subkey(%s) from key(%s).", bin_to_string(poldkey, oldkeylength).c_str(), bin_to_string(pparentkey, parentkeylength).c_str());
			DKC_DELETE(pComDelSKeyObj);
			return false;
		}
		DKC_DELETE(pComDelSKeyObj);

		if(IS_DKC_RES_NOTSUCCESS(rescode)){
			ERR_DKCPRN("Failed to remove subkey(%s) from key(%s) by subcude(%s).", bin_to_string(poldkey, oldkeylength).c_str(), bin_to_string(pparentkey, parentkeylength).c_str(), STR_DKCRES_SUBCODE_TYPE(rescode));
			return false;
		}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
			MSG_DKCPRN("Get result subcode(%s) for removing subkey(%s) from key(%s).", STR_DKCRES_SUBCODE_TYPE(rescode), bin_to_string(poldkey, oldkeylength).c_str(), bin_to_string(pparentkey, parentkeylength).c_str());
		}

		// [3] add subkey(newkey) in parent key
		K2hdkcComAddSubkeys*	pComAddSKeyObj	= GetCommonK2hdkcComAddSubkeys(pK2hObj, pChmObj, SendMsgid, GetDispComNumber(), IsSendWithoutSelf(), true, false);
		rescode									= DKC_INITRESTYPE;
		if(!pComAddSKeyObj->CommandSend(pparentkey, parentkeylength, pnewkey, newkeylength, checkattr, &rescode)){
			ERR_DKCPRN("Failed to add subkey(%s) from key(%s).", bin_to_string(pnewkey, newkeylength).c_str(), bin_to_string(pparentkey, parentkeylength).c_str());
			DKC_DELETE(pComAddSKeyObj);
			return false;
		}
		DKC_DELETE(pComAddSKeyObj);

		if(IS_DKC_RES_NOTSUCCESS(rescode)){
			ERR_DKCPRN("Failed to remove subkey(%s) from key(%s) by subcude(%s).", bin_to_string(pnewkey, newkeylength).c_str(), bin_to_string(pparentkey, parentkeylength).c_str(), STR_DKCRES_SUBCODE_TYPE(rescode));
			return false;
		}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
			MSG_DKCPRN("Get result subcode(%s) for adding subkey(%s) from key(%s).", STR_DKCRES_SUBCODE_TYPE(rescode), bin_to_string(pnewkey, newkeylength).c_str(), bin_to_string(pparentkey, parentkeylength).c_str());
		}
	}
	return true;
}

bool K2hdkcComRen::CommandSend(const unsigned char* poldkey, size_t oldkeylength, const unsigned char* pnewkey, size_t newkeylength, const unsigned char* pparentkey, size_t parentkeylength, bool checkattr, const char* encpass, const time_t* expire, dkcres_type_t* prescode)
{
	if(!pChmObj){
		ERR_DKCPRN("Chmpx object is NULL, so could not send command.");
		return false;
	}

	if(!CommandSend(poldkey, oldkeylength, pnewkey, newkeylength, pparentkey, parentkeylength, checkattr, encpass, expire)){
		ERR_DKCPRN("Failed to send command.");
		return false;
	}
	return GetResponseData(prescode);
}

bool K2hdkcComRen::GetResponseData(dkcres_type_t* prescode) const
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
void K2hdkcComRen::RawDumpComAll(const PDKCCOM_ALL pComAll) const
{
	assert(DKC_COM_REN == pComAll->com_head.comtype);

	if(DKC_NORESTYPE == pComAll->com_head.restype){
		PDKCCOM_REN	pCom = CVT_DKCCOM_REN(pComAll);

		RAW_DKCPRN("  DKCCOM_REN = {");
		RAW_DKCPRN("    oldkey_offset   = (%016" PRIx64 ") %s",	pCom->oldkey_offset, bin_to_string(GET_BIN_DKC_COM_ALL(pComAll, pCom->oldkey_offset, pCom->oldkey_length), pCom->oldkey_length).c_str());
		RAW_DKCPRN("    oldkey_length   = %zu",					pCom->oldkey_length);
		RAW_DKCPRN("    newkey_offset   = (%016" PRIx64 ") %s",	pCom->newkey_offset, bin_to_string(GET_BIN_DKC_COM_ALL(pComAll, pCom->newkey_offset, pCom->newkey_length), pCom->newkey_length).c_str());
		RAW_DKCPRN("    newkey_length   = %zu",					pCom->newkey_length);
		RAW_DKCPRN("  }");

	}else{
		//PDKCRES_REN	pCom = CVT_DKCRES_REN(pComAll);
		RAW_DKCPRN("  DKCRES_REN = {");
			// nothing to print member
		RAW_DKCPRN("  }");
	}
}

/*
 * VIM modelines
 *
 * vim:set ts=4 fenc=utf-8:
 */
