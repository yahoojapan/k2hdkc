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
 * CREATE:   Tue Sep 6 2016
 * REVISION:
 * 
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "k2hdkccomstate.h"
#include "k2hdkccomk2hstate.h"
#include "k2hdkccomutil.h"
#include "k2hdkcutil.h"
#include "k2hdkcdbg.h"

using namespace	std;

//---------------------------------------------------------
// Constructor/Destructor
//---------------------------------------------------------
K2hdkcComState::K2hdkcComState(K2HShm* pk2hash, ChmCntrl* pchmcntrl, uint64_t comnum, bool without_self, bool is_routing_on_server, bool is_wait_on_server) : K2hdkcCommand(pk2hash, pchmcntrl, comnum, without_self, is_routing_on_server, is_wait_on_server, DKC_COM_STATE)
{
	strClassName = "K2hdkcComState";
}

K2hdkcComState::~K2hdkcComState(void)
{
}

//---------------------------------------------------------
// Methods
//---------------------------------------------------------
PDKCCOM_ALL K2hdkcComState::MakeResponseOnlyHeadData(dkcres_type_t subcode, dkcres_type_t errcode)
{
	PDKCCOM_ALL	pCom;
	if(NULL == (pCom = reinterpret_cast<PDKCCOM_ALL>(malloc(sizeof(DKCRES_STATE))))){
		ERR_DKCPRN("Could not allocate memory.");
		return NULL;
	}
	PDKCRES_STATE	pComRes		= CVT_DKCRES_STATE(pCom);
	pComRes->head.comtype		= DKC_COM_STATE;
	pComRes->head.restype		= COMPOSE_DKC_RES(errcode, subcode);
	pComRes->head.comnumber		= GetComNumber();
	pComRes->head.dispcomnumber	= GetDispComNumber();
	pComRes->head.length		= sizeof(DKCRES_STATE);
	pComRes->states_offset		= sizeof(DKCRES_STATE);
	pComRes->states_count		= 0;

	return pCom;
}

PDKCCOM_ALL K2hdkcComState::AllocateResponseData(size_t svrnodecnt)
{
	PDKCCOM_ALL	pComAll;
	if(NULL == (pComAll = reinterpret_cast<PDKCCOM_ALL>(malloc(sizeof(DKCRES_STATE) + (sizeof(DKC_NODESTATE) * svrnodecnt))))){
		ERR_DKCPRN("Could not allocate memory.");
		return NULL;
	}
	PDKCRES_STATE	pComRes		= CVT_DKCRES_STATE(pComAll);
	pComRes->head.comtype		= DKC_COM_STATE;
	pComRes->head.restype		= COMPOSE_DKC_RES(DKC_RES_SUCCESS, DKC_RES_SUBCODE_NOTHING);
	pComRes->head.comnumber		= GetComNumber();
	pComRes->head.dispcomnumber	= GetDispComNumber();
	pComRes->head.length		= sizeof(DKCRES_STATE) + (sizeof(DKC_NODESTATE) * svrnodecnt);
	pComRes->states_offset		= sizeof(DKCRES_STATE);
	pComRes->states_count		= svrnodecnt;

	if(0 < svrnodecnt){
		PDKC_NODESTATE	pState	= reinterpret_cast<PDKC_NODESTATE>(reinterpret_cast<unsigned char*>(pComRes) + pComRes->states_offset);
		memset(pState, 0, (sizeof(DKC_NODESTATE) * svrnodecnt));		// clear all
	}
	return pComAll;
}

bool K2hdkcComState::CommandProcessing(void)
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

