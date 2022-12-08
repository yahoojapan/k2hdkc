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
 * CREATE:   Fri Jul 15 2016
 * REVISION:
 * 
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>

#include <k2hash/k2hashfunc.h>

#include "k2hdkccombase.h"
#include "k2hdkccomget.h"
#include "k2hdkccomgetdirect.h"
#include "k2hdkccomgetsubkeys.h"
#include "k2hdkccomgetattrs.h"
#include "k2hdkccomgetattr.h"
#include "k2hdkccomset.h"
#include "k2hdkccomsetdirect.h"
#include "k2hdkccomsetsubkeys.h"
#include "k2hdkccomsetall.h"
#include "k2hdkccomaddsubkey.h"
#include "k2hdkccomaddsubkeys.h"
#include "k2hdkccomdel.h"
#include "k2hdkccomdelsubkey.h"
#include "k2hdkccomdelsubkeys.h"
#include "k2hdkccomqpush.h"
#include "k2hdkccomqpop.h"
#include "k2hdkccomqdel.h"
#include "k2hdkccomren.h"
#include "k2hdkccomcasinit.h"
#include "k2hdkccomcasget.h"
#include "k2hdkccomcasset.h"
#include "k2hdkccomcasincdec.h"
#include "k2hdkccomreplkey.h"
#include "k2hdkccomk2hstate.h"
#include "k2hdkccomstate.h"
#include "k2hdkccomrepldel.h"
#include "k2hdkccomutil.h"
#include "k2hdkcutil.h"
#include "k2hdkcdbg.h"

using namespace	std;

//---------------------------------------------------------
// DKC Command type Debugging
//---------------------------------------------------------
static const char*	str_dkccom_type[] = {
	"DKC_COM_GET",
	"DKC_COM_GET_DIRECT",
	"DKC_COM_GET_SUBKEYS",
	"DKC_COM_GET_ATTRS",
	"DKC_COM_GET_ATTR",
	"DKC_COM_SET",
	"DKC_COM_SET_DIRECT",
	"DKC_COM_SET_SUBKEYS",
	"DKC_COM_ADD_SUBKEY",
	"DKC_COM_SET_ALL",
	"DKC_COM_ADD_SUBKEYS",
	"DKC_COM_DEL",
	"DKC_COM_DEL_SUBKEYS",
	"DKC_COM_DEL_SUBKEY",
	"DKC_COM_REN",
	"DKC_COM_QPUSH",
	"DKC_COM_QPOP",
	"DKC_COM_QDEL",
	"DKC_COM_CAS_INIT",
	"DKC_COM_CAS_GET",
	"DKC_COM_CAS_SET",
	"DKC_COM_CAS_INCDEC",
	"DKC_COM_REPL_KEY",
	"DKC_COM_K2HSTATE",
	"DKC_COM_STATE",
	"DKC_COM_REPL_DEL"
};

const char* STR_DKCCOM_TYPE(dkccom_type_t comtype)
{
	if(DKC_COM_MAX < comtype){
		// [NOTE]
		// comtype is ungigned, and DKC_COM_GET is zero, then not check minimum value.
		//
		return "UNKNOWN_DKC_COM_TYPE";
	}
	return str_dkccom_type[comtype];
}

//---------------------------------------------------------
// Class Variables
//---------------------------------------------------------
const long			K2hdkcCommand::DEFAULT_RCV_TIMEOUT_MS;
long				K2hdkcCommand::RcvTimeout				= K2hdkcCommand::DEFAULT_RCV_TIMEOUT_MS;
dkccom_setwait_fp	K2hdkcCommand::WaitFp					= NULL;
dkccom_unsetwait_fp	K2hdkcCommand::UnWaitFp					= NULL;
void*				K2hdkcCommand::pWaitFpParam				= NULL;

//---------------------------------------------------------
// Class Method
//---------------------------------------------------------
//
// Get response data type
//
bool K2hdkcCommand::GetResponseCommandType(const unsigned char* pbody, size_t length, dkccom_type_t& comtype, dkcres_type_t& restype, uint64_t& comnum, uint64_t& dispnum)
{
	if(!pbody || 0 == length){
		ERR_DKCPRN("Parameter are wrong.");
		return false;
	}

	const PDKCCOM_ALL	pComAll = reinterpret_cast<const PDKCCOM_ALL>(const_cast<unsigned char*>(pbody));
	if(!IS_SAFE_DKC_COM_ALL(pComAll)){
		return false;
	}
	comtype	= pComAll->com_head.comtype;
	restype	= pComAll->com_head.restype;
	comnum	= pComAll->com_head.comnumber;
	dispnum	= pComAll->com_head.dispcomnumber;

	return true;
}

