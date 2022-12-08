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
 * CREATE:   Mon Jul 11 2016
 * REVISION:
 * 
 */

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

#include <fullock/fullock.h>

#include "k2hdkccommon.h"
#include "k2hdkccntrl.h"
#include "k2hdkccombase.h"
#include "k2hdkcutil.h"
#include "k2hdkcdbg.h"

using namespace	std;

//---------------------------------------------------------
// Class Variable
//---------------------------------------------------------
const int	K2hdkcCntrl::RECEIVE_WAIT_MS;
K2hdkcCntrl	K2hdkcCntrl::singleton;

//---------------------------------------------------------
// Class Methods
//---------------------------------------------------------
void K2hdkcCntrl::SigUsr1handler(int signum)
{
	if(SIGUSR1 == signum){
		// Bumpup debug level.
		BumpupK2hdkcDbgMode();

		K2hdkcDbgMode	chmdbgmode	= GetK2hdkcDbgMode();
		string			strmode		= K2hdkcDbgMode_STR(chmdbgmode);
		cout << "MESSAGE: Caught signal SIGUSR1, bumpup the logging level to " << strmode << "." << endl;

		// If debug level is not same k2hdkc and k2hash, then it should be
		// set same level.
		if(DKCDBG_SILENT == chmdbgmode){
			k2h_set_debug_level_silent();
		}else if(DKCDBG_ERR == chmdbgmode){
			k2h_set_debug_level_error();
		}else if(DKCDBG_WARN == chmdbgmode){
			k2h_set_debug_level_warning();
		}else if(DKCDBG_MSG == chmdbgmode){
			k2h_set_debug_level_message();
		}else if(DKCDBG_DUMP == chmdbgmode){
			k2h_set_debug_level_message();		// k2hash does not have dump mode
		}
	}
}

void K2hdkcCntrl::SigUsr2handler(int signum)
{
	if(SIGUSR2 == signum){
		// Toggle communication debug mode.
		if(K2hdkcComNumber::IsEnable()){
			K2hdkcComNumber::Disable();
		}else{
			K2hdkcComNumber::Enable();
		}
	}
}

void K2hdkcCntrl::SigHupHandler(int signum)
{
	if(SIGHUP == signum){
		// Same as SigExitHandler for break loop
		K2hdkcCntrl::Get()->is_loop = false;
	}
}

void K2hdkcCntrl::SigExitHandler(int signum)
{
	if(SIGTERM == signum || SIGINT == signum){
		// break loop
		K2hdkcCntrl::Get()->is_loop = false;
	}
}

bool K2hdkcCntrl::SetSignalHandler(int signum, void (*handler)(int))
{
	// set handler
	struct sigaction	sa;
	sa.sa_flags			= 0;
	sa.sa_handler		= handler;
	if(0 != sigemptyset(&sa.sa_mask)){
		ERR_DKCPRN("Failed to clear to sigset.");
		return false;
	}
	if(0 != sigaddset(&sa.sa_mask, signum)){
		ERR_DKCPRN("Failed to set signal %d", signum);
		return false;
	}
	if(0 > sigaction(signum, &sa, NULL)){
		ERR_DKCPRN("Could not set signal %d handler by errno=%d", signum, errno);
		return false;
	}
	return true;
}

bool K2hdkcCntrl::InitializeSignal(void)
{
	// set procmask
	sigset_t	sigset;
	if(0 != sigfillset(&sigset)){
		ERR_DKCPRN("Failed to fill to sigset.");
		return false;
	}
	if(0 != sigdelset(&sigset, SIGUSR1) || 0 != sigdelset(&sigset, SIGHUP) || 0 != sigdelset(&sigset, SIGTERM) || 0 != sigdelset(&sigset, SIGINT)){
		ERR_DKCPRN("Failed to unset signal SIGUSR1, SIGHUP, SIGTERM, SIGINT");
		return false;
	}
	if(0 != sigprocmask(SIG_SETMASK, &sigset, NULL)){
		ERR_DKCPRN("Failed to set signal proc mask(errno=%d).", errno);
		return false;
	}

	if(	!K2hdkcCntrl::SetSignalHandler(SIGUSR1,	K2hdkcCntrl::SigUsr1handler)	||
		!K2hdkcCntrl::SetSignalHandler(SIGUSR2,	K2hdkcCntrl::SigUsr2handler)	||
		!K2hdkcCntrl::SetSignalHandler(SIGHUP,	K2hdkcCntrl::SigHupHandler)		||
		!K2hdkcCntrl::SetSignalHandler(SIGTERM,	K2hdkcCntrl::SigExitHandler)	||
		!K2hdkcCntrl::SetSignalHandler(SIGINT,	K2hdkcCntrl::SigExitHandler)	)
	{
		ERR_DKCPRN("Could not set signal SIGUSR1, SIGHUP, SIGTERM, SIGINT handlers");
		return false;
	}
	return true;
}

