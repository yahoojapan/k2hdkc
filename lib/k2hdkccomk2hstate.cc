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
 * CREATE:   Tue Sep 6 2016
 * REVISION:
 * 
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "k2hdkccomk2hstate.h"
#include "k2hdkcutil.h"
#include "k2hdkcdbg.h"

using namespace	std;

//---------------------------------------------------------
// Constructor/Destructor
//---------------------------------------------------------
K2hdkcComK2hState::K2hdkcComK2hState(K2HShm* pk2hash, ChmCntrl* pchmcntrl, uint64_t comnum, bool without_self, bool is_routing_on_server, bool is_wait_on_server) : K2hdkcCommand(pk2hash, pchmcntrl, comnum, without_self, is_routing_on_server, is_wait_on_server, DKC_COM_K2HSTATE)
{
	strClassName = "K2hdkcComK2hState";

	// [NOTE]
	// This command must not use routing mode to send command.
	//
	UnsetRoutingOnSlave();
}

K2hdkcComK2hState::~K2hdkcComK2hState(void)
{
}

//---------------------------------------------------------
// Methods
//---------------------------------------------------------
PDKCCOM_ALL K2hdkcComK2hState::MakeResponseOnlyHeadData(dkcres_type_t subcode, dkcres_type_t errcode)
{
	PDKCCOM_ALL	pComErr;
	if(NULL == (pComErr = reinterpret_cast<PDKCCOM_ALL>(calloc(1, sizeof(DKCRES_K2HSTATE))))){
		ERR_DKCPRN("Could not allocate memory.");
		return NULL;
	}
	PDKCRES_K2HSTATE	pComRes		= CVT_DKCRES_K2HSTATE(pComErr);
	pComRes->head.comtype			= DKC_COM_K2HSTATE;
	pComRes->head.restype			= COMPOSE_DKC_RES(errcode, subcode);
	pComRes->head.comnumber			= GetComNumber();
	pComRes->head.dispcomnumber		= GetDispComNumber();
	pComRes->head.length			= sizeof(DKCRES_K2HSTATE);
	pComRes->nodestate.chmpxid		= CHM_INVALID_CHMPXID;
	pComRes->nodestate.base_hash	= CHM_INVALID_HASHVAL;
	pComRes->nodestate.pending_hash	= CHM_INVALID_HASHVAL;

	return pComErr;
}

bool K2hdkcComK2hState::SetResponseData(const PCHMPX pSelfInfo, const PK2HSTATE pState)
{
	if(!pSelfInfo || !pState){
		ERR_DKCPRN("Parameter are wrong.");
		SetErrorResponseData(DKC_RES_SUBCODE_INTERNAL);
		return false;
	}

	PDKCCOM_ALL	pComAll;
	if(NULL == (pComAll = reinterpret_cast<PDKCCOM_ALL>(calloc(1, sizeof(DKCRES_K2HSTATE))))){
		ERR_DKCPRN("Could not allocate memory.");
		SetErrorResponseData(DKC_RES_SUBCODE_NOMEM);		// could this set?
		return false;
	}
	PDKCRES_K2HSTATE	pComRes		= CVT_DKCRES_K2HSTATE(pComAll);
	pComRes->head.comtype			= DKC_COM_K2HSTATE;
	pComRes->head.restype			= COMPOSE_DKC_RES(DKC_RES_SUCCESS, DKC_RES_SUBCODE_NOTHING);
	pComRes->head.comnumber			= GetComNumber();
	pComRes->head.dispcomnumber		= GetDispComNumber();
	pComRes->head.length			= sizeof(DKCRES_K2HSTATE);
	pComRes->nodestate.chmpxid		= pSelfInfo->chmpxid;
	pComRes->nodestate.base_hash	= pSelfInfo->base_hash;
	pComRes->nodestate.pending_hash	= pSelfInfo->pending_hash;

	memcpy(pComRes->nodestate.name,			pSelfInfo->name,NI_MAXHOST);
	memcpy(&(pComRes->nodestate.k2hstate),	pState,			sizeof(K2HSTATE));

	if(!SetResponseData(pComAll)){
		ERR_DKCPRN("Failed to set send data.");
		DKC_FREE(pComAll);
		SetErrorResponseData(DKC_RES_SUBCODE_INTERNAL);
		return false;
	}
	return true;
}

