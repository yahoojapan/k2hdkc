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
 * CREATE:   Mon May 23 2020
 * REVISION:
 * 
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "k2hdkccomrepldel.h"
#include "k2hdkcutil.h"
#include "k2hdkcdbg.h"

using namespace	std;

//---------------------------------------------------------
// Constructor/Destructor
//---------------------------------------------------------
K2hdkcComReplDel::K2hdkcComReplDel(K2HShm* pk2hash, ChmCntrl* pchmcntrl, uint64_t comnum, bool without_self, bool is_routing_on_server, bool is_wait_on_server) : K2hdkcCommand(pk2hash, pchmcntrl, comnum, without_self, is_routing_on_server, is_wait_on_server, DKC_COM_REPL_DEL)
{
	strClassName = "K2hdkcComReplDel";

	// [NOTE]
	// This command must be replicate mode for all case.
	// (RoutingOnSlave does not care, but set here.)
	//
	SetReplicateSend();
	UnsetRoutingOnSlave();
}

K2hdkcComReplDel::~K2hdkcComReplDel(void)
{
}

//---------------------------------------------------------
// Methods
//---------------------------------------------------------
PDKCCOM_ALL K2hdkcComReplDel::MakeResponseOnlyHeadData(dkcres_type_t subcode, dkcres_type_t errcode)
{
	PDKCCOM_ALL	pComErr;
	if(NULL == (pComErr = reinterpret_cast<PDKCCOM_ALL>(malloc(sizeof(DKCRES_REPL_DEL))))){
		ERR_DKCPRN("Could not allocate memory.");
		return NULL;
	}
	PDKCRES_REPL_DEL	pComRes	= CVT_DKCRES_REPL_DEL(pComErr);
	pComRes->head.comtype		= DKC_COM_REPL_DEL;
	pComRes->head.restype		= COMPOSE_DKC_RES(errcode, subcode);
	pComRes->head.comnumber		= GetComNumber();
	pComRes->head.dispcomnumber	= GetDispComNumber();
	pComRes->head.length		= sizeof(DKCRES_REPL_DEL);

	return pComErr;
}

bool K2hdkcComReplDel::SetSucceedResponseData(void)
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

bool K2hdkcComReplDel::CommandProcessing(void)
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
		const PDKCCOM_REPL_DEL	pCom = CVT_DKCCOM_REPL_DEL(pRcvComAll);
		const unsigned char*	pKey = GET_BIN_DKC_COM_ALL(pRcvComAll, pCom->key_offset, pCom->key_length);
		if(!pKey){
			ERR_DKCPRN("Could not get safe key(%zd offset, %zu byte) in DKCCOM_REPL_DEL(%p).", pCom->key_offset, pCom->key_length, pRcvComAll);
			SetErrorResponseData(DKC_RES_SUBCODE_INVAL);
			return false;
		}

		// check mtime in attributes
		K2HAttrs*	pAttrsObj = NULL;
		if(NULL == (pAttrsObj = pK2hObj->GetAttrs(pKey, pCom->key_length))){
			// there is no attributes for key or the key is not existed.
			WAN_DKCPRN("There is no attributes for key(%s), so it allows to remove this key.", bin_to_string(pKey, pCom->key_length).c_str());
		}else{
			K2hAttrOpsMan	attrman;
			if(!attrman.Initialize(pK2hObj, pKey, pCom->key_length, NULL, 0UL, NULL)){
				// something wrong to get information, but continue...
				WAN_DKCPRN("Something wrong to get attribute information from key(%s), so it allows to remove this key.", bin_to_string(pKey, pCom->key_length).c_str());
			}else{
				// get mtime( we do not check the key expired )
				struct timespec	mtime;
				if(!attrman.GetMTime(*pAttrsObj, mtime)){
					// The existed key does not have mtime
					WAN_DKCPRN("key(%s) does not have mtime attribute, so it allows to remove this key.", bin_to_string(pKey, pCom->key_length).c_str());
				}else{
					// compare timespec
					if(pCom->ts.tv_sec < mtime.tv_sec || (pCom->ts.tv_sec == mtime.tv_sec && pCom->ts.tv_nsec < mtime.tv_nsec)){
						WAN_DKCPRN("the existing key(%s) mtime is newer than requested time, so we do not remove it.", bin_to_string(pKey, pCom->key_length).c_str());
						DKC_DELETE(pAttrsObj);
						SetErrorResponseData(DKC_RES_SUBCODE_REPL);
						return false;
					}
					//MSG_DKCPRN("the existing key(%s) mtime is older than requested time, so we continue to remove processing..", bin_to_string(pKey, pCom->key_length).c_str());
				}
			}
			DKC_DELETE(pAttrsObj);
		}

		// do command
		// 
		if(!pK2hObj->Remove(pKey, pCom->key_length, false)){							// [NOTICE] do not remove subkeys
			ERR_DKCPRN("Failed to remove key(%s)", bin_to_string(pKey, pCom->key_length).c_str());
			SetErrorResponseData(DKC_RES_SUBCODE_REPL);
			return false;
		}

		// set response data
		SetSucceedResponseData();

	}else{
		//
		// receive data is response
		//
		// get receive data
		const PDKCRES_REPL_DEL	pComRes = CVT_DKCRES_REPL_DEL(pRcvComAll);

		// print result
		DKC_RES_PRINT(pComRes->head.comtype, pComRes->head.restype);
	}
	return true;
}

