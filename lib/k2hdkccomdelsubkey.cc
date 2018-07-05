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

#include "k2hdkccomdelsubkey.h"
#include "k2hdkccomdel.h"
#include "k2hdkccomdelsubkeys.h"
#include "k2hdkccomutil.h"
#include "k2hdkcutil.h"
#include "k2hdkcdbg.h"

using namespace	std;

//---------------------------------------------------------
// Constructor/Destructor
//---------------------------------------------------------
K2hdkcComDelSubkey::K2hdkcComDelSubkey(K2HShm* pk2hash, ChmCntrl* pchmcntrl, uint64_t comnum, bool without_self, bool is_routing_on_server, bool is_wait_on_server) : K2hdkcCommand(pk2hash, pchmcntrl, comnum, without_self, is_routing_on_server, is_wait_on_server, DKC_COM_DEL_SUBKEY)
{
	strClassName = "K2hdkcComDelSubkey";
}

K2hdkcComDelSubkey::~K2hdkcComDelSubkey(void)
{
}

//---------------------------------------------------------
// Methods
//---------------------------------------------------------
PDKCCOM_ALL K2hdkcComDelSubkey::MakeResponseOnlyHeadData(dkcres_type_t subcode, dkcres_type_t errcode)
{
	PDKCCOM_ALL	pCom;
	if(NULL == (pCom = reinterpret_cast<PDKCCOM_ALL>(malloc(sizeof(DKCRES_DEL_SUBKEY))))){
		ERR_DKCPRN("Could not allocate memory.");
		return NULL;
	}
	PDKCRES_DEL_SUBKEY	pComRes	= CVT_DKCRES_DEL_SUBKEY(pCom);
	pComRes->head.comtype		= DKC_COM_DEL_SUBKEY;
	pComRes->head.restype		= COMPOSE_DKC_RES(errcode, subcode);
	pComRes->head.comnumber		= GetComNumber();
	pComRes->head.dispcomnumber	= GetDispComNumber();
	pComRes->head.length		= sizeof(DKCRES_DEL_SUBKEY);

	return pCom;
}

bool K2hdkcComDelSubkey::SetSucceedResponseData(void)
{
	return SetErrorResponseData(DKC_RES_SUBCODE_NOTHING, DKC_RES_SUCCESS);
}

bool K2hdkcComDelSubkey::SetErrorResponseData(dkcres_type_t subcode, dkcres_type_t errcode)
{
	PDKCCOM_ALL	pComAll = MakeResponseOnlyHeadData(subcode, errcode);

	// [NOTE]
	// This command is composed of a plurality of other commands.
	// So the response data is set manually here, take care about pRcvComPkt is NULL.
	//
	DKC_FREE(pRcvComAll);
	DKC_FREE(pRcvComPkt);
	pRcvComAll	= pComAll;
	RcvComLength= pComAll->com_head.length;

	// if has slave command object, set response code into it.
	if(pSlaveObj){
		if(!pSlaveObj->SetResponseCode(pRcvComAll->com_head.restype)){
			ERR_DKCPRN("Failed to set response code(%s - %s(0x%016" PRIx64 ")) to slave command object, but coontinue...", STR_DKCRES_RESULT_TYPE(pRcvComAll->com_head.restype), STR_DKCRES_SUBCODE_TYPE(pRcvComAll->com_head.restype), pRcvComAll->com_head.restype);
		}
	}

	// dump
	DumpComAll("Receive(created as the dummy)", pRcvComAll);

	return true;
}

bool K2hdkcComDelSubkey::CommandProcessing(void)
{
	if(!pChmObj || !pRcvComPkt || !pRcvComAll || 0 == RcvComLength){
		ERR_DKCPRN("This object does not initialize yet.");
		SetErrorResponseData(DKC_RES_SUBCODE_INTERNAL);
		return false;
	}

	// [NOTICE]
	// This command is a collection of other commands.
	// So this command does not come here!
	//
	if(DKC_NORESTYPE == pRcvComAll->com_head.restype){
		//
		// receive data is command
		//
		if(!pK2hObj || !IsServerNode){
			ERR_DKCPRN("This object does not initialize yet.");
			SetErrorResponseData(DKC_RES_SUBCODE_INTERNAL);
			return false;
		}

		// nothing to do
		WAN_DKCPRN("Received data is command data, so nothing to do here.");

	}else{
		//
		// receive data is response
		//
		// nothing to do
		MSG_DKCPRN("Received data is response data, so nothing to do here.");
	}
	return true;
}