//---------------------------------------------------------
// Class Methods - Callback
//---------------------------------------------------------
bool K2hdkcCntrl::MergeLastTsCallback(chmpx_h handle, struct timespec* pts)
{
	if(NULL == reinterpret_cast<void*>(handle) || !pts){
		ERR_DKCPRN("Parameter are wrong.");
		return false;
	}
	K2hdkcCntrl*	pCntrl = K2hdkcCntrl::Get();

	if(!pCntrl->k2hobj.IsAttached()){
		ERR_DKCPRN("K2hash does not attach.");
		return false;
	}

	struct timeval	tv;
	if(!pCntrl->k2hobj.GetUpdateTimeval(tv)){
		ERR_DKCPRN("Could not get last update time(timeval) from k2hash.");
		return false;
	}
	pts->tv_sec	= tv.tv_sec;
	pts->tv_nsec= tv.tv_usec * 1000;

	return true;
}

bool K2hdkcCntrl::MergeGetCallback(chmpx_h handle, const PCHM_MERGE_GETPARAM pparam, chmhash_t* pnexthash, PCHMBIN* ppdatas, size_t* pdatacnt)
{
	if(NULL == reinterpret_cast<void*>(handle) || !pparam || !pnexthash || !ppdatas || !pdatacnt){
		ERR_DKCPRN("Parameter are wrong.");
		return false;
	}

	K2hdkcCntrl*	pCntrl = K2hdkcCntrl::Get();
	if(!pCntrl->k2hobj.GetElementsByHash(pparam->starthash, pparam->startts, pparam->endts, pparam->target_hash, pparam->target_max_hash, pparam->old_hash, pparam->old_max_hash, pparam->target_hash_range, pparam->is_expire_check, pnexthash, ppdatas, pdatacnt)){
		ERR_DKCPRN("Failed to get merge data from k2hash.");
		return false;
	}
	return true;
}

bool K2hdkcCntrl::MergeSetCallback(chmpx_h handle, size_t length, const unsigned char* pdata, const struct timespec* pts)
{
	if(NULL == reinterpret_cast<void*>(handle) || !pts){
		ERR_DKCPRN("Parameter are wrong.");
		return false;
	}

	if(0 < length && pdata){
		K2hdkcCntrl*	pCntrl = K2hdkcCntrl::Get();

		if(!pCntrl->k2hobj.SetElementByBinArray(reinterpret_cast<PRALLEDATA>(const_cast<unsigned char*>(pdata)), pts)){
			ERR_DKCPRN("Failed to set merge data to k2hash.");
			return false;
		}
	}else{
		WAN_DKCPRN("merge set data is empty, so skip this and continue...");
	}
	return true;
}

//---------------------------------------------------------
// Class Methods - Callback
//---------------------------------------------------------
bool K2hdkcCntrl::K2hdkcCommandReceiveWaitSetFunc(uint64_t comnum, K2hdkcCommand* pobj, void* pparam)
{
	if(!pobj || !pparam){
		ERR_DKCPRN("Parameters are wrong, probably failed to initialize.");
		return false;
	}
	K2hdkcCntrl*	pCntlObj = reinterpret_cast<K2hdkcCntrl*>(pparam);

	if(!pCntlObj->SetWaitCommandObject(comnum, pobj)){
		ERR_DKCPRN("Failed to add Command Object(%s) to waiting list for waiting response.", STR_DKCCOM_TYPE(pobj->GetCommandType()));
		return false;
	}
	return true;
}

bool K2hdkcCntrl::K2hdkcCommandReceiveWaitUnsetFunc(uint64_t comnum, K2hdkcCommand* pobj, void* pparam)
{
	if(!pobj || !pparam){
		ERR_DKCPRN("Parameters are wrong, probably failed to initialize.");
		return false;
	}
	K2hdkcCntrl*	pCntlObj = reinterpret_cast<K2hdkcCntrl*>(pparam);

	if(!pCntlObj->UnsetWaitCommandObject(comnum, pobj)){
		ERR_DKCPRN("Failed to remove Command Object(%s) to waiting list for waiting response.", STR_DKCCOM_TYPE(pobj->GetCommandType()));
		return false;
	}
	return true;
}