bool K2hdkcComK2hState::CommandProcessing(void)
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

		// get self chmpx information
		PCHMPX	pSelfInfo = pChmObj->DupSelfChmpxInfo();
		if(!pSelfInfo){
			ERR_DKCPRN("Could not get self chmpx information.");
			SetErrorResponseData(DKC_RES_SUBCODE_GETSLEFSVRCHMPX);
			return false;
		}

		// get command data
		const PDKCCOM_K2HSTATE	pCom	= CVT_DKCCOM_K2HSTATE(pRcvComAll);

		// check chmpxid and hash value
		if(pCom->chmpxid != pSelfInfo->chmpxid){
			ERR_DKCPRN("Could not get self chmpx information.");
			SetErrorResponseData(DKC_RES_SUBCODE_NOTSAMECHMPXID);
			ChmCntrl::FreeDupSelfChmpxInfo(pSelfInfo);
			return false;
		}
		if(pCom->base_hash != pSelfInfo->base_hash){
			WAN_DKCPRN("Diffrent base hash of request(%016" PRIx64 ") and self(%016" PRIx64 "), but continue...", pCom->base_hash, pSelfInfo->base_hash);
		}

		// do command
		PK2HSTATE	pState = pK2hObj->GetState();
		if(!pState){
			ERR_DKCPRN("Could not get k2hash state.");
			SetErrorResponseData(DKC_RES_SUBCODE_GETK2HSTATE);
			ChmCntrl::FreeDupSelfChmpxInfo(pSelfInfo);
			return false;
		}

		// set response data
		if(!SetResponseData(pSelfInfo, pState)){
			MSG_DKCPRN("Failed to make response data for getting k2hash state.");
			// continue for responsing
		}

		// free
		ChmCntrl::FreeDupSelfChmpxInfo(pSelfInfo);
		DKC_FREE(pState);

	}else{
		//
		// receive data is response
		//
		// nothing to do
		MSG_DKCPRN("Received data is response data, so nothing to do here.");
	}
	return true;
}