//
// Class factory
//
// [NOTICE]
// pComPkt and pbody are freed in this class, so caller must not free these when returns success from this method.
//
K2hdkcCommand* K2hdkcCommand::GetCommandReceiveObject(K2HShm* pk2hash, ChmCntrl* pchmcntrl, PCOMPKT pComPkt, unsigned char* pbody, size_t length)
{
	if(!pk2hash || !pchmcntrl || !pComPkt || !pbody || 0 == length){
		ERR_DKCPRN("Parameter are wrong.");
		return NULL;
	}

	const PDKCCOM_ALL	pComAll = reinterpret_cast<const PDKCCOM_ALL>(const_cast<unsigned char*>(pbody));
	if(!IS_SAFE_DKC_COM_ALL(pComAll)){
		return NULL;
	}

	// make(initialize) command object
	K2hdkcCommand*	pObj = NULL;
	if(DKC_COM_GET == pComAll->com_head.comtype){
		K2hdkcComGet*	pComObj = new K2hdkcComGet(pk2hash, pchmcntrl, pComAll->com_head.comnumber);
		if(!pComObj){
			ERR_DKCPRN("Could not make command object for DKC_COM_GET.");
			return NULL;
		}
		pObj = pComObj;

	}else if(DKC_COM_GET_DIRECT == pComAll->com_head.comtype){
		K2hdkcComGetDirect*	pComObj = new K2hdkcComGetDirect(pk2hash, pchmcntrl, pComAll->com_head.comnumber);
		if(!pComObj){
			ERR_DKCPRN("Could not make command object for DKC_COM_GET_DIRECT.");
			return NULL;
		}
		pObj = pComObj;

	}else if(DKC_COM_GET_SUBKEYS == pComAll->com_head.comtype){
		K2hdkcComGetSubkeys*	pComObj = new K2hdkcComGetSubkeys(pk2hash, pchmcntrl, pComAll->com_head.comnumber);
		if(!pComObj){
			ERR_DKCPRN("Could not make command object for DKC_COM_GET_SUBKEYS.");
			return NULL;
		}
		pObj = pComObj;

	}else if(DKC_COM_GET_ATTRS == pComAll->com_head.comtype){
		K2hdkcComGetAttrs*	pComObj = new K2hdkcComGetAttrs(pk2hash, pchmcntrl, pComAll->com_head.comnumber);
		if(!pComObj){
			ERR_DKCPRN("Could not make command object for DKC_COM_GET_ATTRS.");
			return NULL;
		}
		pObj = pComObj;

	}else if(DKC_COM_GET_ATTR == pComAll->com_head.comtype){
		K2hdkcComGetAttr*	pComObj = new K2hdkcComGetAttr(pk2hash, pchmcntrl, pComAll->com_head.comnumber);
		if(!pComObj){
			ERR_DKCPRN("Could not make command object for DKC_COM_GET_ATTR.");
			return NULL;
		}
		pObj = pComObj;

	}else if(DKC_COM_SET == pComAll->com_head.comtype){
		K2hdkcComSet*	pComObj = new K2hdkcComSet(pk2hash, pchmcntrl, pComAll->com_head.comnumber);
		if(!pComObj){
			ERR_DKCPRN("Could not make command object for DKC_COM_SET.");
			return NULL;
		}
		pObj = pComObj;

	}else if(DKC_COM_SET_DIRECT == pComAll->com_head.comtype){
		K2hdkcComSetDirect*	pComObj = new K2hdkcComSetDirect(pk2hash, pchmcntrl, pComAll->com_head.comnumber);
		if(!pComObj){
			ERR_DKCPRN("Could not make command object for DKC_COM_SET_DIRECT.");
			return NULL;
		}
		pObj = pComObj;

	}else if(DKC_COM_SET_SUBKEYS == pComAll->com_head.comtype){
		K2hdkcComSetSubkeys*	pComObj = new K2hdkcComSetSubkeys(pk2hash, pchmcntrl, pComAll->com_head.comnumber);
		if(!pComObj){
			ERR_DKCPRN("Could not make command object for DKC_COM_SET_SUBKEYS.");
			return NULL;
		}
		pObj = pComObj;

	}else if(DKC_COM_SET_ALL == pComAll->com_head.comtype){
		K2hdkcComSetAll*	pComObj = new K2hdkcComSetAll(pk2hash, pchmcntrl, pComAll->com_head.comnumber);
		if(!pComObj){
			ERR_DKCPRN("Could not make command object for DKC_COM_SET_ALL.");
			return NULL;
		}
		pObj = pComObj;

	}else if(DKC_COM_ADD_SUBKEYS == pComAll->com_head.comtype){
		K2hdkcComAddSubkeys*	pComObj = new K2hdkcComAddSubkeys(pk2hash, pchmcntrl, pComAll->com_head.comnumber);
		if(!pComObj){
			ERR_DKCPRN("Could not make command object for DKC_COM_ADD_SUBKEYS.");
			return NULL;
		}
		pObj = pComObj;

	}else if(DKC_COM_ADD_SUBKEY == pComAll->com_head.comtype){
		K2hdkcComAddSubkey*	pComObj = new K2hdkcComAddSubkey(pk2hash, pchmcntrl, pComAll->com_head.comnumber);
		if(!pComObj){
			ERR_DKCPRN("Could not make command object for DKC_COM_ADD_SUBKEY.");
			return NULL;
		}
		pObj = pComObj;

	}else if(DKC_COM_DEL == pComAll->com_head.comtype){
		K2hdkcComDel*	pComObj = new K2hdkcComDel(pk2hash, pchmcntrl, pComAll->com_head.comnumber);
		if(!pComObj){
			ERR_DKCPRN("Could not make command object for DKC_COM_DEL.");
			return NULL;
		}
		pObj = pComObj;

	}else if(DKC_COM_DEL_SUBKEYS == pComAll->com_head.comtype){
		K2hdkcComDelSubkeys*	pComObj = new K2hdkcComDelSubkeys(pk2hash, pchmcntrl, pComAll->com_head.comnumber);
		if(!pComObj){
			ERR_DKCPRN("Could not make command object for DKC_COM_DEL_SUBKEYS.");
			return NULL;
		}
		pObj = pComObj;

	}else if(DKC_COM_DEL_SUBKEY == pComAll->com_head.comtype){
		K2hdkcComDelSubkey*	pComObj = new K2hdkcComDelSubkey(pk2hash, pchmcntrl, pComAll->com_head.comnumber);
		if(!pComObj){
			ERR_DKCPRN("Could not make command object for DKC_COM_DEL_SUBKEY.");
			return NULL;
		}
		pObj = pComObj;

	}else if(DKC_COM_REN == pComAll->com_head.comtype){
		K2hdkcComRen*	pComObj = new K2hdkcComRen(pk2hash, pchmcntrl, pComAll->com_head.comnumber);
		if(!pComObj){
			ERR_DKCPRN("Could not make command object for DKC_COM_REN.");
			return NULL;
		}
		pObj = pComObj;

	}else if(DKC_COM_QPUSH == pComAll->com_head.comtype){
		K2hdkcComQPush*	pComObj = new K2hdkcComQPush(pk2hash, pchmcntrl, pComAll->com_head.comnumber);
		if(!pComObj){
			ERR_DKCPRN("Could not make command object for DKC_COM_QPUSH.");
			return NULL;
		}
		pObj = pComObj;

	}else if(DKC_COM_QPOP == pComAll->com_head.comtype){
		K2hdkcComQPop*	pComObj = new K2hdkcComQPop(pk2hash, pchmcntrl, pComAll->com_head.comnumber);
		if(!pComObj){
			ERR_DKCPRN("Could not make command object for DKC_COM_QPOP.");
			return NULL;
		}
		pObj = pComObj;

	}else if(DKC_COM_QDEL == pComAll->com_head.comtype){
		K2hdkcComQDel*	pComObj = new K2hdkcComQDel(pk2hash, pchmcntrl, pComAll->com_head.comnumber);
		if(!pComObj){
			ERR_DKCPRN("Could not make command object for DKC_COM_QDEL.");
			return NULL;
		}
		pObj = pComObj;

	}else if(DKC_COM_CAS_INIT == pComAll->com_head.comtype){
		K2hdkcComCasInit*	pComObj = new K2hdkcComCasInit(pk2hash, pchmcntrl, pComAll->com_head.comnumber);
		if(!pComObj){
			ERR_DKCPRN("Could not make command object for DKC_COM_CAS_INIT.");
			return NULL;
		}
		pObj = pComObj;

	}else if(DKC_COM_CAS_GET == pComAll->com_head.comtype){
		K2hdkcComCasGet*	pComObj = new K2hdkcComCasGet(pk2hash, pchmcntrl, pComAll->com_head.comnumber);
		if(!pComObj){
			ERR_DKCPRN("Could not make command object for DKC_COM_CAS_GET.");
			return NULL;
		}
		pObj = pComObj;

	}else if(DKC_COM_CAS_SET == pComAll->com_head.comtype){
		K2hdkcComCasSet*	pComObj = new K2hdkcComCasSet(pk2hash, pchmcntrl, pComAll->com_head.comnumber);
		if(!pComObj){
			ERR_DKCPRN("Could not make command object for DKC_COM_CAS_SET.");
			return NULL;
		}
		pObj = pComObj;

	}else if(DKC_COM_CAS_INCDEC == pComAll->com_head.comtype){
		K2hdkcComCasIncDec*	pComObj = new K2hdkcComCasIncDec(pk2hash, pchmcntrl, pComAll->com_head.comnumber);
		if(!pComObj){
			ERR_DKCPRN("Could not make command object for DKC_COM_CAS_INCDEC.");
			return NULL;
		}
		pObj = pComObj;

	}else if(DKC_COM_REPL_KEY == pComAll->com_head.comtype){
		K2hdkcComReplKey*	pComObj = new K2hdkcComReplKey(pk2hash, pchmcntrl, pComAll->com_head.comnumber);
		if(!pComObj){
			ERR_DKCPRN("Could not make command object for DKC_COM_REPL_KEY.");
			return NULL;
		}
		pObj = pComObj;

	}else if(DKC_COM_REPL_DEL == pComAll->com_head.comtype){
		K2hdkcComReplDel*	pComObj = new K2hdkcComReplDel(pk2hash, pchmcntrl, pComAll->com_head.comnumber);
		if(!pComObj){
			ERR_DKCPRN("Could not make command object for DKC_COM_REPL_DEL.");
			return NULL;
		}
		pObj = pComObj;

	}else if(DKC_COM_K2HSTATE == pComAll->com_head.comtype){
		K2hdkcComK2hState*	pComObj = new K2hdkcComK2hState(pk2hash, pchmcntrl, pComAll->com_head.comnumber);
		if(!pComObj){
			ERR_DKCPRN("Could not make command object for DKC_COM_K2HSTATE.");
			return NULL;
		}
		pObj = pComObj;

	}else{
		ERR_DKCPRN("DKCCOM_ALL command type(0x%016" PRIx64 ") is unknown, why come here...", pComAll->com_head.comtype);
	}

	// set received data to command object
	if(pObj){
		if(!pObj->SetReceiveData(pComPkt, pComAll)){
			ERR_DKCPRN("Could not set received data to command object( for DKCCOM_ALL command type(0x%016" PRIx64 ")).", pComAll->com_head.comtype);
			DKC_DELETE(pObj);
		}
	}

	return pObj;
}