//---------------------------------------------------------
// Class Methods - Processing Callback
//---------------------------------------------------------
bool K2hdkcCntrl::WorkerProc(void* pobj, bool* piswork)
{
	if(!pobj || !piswork){
		ERR_DKCPRN("Parameter are wrong.");
		return false;									// exit thread
	}
	K2hdkcCntrl*	pCntrlObj	= reinterpret_cast<K2hdkcCntrl*>(pobj);

	// loop as long as data
	bool			is_work		= false;
	for(is_work = false; true; is_work = true){
		// retrieve parameter
		PDKC_RCVPROC_DATA	pRcvData = NULL;
		while(!fullock::flck_trylock_noshared_mutex(&(pCntrlObj->procdatas_lockval)));	// MUTEX LOCK
		if(!pCntrlObj->rcvprocdatas.empty()){
			pRcvData = pCntrlObj->rcvprocdatas.front();
			pCntrlObj->rcvprocdatas.erase(pCntrlObj->rcvprocdatas.begin());
		}
		fullock::flck_unlock_noshared_mutex(&(pCntrlObj->procdatas_lockval));			// MUTEX UNLOCK

		// check parameter
		if(!pRcvData){
			//MSG_DKCPRN("Nothing to do working because there is no stacked data.");
			break;
		}
		if(!pRcvData->pComPkt || !pRcvData->pbody || 0 == pRcvData->length){
			ERR_DKCPRN("Retrieved receiving data from stack has wrong empty pointers...");
			CHM_Free(pRcvData->pComPkt);
			CHM_Free(pRcvData->pbody);
			DKC_DELETE(pRcvData)
			break;
		}

		// dispatch processing
		K2hdkcCommand*	pComObj = K2hdkcCommand::GetCommandReceiveObject(&(pCntrlObj->k2hobj), &(pCntrlObj->chmobj), pRcvData->pComPkt, pRcvData->pbody, pRcvData->length);
		if(!pComObj){
			ERR_DKCPRN("Could not get command object, so this case does not continue to process. so remove this received data...");
			CHM_Free(pRcvData->pComPkt);
			CHM_Free(pRcvData->pbody);
			DKC_DELETE(pRcvData)
			break;
		}

		// processing
		if(false == pComObj->CommandProcess()){
			// this is command result, thus debug message level should be msg.
			MSG_DKCPRN("Failed processing for received data.");
		}
		DKC_DELETE(pComObj);
	}
	*piswork = is_work;
	return true;										// continue thread
}

//---------------------------------------------------------
// Constructor/Destructor
//---------------------------------------------------------
K2hdkcCntrl::K2hdkcCntrl(void) : ThreadManager(this), chmpxctlport(CHM_INVALID_PORT), chmpxcuk(""), no_giveup_rejoin(false), is_loop(false), waitmap_lockval(FLCK_NOSHARED_MUTEX_VAL_UNLOCKED), procdatas_lockval(FLCK_NOSHARED_MUTEX_VAL_UNLOCKED)
{
}

K2hdkcCntrl::~K2hdkcCntrl(void)
{
	Clean();
}

