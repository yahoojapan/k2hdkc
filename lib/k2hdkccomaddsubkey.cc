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

#include "k2hdkccomaddsubkey.h"
#include "k2hdkccomset.h"
#include "k2hdkccomaddsubkeys.h"
#include "k2hdkccomutil.h"
#include "k2hdkcutil.h"
#include "k2hdkcdbg.h"

using namespace	std;

//---------------------------------------------------------
// Constructor/Destructor
//---------------------------------------------------------
K2hdkcComAddSubkey::K2hdkcComAddSubkey(K2HShm* pk2hash, ChmCntrl* pchmcntrl, uint64_t comnum, bool without_self, bool is_routing_on_server, bool is_wait_on_server) : K2hdkcCommand(pk2hash, pchmcntrl, comnum, without_self, is_routing_on_server, is_wait_on_server, DKC_COM_ADD_SUBKEY)
{
	strClassName = "K2hdkcComAddSubkey";
}

K2hdkcComAddSubkey::~K2hdkcComAddSubkey(void)
{
}

//---------------------------------------------------------
// Methods
//---------------------------------------------------------
PDKCCOM_ALL K2hdkcComAddSubkey::MakeResponseOnlyHeadData(dkcres_type_t subcode, dkcres_type_t errcode)
{
	PDKCCOM_ALL	pCom;
	if(NULL == (pCom = reinterpret_cast<PDKCCOM_ALL>(malloc(sizeof(DKCRES_ADD_SUBKEY))))){
		ERR_DKCPRN("Could not allocate memory.");
		return NULL;
	}
	PDKCRES_ADD_SUBKEY	pComRes	= CVT_DKCRES_ADD_SUBKEY(pCom);
	pComRes->head.comtype		= DKC_COM_ADD_SUBKEY;
	pComRes->head.restype		= COMPOSE_DKC_RES(errcode, subcode);
	pComRes->head.comnumber		= GetComNumber();
	pComRes->head.dispcomnumber	= GetDispComNumber();
	pComRes->head.length		= sizeof(DKCRES_ADD_SUBKEY);

	return pCom;
}

bool K2hdkcComAddSubkey::SetSucceedResponseData(void)
{
	return SetErrorResponseData(DKC_RES_SUBCODE_NOTHING, DKC_RES_SUCCESS);
}

bool K2hdkcComAddSubkey::SetErrorResponseData(dkcres_type_t subcode, dkcres_type_t errcode)
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
			ERR_DKCPRN("Failed to set response code(%s - %s(0x%016" PRIx64 ")) to slave command object, but continue...", STR_DKCRES_RESULT_TYPE(pRcvComAll->com_head.restype), STR_DKCRES_SUBCODE_TYPE(pRcvComAll->com_head.restype), pRcvComAll->com_head.restype);
		}
	}

	// dump
	DumpComAll("Receive(created as the dummy)", pRcvComAll);

	return true;
}

bool K2hdkcComAddSubkey::CommandProcessing(void)
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