K2hdkcCommand* K2hdkcCommand::GetSlaveCommandSendObject(dkccom_type_t comtype, const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, uint64_t comnum)
{
	if(!IS_SAFE_DKCCOM_TYPE(comtype)){
		ERR_DKCPRN("Parameter are wrong.");
		return NULL;
	}
	// create slave command object
	K2hdkcSlave*	pslave = new K2hdkcSlave();

	// initialize slave command object
	if(!pslave->Initialize(config, ctlport, cuk, is_auto_rejoin)){
		ERR_DKCPRN("Could not create slave command object.");
		DKC_DELETE(pslave);
		return NULL;
	}

	// open msgid by slave command object
	if(!pslave->Open(no_giveup_rejoin)){
		ERR_DKCPRN("Could not open msgid by slave command object.");
		pslave->Clean();
		DKC_DELETE(pslave);
		return NULL;
	}

	// create command object
	K2hdkcCommand*	pComObj = K2hdkcCommand::GetCommandSendObject(NULL, pslave->GetChmCntrlObject(), comtype, pslave->GetMsgid(), comnum);
	if(!pComObj){
		ERR_DKCPRN("Could not create command object for slave command.");
		pslave->Clean();
		DKC_DELETE(pslave);
		return NULL;
	}

	// set slave command object to command object
	pComObj->pSlaveObj		= pslave;
	pComObj->IsCreateSlave	= true;

	return pComObj;
}

