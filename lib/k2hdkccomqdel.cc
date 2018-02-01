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

#include "k2hdkccomqdel.h"
#include "k2hdkccomqpop.h"
#include "k2hdkccomutil.h"
#include "k2hdkcutil.h"
#include "k2hdkcdbg.h"

using namespace	std;

//---------------------------------------------------------
// Constructor/Destructor
//---------------------------------------------------------
K2hdkcComQDel::K2hdkcComQDel(K2HShm* pk2hash, ChmCntrl* pchmcntrl, uint64_t comnum, bool without_self, bool is_routing_on_server, bool is_wait_on_server) : K2hdkcCommand(pk2hash, pchmcntrl, comnum, without_self, is_routing_on_server, is_wait_on_server, DKC_COM_QDEL)
{
	strClassName = "K2hdkcComQDel";
}

K2hdkcComQDel::~K2hdkcComQDel(void)
{
}

//---------------------------------------------------------
// Methods
//---------------------------------------------------------
PDKCCOM_ALL K2hdkcComQDel::MakeResponseOnlyHeadData(dkcres_type_t subcode, dkcres_type_t errcode)
{
	PDKCCOM_ALL	pCom;
	if(NULL == (pCom = reinterpret_cast<PDKCCOM_ALL>(malloc(sizeof(DKCRES_QDEL))))){
		ERR_DKCPRN("Could not allocate memory.");
		return NULL;
	}
	PDKCRES_QDEL	pComRes		= CVT_DKCRES_QDEL(pCom);
	pComRes->head.comtype		= DKC_COM_QDEL;
	pComRes->head.restype		= COMPOSE_DKC_RES(errcode, subcode);
	pComRes->head.comnumber		= GetComNumber();
	pComRes->head.dispcomnumber	= GetDispComNumber();
	pComRes->head.length		= sizeof(DKCRES_QDEL);

	return pCom;
}

bool K2hdkcComQDel::SetSucceedResponseData(void)
{
	return SetErrorResponseData(DKC_RES_SUBCODE_NOTHING, DKC_RES_SUCCESS);
}

bool K2hdkcComQDel::SetErrorResponseData(dkcres_type_t subcode, dkcres_type_t errcode)
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

bool K2hdkcComQDel::CommandProcessing(void)
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

bool K2hdkcComQDel::CommandSend(const unsigned char* pprefix, size_t prefixlength, int count, bool is_keyqueue, bool is_fifo, bool checkattr, const char* encpass)
{
	if(count <= 0){
		ERR_DKCPRN("Parameter is wrong.");
		return false;
	}

	// do command
	// 
	// [NOTICE]
	// This command runs loop with calling pop command until count.
	//
	for(int cnt = 0; cnt < count; ++cnt){
		// get key
		K2hdkcComQPop*			pComQPop	= GetCommonK2hdkcComQPop(pK2hObj, pChmObj, SendMsgid, GetDispComNumber(), IsSendWithoutSelf(), true, false);
		dkcres_type_t			rescode		= DKC_INITRESTYPE;
		const unsigned char*	pGetVal		= NULL;
		size_t					GetValLength= 0;
		if(is_keyqueue){
			const unsigned char*	pGetKey		= NULL;
			size_t					GetKeyLength= 0;
			if(!pComQPop->KeyQueueCommandSend(pprefix, prefixlength, is_fifo, checkattr, encpass, &pGetKey, &GetKeyLength, &pGetVal, &GetValLength, &rescode)){
				ERR_DKCPRN("Failed to pop queue by merker prefix(%s) in DKCCOM_QDEL(%p).", bin_to_string(pprefix, prefixlength).c_str(), pRcvComAll);
				DKC_DELETE(pComQPop);
				SetErrorResponseData(DKC_RES_SUBCODE_POPQUQUE);
				return false;
			}
		}else{
			if(!pComQPop->QueueCommandSend(pprefix, prefixlength, is_fifo, checkattr, encpass, &pGetVal, &GetValLength, &rescode)){
				ERR_DKCPRN("Failed to pop queue by merker prefix(%s) in DKCCOM_QDEL(%p).", bin_to_string(pprefix, prefixlength).c_str(), pRcvComAll);
				DKC_DELETE(pComQPop);
				SetErrorResponseData(DKC_RES_SUBCODE_POPQUQUE);
				return false;
			}
		}
		if(IS_DKC_RES_NOTSUCCESS(rescode)){
			ERR_DKCPRN("Failed to pop queue by marker prefix(%s) in DKCCOM_QDEL(%p) by subcude(%s).", bin_to_string(pprefix, prefixlength).c_str(), pRcvComAll, STR_DKCRES_SUBCODE_TYPE(rescode));
			DKC_DELETE(pComQPop);
			SetErrorResponseData(GET_DKC_RES_SUBCODE(rescode));
			return false;

		}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
			MSG_DKCPRN("Get result subcode(%s) for popping queue by marker prefix(%s) in DKCCOM_QDEL(%p).", STR_DKCRES_SUBCODE_TYPE(rescode), bin_to_string(pprefix, prefixlength).c_str(), pRcvComAll);
		}
		DKC_DELETE(pComQPop);
	}
	// set dummy response
	SetSucceedResponseData();

	return true;
}

bool K2hdkcComQDel::CommandSend(const unsigned char* pprefix, size_t prefixlength, int count, bool is_keyqueue, bool is_fifo, bool checkattr, const char* encpass, dkcres_type_t* prescode)
{
	if(!pChmObj){
		ERR_DKCPRN("Chmpx object is NULL, so could not send command.");
		return false;
	}

	if(!CommandSend(pprefix, prefixlength, count, is_keyqueue, is_fifo, checkattr, encpass)){
		ERR_DKCPRN("Failed to send command.");
		return false;
	}
	return GetResponseData(prescode);
}

bool K2hdkcComQDel::GetResponseData(dkcres_type_t* prescode) const
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
void K2hdkcComQDel::RawDumpComAll(const PDKCCOM_ALL pComAll) const
{
	assert(DKC_COM_QDEL == pComAll->com_head.comtype);

	// no command data for this class
	return;
}

/*
 * VIM modelines
 *
 * vim:set ts=4 fenc=utf-8:
 */