// [NOTE]
// This method is virtual method over writing base class.
// Since there are no arguments and this command is not a communication command.
//
bool K2hdkcComState::CommandSend(void)
{
	if(IsServerNode){
		ERR_DKCPRN("This get server node k2hash state command must be called from only slave node.");
		return false;
	}

	// Get all server node list
	dkcchmpxhashmap_t	svrnodemap;
	if(0 == GetServerNodeMapList(svrnodemap)){
		ERR_DKCPRN("There is no server node.");
		SetErrorResponseData(DKC_RES_SUBCODE_NOSERVERNODE);
		return false;
	}

	// Allocate response area for success
	PDKCCOM_ALL	pComResAll = AllocateResponseData(svrnodemap.size());
	if(!pComResAll){
		ERR_DKCPRN("Something error occurred during allocation.");
		SetErrorResponseData(DKC_RES_SUBCODE_NOMEM);
		return false;
	}
	PDKCRES_STATE		pComRes		= CVT_DKCRES_STATE(pComResAll);

	// Loop for getting k2hash state and storing response
	PDKC_NODESTATE		pResState	= reinterpret_cast<PDKC_NODESTATE>(reinterpret_cast<unsigned char*>(pComRes) + pComRes->states_offset);
	K2hdkcComK2hState*	pComK2hState= NULL;
	dkcres_type_t		rescode		= DKC_INITRESTYPE;
	chmpxid_t			chmpxid		= CHM_INVALID_CHMPXID;
	const char*			pname		= NULL;
	chmhash_t			base_hash	= CHM_INVALID_HASHVAL;
	chmhash_t			pending_hash= CHM_INVALID_HASHVAL;
	const K2HSTATE*		pState		= NULL;
	size_t				setcount	= 0;
	for(dkcchmpxhashmap_t::const_iterator iter = svrnodemap.begin(); iter != svrnodemap.end(); ++iter){
		// check base hash value
		if(CHM_INVALID_HASHVAL == iter->second){
			MSG_DKCPRN("chmpxid(%016" PRIx64 ") base hash(%016" PRIx64 ") is invalid, so can not send command. skip it.", iter->first, iter->second);
			continue;
		}

		// get k2hash state
		pComK2hState	= GetCommonK2hdkcComK2hState(pK2hObj, pChmObj, SendMsgid, GetDispComNumber(), IsSendWithoutSelf(), true, false);
		rescode			= DKC_INITRESTYPE;
		chmpxid			= CHM_INVALID_CHMPXID;
		pname			= NULL;
		base_hash		= CHM_INVALID_HASHVAL;
		pending_hash	= CHM_INVALID_HASHVAL;
		pState			= NULL;
		if(!pComK2hState->CommandSend(iter->first, iter->second, &chmpxid, &pname, &base_hash, &pending_hash, &pState, &rescode)){
			ERR_DKCPRN("Failed to get k2hash state for chmpxid(%016" PRIx64 ") - base hash(%016" PRIx64 ") in DKC_COM_STATE, but continue...", iter->first, iter->second);
			DKC_DELETE(pComK2hState);
			continue;
		}
		if(IS_DKC_RES_NOTSUCCESS(rescode)){
			ERR_DKCPRN("Failed to get k2hash state for chmpxid(%016" PRIx64 ") - base hash(%016" PRIx64 ") in DKC_COM_STATE by subcode(%s), but continue...", iter->first, iter->second, STR_DKCRES_SUBCODE_TYPE(rescode));
			DKC_DELETE(pComK2hState);
			continue;

		}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
			MSG_DKCPRN("Get result subcode(%s) for getting k2hash state for chmpxid(%016" PRIx64 ") - base hash(%016" PRIx64 ") in DKC_COM_STATE", STR_DKCRES_SUBCODE_TYPE(rescode), iter->first, iter->second);
		}
		DKC_DELETE(pComK2hState);

		// set result into response
		pResState[setcount].chmpxid		= chmpxid;
		pResState[setcount].base_hash	= base_hash;
		pResState[setcount].pending_hash= pending_hash;
		strncpy(pResState[setcount].name, pname, NI_MAXHOST - 1);
		memcpy(&(pResState[setcount].k2hstate), pState, sizeof(K2HSTATE));

		++setcount;
	}
	// check count
	if(0 == setcount){
		WAN_DKCPRN("No k2hash state from server nodes.");
		DKC_FREE(pComResAll);
		SetErrorResponseData(DKC_RES_SUBCODE_NODATA, DKC_RES_SUCCESS);
		return false;
	}
	// reset count
	pComRes->states_count	= setcount;

	// set response manually(because we do not have compkt)
	pComRes->head.length	= sizeof(DKCRES_STATE) + (sizeof(DKC_NODESTATE) * setcount);	// set correct length
	pRcvComPkt				= NULL;
	pRcvComAll				= pComResAll;
	RcvComLength			= pComResAll->com_head.length;

	return true;
}