K2hdkcCommand* K2hdkcCommand::GetCommandSendObject(K2HShm* pk2hash, ChmCntrl* pchmcntrl, dkccom_type_t comtype, msgid_t msgid, uint64_t comnum, bool without_self, bool is_routing_on_server, bool is_wait_on_server)
{
	if(!pchmcntrl || !IS_SAFE_DKCCOM_TYPE(comtype)){
		ERR_DKCPRN("Parameter are wrong.");
		return NULL;
	}
	if(!pk2hash && pchmcntrl->IsClientOnSvrType()){
		ERR_DKCPRN("Parameter k2hash pointer must be not null on server node.");
		return NULL;
	}

	// check msgid
	//
	// On server node we do not use msgid, but on slave node we have to use msgid for sending command.
	// Then if msgid is CHM_INVALID_MSGID on slave node, we try to open msgid and use it.
	//
	bool	is_need_open_msgid = false;
	if(CHM_INVALID_MSGID == msgid && !pchmcntrl->IsClientOnSvrType()){
		is_need_open_msgid = true;
	}

	// make(initialize) command object
	K2hdkcCommand*	pObj = NULL;
	if(DKC_COM_GET == comtype){
		if(NULL == (pObj = new K2hdkcComGet(pk2hash, pchmcntrl, comnum, without_self, is_routing_on_server, is_wait_on_server))){
			ERR_DKCPRN("Could not make command object for DKC_COM_GET.");
			// cppcheck-suppress memleak
			return NULL;
		}

	}else if(DKC_COM_GET_DIRECT == comtype){
		if(NULL == (pObj = new K2hdkcComGetDirect(pk2hash, pchmcntrl, comnum, without_self, is_routing_on_server, is_wait_on_server))){
			ERR_DKCPRN("Could not make command object for DKC_COM_GET_DIRECT.");
			// cppcheck-suppress memleak
			return NULL;
		}

	}else if(DKC_COM_GET_SUBKEYS == comtype){
		if(NULL == (pObj = new K2hdkcComGetSubkeys(pk2hash, pchmcntrl, comnum, without_self, is_routing_on_server, is_wait_on_server))){
			ERR_DKCPRN("Could not make command object for DKC_COM_GET_SUBKEYS.");
			// cppcheck-suppress memleak
			return NULL;
		}

	}else if(DKC_COM_GET_ATTRS == comtype){
		if(NULL == (pObj = new K2hdkcComGetAttrs(pk2hash, pchmcntrl, comnum, without_self, is_routing_on_server, is_wait_on_server))){
			ERR_DKCPRN("Could not make command object for DKC_COM_GET_ATTRS.");
			// cppcheck-suppress memleak
			return NULL;
		}

	}else if(DKC_COM_GET_ATTR == comtype){
		if(NULL == (pObj = new K2hdkcComGetAttr(pk2hash, pchmcntrl, comnum, without_self, is_routing_on_server, is_wait_on_server))){
			ERR_DKCPRN("Could not make command object for DKC_COM_GET_ATTR.");
			// cppcheck-suppress memleak
			return NULL;
		}

	}else if(DKC_COM_SET == comtype){
		if(NULL == (pObj = new K2hdkcComSet(pk2hash, pchmcntrl, comnum, without_self, is_routing_on_server, is_wait_on_server))){
			ERR_DKCPRN("Could not make command object for DKC_COM_SET.");
			// cppcheck-suppress memleak
			return NULL;
		}

	}else if(DKC_COM_SET_DIRECT == comtype){
		if(NULL == (pObj = new K2hdkcComSetDirect(pk2hash, pchmcntrl, comnum, without_self, is_routing_on_server, is_wait_on_server))){
			ERR_DKCPRN("Could not make command object for DKC_COM_SET_DIRECT.");
			// cppcheck-suppress memleak
			return NULL;
		}

	}else if(DKC_COM_SET_SUBKEYS == comtype){
		if(NULL == (pObj = new K2hdkcComSetSubkeys(pk2hash, pchmcntrl, comnum, without_self, is_routing_on_server, is_wait_on_server))){
			ERR_DKCPRN("Could not make command object for DKC_COM_SET_SUBKEYS.");
			// cppcheck-suppress memleak
			return NULL;
		}

	}else if(DKC_COM_SET_ALL == comtype){
		if(NULL == (pObj = new K2hdkcComSetAll(pk2hash, pchmcntrl, comnum, without_self, is_routing_on_server, is_wait_on_server))){
			ERR_DKCPRN("Could not make command object for DKC_COM_SET_ALL.");
			// cppcheck-suppress memleak
			return NULL;
		}

	}else if(DKC_COM_ADD_SUBKEYS == comtype){
		if(NULL == (pObj = new K2hdkcComAddSubkeys(pk2hash, pchmcntrl, comnum, without_self, is_routing_on_server, is_wait_on_server))){
			ERR_DKCPRN("Could not make command object for DKC_COM_ADD_SUBKEYS.");
			// cppcheck-suppress memleak
			return NULL;
		}

	}else if(DKC_COM_ADD_SUBKEY == comtype){
		if(NULL == (pObj = new K2hdkcComAddSubkey(pk2hash, pchmcntrl, comnum, without_self, is_routing_on_server, is_wait_on_server))){
			ERR_DKCPRN("Could not make command object for DKC_COM_ADD_SUBKEY.");
			// cppcheck-suppress memleak
			return NULL;
		}

	}else if(DKC_COM_DEL == comtype){
		if(NULL == (pObj = new K2hdkcComDel(pk2hash, pchmcntrl, comnum, without_self, is_routing_on_server, is_wait_on_server))){
			ERR_DKCPRN("Could not make command object for DKC_COM_DEL.");
			// cppcheck-suppress memleak
			return NULL;
		}

	}else if(DKC_COM_DEL_SUBKEYS == comtype){
		if(NULL == (pObj = new K2hdkcComDelSubkeys(pk2hash, pchmcntrl, comnum, without_self, is_routing_on_server, is_wait_on_server))){
			ERR_DKCPRN("Could not make command object for DKC_COM_DEL_SUBKEYS.");
			// cppcheck-suppress memleak
			return NULL;
		}

	}else if(DKC_COM_DEL_SUBKEY == comtype){
		if(NULL == (pObj = new K2hdkcComDelSubkey(pk2hash, pchmcntrl, comnum, without_self, is_routing_on_server, is_wait_on_server))){
			ERR_DKCPRN("Could not make command object for DKC_COM_DEL_SUBKEY.");
			// cppcheck-suppress memleak
			return NULL;
		}

	}else if(DKC_COM_REN == comtype){
		if(NULL == (pObj = new K2hdkcComRen(pk2hash, pchmcntrl, comnum, without_self, is_routing_on_server, is_wait_on_server))){
			ERR_DKCPRN("Could not make command object for DKC_COM_REN.");
			// cppcheck-suppress memleak
			return NULL;
		}

	}else if(DKC_COM_QPUSH == comtype){
		if(NULL == (pObj = new K2hdkcComQPush(pk2hash, pchmcntrl, comnum, without_self, is_routing_on_server, is_wait_on_server))){
			ERR_DKCPRN("Could not make command object for DKC_COM_QPUSH.");
			// cppcheck-suppress memleak
			return NULL;
		}

	}else if(DKC_COM_QPOP == comtype){
		if(NULL == (pObj = new K2hdkcComQPop(pk2hash, pchmcntrl, comnum, without_self, is_routing_on_server, is_wait_on_server))){
			ERR_DKCPRN("Could not make command object for DKC_COM_QPOP.");
			// cppcheck-suppress memleak
			return NULL;
		}

	}else if(DKC_COM_QDEL == comtype){
		if(NULL == (pObj = new K2hdkcComQDel(pk2hash, pchmcntrl, comnum, without_self, is_routing_on_server, is_wait_on_server))){
			ERR_DKCPRN("Could not make command object for DKC_COM_QDEL.");
			// cppcheck-suppress memleak
			return NULL;
		}

	}else if(DKC_COM_CAS_INIT == comtype){
		if(NULL == (pObj = new K2hdkcComCasInit(pk2hash, pchmcntrl, comnum, without_self, is_routing_on_server, is_wait_on_server))){
			ERR_DKCPRN("Could not make command object for DKC_COM_CAS_INIT.");
			// cppcheck-suppress memleak
			return NULL;
		}

	}else if(DKC_COM_CAS_GET == comtype){
		if(NULL == (pObj = new K2hdkcComCasGet(pk2hash, pchmcntrl, comnum, without_self, is_routing_on_server, is_wait_on_server))){
			ERR_DKCPRN("Could not make command object for DKC_COM_CAS_GET.");
			// cppcheck-suppress memleak
			return NULL;
		}

	}else if(DKC_COM_CAS_SET == comtype){
		if(NULL == (pObj = new K2hdkcComCasSet(pk2hash, pchmcntrl, comnum, without_self, is_routing_on_server, is_wait_on_server))){
			ERR_DKCPRN("Could not make command object for DKC_COM_CAS_SET.");
			// cppcheck-suppress memleak
			return NULL;
		}

	}else if(DKC_COM_CAS_INCDEC == comtype){
		if(NULL == (pObj = new K2hdkcComCasIncDec(pk2hash, pchmcntrl, comnum, without_self, is_routing_on_server, is_wait_on_server))){
			ERR_DKCPRN("Could not make command object for DKC_COM_CAS_INCDEC.");
			// cppcheck-suppress memleak
			return NULL;
		}

	}else if(DKC_COM_REPL_KEY == comtype){
		if(NULL == (pObj = new K2hdkcComReplKey(pk2hash, pchmcntrl, comnum, without_self, is_routing_on_server, is_wait_on_server))){
			ERR_DKCPRN("Could not make command object for DKC_COM_REPL_KEY.");
			// cppcheck-suppress memleak
			return NULL;
		}

	}else if(DKC_COM_REPL_DEL == comtype){
		if(NULL == (pObj = new K2hdkcComReplDel(pk2hash, pchmcntrl, comnum, without_self, is_routing_on_server, is_wait_on_server))){
			ERR_DKCPRN("Could not make command object for DKC_COM_REPL_DEL.");
			// cppcheck-suppress memleak
			return NULL;
		}

	}else if(DKC_COM_K2HSTATE == comtype){
		if(NULL == (pObj = new K2hdkcComK2hState(pk2hash, pchmcntrl, comnum, without_self, is_routing_on_server, is_wait_on_server))){
			ERR_DKCPRN("Could not make command object for DKC_COM_K2HSTATE.");
			// cppcheck-suppress memleak
			return NULL;
		}

	}else if(DKC_COM_STATE == comtype){
		if(NULL == (pObj = new K2hdkcComState(pk2hash, pchmcntrl, comnum, without_self, is_routing_on_server, is_wait_on_server))){
			ERR_DKCPRN("Could not make command object for DKC_COM_STATE.");
			// cppcheck-suppress memleak
			return NULL;
		}

	}else{
		ERR_DKCPRN("Unknown command type(0x%016" PRIx64 ") is specified.", comtype);
		return NULL;
	}

	// set msgid(if need, open msgid)
	if(is_need_open_msgid){
		if(CHM_INVALID_MSGID == (pObj->SendMsgid = pchmcntrl->Open())){				// [NOTE] no_giveup_rejoin is false
			ERR_DKCPRN("Failed to open msgid for send command type(0x%016" PRIx64 ").", comtype);
			DKC_DELETE(pObj);
			return NULL;
		}
		pObj->IsLocalMsgid	= true;
	}else{
		pObj->SendMsgid		= msgid;
		pObj->IsLocalMsgid	= false;
	}
	return pObj;
}

