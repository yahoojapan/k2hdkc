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
 * CREATE:   Mon Aug 8 2016
 * REVISION:
 * 
 */

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

#include "k2hdkccommon.h"
#include "k2hdkcslave.h"
#include "k2hdkccombase.h"
#include "k2hdkcdbg.h"

using namespace	std;

//---------------------------------------------------------
// Class Methods - Get Command object
//---------------------------------------------------------
K2hdkcCommand* K2hdkcSlave::GetSlaveCommandSendObject(dkccom_type_t comtype, K2hdkcSlave* pslaveobj, uint64_t comnum)
{
	if(!pslaveobj || !IS_SAFE_DKCCOM_TYPE(comtype)){
		ERR_DKCPRN("Parameter are wrong.");
		return NULL;
	}
	if(!pslaveobj->IsInitialize() || !pslaveobj->IsOpen()){
		ERR_DKCPRN("Slave command object does not initialize or open msgid, yet.");
		return NULL;
	}

	// create command object(do not set slave command object)
	K2hdkcCommand*	pComObj = K2hdkcCommand::GetCommandSendObject(NULL, pslaveobj->GetChmCntrlObject(), comtype, pslaveobj->GetMsgid(), comnum);
	if(!pComObj){
		ERR_DKCPRN("Could not create command object for slave command.");
		return NULL;
	}
	if(!pComObj->SetPermanentSlave(pslaveobj)){
		WAN_DKCPRN("Failed to Slave command object to command object, but continue...");
	}
	return pComObj;
}

//---------------------------------------------------------
// Constructor/Destructor
//---------------------------------------------------------
K2hdkcSlave::K2hdkcSlave(void) : chmpxconfig(""), chmpxctlport(CHM_INVALID_PORT), chmpxautorejoin(false), msgid(CHM_INVALID_MSGID), lastrescode(DKC_NORESTYPE)
{
}

K2hdkcSlave::~K2hdkcSlave(void)
{
	Clean();
}

//---------------------------------------------------------
// Methods
//---------------------------------------------------------
bool K2hdkcSlave::Initialize(const char* config, short ctlport, bool is_auto_rejoin)
{
	if(DKCEMPTYSTR(config)){
		MSG_DKCPRN("config parameter is empty, thus using environment.");
	}
	if(IsInitialize()){
		ERR_DKCPRN("This object already initialized, thus uninitialize it.");
		if(!Clean()){
			return false;
		}
	}

	// join chmpx
	if(!chmobj.InitializeOnSlave(config, is_auto_rejoin, ctlport)){
		ERR_DKCPRN("Could not initialize slave chmpx object.");
		return false;
	}
	chmpxconfig		= config;
	chmpxctlport	= ctlport;
	chmpxautorejoin	= is_auto_rejoin;
	msgid			= CHM_INVALID_MSGID;
	return true;
}

bool K2hdkcSlave::Clean(bool is_clean_bup)
{
	if(!IsInitialize()){
		MSG_DKCPRN("This object does not initialized yet, so no not need to clean.");
		return true;
	}
	if(IsOpen()){
		// has opened msgid, so close it.
		if(!Close()){
			ERR_DKCPRN("Could not close opened msgid, but continue...");
		}
	}
	if(!chmobj.Clean(is_clean_bup)){
		ERR_DKCPRN("Failed to clean chmpx");
		return false;
	}
	chmpxconfig		= "";
	chmpxctlport	= CHM_INVALID_PORT;
	chmpxautorejoin	= false;
	msgid			= CHM_INVALID_MSGID;
	return true;
}

bool K2hdkcSlave::Open(bool no_giveup_rejoin)
{
	if(!IsInitialize()){
		ERR_DKCPRN("This object does not initialized yet.");
		return false;
	}
	if(IsOpen()){
		// already open, but close it for reopen.
		if(!Close()){
			ERR_DKCPRN("Could not close opened msgid,");
			return false;
		}
	}
	if(CHM_INVALID_MSGID == (msgid = chmobj.Open(no_giveup_rejoin))){
		ERR_DKCPRN("Could not open msgid on slave chmpx.");
		return false;
	}
	return true;
}

bool K2hdkcSlave::Close(void)
{
	if(CHM_INVALID_MSGID == msgid){
		WAN_DKCPRN("Already close msgid.");
		return true;
	}
	if(!chmobj.Close(msgid)){
		ERR_DKCPRN("Failed to close msgid(0x%016" PRIx64 ") on slave chmpx", msgid);
		return false;
	}
	msgid = CHM_INVALID_MSGID;

	return true;
}

//---------------------------------------------------------
// Methods - Response code
//---------------------------------------------------------
bool K2hdkcSlave::SetResponseCode(dkcres_type_t rescode)
{
	if(DKC_NORESTYPE == rescode){
		ERR_DKCPRN("Parameter is wrong.");
		return false;
	}
	lastrescode = rescode;
	return true;
}

bool K2hdkcSlave::GetResponseCode(dkcres_type_t& rescode) const
{
	if(DKC_NORESTYPE == lastrescode){
		ERR_DKCPRN("There is no last response code.");
		return false;
	}
	rescode = lastrescode;
	return true;
}


/*
 * VIM modelines
 *
 * vim:set ts=4 fenc=utf-8:
 */