bool K2hdkcComReplDel::CommandSend(k2h_hash_t hash, k2h_hash_t subhash, const unsigned char* pkey, size_t keylength, const struct timespec ts)
{
	if(!pkey || 0 == keylength){
		ERR_DKCPRN("Parameter are wrong.");
		return false;
	}
	if(!IsServerNode){
		MSG_DKCPRN("This DKC_COM_REPL_DEL command must be sent on server node.");
		return false;
	}

	// make send data
	PDKCCOM_ALL	pComAll;
	if(NULL == (pComAll = reinterpret_cast<PDKCCOM_ALL>(malloc(sizeof(DKCCOM_REPL_DEL) + keylength)))){
		ERR_DKCPRN("Could not allocate memory.");
		return false;
	}
	PDKCCOM_REPL_DEL	pComReplDel	= CVT_DKCCOM_REPL_DEL(pComAll);
	pComReplDel->head.comtype		= DKC_COM_REPL_DEL;
	pComReplDel->head.restype		= DKC_NORESTYPE;
	pComReplDel->head.comnumber		= GetComNumber();
	pComReplDel->head.dispcomnumber	= GetDispComNumber();
	pComReplDel->head.length		= sizeof(DKCCOM_REPL_DEL) + keylength;
	pComReplDel->hash				= hash;
	pComReplDel->subhash			= subhash;
	pComReplDel->key_offset			= sizeof(DKCCOM_REPL_DEL);
	pComReplDel->key_length			= keylength;
	pComReplDel->ts					= ts;

	unsigned char*	pdata			= reinterpret_cast<unsigned char*>(pComReplDel) + pComReplDel->key_offset;
	memcpy(pdata, pkey, keylength);

	if(!SetSendData(pComAll, K2hdkcCommand::MakeChmpxHash((reinterpret_cast<unsigned char*>(pComReplDel) + pComReplDel->key_offset), keylength))){
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

bool K2hdkcComReplDel::CommandSend(k2h_hash_t hash, k2h_hash_t subhash, const unsigned char* pkey, size_t keylength, const struct timespec ts, dkcres_type_t* prescode)
{
	if(!pChmObj){
		ERR_DKCPRN("Chmpx object is NULL, so could not send command.");
		return false;
	}

	if(!CommandSend(hash, subhash, pkey, keylength, ts)){
		ERR_DKCPRN("Failed to send command.");
		return false;
	}
	return GetResponseData(prescode);
}

bool K2hdkcComReplDel::GetResponseData(dkcres_type_t* prescode) const
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
void K2hdkcComReplDel::RawDumpComAll(const PDKCCOM_ALL pComAll) const
{
	assert(DKC_COM_REPL_DEL == pComAll->com_head.comtype);

	if(DKC_NORESTYPE == pComAll->com_head.restype){
		PDKCCOM_REPL_DEL	pCom = CVT_DKCCOM_REPL_DEL(pComAll);

		RAW_DKCPRN("  DKCCOM_REPL_DEL = {");
		RAW_DKCPRN("    hash            = 0x%016" PRIx64 ,		pCom->hash);
		RAW_DKCPRN("    subhash         = 0x%016" PRIx64 ,		pCom->subhash);
		RAW_DKCPRN("    key_offset      = (%016" PRIx64 ") %s",	pCom->key_offset, bin_to_string(GET_BIN_DKC_COM_ALL(pComAll, pCom->key_offset, pCom->key_length), pCom->key_length).c_str());
		RAW_DKCPRN("    key_length      = %zu",					pCom->key_length);
		RAW_DKCPRN("    ts              = %s (%s)",				STR_DKC_TIMESPEC(&(pCom->ts)).c_str(), STR_DKC_TIMESPEC_NS(&(pCom->ts)).c_str());
		RAW_DKCPRN("  }");

	}else{
		//PDKCRES_REPL_DEL	pCom = CVT_DKCRES_REPL_DEL(pComAll);
		RAW_DKCPRN("  DKCRES_REPL_DEL = {");
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