bool K2hdkcComK2hState::CommandSend(chmpxid_t chmpxid, chmhash_t base_hash)
{
	if(IsServerNode){
		ERR_DKCPRN("This get server node k2hash state command must be called from only slave node.");
		return false;
	}
	if(CHM_INVALID_CHMPXID == chmpxid || CHM_INVALID_HASHVAL == base_hash){
		ERR_DKCPRN("Invalid chmpxid(%016" PRIx64 ") or base hash value(%016" PRIx64 ").", chmpxid, base_hash);
		return false;
	}

	// make send data
	PDKCCOM_ALL	pComAll;
	if(NULL == (pComAll = reinterpret_cast<PDKCCOM_ALL>(malloc(sizeof(DKCCOM_K2HSTATE))))){
		ERR_DKCPRN("Could not allocate memory.");
		return false;
	}
	PDKCCOM_K2HSTATE	pComK2hState	= CVT_DKCCOM_K2HSTATE(pComAll);
	pComK2hState->head.comtype			= DKC_COM_K2HSTATE;
	pComK2hState->head.restype			= DKC_NORESTYPE;
	pComK2hState->head.comnumber		= GetComNumber();
	pComK2hState->head.dispcomnumber	= GetDispComNumber();
	pComK2hState->head.length			= sizeof(DKCCOM_K2HSTATE);
	pComK2hState->chmpxid				= chmpxid;
	pComK2hState->base_hash				= base_hash;

	if(!SetSendData(pComAll, base_hash)){
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

bool K2hdkcComK2hState::CommandSend(chmpxid_t chmpxid, chmhash_t base_hash, chmpxid_t* pchmpxid, const char** ppname, chmhash_t* pbase_hash, chmhash_t* ppending_hash, const K2HSTATE** ppState, dkcres_type_t* prescode)
{
	if(!pChmObj){
		ERR_DKCPRN("Chmpx object is NULL, so could not send command.");
		return false;
	}

	if(!CommandSend(chmpxid, base_hash)){
		ERR_DKCPRN("Failed to send command.");
		return false;
	}
	return GetResponseData(pchmpxid, ppname, pbase_hash, ppending_hash, ppState, prescode);
}

bool K2hdkcComK2hState::GetResponseData(chmpxid_t* pchmpxid, const char** ppname, chmhash_t* pbase_hash, chmhash_t* ppending_hash, const K2HSTATE** ppState, dkcres_type_t* prescode) const
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
	PDKCRES_K2HSTATE	pResK2hState = CVT_DKCRES_K2HSTATE(pcomall);
	if(DKC_COM_K2HSTATE != pResK2hState->head.comtype || DKC_NORESTYPE == pResK2hState->head.restype || RcvComLength != pResK2hState->head.length){
		ERR_DKCPRN("Response(received) data is somthing wrong(internal error: data is invalid).");
		return false;
	}
	if(pchmpxid){
		*pchmpxid = pResK2hState->nodestate.chmpxid;
	}
	if(ppname){
		*ppname = &(pResK2hState->nodestate.name[0]);
	}
	if(pbase_hash){
		*pbase_hash = pResK2hState->nodestate.base_hash;
	}
	if(ppending_hash){
		*ppending_hash = pResK2hState->nodestate.pending_hash;
	}
	if(ppState){
		*ppState = &(pResK2hState->nodestate.k2hstate);
	}
	return true;
}

//---------------------------------------------------------
// Methods - Dump
//---------------------------------------------------------
void K2hdkcComK2hState::RawDumpComAll(const PDKCCOM_ALL pComAll) const
{
	assert(DKC_COM_K2HSTATE == pComAll->com_head.comtype);

	if(DKC_NORESTYPE == pComAll->com_head.restype){
		PDKCCOM_K2HSTATE	pCom = CVT_DKCCOM_K2HSTATE(pComAll);

		RAW_DKCPRN("  DKCCOM_K2HSTATE = {");
		RAW_DKCPRN("    chmpxid         = %016" PRIx64 ,	pCom->chmpxid);
		RAW_DKCPRN("    base_hash       = %016" PRIx64 ,	pCom->base_hash);
		RAW_DKCPRN("  }");

	}else{
		PDKCRES_K2HSTATE	pCom = CVT_DKCRES_K2HSTATE(pComAll);

		RAW_DKCPRN("  DKCRES_K2HSTATE = {");
		RAW_DKCPRN("    chmpxid                     = %016" PRIx64 ,	pCom->nodestate.chmpxid);
		RAW_DKCPRN("    name                        = %s",				pCom->nodestate.name);
		RAW_DKCPRN("    base_hash                   = %016" PRIx64 ,	pCom->nodestate.base_hash);
		RAW_DKCPRN("    pending_hash                = %016" PRIx64 ,	pCom->nodestate.pending_hash);
		RAW_DKCPRN("    K2HSTATE = {");
		RAW_DKCPRN("      version                   = %s",				pCom->nodestate.k2hstate.version);
		RAW_DKCPRN("      hash_version              = %s",				pCom->nodestate.k2hstate.hash_version);
		RAW_DKCPRN("      trans_version             = %s",				pCom->nodestate.k2hstate.trans_version);
		RAW_DKCPRN("      trans_pool_count          = %d",				pCom->nodestate.k2hstate.trans_pool_count);
		RAW_DKCPRN("      max_mask                  = %016" PRIx64 ,	pCom->nodestate.k2hstate.max_mask);
		RAW_DKCPRN("      min_mask                  = %016" PRIx64 ,	pCom->nodestate.k2hstate.min_mask);
		RAW_DKCPRN("      cur_mask                  = %016" PRIx64 ,	pCom->nodestate.k2hstate.cur_mask);
		RAW_DKCPRN("      collision_mask            = %016" PRIx64 ,	pCom->nodestate.k2hstate.collision_mask);
		RAW_DKCPRN("      max_element_count         = %lu",				pCom->nodestate.k2hstate.max_element_count);
		RAW_DKCPRN("      total_size                = %zu",				pCom->nodestate.k2hstate.total_size);
		RAW_DKCPRN("      page_size                 = %zu",				pCom->nodestate.k2hstate.page_size);
		RAW_DKCPRN("      file_size                 = %zu",				pCom->nodestate.k2hstate.file_size);
		RAW_DKCPRN("      total_used_size           = %zu",				pCom->nodestate.k2hstate.total_used_size);
		RAW_DKCPRN("      total_map_size            = %zu",				pCom->nodestate.k2hstate.total_map_size);
		RAW_DKCPRN("      total_element_size        = %zu",				pCom->nodestate.k2hstate.total_element_size);
		RAW_DKCPRN("      total_page_size           = %zu",				pCom->nodestate.k2hstate.total_page_size);
		RAW_DKCPRN("      total_area_count          = %ld",				pCom->nodestate.k2hstate.total_area_count);
		RAW_DKCPRN("      total_element_count       = %ld",				pCom->nodestate.k2hstate.total_element_count);
		RAW_DKCPRN("      total_page_count          = %ld",				pCom->nodestate.k2hstate.total_page_count);
		RAW_DKCPRN("      assigned_area_count       = %ld",				pCom->nodestate.k2hstate.assigned_area_count);
		RAW_DKCPRN("      assigned_key_count        = %ld",				pCom->nodestate.k2hstate.assigned_key_count);
		RAW_DKCPRN("      assigned_ckey_count       = %ld",				pCom->nodestate.k2hstate.assigned_ckey_count);
		RAW_DKCPRN("      assigned_element_count    = %ld",				pCom->nodestate.k2hstate.assigned_element_count);
		RAW_DKCPRN("      assigned_page_count       = %ld",				pCom->nodestate.k2hstate.assigned_page_count);
		RAW_DKCPRN("      unassigned_element_count  = %ld",				pCom->nodestate.k2hstate.unassigned_element_count);
		RAW_DKCPRN("      unassigned_page_count     = %ld",				pCom->nodestate.k2hstate.unassigned_page_count);
		RAW_DKCPRN("      last_update               = %jds %jdusec",	pCom->nodestate.k2hstate.last_update.tv_sec, pCom->nodestate.k2hstate.last_update.tv_usec);
		RAW_DKCPRN("      last_area_update          = %jds %jdusec",	pCom->nodestate.k2hstate.last_area_update.tv_sec, pCom->nodestate.k2hstate.last_area_update.tv_usec);
		RAW_DKCPRN("    }");
		RAW_DKCPRN("  }");
	}
}

/*
 * VIM modelines
 *
 * vim:set ts=4 fenc=utf-8:
 */