//---------------------------------------------------------
// Methods
//---------------------------------------------------------
bool K2hdkcCntrl::Initialize(const K2hdkcConfig* pk2hdkcconf, short ctlport, const char* cuk, bool nogiveuprejoin)
{
	if(!pk2hdkcconf || !pk2hdkcconf->IsLoaded()){
		ERR_DKCPRN("Parameter is wrong or does not load configuration file.");
		return false;
	}

	// Set signal
	if(!K2hdkcCntrl::InitializeSignal()){
		ERR_DKCPRN("Could not set signal procmask, handler, etc...");
		return false;
	}

	// Initialize k2hash
	if(!pk2hdkcconf->InitializeK2hash(k2hobj)){
		ERR_DKCPRN("Failed to initialize(create or attach) k2hash.");
		return false;
	}

	// get chmpx configuration
	string	ChmpxConfig;
	if(!pk2hdkcconf->GetServerNodeConfiguration(ChmpxConfig)){
		ERR_DKCPRN("Could not find server node configuration.");
		k2hobj.Detach();
		return false;
	}

	// check chmpx MQ ACK mode
	{
		// initialize for duplicated chmpx information
		ChmCntrl	chmtestobj;
		if(!chmtestobj.OnlyAttachInitialize(ChmpxConfig.c_str(), ctlport, (DKCEMPTYSTR(cuk) ? NULL : cuk))){
			ERR_DKCPRN("Could not initialize(attach) chmpx shared memory for testing.");
			k2hobj.Detach();
			return false;
		}
		// get information
		PCHMINFOEX	pAllInfo = chmtestobj.DupAllChmInfo();
		if(!pAllInfo || !pAllInfo->pchminfo){
			ERR_DKCPRN("Something error occurred in getting chmpx information for testing.");
			if(pAllInfo){
				ChmCntrl::FreeDupAllChmInfo(pAllInfo);
			}
			chmtestobj.Clean();
			k2hobj.Detach();
			return false;
		}
		if(pAllInfo->pchminfo->mq_ack){
			ERR_DKCPRN("Chmpx MQ Ack mode is true(on), MQ Ack mode must be OFF(FALSE) for k2hdkc process.");
			ChmCntrl::FreeDupAllChmInfo(pAllInfo);
			chmtestobj.Clean();
			k2hobj.Detach();
			return false;
		}
		MSG_DKCPRN("Chmpx MQ Ack mode is false(off).");
		ChmCntrl::FreeDupAllChmInfo(pAllInfo);
		chmtestobj.Clean();
	}

	// set rejoin mode
	no_giveup_rejoin = nogiveuprejoin;

	// join chmpx
	if(!chmobj.InitializeOnServer(ChmpxConfig.c_str(), true, K2hdkcCntrl::MergeGetCallback, K2hdkcCntrl::MergeSetCallback, K2hdkcCntrl::MergeLastTsCallback, ctlport, (DKCEMPTYSTR(cuk) ? NULL : cuk))){
		ERR_DKCPRN("Could not initialize chmpx object.");
		k2hobj.Detach();
		return false;
	}
	chmpxctlport= ctlport;
	chmpxcuk	= DKCEMPTYSTR(cuk) ? "" : cuk;
	is_loop		= false;

	// set timeout for receiving command
	long	rcvtimeout;
	if(0 < (rcvtimeout = pk2hdkcconf->GetReceiveTimeoutMs())){
		K2hdkcCommand::SetReceiveTimeout(rcvtimeout);
	}

	// set command response for receiving callback
	K2hdkcCommand::SetReceiveWaitFunc(K2hdkcCntrl::K2hdkcCommandReceiveWaitSetFunc, K2hdkcCntrl::K2hdkcCommandReceiveWaitUnsetFunc, this);

	// Initialize processing thread object
	if(!ThreadManager.Initialize(pk2hdkcconf->GetMinThreadCount(), pk2hdkcconf->GetMaxThreadCount(), pk2hdkcconf->GetReduceThreadTime())){
		ERR_DKCPRN("Could not initialize processing thread object.");
		k2hobj.Detach();
		chmobj.Clean();
		chmpxcuk.clear();
		chmpxctlport = CHM_INVALID_PORT;
		return false;
	}

	return true;
}

bool K2hdkcCntrl::Clean(void)
{
	ThreadManager.Clean();

	if(k2hobj.IsAttached()){
		k2hobj.Detach();
	}
	chmobj.Clean();
	chmpxcuk.clear();
	chmpxctlport = CHM_INVALID_PORT;

	// cleanup rest data
	while(!fullock::flck_trylock_noshared_mutex(&procdatas_lockval));	// MUTEX LOCK

	for(k2hdkcrcvprocdata_t::iterator iter = rcvprocdatas.begin(); iter != rcvprocdatas.end(); ++iter){
		if(*iter){
			CHM_Free((*iter)->pComPkt);
			CHM_Free((*iter)->pbody);
		}
		DKC_DELETE((*iter));
	}
	rcvprocdatas.clear();

	fullock::flck_unlock_noshared_mutex(&procdatas_lockval);			// MUTEX UNLOCK

	return true;
}