//
// Utility
//
chmhash_t K2hdkcCommand::MakeChmpxHash(unsigned char* byptr, size_t length)
{
	if(!byptr || 0 == length){
		ERR_DKCPRN("Parameter are wrong.");
		return 0L;
	}
	CHMBIN	chmbin;
	chmbin.byptr	= byptr;
	chmbin.length	= length;

	return make_chmbin_hash(&chmbin);
}

//---------------------------------------------------------
// Constructor/Destructor
//---------------------------------------------------------
K2hdkcCommand::K2hdkcCommand(K2HShm* pk2hash, ChmCntrl* pchmcntrl, uint64_t comnum, bool without_self, bool is_routing_on_server, bool is_wait_on_server, dkccom_type_t comtype) :
		strClassName("K2hdkcCommand"), pK2hObj(pk2hash), pChmObj(pchmcntrl), IsServerNode(false), ComNumber(comnum), DispComNumber(K2hdkcComNumber::Get()), IsNeedReply(false),
		WithoutSelfOnServer(without_self), IsWaitResOnServer(is_wait_on_server), RoutingOnServer(is_routing_on_server), RoutingOnSlave(true), ReplicateSend(false), TriggerResponse(true), MainCommandType(comtype),
		pRcvComAll(NULL), RcvComLength(0L), pRcvComPkt(NULL),
		pSendComAll(NULL), SendLength(0L), SendMsgid(CHM_INVALID_MSGID), SendHash(0L), IsLocalMsgid(false), IsCreateSlave(false), pSlaveObj(NULL)
{
	assert(pChmObj);

	// set chmpx node type
	if(true == (IsServerNode = pChmObj->IsClientOnSvrType())){
		assert(pK2hObj);
	}

	// when debugging mode, set communication number
	if(K2hdkcComNumber::INIT_NUMBER == ComNumber && K2hdkcComNumber::IsEnable()){
		ComNumber = K2hdkcComNumber::Get();
	}

	// initialize condition values
	pthread_mutex_init(&cond_mutex, NULL);
	if(0 != pthread_cond_init(&cond_val, NULL)){
		ERR_DKCPRN("Failed to initialize condition for waiting response.");
	}
}

K2hdkcCommand::~K2hdkcCommand(void)
{
	// [NOTE]
	// It is not necessary to call Clean() in the destructor of the subclass,
	// because this base class call it, and Clean() is virtual method.
	// Thus you should implement only Clean() method in subclass.
	//
	K2hdkcCommand::Clean();
}

//---------------------------------------------------------
// Methods
//---------------------------------------------------------
bool K2hdkcCommand::Clean(void)
{
	bool	result	= true;
	pK2hObj			= NULL;

	// free
	DKC_FREE(pRcvComAll);
	DKC_FREE(pRcvComPkt);
	RcvComLength	= 0L;

	// close msgid or delete slave command object if need
	if(IsLocalMsgid && pSlaveObj){
		ERR_DKCPRN("Local msgid opened flag is true and has slave command object, so this is something wrong. but continue...");
	}
	if(IsLocalMsgid){
		if(!pChmObj){
			ERR_DKCPRN("Why chmpx object is NULL.");
		}else{
			if(!pChmObj->Close(SendMsgid)){
				ERR_DKCPRN("Failed to close msgid(0x%016" PRIx64 "), but continue...", SendMsgid);
			}
		}
	}
	pChmObj = NULL;
	CleanSlave();

	SendHash	= 0L;
	IsLocalMsgid= false;
	SendLength	= 0L;
	DKC_FREE(pSendComAll);

	if(0 != pthread_cond_destroy(&cond_val)){
		ERR_DKCPRN("Failed to destroy condition variable.");
		result = false;
	}
	if(0 != pthread_mutex_destroy(&cond_mutex)){
		ERR_DKCPRN("Failed to destroy condition mutex variable.");
		result = false;
	}
	return result;
}

bool K2hdkcCommand::CleanSlave(void)
{
	bool	result = true;
	if(IsCreateSlave && pSlaveObj){
		if(false == (result = pSlaveObj->Clean())){
			ERR_DKCPRN("Failed to uninitialize slave command object, but continue...");
		}
		DKC_DELETE(pSlaveObj);		// force
	}
	IsCreateSlave = false;			// force
	return result;
}