bool K2hdkcComState::CommandSend(const DKC_NODESTATE** ppStates, size_t* pstatecount, dkcres_type_t* prescode)
{
	if(!pChmObj){
		ERR_DKCPRN("Chmpx object is NULL, so could not send command.");
		return false;
	}

	if(!CommandSend()){
		ERR_DKCPRN("Failed to send command.");
		return false;
	}
	return GetResponseData(ppStates, pstatecount, prescode);
}

bool K2hdkcComState::GetResponseData(const DKC_NODESTATE** ppStates, size_t* pstatecount, dkcres_type_t* prescode) const
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
	PDKCRES_STATE	pComRes = CVT_DKCRES_STATE(pcomall);
	if(DKC_COM_STATE != pComRes->head.comtype || DKC_NORESTYPE == pComRes->head.restype || RcvComLength != pComRes->head.length){
		ERR_DKCPRN("Response(received) data is something wrong(internal error: data is invalid).");
		return false;
	}

	if(ppStates){
		*ppStates = reinterpret_cast<const DKC_NODESTATE*>(GET_BIN_DKC_COM_ALL(pcomall, pComRes->states_offset, (sizeof(DKC_NODESTATE) * pComRes->states_count)));;
	}
	if(pstatecount){
		*pstatecount = pComRes->states_count;
	}
	return true;
}

//---------------------------------------------------------
// Methods - Utility
//---------------------------------------------------------
size_t K2hdkcComState::GetServerNodeMapList(dkcchmpxhashmap_t& idhashmap)
{
	idhashmap.clear();

	if(!pChmObj){
		ERR_DKCPRN("This object does not initialize yet.");
		return idhashmap.size();
	}

	// get all k2hdkc server nodes
	PCHMINFOEX	pChmpxInfos = pChmObj->DupAllChmInfo();
	if(!pChmpxInfos){
		ERR_DKCPRN("Failed to get all chmpx information.");
		return idhashmap.size();
	}

	if(pChmpxInfos->pchminfo && 0 < pChmpxInfos->pchminfo->chmpx_man.chmpx_server_count){
		for(PCHMPXLIST pchmpxlist = pChmpxInfos->pchminfo->chmpx_man.chmpx_servers; pchmpxlist; pchmpxlist = pchmpxlist->next){
			idhashmap[pchmpxlist->chmpx.chmpxid] = pchmpxlist->chmpx.base_hash;
		}
	}
	ChmCntrl::FreeDupAllChmInfo(pChmpxInfos);

	return idhashmap.size();
}