//---------------------------------------------------------
// Methods - Main Loop
//---------------------------------------------------------
bool K2hdkcCntrl::Run(void)
{
	// Create processing thread
	if(0 >= ThreadManager.CreateThreads(K2hdkcCntrl::WorkerProc)){
		ERR_DKCPRN("Could not run threads for processing.");
		return false;
	}

	MSG_DKCPRN("Start to loop main.");
	for(is_loop = true; is_loop; ){
		PCOMPKT			pComPkt;
		unsigned char*	pbody;
		size_t			length;

		// receive (block until receiving any command)
		pComPkt	= NULL;
		pbody	= NULL;
		length	= 0;
		if(!chmobj.Receive(&pComPkt, &pbody, &length, K2hdkcCntrl::RECEIVE_WAIT_MS, no_giveup_rejoin)){
			// [NOTE]
			// if no_giveup_rejoin is false(default) and chmpx id down, come here at a high frequency!!
			//
			//WAN_DKCPRN("Could not receive command on server node.");
			continue;
		}

		if(pbody && 0 < length){
			// get command type and number
			dkccom_type_t	comtype	= DKC_COM_UNKNOWN;
			dkcres_type_t	restype	= DKC_NORESTYPE;
			uint64_t		comnum	= K2hdkcComNumber::INIT_NUMBER;
			uint64_t		dispnum	= K2hdkcComNumber::INIT_NUMBER;
			if(!K2hdkcCommand::GetResponseCommandType(pbody, length, comtype, restype, comnum, dispnum)){
				ERR_DKCPRN("Received response is not safe.");
				continue;
			}

			// check waiting object
			K2hdkcCommand*	pComObj = NULL;
			if(DKC_NORESTYPE != restype){
				// received data is response
				pComObj = FindWaitCommandObject(comnum, comtype);
			}
			if(pComObj){
				// found command object which is waiting response
				const PDKCCOM_ALL	pComAll = reinterpret_cast<const PDKCCOM_ALL>(const_cast<unsigned char*>(pbody));

				if(!pComObj->SetReceiveData(pComPkt, pComAll)){
					ERR_DKCPRN("Could not set received data to command object( for DKCCOM_ALL command type(0x%016" PRIx64 ")).", pComAll->com_head.comtype);
					CHM_Free(pComPkt);
					CHM_Free(pbody);
				}
			}else{
				// dispatch processing
				bool	need_free = true;
				if(!Processing(pComPkt, pbody, length, need_free)){
					ERR_DKCPRN("Failed to dispatch and process received command.");		// continue...
				}
				if(need_free){
					CHM_Free(pComPkt);
					CHM_Free(pbody);
				}
			}
		}else{
			if(pComPkt){
				WAN_DKCPRN("Received empty body message but pComPkt(%p) is not empty.", pComPkt);
			}
			CHM_Free(pComPkt);
			CHM_Free(pbody);
		}
		// check thread exiting
		if(!ThreadManager.CheckExitThreads()){
			WAN_DKCPRN("Failed to checking thread exiting, but continue...");
		}
	}
	MSG_DKCPRN("Break(end of) loop main.");

	// clean
	Clean();
	MSG_DKCPRN("Finish main run method.");

	return true;
}

//---------------------------------------------------------
// Methods - Processing
//---------------------------------------------------------
bool K2hdkcCntrl::Processing(PCOMPKT pComPkt, unsigned char* pbody, size_t length, bool& need_free)
{
	need_free = true;

	if(!pComPkt || !pbody || 0 == length){
		ERR_DKCPRN("Parameter are wrong.");
		return false;
	}

	// make buffer
	PDKC_RCVPROC_DATA	pData	= new DKC_RCVPROC_DATA;
	pData->pComPkt				= pComPkt;
	pData->pbody				= pbody;
	pData->length				= length;

	// push buffer
	while(!fullock::flck_trylock_noshared_mutex(&procdatas_lockval));	// MUTEX LOCK
	rcvprocdatas.push_back(pData);
	fullock::flck_unlock_noshared_mutex(&procdatas_lockval);			// MUTEX UNLOCK

	need_free = false;

	// dispatch thread
	if(!ThreadManager.WakeupThreads()){
		ERR_DKCPRN("Could not wakeup processing thread.");
		return false;
	}
	return true;
}