bool K2hdkcComAddSubkey::CommandSend(const unsigned char* pkey, size_t keylength, const unsigned char* psubkey, size_t subkeylength, const unsigned char* pskeyval, size_t skeyvallength, bool checkattr, const char* encpass, const time_t* expire)
{
	if(!pkey || 0 == keylength || !psubkey || 0 == subkeylength){
		ERR_DKCPRN("Parameter are wrong.");
		return false;
	}

	// do command
	// 
	// [NOTICE]
	// This command is a collection of other 2 commands which are setting subkey and
	// adding subkey commands.
	// The if some operation is occurred to key and subkey by other process during
	// from modifying(creating) subkey to adding subkey name to key's subkey list,
	// maybe there is a possibility that the inconvenience will occur.
	//
	// [1] make subkey by another command
	K2hdkcComSet*	pComSetObj	= GetCommonK2hdkcComSet(pK2hObj, pChmObj, SendMsgid, GetDispComNumber(), IsSendWithoutSelf(), true, false);
	dkcres_type_t	rescode		= DKC_INITRESTYPE;
	if(!pComSetObj->CommandSend(psubkey, subkeylength, pskeyval, skeyvallength, false, encpass, expire, &rescode)){					// [NOTE] keep subkeys list
		ERR_DKCPRN("Failed to set subkey(%s) and subkey val(%s) in DKCCOM_ADD_SUBKEY(%p).", bin_to_string(psubkey, subkeylength).c_str(), bin_to_string(pskeyval, skeyvallength).c_str(), pRcvComAll);
		DKC_DELETE(pComSetObj);
		SetErrorResponseData(DKC_RES_SUBCODE_SETVAL);
		return false;
	}
	DKC_DELETE(pComSetObj);

	if(IS_DKC_RES_NOTSUCCESS(rescode)){
		ERR_DKCPRN("Failed to set subkey(%s) and subkey val(%s) in DKCCOM_ADD_SUBKEY(%p) by subcode(%s).", bin_to_string(psubkey, subkeylength).c_str(), bin_to_string(pskeyval, skeyvallength).c_str(), pRcvComAll, STR_DKCRES_SUBCODE_TYPE(rescode));
		SetErrorResponseData(GET_DKC_RES_SUBCODE(rescode));
		return false;
	}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
		MSG_DKCPRN("Get result subcode(%s) for setting subkey(%s) and subkey val(%s) in DKCCOM_ADD_SUBKEY(%p).", STR_DKCRES_SUBCODE_TYPE(rescode), bin_to_string(psubkey, subkeylength).c_str(), bin_to_string(pskeyval, skeyvallength).c_str(), pRcvComAll);
	}

	// [2] add subkey into key's subkeys list
	K2hdkcComAddSubkeys*	pComAddSubkeys	= GetCommonK2hdkcComAddSubkeys(pK2hObj, pChmObj, SendMsgid, GetDispComNumber(), IsSendWithoutSelf(), true, false);
	rescode									= DKC_INITRESTYPE;
	if(!pComAddSubkeys->CommandSend(pkey, keylength, psubkey, subkeylength, checkattr, &rescode)){
		ERR_DKCPRN("Failed to add subkey(%s) to key(%s) in DKCCOM_ADD_SUBKEY(%p).", bin_to_string(psubkey, subkeylength).c_str(), bin_to_string(pkey, keylength).c_str(), pRcvComAll);
		DKC_DELETE(pComAddSubkeys);
		SetErrorResponseData(DKC_RES_SUBCODE_ADDSKEYLIST);
		return false;
	}
	DKC_DELETE(pComAddSubkeys);

	if(IS_DKC_RES_NOTSUCCESS(rescode)){
		ERR_DKCPRN("Failed to add subkey(%s) to key(%s) in DKCCOM_ADD_SUBKEY(%p) by subcode(%s).", bin_to_string(psubkey, subkeylength).c_str(), bin_to_string(pkey, keylength).c_str(), pRcvComAll, STR_DKCRES_SUBCODE_TYPE(rescode));
		SetErrorResponseData(GET_DKC_RES_SUBCODE(rescode));
		return false;
	}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
		MSG_DKCPRN("Get result subcode(%s) for adding subkey(%s) to key(%s) in DKCCOM_ADD_SUBKEY(%p).", STR_DKCRES_SUBCODE_TYPE(rescode), bin_to_string(psubkey, subkeylength).c_str(), bin_to_string(pkey, keylength).c_str(), pRcvComAll);
	}

	// set dummy response
	SetSucceedResponseData();

	return true;
}

bool K2hdkcComAddSubkey::CommandSend(const unsigned char* pkey, size_t keylength, const unsigned char* psubkey, size_t subkeylength, const unsigned char* pskeyval, size_t skeyvallength, bool checkattr, const char* encpass, const time_t* expire, dkcres_type_t* prescode)
{
	if(!pChmObj){
		ERR_DKCPRN("Chmpx object is NULL, so could not send command.");
		return false;
	}

	if(!CommandSend(pkey, keylength, psubkey, subkeylength, pskeyval, skeyvallength, checkattr, encpass, expire)){
		ERR_DKCPRN("Failed to send command.");
		return false;
	}
	return GetResponseData(prescode);
}

bool K2hdkcComAddSubkey::GetResponseData(dkcres_type_t* prescode) const
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
void K2hdkcComAddSubkey::RawDumpComAll(const PDKCCOM_ALL pComAll) const
{
	assert(DKC_COM_ADD_SUBKEY == pComAll->com_head.comtype);

	// no command data for this class
	return;
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noexpandtab sw=4 ts=4 fdm=marker
 * vim<600: noexpandtab sw=4 ts=4
 */