//---------------------------------------------------------
// Methods - Dump
//---------------------------------------------------------
void K2hdkcComState::RawDumpComAll(const PDKCCOM_ALL pComAll) const
{
	assert(DKC_COM_STATE == pComAll->com_head.comtype);

	if(DKC_NORESTYPE == pComAll->com_head.restype){
		// no command data for this class
	}else{
		PDKCRES_STATE			pCom		= CVT_DKCRES_STATE(pComAll);
		const DKC_NODESTATE*	pResState	= reinterpret_cast<const DKC_NODESTATE*>(GET_BIN_DKC_COM_ALL(pComAll, pCom->states_offset, (sizeof(DKC_NODESTATE) * pCom->states_count)));

		RAW_DKCPRN("  DKCRES_STATE = {");
		RAW_DKCPRN("    states_offset               = %016" PRIx64 ,	pCom->states_offset);
		RAW_DKCPRN("    states_count                = %zu",				pCom->states_count);

		for(size_t pos = 0; pos < pCom->states_count; ++pos){
			RAW_DKCPRN("    state [%zu] = {",							pos);
			RAW_DKCPRN("      chmpxid                   = %016" PRIx64 ,pResState[pos].chmpxid);
			RAW_DKCPRN("      name                      = %s",			pResState[pos].name);
			RAW_DKCPRN("      base_hash                 = %016" PRIx64 ,pResState[pos].base_hash);
			RAW_DKCPRN("      pending_hash              = %016" PRIx64 ,pResState[pos].pending_hash);
			RAW_DKCPRN("      K2HSTATE = {");
			RAW_DKCPRN("        version                 = %s",			pResState[pos].k2hstate.version);
			RAW_DKCPRN("        hash_version            = %s",			pResState[pos].k2hstate.hash_version);
			RAW_DKCPRN("        trans_version           = %s",			pResState[pos].k2hstate.trans_version);
			RAW_DKCPRN("        trans_pool_count        = %d",			pResState[pos].k2hstate.trans_pool_count);
			RAW_DKCPRN("        max_mask                = %016" PRIx64 ,pResState[pos].k2hstate.max_mask);
			RAW_DKCPRN("        min_mask                = %016" PRIx64 ,pResState[pos].k2hstate.min_mask);
			RAW_DKCPRN("        cur_mask                = %016" PRIx64 ,pResState[pos].k2hstate.cur_mask);
			RAW_DKCPRN("        collision_mask          = %016" PRIx64 ,pResState[pos].k2hstate.collision_mask);
			RAW_DKCPRN("        max_element_count       = %lu",			pResState[pos].k2hstate.max_element_count);
			RAW_DKCPRN("        total_size              = %zu",			pResState[pos].k2hstate.total_size);
			RAW_DKCPRN("        page_size               = %zu",			pResState[pos].k2hstate.page_size);
			RAW_DKCPRN("        file_size               = %zu",			pResState[pos].k2hstate.file_size);
			RAW_DKCPRN("        total_used_size         = %zu",			pResState[pos].k2hstate.total_used_size);
			RAW_DKCPRN("        total_map_size          = %zu",			pResState[pos].k2hstate.total_map_size);
			RAW_DKCPRN("        total_element_size      = %zu",			pResState[pos].k2hstate.total_element_size);
			RAW_DKCPRN("        total_page_size         = %zu",			pResState[pos].k2hstate.total_page_size);
			RAW_DKCPRN("        total_area_count        = %ld",			pResState[pos].k2hstate.total_area_count);
			RAW_DKCPRN("        total_element_count     = %ld",			pResState[pos].k2hstate.total_element_count);
			RAW_DKCPRN("        total_page_count        = %ld",			pResState[pos].k2hstate.total_page_count);
			RAW_DKCPRN("        assigned_area_count     = %ld",			pResState[pos].k2hstate.assigned_area_count);
			RAW_DKCPRN("        assigned_key_count      = %ld",			pResState[pos].k2hstate.assigned_key_count);
			RAW_DKCPRN("        assigned_ckey_count     = %ld",			pResState[pos].k2hstate.assigned_ckey_count);
			RAW_DKCPRN("        assigned_element_count  = %ld",			pResState[pos].k2hstate.assigned_element_count);
			RAW_DKCPRN("        assigned_page_count     = %ld",			pResState[pos].k2hstate.assigned_page_count);
			RAW_DKCPRN("        unassigned_element_count= %ld",			pResState[pos].k2hstate.unassigned_element_count);
			RAW_DKCPRN("        unassigned_page_count   = %ld",			pResState[pos].k2hstate.unassigned_page_count);
			RAW_DKCPRN("        last_update             = %jds %jdusec",pResState[pos].k2hstate.last_update.tv_sec, pResState[pos].k2hstate.last_update.tv_usec);
			RAW_DKCPRN("        last_area_update        = %jds %jdusec",pResState[pos].k2hstate.last_area_update.tv_sec, pResState[pos].k2hstate.last_area_update.tv_usec);
			RAW_DKCPRN("      }");
			RAW_DKCPRN("    }");
		}
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