//---------------------------------------------------------
// Methods - Wait Receiving
//---------------------------------------------------------
bool K2hdkcCntrl::SetWaitCommandObject(uint64_t comnum, K2hdkcCommand* pobj)
{
	if(!pobj){
		ERR_DKCPRN("Parameter is wrong.");
		return false;
	}
	dkccom_type_t	comtype	= pobj->GetCommandType();

	while(!fullock::flck_trylock_noshared_mutex(&waitmap_lockval));			// MUTEX LOCK

	// check
	k2hdkcwaitresmap_t::iterator	iternum	= waitmap.find(comnum);
	k2hdkcwaitcommap_t*				pcommap = NULL;
	if(waitmap.end() != iternum){
		pcommap = iternum->second;
		if(pcommap){
			k2hdkcwaitcommap_t::iterator	itercom	= pcommap->find(comtype);
			if(pcommap->end() != itercom){
				if(!itercom->second){
					// remove null(wrong) entry
					WAN_DKCPRN("Already set communication number(0x%016" PRIx64 ") - Command type(0x%016" PRIx64 "), but Command Object is NULL. so remove it and continue...", comnum, comtype);
					pcommap->erase(itercom);

				}else if(itercom->second == pobj){
					// already has same object
					WAN_DKCPRN("Already set communication number(0x%016" PRIx64 ") - Command type(0x%016" PRIx64 ") - Command Object(%p) in waiting list.", comnum, comtype, pobj);
					fullock::flck_unlock_noshared_mutex(&waitmap_lockval);			// MUTEX UNLOCK
					return true;
				}else{
					// already has another object
					ERR_DKCPRN("Already set communication number(0x%016" PRIx64 ") - Command type(0x%016" PRIx64 ") - Command Object(%p), but already another command object(not %p).", comnum, comtype, itercom->second, pobj);
					fullock::flck_unlock_noshared_mutex(&waitmap_lockval);			// MUTEX UNLOCK
					return false;
				}
			}
		}else{
			// make and add(over write) new comtype mapping
			WAN_DKCPRN("Already set communication number(0x%016" PRIx64 "), but command type mapping is null. so remove it, and continue...", comnum);
			pcommap = new k2hdkcwaitcommap_t;
			waitmap[comnum] = pcommap;
		}
	}else{
		// make and add new comtype mapping
		pcommap = new k2hdkcwaitcommap_t;
		waitmap[comnum] = pcommap;
	}
	// add comtype mapping to object
	(*pcommap)[comtype] = pobj;

	fullock::flck_unlock_noshared_mutex(&waitmap_lockval);					// MUTEX UNLOCK

	return true;
}

bool K2hdkcCntrl::UnsetWaitCommandObject(uint64_t comnum, K2hdkcCommand* pobj)
{
	if(!pobj){
		ERR_DKCPRN("Parameter is wrong.");
		return false;
	}
	dkccom_type_t	comtype	= pobj->GetCommandType();

	if(!K2hdkcCntrl::FindWaitCommandObject(comnum, comtype)){
		WAN_DKCPRN("Not found same communication number(0x%016" PRIx64 ") - Command type(0x%016" PRIx64 ") - K2hdkcCommand object(%p).", comnum, comtype, pobj);
	}
	return true;	// always returns true
}

K2hdkcCommand* K2hdkcCntrl::FindWaitCommandObject(uint64_t comnum, dkccom_type_t comtype)
{
	while(!fullock::flck_trylock_noshared_mutex(&waitmap_lockval));			// MUTEX LOCK

	// check
	K2hdkcCommand*					pResObj	= NULL;
	k2hdkcwaitresmap_t::iterator	iternum	= waitmap.find(comnum);
	if(waitmap.end() != iternum){
		k2hdkcwaitcommap_t*			pcommap = iternum->second;
		if(pcommap){
			k2hdkcwaitcommap_t::iterator	itercom	= pcommap->find(comtype);
			if(pcommap->end() != itercom){
				if(itercom->second){
					// found
					pResObj = itercom->second;
					// remove it
					pcommap->erase(itercom);

				}else{
					WAN_DKCPRN("Not found communication number(0x%016" PRIx64 ") - Command type(0x%016" PRIx64 "), but Command Object is NULL. so remove it.", comnum, comtype);
					pcommap->erase(itercom);
				}
				// check empty
				// cppcheck-suppress stlSize
				if(0 == pcommap->size()){
					// cppcheck-suppress unknownMacro
					DKC_DELETE(pcommap)
					waitmap.erase(iternum);
				}
			}
		}else{
			WAN_DKCPRN("Not found communication number(0x%016" PRIx64 "), but command type mapping is null. so remove it, and continue...", comnum);
			waitmap.erase(iternum);
		}
	}
	fullock::flck_unlock_noshared_mutex(&waitmap_lockval);					// MUTEX UNLOCK

	return pResObj;
}

/*
 * VIM modelines
 *
 * vim:set ts=4 fenc=utf-8:
 */