bool K2hdkcComDelSubkey::CommandSend(const unsigned char* pkey, size_t keylength, const unsigned char* psubkey, size_t subkeylength, bool is_subkeys, bool checkattr)
{
	if(!pkey || 0 == keylength || !psubkey || 0 == subkeylength){
		ERR_DKCPRN("Parameter are wrong.");
		return false;
	}

	// do command
	// 
	// [NOTICE]
	// This command is a collection of other 2 commands which are removing subkey and
	// removing subkey from key's subkeys list commands.
	// The if some operation is occuured to key and subkey by other process during
	// from modifying(creating) subkey to adding subkey name to key's subkey list,
	// maybe there is a possibility that the inconvenience will occur.
	//

	// [1] remove subkey
	K2hdkcComDel*	pComDel	= GetCommonK2hdkcComDel(pK2hObj, pChmObj, SendMsgid, GetDispComNumber(), IsSendWithoutSelf(), true, false);
	dkcres_type_t	rescode	= DKC_INITRESTYPE;
	if(!pComDel->CommandSend(psubkey, subkeylength, is_subkeys, checkattr, &rescode)){
		ERR_DKCPRN("Failed to remove subkey(%s) in key(%s) in DKC_COM_DEL_SUBKEYS(%p), but continue...", bin_to_string(psubkey, subkeylength).c_str(), bin_to_string(pkey, keylength).c_str(), pRcvComAll);
	}
	DKC_DELETE(pComDel);

	// [2] remove subkey name from key's subkeys list
	K2hdkcComDelSubkeys*	pComDelSubkeys	= GetCommonK2hdkcComDelSubkeys(pK2hObj, pChmObj, SendMsgid, GetDispComNumber(), IsSendWithoutSelf(), true, false);
	rescode									= DKC_INITRESTYPE;
	if(!pComDelSubkeys->CommandSend(pkey, keylength, psubkey, subkeylength, checkattr, &rescode)){
		ERR_DKCPRN("Failed to remove subkey(%s) from key(%s) subkeys list in DKC_COM_DEL_SUBKEYS(%p)", bin_to_string(psubkey, subkeylength).c_str(), bin_to_string(pkey, keylength).c_str(), pRcvComAll);
		DKC_DELETE(pComDelSubkeys);
		SetErrorResponseData(DKC_RES_SUBCODE_DELSKEYLIST);
		return false;
	}
	DKC_DELETE(pComDelSubkeys);

	if(IS_DKC_RES_NOTSUCCESS(rescode)){
		ERR_DKCPRN("Failed to remove subkey(%s) from key(%s) subkeys list in DKC_COM_DEL_SUBKEYS(%p) by subcude(%s)", bin_to_string(psubkey, subkeylength).c_str(), bin_to_string(pkey, keylength).c_str(), pRcvComAll, STR_DKCRES_SUBCODE_TYPE(rescode));
		SetErrorResponseData(GET_DKC_RES_SUBCODE(rescode));
		return false;
	}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
		MSG_DKCPRN("Get result subcode(%s) for removing subkey(%s) from key(%s) subkeys list in DKC_COM_DEL_SUBKEYS(%p)", STR_DKCRES_SUBCODE_TYPE(rescode), bin_to_string(psubkey, subkeylength).c_str(), bin_to_string(pkey, keylength).c_str(), pRcvComAll);
	}

	// set dummy response
	SetSucceedResponseData();

	return true;
}

bool K2hdkcComDelSubkey::CommandSend(const unsigned char* pkey, size_t keylength, const unsigned char* psubkey, size_t subkeylength, bool is_subkeys, bool checkattr, dkcres_type_t* prescode)
{
	if(!pChmObj){
		ERR_DKCPRN("Chmpx object is NULL, so could not send command.");
		return false;
	}

	if(!CommandSend(pkey, keylength, psubkey, subkeylength, is_subkeys, checkattr)){
		ERR_DKCPRN("Failed to send command.");
		return false;
	}
	return GetResponseData(prescode);
}

bool K2hdkcComDelSubkey::GetResponseData(dkcres_type_t* prescode) const
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
void K2hdkcComDelSubkey::RawDumpComAll(const PDKCCOM_ALL pComAll) const
{
	assert(DKC_COM_DEL_SUBKEY == pComAll->com_head.comtype);

	// no command data for this class
	return;
}

/*
 * VIM modelines
 *
 * vim:set ts=4 fenc=utf-8:
 */
