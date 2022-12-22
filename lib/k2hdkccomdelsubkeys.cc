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

#include "k2hdkccomdelsubkeys.h"
#include "k2hdkcutil.h"
#include "k2hdkcdbg.h"

using namespace	std;

//---------------------------------------------------------
// Constructor/Destructor
//---------------------------------------------------------
K2hdkcComDelSubkeys::K2hdkcComDelSubkeys(K2HShm* pk2hash, ChmCntrl* pchmcntrl, uint64_t comnum, bool without_self, bool is_routing_on_server, bool is_wait_on_server) : K2hdkcCommand(pk2hash, pchmcntrl, comnum, without_self, is_routing_on_server, is_wait_on_server, DKC_COM_DEL_SUBKEYS)
{
	strClassName = "K2hdkcComDelSubkeys";
}

K2hdkcComDelSubkeys::~K2hdkcComDelSubkeys(void)
{
}

//---------------------------------------------------------
// Methods
//---------------------------------------------------------
PDKCCOM_ALL K2hdkcComDelSubkeys::MakeResponseOnlyHeadData(dkcres_type_t subcode, dkcres_type_t errcode)
{
	PDKCCOM_ALL	pComErr;
	if(NULL == (pComErr = reinterpret_cast<PDKCCOM_ALL>(malloc(sizeof(DKCRES_DEL_SUBKEYS))))){
		ERR_DKCPRN("Could not allocate memory.");
		return NULL;
	}
	PDKCRES_DEL_SUBKEYS	pComRes	= CVT_DKCRES_DEL_SUBKEYS(pComErr);
	pComRes->head.comtype		= DKC_COM_DEL_SUBKEYS;
	pComRes->head.restype		= COMPOSE_DKC_RES(errcode, subcode);
	pComRes->head.comnumber		= GetComNumber();
	pComRes->head.dispcomnumber	= GetDispComNumber();
	pComRes->head.length		= sizeof(DKCRES_DEL_SUBKEYS);

	return pComErr;
}

bool K2hdkcComDelSubkeys::SetSucceedResponseData(void)
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

bool K2hdkcComDelSubkeys::CommandProcessing(void)
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
		const PDKCCOM_DEL_SUBKEYS	pCom	= CVT_DKCCOM_DEL_SUBKEYS(pRcvComAll);
		const unsigned char*		pKey	= GET_BIN_DKC_COM_ALL(pRcvComAll, pCom->key_offset, pCom->key_length);
		const unsigned char*		pSubkey	= GET_BIN_DKC_COM_ALL(pRcvComAll, pCom->subkey_offset, pCom->subkey_length);
		if(!pKey || !pSubkey){
			ERR_DKCPRN("Could not get safe key(%zd offset, %zu byte) and subkey(%zd offset, %zu byte) in DKCCOM_DEL_SUBKEYS(%p).", pCom->key_offset, pCom->key_length, pCom->subkey_offset, pCom->subkey_length, pRcvComAll);
			SetErrorResponseData(DKC_RES_SUBCODE_INVAL);
			return false;
		}

		// for locking data
		K2HDALock*	pLock = new K2HDALock(pK2hObj, false, pKey, pCom->key_length);

		// get subkeys
		K2HSubKeys*	pSubKeys;
		if(NULL == (pSubKeys = pK2hObj->GetSubKeys(pKey, pCom->key_length, pCom->check_attr))){
			MSG_DKCPRN("Error or No subkeys for key(%s), but continue to deleting subkey.", bin_to_string(pKey, pCom->key_length).c_str());
			SetSucceedResponseData();
			DKC_DELETE(pLock);
			return true;												// [NOTICE] this case is succeed!
		}

		// delete subkey from list
		if(!pSubKeys->erase(pSubkey, pCom->subkey_length)){
			ERR_DKCPRN("Failed to remove subkey(%s) to key(%s) in DKCCOM_DEL_SUBKEYS(%p).", bin_to_string(pSubkey, pCom->subkey_length).c_str(), bin_to_string(pKey, pCom->key_length).c_str(), pRcvComAll);
			SetErrorResponseData(DKC_RES_SUBCODE_DELSKEYLIST);
			DKC_DELETE(pSubKeys);
			DKC_DELETE(pLock);
			return false;
		}

		// serialize subkeys
		unsigned char*	pNewSubkeys			= NULL;
		size_t			new_subkeys_length	= 0;
		if(!pSubKeys->Serialize(&pNewSubkeys, new_subkeys_length)){
			ERR_DKCPRN("Failed to serialize key(%s)'s subkeys in DKCCOM_DEL_SUBKEYS(%p).", bin_to_string(pKey, pCom->key_length).c_str(), pRcvComAll);
			SetErrorResponseData(DKC_RES_SUBCODE_INTERNAL);
			DKC_DELETE(pSubKeys);
			DKC_DELETE(pLock);
			return false;
		}
		DKC_DELETE(pSubKeys);

		// unlock
		DKC_DELETE(pLock);

		// set new subkeys
		if(!pK2hObj->ReplaceSubkeys(pKey, pCom->key_length, pNewSubkeys, new_subkeys_length)){
			ERR_DKCPRN("Failed to remove subkey(%s) to key(%s) in DKCCOM_DEL_SUBKEYS(%p).", bin_to_string(pSubkey, pCom->subkey_length).c_str(), bin_to_string(pKey, pCom->key_length).c_str(), pRcvComAll);
			SetErrorResponseData(DKC_RES_SUBCODE_DELSKEYLIST);
			DKC_FREE(pNewSubkeys);
			return false;
		}
		DKC_FREE(pNewSubkeys);

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