bool K2hdkcCommand::CommandProcessing(void)
{
	WAN_DKCPRN("This processing method is base common class. this method does nothing.");
	return true;
}

bool K2hdkcCommand::CommandProcess(void)
{
	IsNeedReply = true;

	// do processing
	bool	result;
	if(false == (result = CommandProcessing())){
		// this is command result, thus debug message level should be msg.
		MSG_DKCPRN("Processing returns failed.");
	}

	// do response
	if(!ReplyResponse()){
		ERR_DKCPRN("Failed sending response.");
		result = false;
	}
	return result;
}

bool K2hdkcCommand::CommandSend(void)
{
	if((IsServerNode && !pK2hObj) || !pChmObj || !pSendComAll || 0 == SendLength){
		ERR_DKCPRN("This object does not initialize yet.");
		return false;
	}

	// do sending
	if(!CommandSending()){
		ERR_DKCPRN("Failed sending command.");
		return false;
	}
	return true;
}

bool K2hdkcCommand::GetResponseData(PDKCCOM_ALL* ppcomall, size_t* plength, dkcres_type_t* prescode) const
{
	if(!pRcvComAll || 0 == RcvComLength){
		ERR_DKCPRN("There is no response(received) data.");
		return false;
	}
	if(DKC_NORESTYPE == pRcvComAll->com_head.restype){
		ERR_DKCPRN("Response(received) data is not received data type.");
		return false;
	}
	if(ppcomall){
		*ppcomall = pRcvComAll;
	}
	if(plength){
		*plength = RcvComLength;
	}
	if(prescode){
		*prescode = pRcvComAll->com_head.restype;
	}
	return true;
}

bool K2hdkcCommand::GetResponseCode(dkcres_type_t& rescode) const
{
	if(!pRcvComAll || 0 == RcvComLength){
		ERR_DKCPRN("There is no response(received) data.");
		return false;
	}
	if(DKC_NORESTYPE == pRcvComAll->com_head.restype){
		ERR_DKCPRN("Response(received) data is not received data type.");
		return false;
	}
	rescode = pRcvComAll->com_head.restype;
	return true;
}

bool K2hdkcCommand::SetPermanentSlave(K2hdkcSlave* pSlave)
{
	if(!pSlave){
		ERR_DKCPRN("Parameter is wrong.");
		return false;
	}
	if(IsCreateSlave){
		WAN_DKCPRN("This class has already created slave command object, but destroy it and set object.");
		CleanSlave();
	}
	pSlaveObj		= pSlave;
	IsCreateSlave	= false;

	return true;
}

//---------------------------------------------------------
// Methods - Set packets
//---------------------------------------------------------
//
// [NOTICE]
// pComPkt and pbody are freed in this class, so caller must not free these when returns success from this method.
//
bool K2hdkcCommand::SetReceiveData(PCOMPKT pComPkt, PDKCCOM_ALL pComAll)
{
	if(!pComPkt || !pComAll){
		ERR_DKCPRN("Parameter are wrong.");
		return false;
	}
	// logging
	// cppcheck-suppress literalWithCharPtrCompare
	COMLOG_PRN(GetDispComNumber(), GetComNumber(), "Received %s: %s(%" PRIu64 ")", (DKC_NORESTYPE == pComAll->com_head.restype ? "command" : "response"), STR_DKCCOM_TYPE(pComAll->com_head.comtype), pComAll->com_head.comtype);

	pRcvComPkt		= pComPkt;
	pRcvComAll		= pComAll;
	RcvComLength	= pComAll->com_head.length;

	// wakeup thread when waiting response
	if(IsServerNode && IsWaitResOnServer && K2hdkcCommand::WaitFp && K2hdkcCommand::UnWaitFp && !TriggerResponse){
		// notification flag
		TriggerResponse = true;

		int		condres;
		if(0 != (condres = pthread_mutex_lock(&cond_mutex))){
			ERR_DKCPRN("Could not lock mutex for condition by error code(%d).", condres);
			return true;						// [NOTICE]
		}
		if(0 != (condres = pthread_cond_broadcast(&cond_val))){				// Wakeup all thread
			ERR_DKCPRN("Could not signal cond(return code = %d).", condres);
			pthread_mutex_unlock(&cond_mutex);
			return true;						// [NOTICE]
		}
		pthread_mutex_unlock(&cond_mutex);
	}

	return true;
}

bool K2hdkcCommand::SetSendData(PDKCCOM_ALL pComAll, chmhash_t hash)
{
	if(!IS_SAFE_DKC_COM_ALL(pComAll)){
		return false;
	}
	DKC_FREE(pSendComAll);

	pSendComAll	= pComAll;
	SendLength	= pComAll->com_head.length;
	SendHash	= hash;

	return true;
}

//---------------------------------------------------------
// Methods - Response
//---------------------------------------------------------
bool K2hdkcCommand::SetResponseData(PDKCCOM_ALL pComAll)
{
	if(!pRcvComPkt){
		ERR_DKCPRN("Command ComPkt is NULL, so must set receive data before setting response data.");
		return false;
	}
	return SetSendData(pComAll, 0L);		// Do not care of hash value for response
}

bool K2hdkcCommand::SetErrorResponseData(dkcres_type_t subcode, dkcres_type_t errcode)
{
	PDKCCOM_ALL	pResAll = MakeResponseOnlyHeadData(subcode, errcode);
	if(!pResAll){
		//MSG_DKCPRN("Failed to make error response data, so could not set response data.");
		return false;
	}
	return SetResponseData(pResAll);
}

PDKCCOM_ALL K2hdkcCommand::MakeResponseOnlyHeadData(dkcres_type_t subcode, dkcres_type_t errcode)
{
	WAN_DKCPRN("This make error response data method is base common class. this method does nothing.");
	return NULL;
}

bool K2hdkcCommand::ReplyResponse(void)
{
	if(!pChmObj || !pRcvComPkt){
		//MSG_DKCPRN("Chmpx object or Command ComPkt is NULL, so could not send response.");
		return true;
	}
	if(!pSendComAll || 0 == SendLength){
		//MSG_DKCPRN("Nothing to response data, so return with nothing to do.");
		return true;
	}
	if(!IsNeedReply){
		//MSG_DKCPRN("Already reply response");
		return true;
	}
	IsNeedReply = false;

	if(!pChmObj->Reply(pRcvComPkt, reinterpret_cast<unsigned char*>(pSendComAll), SendLength)){
		ERR_DKCPRN("Failed to reply response.");
		// cppcheck-suppress literalWithCharPtrCompare
		COMLOG_DMP_PRN(GetDispComNumber(), GetComNumber(), "Failed to Reply command: %s(%" PRIu64 ")", STR_DKCCOM_TYPE(pSendComAll->com_head.comtype), pSendComAll->com_head.comtype);
		return false;
	}
	// cppcheck-suppress literalWithCharPtrCompare
	COMLOG_DMP_PRN(GetDispComNumber(), GetComNumber(), "Replied command: %s(%" PRIu64 ")", STR_DKCCOM_TYPE(pSendComAll->com_head.comtype), pSendComAll->com_head.comtype);

	return true;
}