bool K2hdkcComDelSubkeys::CommandSend(const unsigned char* pkey, size_t keylength, const unsigned char* psubkey, size_t subkeylength, bool checkattr)
{
	if(!pkey || 0 == keylength || !psubkey || 0 == subkeylength){
		ERR_DKCPRN("Parameter are wrong.");
		return false;
	}

	// make send data
	PDKCCOM_ALL	pComAll;
	if(NULL == (pComAll = reinterpret_cast<PDKCCOM_ALL>(malloc(sizeof(DKCCOM_DEL_SUBKEYS) + keylength + subkeylength)))){
		ERR_DKCPRN("Could not allocate memory.");
		return false;
	}
	PDKCCOM_DEL_SUBKEYS	pComDelSubkeys	= CVT_DKCCOM_DEL_SUBKEYS(pComAll);
	pComDelSubkeys->head.comtype		= DKC_COM_DEL_SUBKEYS;
	pComDelSubkeys->head.restype		= DKC_NORESTYPE;
	pComDelSubkeys->head.comnumber		= GetComNumber();
	pComDelSubkeys->head.dispcomnumber	= GetDispComNumber();
	pComDelSubkeys->head.length			= sizeof(DKCCOM_DEL_SUBKEYS) + keylength + subkeylength;
	pComDelSubkeys->key_offset			= sizeof(DKCCOM_DEL_SUBKEYS);
	pComDelSubkeys->key_length			= keylength;
	pComDelSubkeys->subkey_offset		= sizeof(DKCCOM_DEL_SUBKEYS) + keylength;
	pComDelSubkeys->subkey_length		= subkeylength;
	pComDelSubkeys->check_attr			= checkattr;

	unsigned char*	pdata				= reinterpret_cast<unsigned char*>(pComDelSubkeys) + pComDelSubkeys->key_offset;
	memcpy(pdata, pkey, keylength);
	pdata								= reinterpret_cast<unsigned char*>(pComDelSubkeys) + pComDelSubkeys->subkey_offset;
	memcpy(pdata, psubkey, subkeylength);

	if(!SetSendData(pComAll, K2hdkcCommand::MakeChmpxHash((reinterpret_cast<unsigned char*>(pComDelSubkeys) + pComDelSubkeys->key_offset), keylength))){
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

bool K2hdkcComDelSubkeys::CommandSend(const unsigned char* pkey, size_t keylength, const unsigned char* psubkey, size_t subkeylength, bool checkattr, dkcres_type_t* prescode)
{
	if(!pChmObj){
		ERR_DKCPRN("Chmpx object is NULL, so could not send command.");
		return false;
	}

	if(!CommandSend(pkey, keylength, psubkey, subkeylength, checkattr)){
		ERR_DKCPRN("Failed to send command.");
		return false;
	}
	return GetResponseData(prescode);
}

bool K2hdkcComDelSubkeys::GetResponseData(dkcres_type_t* prescode) const
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
void K2hdkcComDelSubkeys::RawDumpComAll(const PDKCCOM_ALL pComAll) const
{
	assert(DKC_COM_DEL_SUBKEYS == pComAll->com_head.comtype);

	if(DKC_NORESTYPE == pComAll->com_head.restype){
		PDKCCOM_DEL_SUBKEYS	pCom = CVT_DKCCOM_DEL_SUBKEYS(pComAll);

		RAW_DKCPRN("  DKCCOM_DEL_SUBKEYS = {");
		RAW_DKCPRN("    key_offset      = (%016" PRIx64 ") %s",	pCom->key_offset, bin_to_string(GET_BIN_DKC_COM_ALL(pComAll, pCom->key_offset, pCom->key_length), pCom->key_length).c_str());
		RAW_DKCPRN("    key_length      = %zu",					pCom->key_length);
		RAW_DKCPRN("    subkey_offset   = (%016" PRIx64 ") %s",	pCom->subkey_offset, bin_to_string(GET_BIN_DKC_COM_ALL(pComAll, pCom->subkey_offset, pCom->subkey_length), pCom->subkey_length).c_str());
		RAW_DKCPRN("    subkey_length   = %zu",					pCom->subkey_length);
		RAW_DKCPRN("    check_attr      = %s",					pCom->check_attr ? "yes" : "no");
		RAW_DKCPRN("  }");

	}else{
		//PDKCRES_DEL_SUBKEYS	pCom = CVT_DKCRES_DEL_SUBKEYS(pComAll);
		RAW_DKCPRN("  DKCRES_DEL_SUBKEYS = {");
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