//---------------------------------------------------------
// Methods - Send/Replicate
//---------------------------------------------------------
bool K2hdkcCommand::CommandSending(void)
{
	if(!pChmObj){
		ERR_DKCPRN("Chmpx object is NULL, so could not send command.");
		return false;
	}
	if(!pSendComAll || 0 == SendLength){
		ERR_DKCPRN("There is no send data, the data is not set yet.");
		return false;
	}
	if(!IsServerNode && CHM_INVALID_MSGID == SendMsgid){
		ERR_DKCPRN("Could not send command, because this process is on slave node but do not set msgid.");
		return false;
	}

	// dump
	DumpComAll("Send", pSendComAll);

	// Set callback before sending(a case of on server node)
	if(IsWaitResOnServer && K2hdkcCommand::WaitFp && K2hdkcCommand::UnWaitFp){
		// set waiting flag for condition
		TriggerResponse = false;

		// set callback
		if(!K2hdkcCommand::WaitFp(pSendComAll->com_head.comnumber, this, K2hdkcCommand::pWaitFpParam)){
			// cppcheck-suppress literalWithCharPtrCompare
			ERR_DKCPRN("Failed to set receive waiting callback for type(%s).", STR_DKCCOM_TYPE(pSendComAll->com_head.comtype));
			TriggerResponse = true;			// reset
			return false;
		}
	}

	// send command
	bool	result;
	if(IsServerNode){
		// send all server without self chmpx.
		//
		if(ReplicateSend){
			result = pChmObj->Replicate(reinterpret_cast<unsigned char*>(pSendComAll), SendLength, SendHash, WithoutSelfOnServer);			// if replication, should be without self
		}else{
			result = pChmObj->Send(reinterpret_cast<unsigned char*>(pSendComAll), SendLength, SendHash, RoutingOnServer, WithoutSelfOnServer);
		}
	}else{
		// send only one server.(use not routing mode)
		if(ReplicateSend){
			result = pChmObj->Replicate(SendMsgid, reinterpret_cast<unsigned char*>(pSendComAll), SendLength, SendHash);
		}else{
			result = pChmObj->Send(SendMsgid, reinterpret_cast<unsigned char*>(pSendComAll), SendLength, SendHash, NULL, RoutingOnSlave);
		}
	}
	// cppcheck-suppress literalWithCharPtrCompare
	COMLOG_PRN(GetDispComNumber(), GetComNumber(), "Sent command: %s(%" PRIu64 ")", STR_DKCCOM_TYPE(pSendComAll->com_head.comtype), pSendComAll->com_head.comtype);

	if(!result){
		// cppcheck-suppress literalWithCharPtrCompare
		ERR_DKCPRN("Failed to send command for type(%s).", STR_DKCCOM_TYPE(pSendComAll->com_head.comtype));

		// Unset callback(a case of on server node)
		if(IsWaitResOnServer && K2hdkcCommand::WaitFp){
			// unset callback
			TriggerResponse = true;			// reset
			if(!K2hdkcCommand::UnWaitFp(pSendComAll->com_head.comnumber, this, K2hdkcCommand::pWaitFpParam)){
				// cppcheck-suppress literalWithCharPtrCompare
				ERR_DKCPRN("Failed to unset receive waiting callback for type(%s).", STR_DKCCOM_TYPE(pSendComAll->com_head.comtype));
			}
		}
		return false;
	}

	// receive command result
	//
	// [NOTE]
	// if the departure chmpx is server node, the command type must not need to receive
	// the response here. because the sending method for server node is not used msgid,
	// thus it uses common msgid in chmpx.
	// then you are going to receive the response in MAIN LOOP.
	//
	if(!IsServerNode){
		// a case of on slave node
		PCOMPKT			pComPkt	= NULL;
		unsigned char*	pbody	= NULL;
		size_t			length	= 0;
		if(false == (result = pChmObj->Receive(SendMsgid, &pComPkt, &pbody, &length, K2hdkcCommand::RcvTimeout)) || !pComPkt || !pbody || 0 == length){
			// cppcheck-suppress literalWithCharPtrCompare
			ERR_DKCPRN("Failed to receive command response for type(%s) : pComPkt(%p), pbody(%p), length(%zu).", STR_DKCCOM_TYPE(pSendComAll->com_head.comtype), pComPkt, pbody, length);
			DKC_FREE(pComPkt);
			DKC_FREE(pbody);
			return false;
		}
		// cppcheck-suppress literalWithCharPtrCompare
		DMP_DKCPRN("Succeed to receive command response for type(%s) : pComPkt(%p), pbody(%p), length(%zu).", STR_DKCCOM_TYPE(pSendComAll->com_head.comtype), pComPkt, pbody, length);

		DKC_FREE(pRcvComAll);
		DKC_FREE(pRcvComPkt);
		pRcvComAll	= reinterpret_cast<PDKCCOM_ALL>(pbody);
		pRcvComPkt	= pComPkt;
		RcvComLength= length;

		// if has slave command object, set response code into it.
		if(pSlaveObj){
			if(!pSlaveObj->SetResponseCode(pRcvComAll->com_head.restype)){
				ERR_DKCPRN("Failed to set response code(%s - %s(0x%016" PRIx64 ")) to slave command object, but continue...", STR_DKCRES_RESULT_TYPE(pRcvComAll->com_head.restype), STR_DKCRES_SUBCODE_TYPE(pRcvComAll->com_head.restype), pRcvComAll->com_head.restype);
			}
		}

		// dump
		DumpComAll("Receive", pRcvComAll);

	}else if(IsWaitResOnServer && K2hdkcCommand::WaitFp && K2hdkcCommand::UnWaitFp){
		// wait response
		struct timespec	abstimeout;
		while(!TriggerResponse){
			make_current_timespec_with_mergin((K2hdkcCommand::RcvTimeout * 1000 * 1000), abstimeout);		// convert RcvTimeout(ms) to ns

			// wait condition
			int	condres;
			if(0 != (condres = pthread_mutex_lock(&cond_mutex))){
				// cppcheck-suppress literalWithCharPtrCompare
				ERR_DKCPRN("Could not lock mutex for condition by error code(%d), for command type(%s).", condres, STR_DKCCOM_TYPE(pSendComAll->com_head.comtype));
				TriggerResponse = true;			// reset
				result			= false;		// error

			}else{
				condres			= pthread_cond_timedwait(&cond_val, &cond_mutex, &abstimeout);
				int	unlockres	= pthread_mutex_unlock(&cond_mutex);
				if(0 != unlockres){
					// cppcheck-suppress literalWithCharPtrCompare
					WAN_DKCPRN("Could not unlock mutex for condition by error code(%d), for command type(%s), but continue...", unlockres, STR_DKCCOM_TYPE(pSendComAll->com_head.comtype));
				}

				// check condition result
				if(ETIMEDOUT == condres){
					// timeouted
					// cppcheck-suppress literalWithCharPtrCompare
					ERR_DKCPRN("Could not get response for command type(%s) by timeout.", STR_DKCCOM_TYPE(pSendComAll->com_head.comtype));
					TriggerResponse	= true;		// reset
					result			= false;	// error

				}else if(EINTR == condres){
					// signal break
					// cppcheck-suppress literalWithCharPtrCompare
					MSG_DKCPRN("Could not get response for command type(%s) by signal, so retry to wait.", STR_DKCCOM_TYPE(pSendComAll->com_head.comtype));

				}else if(0 != condres){
					// success
					TriggerResponse = true;		// reset
					result			= true;
				}
			}
		}
	}
	return result;
}

bool K2hdkcCommand::CommandReplicate(const unsigned char* pkey, size_t keylength, const struct timespec ts)
{
	if(!pK2hObj || !pChmObj){
		ERR_DKCPRN("K2hash and Chmpx object are NULL, so could not send replicate command.");
		return false;
	}
	if(!IsServerNode){
		MSG_DKCPRN("Called replicate command from client on slave node, this method must be called from on server node.");
		return false;
	}

	// get replication datas which are binary
	ssize_t			vallength	= 0;
	unsigned char*	pval		= NULL;
	if(0 > (vallength = pK2hObj->Get(pkey, keylength, &pval, false, NULL)) || !pval){		// not check attributes and no encrypt pass
		MSG_DKCPRN("There is no value for key(%s).", bin_to_string(pkey, keylength).c_str());
		DKC_FREE(pval);
		vallength = 0;
	}
	K2HSubKeys*		pSKeysObj		= NULL;
	size_t			subkeyslength	= 0;
	unsigned char*	psubkeys		= NULL;
	if(NULL == (pSKeysObj = pK2hObj->GetSubKeys(pkey, keylength, false))){					// not check attributes and no encrypt pass
		MSG_DKCPRN("There is no subkeys list for key(%s).", bin_to_string(pkey, keylength).c_str());
	}else{
		if(!pSKeysObj->Serialize(&psubkeys, subkeyslength)){
			ERR_DKCPRN("Failed to convert subkeys list to binary data for key(%s), but continue...", bin_to_string(pkey, keylength).c_str());
		}
		DKC_DELETE(pSKeysObj);
	}
	K2HAttrs* 		pAttrsObj	= NULL;
	size_t			attrslength	= 0;
	unsigned char*	pattrs		= NULL;
	if(NULL == (pAttrsObj = pK2hObj->GetAttrs(pkey, keylength))){
		MSG_DKCPRN("There is no attributes for key(%s).", bin_to_string(pkey, keylength).c_str());
	}else{
		if(!pAttrsObj->Serialize(&pattrs, attrslength)){
			ERR_DKCPRN("Failed to convert attributes to binary data for key(%s), but continue...", bin_to_string(pkey, keylength).c_str());
		}
		DKC_DELETE(pAttrsObj);
	}

	// hash value made by same logic in k2hash
	k2h_hash_t	hash	= K2H_HASH_FUNC(reinterpret_cast<const void*>(pkey), keylength);
	k2h_hash_t	subhash	= K2H_2ND_HASH_FUNC(reinterpret_cast<const void*>(pkey), keylength);

	// Replicate Command object
	K2hdkcComReplKey*	pReplKey = GetCommonK2hdkcComReplKey(pK2hObj, pChmObj, SendMsgid, GetDispComNumber(), true, true, false);	// [NOTICE] always without self! and not wait response
	bool				result;
	if(false == (result = pReplKey->CommandSend(hash, subhash, pkey, keylength, pval, vallength, psubkeys, subkeyslength, pattrs, attrslength, ts))){
		ERR_DKCPRN("Failed to replicate command for key(%s).", bin_to_string(pkey, keylength).c_str());
	}
	DKC_FREE(pval);
	DKC_FREE(psubkeys);
	DKC_FREE(pattrs);
	DKC_DELETE(pReplKey);

	// [NOTE]
	// This replicate command is sent on server node, so this response is received in event loop.
	// Then we do not get the response here.
	//
	return result;
}

//---------------------------------------------------------
// Methods - Dump
//---------------------------------------------------------
void K2hdkcCommand::Dump(void) const
{
	if(IS_DKCDBG_DUMP()){
		DumpComAll("Receive", pRcvComAll);
		DumpComAll("Send", pSendComAll);
	}
}

void K2hdkcCommand::DumpComAll(const char* pprefix, const PDKCCOM_ALL pComAll) const
{
	DMP_DKCPRN("%s: %s DKCCOM_ALL(%p) = {", GetClassName(), DKCEMPTYSTR(pprefix) ? "" : pprefix, pComAll);
	if(!pComAll){
		return;
	}

	// print head
	RAW_DKCPRN("  DKCCOM_HEAD = {");
	// cppcheck-suppress literalWithCharPtrCompare
	RAW_DKCPRN("    comtype       = %s(0x%016" PRIx64 ")",	STR_DKCCOM_TYPE(pComAll->com_head.comtype), pComAll->com_head.comtype);
	RAW_DKCPRN("    restype       = RESULT:0x%08" PRIx32 ", SUBCODE:0x%08" PRIx32 " (0x%016" PRIx64 ")", static_cast<uint32_t>(GET_DKC_RES_RESULT(pComAll->com_head.restype)), static_cast<uint32_t>(GET_DKC_RES_SUBCODE(pComAll->com_head.restype)), pComAll->com_head.restype);
	RAW_DKCPRN("    comnumber     = 0x%016" PRIx64 , 		pComAll->com_head.comnumber);
	RAW_DKCPRN("    dispcomnumber = 0x%016" PRIx64 , 		pComAll->com_head.dispcomnumber);
	RAW_DKCPRN("    length        = %zu byte",				pComAll->com_head.length);
	RAW_DKCPRN("  }");

	// print content of comall
	RawDumpComAll(pComAll);

	RAW_DKCPRN("}");
}

void K2hdkcCommand::RawDumpComAll(const PDKCCOM_ALL pComAll) const
{
	// nothing to print
	return;
}

/*
 * VIM modelines
 *
 * vim:set ts=4 fenc=utf-8:
 */
