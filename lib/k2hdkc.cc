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
 * CREATE:   Thu Mar 31 2016
 * REVISION:
 * 
 */

#include <stdlib.h>
#include <stdio.h>

#include <chmpx/chmpx.h>

#include "k2hdkccommon.h"
#include "k2hdkc.h"
#include "k2hdkccom.h"
#include "k2hdkcutil.h"
#include "k2hdkcdbg.h"

using namespace	std;

//---------------------------------------------------------
// Global variables
//---------------------------------------------------------
// [NOTE]
// This lastest response code buffer is not safe for multithread.
// If you need to get safe it, you can use permanent slave object.
//
static dkcres_type_t	Last_Response_Code = DKC_NORESTYPE;

//---------------------------------------------------------
// Functions - Debug
//---------------------------------------------------------
void k2hdkc_bump_debug_level(void)
{
	::BumpupK2hdkcDbgMode();
}

void k2hdkc_set_debug_level_silent(void)
{
	::SetK2hdkcDbgMode(DKCDBG_SILENT);
}

void k2hdkc_set_debug_level_error(void)
{
	::SetK2hdkcDbgMode(DKCDBG_ERR);
}

void k2hdkc_set_debug_level_warning(void)
{
	::SetK2hdkcDbgMode(DKCDBG_WARN);
}

void k2hdkc_set_debug_level_message(void)
{
	::SetK2hdkcDbgMode(DKCDBG_MSG);
}

void k2hdkc_set_debug_level_dump(void)
{
	::SetK2hdkcDbgMode(DKCDBG_DUMP);
}

bool k2hdkc_set_debug_file(const char* filepath)
{
	bool result;
	if(DKCEMPTYSTR(filepath)){
		result = ::UnsetK2hdkcDbgFile();
	}else{
		result = ::SetK2hdkcDbgFile(filepath);
	}
	return result;
}

bool k2hdkc_unset_debug_file(void)
{
	return ::UnsetK2hdkcDbgFile();
}

bool k2hdkc_load_debug_env(void)
{
	return ::LoadK2hdkcDbgEnv();
}

bool k2hdkc_is_enable_comlog(void)
{
	return K2hdkcComNumber::IsEnable();
}

void k2hdkc_enable_comlog(void)
{
	::EnableK2hdkcComLog();
}

void k2hdkc_disable_comlog(void)
{
	::DsableK2hdkcComLog();
}

void k2hdkc_toggle_comlog(void)
{
	if(K2hdkcComNumber::IsEnable()){
		::DsableK2hdkcComLog();
	}else{
		::EnableK2hdkcComLog();
	}
}

//---------------------------------------------------------
// Functions - Get Last response code
//---------------------------------------------------------
dkcres_type_t k2hdkc_get_lastres_code(void)
{
	return Last_Response_Code;
}

dkcres_type_t k2hdkc_get_lastres_subcode(void)
{
	return GET_DKC_RES_SUBCODE(Last_Response_Code);
}

bool k2hdkc_is_lastres_success(void)
{
	return IS_DKC_RES_SUCCESS(Last_Response_Code);
}

dkcres_type_t k2hdkc_get_res_code(k2hdkc_chmpx_h handle)
{
	if(K2HDKC_INVALID_HANDLE == handle){
		ERR_DKCPRN("Parameter handle is invalid.");
		return DKC_NORESTYPE;
	}
	K2hdkcSlave*	pSlave = reinterpret_cast<K2hdkcSlave*>(handle);
	if(!pSlave){
		ERR_DKCPRN("Parameter handle is invalid.");
		return DKC_NORESTYPE;
	}
	return pSlave->GetResponseCode();
}

dkcres_type_t k2hdkc_get_res_subcode(k2hdkc_chmpx_h handle)
{
	dkcres_type_t	rescode = k2hdkc_get_res_code(handle);
	if(DKC_NORESTYPE == rescode){
		return rescode;
	}
	return GET_DKC_RES_SUBCODE(rescode);
}

bool k2hdkc_is_res_success(k2hdkc_chmpx_h handle)
{
	dkcres_type_t	rescode = k2hdkc_get_res_code(handle);
	if(DKC_NORESTYPE == rescode){
		return false;
	}
	return IS_DKC_RES_SUCCESS(rescode);
}

//---------------------------------------------------------
// Functions - Chmpx slave object
//---------------------------------------------------------
static K2hdkcSlave* CreateOpenedMsgidSlaveObject(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, bool is_clean_bup)
{
	if(DKCEMPTYSTR(config)){
		MSG_DKCPRN("config parameter is empty, thus using environment.");
	}
	K2hdkcSlave*	pSlave = new K2hdkcSlave();
	if(!pSlave->Initialize(config, ctlport, is_auto_rejoin)){
		ERR_DKCPRN("Could not join chmpx slave node.");
		DKC_DELETE(pSlave);
		return NULL;
	}
	if(!pSlave->Open(no_giveup_rejoin)){
		ERR_DKCPRN("Could not open msgid for slave node.");
		pSlave->Clean(is_clean_bup);
		DKC_DELETE(pSlave);
		return K2HDKC_INVALID_HANDLE;
	}
	return pSlave;
}

static bool DestoryOpenedMsgidSlaveObject(K2hdkcSlave* pSlave, bool is_clean_bup)
{
	if(!pSlave){
		ERR_DKCPRN("Parameter is wrong.");
		return false;
	}
	if(!pSlave->Close()){
		WAN_DKCPRN("Failed to close msgid in slave chmpx object, but continue...");
	}
	if(!pSlave->Clean(is_clean_bup)){
		WAN_DKCPRN("Failed to uninitialize slave chmpx, but continue...");
	}
	DKC_DELETE(pSlave);
	return true;
}

k2hdkc_chmpx_h k2hdkc_open_chmpx(const char* config)
{
	return k2hdkc_open_chmpx_ex(config, CHM_INVALID_PORT, false, false, true);
}

k2hdkc_chmpx_h k2hdkc_open_chmpx_ex(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, bool is_clean_bup)
{
	K2hdkcSlave*	pSlave = CreateOpenedMsgidSlaveObject(config, ctlport, is_auto_rejoin, no_giveup_rejoin, is_clean_bup);
	if(!pSlave){
		return K2HDKC_INVALID_HANDLE;
	}
	return reinterpret_cast<k2hdkc_chmpx_h>(pSlave);
}

bool k2hdkc_close_chmpx(k2hdkc_chmpx_h handle)
{
	return k2hdkc_close_chmpx_ex(handle, true);
}

bool k2hdkc_close_chmpx_ex(k2hdkc_chmpx_h handle, bool is_clean_bup)
{
	if(K2HDKC_INVALID_HANDLE == handle){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}
	K2hdkcSlave*	pSlave = reinterpret_cast<K2hdkcSlave*>(handle);
	if(!pSlave){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}
	return DestoryOpenedMsgidSlaveObject(pSlave, is_clean_bup);
}

//---------------------------------------------------------
// Functions - Get State
//---------------------------------------------------------
static bool K2hdkcFullGetState(K2hdkcComState* pcomobj, PDKC_NODESTATE* ppstates, size_t* pstatecount)
{
	if(!pcomobj || !ppstates || !pstatecount){
		ERR_DKCPRN("Parameters are wrong.");
		return false;
	}
	*ppstates	= NULL;
	*pstatecount= 0UL;

	const DKC_NODESTATE*	pstates		= NULL;
	size_t					statecount	= 0UL;
	if(!pcomobj->CommandSend(&pstates, &statecount, &Last_Response_Code)){
		return false;
	}
	if(!pstates || 0 == statecount){
		return true;
	}

	// copy to new allocated area
	PDKC_NODESTATE	pdststates;
	if(NULL == (pdststates = reinterpret_cast<PDKC_NODESTATE>(calloc(statecount, sizeof(DKC_NODESTATE))))){
		ERR_DKCPRN("Could not allocate memory.");
		return false;
	}
	// copy
	for(size_t cnt = 0; cnt < statecount; ++cnt){
		memcpy(&(pdststates[cnt]), &(pstates[cnt]), sizeof(DKC_NODESTATE));
	}
	*ppstates	= pdststates;
	*pstatecount= statecount;

	return true;
}

bool k2hdkc_get_state(const char* config, PDKC_NODESTATE* ppstates, size_t* pstatecount)
{
	return k2hdkc_ex_get_state(config, CHM_INVALID_PORT, false, false, ppstates, pstatecount);
}

PDKC_NODESTATE k2hdkc_get_direct_state(const char* config, size_t* pstatecount)
{
	return k2hdkc_ex_get_direct_state(config, CHM_INVALID_PORT, false, false, pstatecount);
}

bool k2hdkc_ex_get_state(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, PDKC_NODESTATE* ppstates, size_t* pstatecount)
{
	K2hdkcComState*	pComObj = GetOtSlaveK2hdkcComState(config, ctlport, is_auto_rejoin, no_giveup_rejoin);

	bool	result = K2hdkcFullGetState(pComObj, ppstates, pstatecount);
	if(!result){
		ERR_DKCPRN("Failed to get state.");
	}
	DKC_DELETE(pComObj);

	return result;
}

PDKC_NODESTATE k2hdkc_ex_get_direct_state(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, size_t* pstatecount)
{
	PDKC_NODESTATE	pstates = NULL;
	if(!k2hdkc_ex_get_state(config, ctlport, is_auto_rejoin, no_giveup_rejoin, &pstates, pstatecount) || !pstates){
		return NULL;
	}
	return pstates;
}

bool k2hdkc_pm_get_state(k2hdkc_chmpx_h handle, PDKC_NODESTATE* ppstates, size_t* pstatecount)
{
	if(K2HDKC_INVALID_HANDLE == handle){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}
	K2hdkcSlave*	pSlave = reinterpret_cast<K2hdkcSlave*>(handle);
	if(!pSlave){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}
	K2hdkcComState*	pComObj = GetPmSlaveK2hdkcComState(pSlave);

	bool	result = K2hdkcFullGetState(pComObj, ppstates, pstatecount);
	if(!result){
		ERR_DKCPRN("Failed to get state.");
	}
	DKC_DELETE(pComObj);

	return result;
}

PDKC_NODESTATE k2hdkc_pm_get_direct_state(k2hdkc_chmpx_h handle, size_t* pstatecount)
{
	PDKC_NODESTATE	pstates = NULL;
	if(!k2hdkc_pm_get_state(handle, &pstates, pstatecount) || !pstates){
		return NULL;
	}
	return pstates;
}

//---------------------------------------------------------
// Functions - Get value
//---------------------------------------------------------
static bool K2hdkcFullGetValue(K2hdkcComGet* pcomobj, const unsigned char* pkey, size_t keylength, bool checkattr, const char* encpass, unsigned char** ppval, size_t* pvallength)
{
	if(!pcomobj || !pkey || 0 == keylength || !ppval || !pvallength){
		ERR_DKCPRN("Parameters are wrong.");
		return false;
	}
	const unsigned char*	pvaltmp		= NULL;
	size_t					valtmplen	= 0L;
	bool	result = pcomobj->CommandSend(pkey, keylength, checkattr, encpass, &pvaltmp, &valtmplen, &Last_Response_Code);
	if(result){
		if(pvaltmp && 0 < valtmplen){
			*ppval		= k2hbindup(pvaltmp, valtmplen);
			*pvallength	= valtmplen;
		}else{
			*ppval		= NULL;
			*pvallength	= 0L;
		}
	}
	return result;
}

bool k2hdkc_get_value(const char* config, const unsigned char* pkey, size_t keylength, unsigned char** ppval, size_t* pvallength)
{
	return k2hdkc_ex_get_value(config, CHM_INVALID_PORT, false, false, pkey, keylength, ppval, pvallength);
}
unsigned char* k2hdkc_get_direct_value(const char* config, const unsigned char* pkey, size_t keylength, size_t* pvallength)
{
	return k2hdkc_ex_get_direct_value(config, CHM_INVALID_PORT, false, false, pkey, keylength, pvallength);
}

bool k2hdkc_get_str_value(const char* config, const char* pkey, char** ppval)
{
	return k2hdkc_ex_get_str_value(config, CHM_INVALID_PORT, false, false, pkey, ppval);
}

char* k2hdkc_get_str_direct_value(const char* config, const char* pkey)
{
	return k2hdkc_ex_get_str_direct_value(config, CHM_INVALID_PORT, false, false, pkey);
}

bool k2hdkc_get_value_wp(const char* config, const unsigned char* pkey, size_t keylength, const char* encpass, unsigned char** ppval, size_t* pvallength)
{
	return k2hdkc_ex_get_value_wp(config, CHM_INVALID_PORT, false, false, pkey, keylength, encpass, ppval, pvallength);
}

unsigned char* k2hdkc_get_direct_value_wp(const char* config, const unsigned char* pkey, size_t keylength, const char* encpass, size_t* pvallength)
{
	return k2hdkc_ex_get_direct_value_wp(config, CHM_INVALID_PORT, false, false, pkey, keylength, encpass, pvallength);
}

bool k2hdkc_get_str_value_wp(const char* config, const char* pkey, const char* encpass, char** ppval)
{
	return k2hdkc_ex_get_str_value_wp(config, CHM_INVALID_PORT, false, false, pkey, encpass, ppval);
}

char* k2hdkc_get_str_direct_value_wp(const char* config, const char* pkey, const char* encpass)
{
	return k2hdkc_ex_get_str_direct_value_wp(config, CHM_INVALID_PORT, false, false, pkey, encpass);
}

bool k2hdkc_get_value_np(const char* config, const unsigned char* pkey, size_t keylength, unsigned char** ppval, size_t* pvallength)
{
	return k2hdkc_ex_get_value_np(config, CHM_INVALID_PORT, false, false, pkey, keylength, ppval, pvallength);
}

unsigned char* k2hdkc_get_direct_value_np(const char* config, const unsigned char* pkey, size_t keylength, size_t* pvallength)
{
	return k2hdkc_ex_get_direct_value_np(config, CHM_INVALID_PORT, false, false, pkey, keylength, pvallength);
}

bool k2hdkc_get_str_value_np(const char* config, const char* pkey, char** ppval)
{
	return k2hdkc_ex_get_str_value_np(config, CHM_INVALID_PORT, false, false, pkey, ppval);
}

char* k2hdkc_get_str_direct_value_np(const char* config, const char* pkey)
{
	return k2hdkc_ex_get_str_direct_value_np(config, CHM_INVALID_PORT, false, false, pkey);
}

bool k2hdkc_ex_get_value(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, unsigned char** ppval, size_t* pvallength)
{
	K2hdkcComGet*	pComObj = GetOtSlaveK2hdkcComGet(config, ctlport, is_auto_rejoin, no_giveup_rejoin);

	bool	result = K2hdkcFullGetValue(pComObj, pkey, keylength, true, NULL, ppval, pvallength);
	if(!result){
		ERR_DKCPRN("Failed to get value.");
	}
	DKC_DELETE(pComObj);

	return result;
}

unsigned char* k2hdkc_ex_get_direct_value(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, size_t* pvallength)
{
	unsigned char*	pval = NULL;
	if(!k2hdkc_ex_get_value(config, ctlport, is_auto_rejoin, no_giveup_rejoin, pkey, keylength, &pval, pvallength) || !pval){
		return NULL;
	}
	return pval;
}

bool k2hdkc_ex_get_str_value(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, char** ppval)
{
	unsigned char*	pval		= NULL;
	size_t			vallength	= 0L;
	if(!k2hdkc_ex_get_value(config, ctlport, is_auto_rejoin, no_giveup_rejoin, reinterpret_cast<const unsigned char*>(pkey), (pkey ? strlen(pkey) + 1 : 0), &pval, &vallength) || !pval || 0 == vallength){
		DKC_FREE(pval);
		return false;
	}
	*ppval = reinterpret_cast<char*>(pval);
	return true;
}

char* k2hdkc_ex_get_str_direct_value(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey)
{
	unsigned char*	pval		= NULL;
	size_t			vallength	= 0L;
	if(!k2hdkc_ex_get_value(config, ctlport, is_auto_rejoin, no_giveup_rejoin, reinterpret_cast<const unsigned char*>(pkey), (pkey ? strlen(pkey) + 1 : 0), &pval, &vallength) || !pval || 0 == vallength){
		DKC_FREE(pval);
		return NULL;
	}
	return reinterpret_cast<char*>(pval);
}

bool k2hdkc_ex_get_value_wp(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, const char* encpass, unsigned char** ppval, size_t* pvallength)
{
	K2hdkcComGet*	pComObj = GetOtSlaveK2hdkcComGet(config, ctlport, is_auto_rejoin, no_giveup_rejoin);

	bool	result = K2hdkcFullGetValue(pComObj, pkey, keylength, true, encpass, ppval, pvallength);
	if(!result){
		ERR_DKCPRN("Failed to get value.");
	}
	DKC_DELETE(pComObj);

	return result;
}

unsigned char* k2hdkc_ex_get_direct_value_wp(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, const char* encpass, size_t* pvallength)
{
	unsigned char*	pval = NULL;
	if(!k2hdkc_ex_get_value_wp(config, ctlport, is_auto_rejoin, no_giveup_rejoin, pkey, keylength, encpass, &pval, pvallength) || !pval){
		return NULL;
	}
	return pval;
}

bool k2hdkc_ex_get_str_value_wp(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, const char* encpass, char** ppval)
{
	unsigned char*	pval		= NULL;
	size_t			vallength	= 0L;
	if(!k2hdkc_ex_get_value_wp(config, ctlport, is_auto_rejoin, no_giveup_rejoin, reinterpret_cast<const unsigned char*>(pkey), (pkey ? strlen(pkey) + 1 : 0), encpass, &pval, &vallength) || !pval || 0 == vallength){
		DKC_FREE(pval);
		return false;
	}
	*ppval = reinterpret_cast<char*>(pval);
	return true;
}

char* k2hdkc_ex_get_str_direct_value_wp(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, const char* encpass)
{
	unsigned char*	pval		= NULL;
	size_t			vallength	= 0L;
	if(!k2hdkc_ex_get_value_wp(config, ctlport, is_auto_rejoin, no_giveup_rejoin, reinterpret_cast<const unsigned char*>(pkey), (pkey ? strlen(pkey) + 1 : 0), encpass, &pval, &vallength) || !pval || 0 == vallength){
		DKC_FREE(pval);
		return NULL;
	}
	return reinterpret_cast<char*>(pval);
}

bool k2hdkc_ex_get_value_np(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, unsigned char** ppval, size_t* pvallength)
{
	K2hdkcComGet*	pComObj = GetOtSlaveK2hdkcComGet(config, ctlport, is_auto_rejoin, no_giveup_rejoin);

	bool	result = K2hdkcFullGetValue(pComObj, pkey, keylength, false, NULL, ppval, pvallength);
	if(!result){
		ERR_DKCPRN("Failed to get value.");
	}
	DKC_DELETE(pComObj);

	return result;
}
unsigned char* k2hdkc_ex_get_direct_value_np(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, size_t* pvallength)
{
	unsigned char*	pval = NULL;
	if(!k2hdkc_ex_get_value_np(config, ctlport, is_auto_rejoin, no_giveup_rejoin, pkey, keylength, &pval, pvallength) || !pval){
		return NULL;
	}
	return pval;
}

bool k2hdkc_ex_get_str_value_np(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, char** ppval)
{
	unsigned char*	pval		= NULL;
	size_t			vallength	= 0L;
	if(!k2hdkc_ex_get_value_np(config, ctlport, is_auto_rejoin, no_giveup_rejoin, reinterpret_cast<const unsigned char*>(pkey), (pkey ? strlen(pkey) + 1 : 0), &pval, &vallength) || !pval || 0 == vallength){
		DKC_FREE(pval);
		return false;
	}
	*ppval = reinterpret_cast<char*>(pval);
	return true;
}

char* k2hdkc_ex_get_str_direct_value_np(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey)
{
	unsigned char*	pval		= NULL;
	size_t			vallength	= 0L;
	if(!k2hdkc_ex_get_value_np(config, ctlport, is_auto_rejoin, no_giveup_rejoin, reinterpret_cast<const unsigned char*>(pkey), (pkey ? strlen(pkey) + 1 : 0), &pval, &vallength) || !pval || 0 == vallength){
		DKC_FREE(pval);
		return NULL;
	}
	return reinterpret_cast<char*>(pval);
}

bool k2hdkc_pm_get_value(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, unsigned char** ppval, size_t* pvallength)
{
	if(K2HDKC_INVALID_HANDLE == handle){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}
	K2hdkcSlave*	pSlave = reinterpret_cast<K2hdkcSlave*>(handle);
	if(!pSlave){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}
	K2hdkcComGet*	pComObj = GetPmSlaveK2hdkcComGet(pSlave);

	bool	result = K2hdkcFullGetValue(pComObj, pkey, keylength, true, NULL, ppval, pvallength);
	if(!result){
		ERR_DKCPRN("Failed to get value.");
	}
	DKC_DELETE(pComObj);

	return result;
}

unsigned char* k2hdkc_pm_get_direct_value(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, size_t* pvallength)
{
	unsigned char*	pval = NULL;
	if(!k2hdkc_pm_get_value(handle, pkey, keylength, &pval, pvallength) || !pval){
		return NULL;
	}
	return pval;
}

bool k2hdkc_pm_get_str_value(k2hdkc_chmpx_h handle, const char* pkey, char** ppval)
{
	unsigned char*	pval		= NULL;
	size_t			vallength	= 0L;
	if(!k2hdkc_pm_get_value(handle, reinterpret_cast<const unsigned char*>(pkey), (pkey ? strlen(pkey) + 1 : 0), &pval, &vallength) || !pval || 0 == vallength){
		DKC_FREE(pval);
		return false;
	}
	*ppval = reinterpret_cast<char*>(pval);
	return true;
}

char* k2hdkc_pm_get_str_direct_value(k2hdkc_chmpx_h handle, const char* pkey)
{
	unsigned char*	pval		= NULL;
	size_t			vallength	= 0L;
	if(!k2hdkc_pm_get_value(handle, reinterpret_cast<const unsigned char*>(pkey), (pkey ? strlen(pkey) + 1 : 0), &pval, &vallength) || !pval || 0 == vallength){
		DKC_FREE(pval);
		return NULL;
	}
	return reinterpret_cast<char*>(pval);
}

bool k2hdkc_pm_get_value_wp(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, const char* encpass, unsigned char** ppval, size_t* pvallength)
{
	if(K2HDKC_INVALID_HANDLE == handle){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}
	K2hdkcSlave*	pSlave = reinterpret_cast<K2hdkcSlave*>(handle);
	if(!pSlave){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}
	K2hdkcComGet*	pComObj = GetPmSlaveK2hdkcComGet(pSlave);

	bool	result = K2hdkcFullGetValue(pComObj, pkey, keylength, true, encpass, ppval, pvallength);
	if(!result){
		ERR_DKCPRN("Failed to get value.");
	}
	DKC_DELETE(pComObj);

	return result;
}

unsigned char* k2hdkc_pm_get_direct_value_wp(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, const char* encpass, size_t* pvallength)
{
	unsigned char*	pval = NULL;
	if(!k2hdkc_pm_get_value_wp(handle, pkey, keylength, encpass, &pval, pvallength) || !pval){
		return NULL;
	}
	return pval;
}

bool k2hdkc_pm_get_str_value_wp(k2hdkc_chmpx_h handle, const char* pkey, const char* encpass, char** ppval)
{
	unsigned char*	pval		= NULL;
	size_t			vallength	= 0L;
	if(!k2hdkc_pm_get_value_wp(handle, reinterpret_cast<const unsigned char*>(pkey), (pkey ? strlen(pkey) + 1 : 0), encpass, &pval, &vallength) || !pval || 0 == vallength){
		DKC_FREE(pval);
		return false;
	}
	*ppval = reinterpret_cast<char*>(pval);
	return true;
}

char* k2hdkc_pm_get_str_direct_value_wp(k2hdkc_chmpx_h handle, const char* pkey, const char* encpass)
{
	unsigned char*	pval		= NULL;
	size_t			vallength	= 0L;
	if(!k2hdkc_pm_get_value_wp(handle, reinterpret_cast<const unsigned char*>(pkey), (pkey ? strlen(pkey) + 1 : 0), encpass, &pval, &vallength) || !pval || 0 == vallength){
		DKC_FREE(pval);
		return NULL;
	}
	return reinterpret_cast<char*>(pval);
}

bool k2hdkc_pm_get_value_np(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, unsigned char** ppval, size_t* pvallength)
{
	if(K2HDKC_INVALID_HANDLE == handle){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}
	K2hdkcSlave*	pSlave = reinterpret_cast<K2hdkcSlave*>(handle);
	if(!pSlave){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}
	K2hdkcComGet*	pComObj = GetPmSlaveK2hdkcComGet(pSlave);

	bool	result = K2hdkcFullGetValue(pComObj, pkey, keylength, false, NULL, ppval, pvallength);
	if(!result){
		ERR_DKCPRN("Failed to get value.");
	}
	DKC_DELETE(pComObj);

	return result;
}

unsigned char* k2hdkc_pm_get_direct_value_np(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, size_t* pvallength)
{
	unsigned char*	pval = NULL;
	if(!k2hdkc_pm_get_value_np(handle, pkey, keylength, &pval, pvallength) || !pval){
		return NULL;
	}
	return pval;
}

bool k2hdkc_pm_get_str_value_np(k2hdkc_chmpx_h handle, const char* pkey, char** ppval)
{
	unsigned char*	pval		= NULL;
	size_t			vallength	= 0L;
	if(!k2hdkc_pm_get_value_np(handle, reinterpret_cast<const unsigned char*>(pkey), (pkey ? strlen(pkey) + 1 : 0), &pval, &vallength) || !pval || 0 == vallength){
		DKC_FREE(pval);
		return false;
	}
	*ppval = reinterpret_cast<char*>(pval);
	return true;
}

char* k2hdkc_pm_get_str_direct_value_np(k2hdkc_chmpx_h handle, const char* pkey)
{
	unsigned char*	pval		= NULL;
	size_t			vallength	= 0L;
	if(!k2hdkc_pm_get_value_np(handle, reinterpret_cast<const unsigned char*>(pkey), (pkey ? strlen(pkey) + 1 : 0), &pval, &vallength) || !pval || 0 == vallength){
		DKC_FREE(pval);
		return NULL;
	}
	return reinterpret_cast<char*>(pval);
}

//---------------------------------------------------------
// Functions - Get Direct Access(offset) Value
//---------------------------------------------------------
static bool K2hdkcFullGetDirect(K2hdkcComGetDirect* pcomobj, const unsigned char* pkey, size_t keylength, off_t getpos, size_t val_length, unsigned char** ppval, size_t* pvallength)
{
	if(!pcomobj || !pkey || 0 == keylength || getpos < 0 || 0 == val_length || !ppval || !pvallength){
		ERR_DKCPRN("Parameters are wrong.");
		return false;
	}
	const unsigned char*	pvaltmp		= NULL;
	size_t					valtmplen	= 0L;
	bool	result = pcomobj->CommandSend(pkey, keylength, getpos, val_length, &pvaltmp, &valtmplen, &Last_Response_Code);
	if(result){
		if(pvaltmp && 0 < valtmplen){
			*ppval		= k2hbindup(pvaltmp, valtmplen);
			*pvallength	= valtmplen;
		}else{
			*ppval		= NULL;
			*pvallength	= 0L;
		}
	}
	return result;
}

bool k2hdkc_da_get_value(const char* config, const unsigned char* pkey, size_t keylength, off_t getpos, size_t val_length, unsigned char** ppval, size_t* pvallength)
{
	return k2hdkc_ex_da_get_value(config, CHM_INVALID_PORT, false, false, pkey, keylength, getpos, val_length, ppval, pvallength);
}

unsigned char* k2hdkc_da_get_direct_value(const char* config, const unsigned char* pkey, size_t keylength, off_t getpos, size_t val_length, size_t* pvallength)
{
	return k2hdkc_ex_da_get_direct_value(config, CHM_INVALID_PORT, false, false, pkey, keylength, getpos, val_length, pvallength);
}

bool k2hdkc_da_get_str_value(const char* config, const char* pkey, off_t getpos, size_t val_length, char** ppval)
{
	return k2hdkc_ex_da_get_str_value(config, CHM_INVALID_PORT, false, false, pkey, getpos, val_length, ppval);
}

char* k2hdkc_da_get_str_direct_value(const char* config, const char* pkey, off_t getpos, size_t val_length)
{
	return k2hdkc_ex_da_get_str_direct_value(config, CHM_INVALID_PORT, false, false, pkey, getpos, val_length);
}

bool k2hdkc_ex_da_get_value(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, off_t getpos, size_t val_length, unsigned char** ppval, size_t* pvallength)
{
	K2hdkcComGetDirect*	pComObj	= GetOtSlaveK2hdkcComGetDirect(config, ctlport, is_auto_rejoin, no_giveup_rejoin);
	bool				result	= K2hdkcFullGetDirect(pComObj, pkey, keylength, getpos, val_length, ppval, pvallength);
	if(!result){
		ERR_DKCPRN("Failed to get value.");
	}
	DKC_DELETE(pComObj);

	return result;
}

unsigned char* k2hdkc_ex_da_get_direct_value(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, off_t getpos, size_t val_length, size_t* pvallength)
{
	unsigned char*	pval = NULL;
	if(!k2hdkc_ex_da_get_value(config, ctlport, is_auto_rejoin, no_giveup_rejoin, pkey, keylength, getpos, val_length, &pval, pvallength)){
		return NULL;
	}
	return pval;
}

bool k2hdkc_ex_da_get_str_value(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, off_t getpos, size_t val_length, char** ppval)
{
	unsigned char*	pval		= NULL;
	size_t			vallength	= 0L;
	if(!k2hdkc_ex_da_get_value(config, ctlport, is_auto_rejoin, no_giveup_rejoin, reinterpret_cast<const unsigned char*>(pkey), (pkey ? strlen(pkey) + 1 : 0), getpos, val_length, &pval, &vallength) || !pval || 0 == vallength){
		DKC_FREE(pval);
		return false;
	}
	*ppval = reinterpret_cast<char*>(pval);
	return true;
}

char* k2hdkc_ex_da_get_str_direct_value(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, off_t getpos, size_t val_length)
{
	unsigned char*	pval		= NULL;
	size_t			vallength	= 0L;
	if(!k2hdkc_ex_da_get_value(config, ctlport, is_auto_rejoin, no_giveup_rejoin, reinterpret_cast<const unsigned char*>(pkey), (pkey ? strlen(pkey) + 1 : 0), getpos, val_length, &pval, &vallength) || !pval || 0 == vallength){
		DKC_FREE(pval);
		return NULL;
	}
	return reinterpret_cast<char*>(pval);
}

bool k2hdkc_pm_da_get_value(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, off_t getpos, size_t val_length, unsigned char** ppval, size_t* pvallength)
{
	if(K2HDKC_INVALID_HANDLE == handle){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}
	K2hdkcSlave*	pSlave = reinterpret_cast<K2hdkcSlave*>(handle);
	if(!pSlave){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}

	K2hdkcComGetDirect*	pComObj	= GetPmSlaveK2hdkcComGetDirect(pSlave);
	bool				result	= K2hdkcFullGetDirect(pComObj, pkey, keylength, getpos, val_length, ppval, pvallength);
	if(!result){
		ERR_DKCPRN("Failed to get value.");
	}
	DKC_DELETE(pComObj);

	return result;
}

unsigned char* k2hdkc_pm_da_get_direct_value(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, off_t getpos, size_t val_length, size_t* pvallength)
{
	unsigned char*	pval = NULL;
	if(!k2hdkc_pm_da_get_value(handle, pkey, keylength, getpos, val_length, &pval, pvallength)){
		return NULL;
	}
	return pval;
}

bool k2hdkc_pm_da_get_str_value(k2hdkc_chmpx_h handle, const char* pkey, off_t getpos, size_t val_length, char** ppval)
{
	unsigned char*	pval		= NULL;
	size_t			vallength	= 0L;
	if(!k2hdkc_pm_da_get_value(handle, reinterpret_cast<const unsigned char*>(pkey), (pkey ? strlen(pkey) + 1 : 0), getpos, val_length, &pval, &vallength) || !pval || 0 == vallength){
		DKC_FREE(pval);
		return false;
	}
	*ppval = reinterpret_cast<char*>(pval);
	return true;
}

char* k2hdkc_pm_da_get_str_direct_value(k2hdkc_chmpx_h handle, const char* pkey, off_t getpos, size_t val_length)
{
	unsigned char*	pval		= NULL;
	size_t			vallength	= 0L;
	if(!k2hdkc_pm_da_get_value(handle, reinterpret_cast<const unsigned char*>(pkey), (pkey ? strlen(pkey) + 1 : 0), getpos, val_length, &pval, &vallength) || !pval || 0 == vallength){
		DKC_FREE(pval);
		return NULL;
	}
	return reinterpret_cast<char*>(pval);
}

//---------------------------------------------------------
// Functions - Get Subkeys
//---------------------------------------------------------
static bool K2hdkcFullGetSubkeys(K2hdkcComGetSubkeys* pcomobj, const unsigned char* pkey, size_t keylength, bool checkattr, K2HSubKeys** ppSubKeys)
{
	if(!pcomobj || !pkey || 0 == keylength || !ppSubKeys){
		ERR_DKCPRN("Parameters are wrong.");
		return false;
	}
	return pcomobj->CommandSend(pkey, keylength, checkattr, ppSubKeys, &Last_Response_Code);
}

// [NOTE]
// This function hes not been declared in header, but has been called from k2hdkc tools.
//
bool K2hdkcCvtSubkeysToPack(K2HSubKeys* pSubKeys, PK2HDKCKEYPCK* ppskeypck, int* pskeypckcnt)
{
	if(!pSubKeys || !ppskeypck || !pskeypckcnt){
		ERR_DKCPRN("Parameters are wrong.");
		return false;
	}
	*ppskeypck	= NULL;
	*pskeypckcnt= 0UL;

	if(0 == pSubKeys->size()){
		return true;
	}
	size_t			subkeycnt = pSubKeys->size();
	PK2HDKCKEYPCK	pskeypck;
	if(NULL == (pskeypck = reinterpret_cast<PK2HDKCKEYPCK>(calloc(subkeycnt, sizeof(K2HDKCKEYPCK))))){
		ERR_DKCPRN("Could not allocate memory.");
		return false;
	}
	// copy
	int	setpos = 0;
	for(K2HSubKeys::iterator iter = pSubKeys->begin(); iter != pSubKeys->end(); iter++){
		if(0UL == iter->length){
			WAN_DKCPRN("Subkey is empty.");
			continue;
		}
		if(NULL == (pskeypck[setpos].pkey = reinterpret_cast<unsigned char*>(malloc(iter->length)))){
			ERR_DKCPRN("Could not allocate memory.");
			DKC_FREE_KEYPACK(pskeypck, subkeycnt);
			return false;
		}
		memcpy(pskeypck[setpos].pkey, iter->pSubKey, iter->length);
		pskeypck[setpos].length = iter->length;
		setpos++;
	}
	if(0 == setpos){
		//MSG_DKCPRN("Not have subkeys.");
		DKC_FREE_KEYPACK(pskeypck, subkeycnt);
	}else{
		*ppskeypck	= pskeypck;
		*pskeypckcnt= setpos;
	}
	return true;
}

static int K2hdkcCvtSubkeysToStringArray(K2HSubKeys* pSubKeys, char*** ppskeyarray)
{
	if(!pSubKeys || !ppskeyarray){
		ERR_DKCPRN("Parameters are wrong.");
		return -1;
	}
	*ppskeyarray = NULL;

	if(0 == pSubKeys->size()){
		return true;
	}
	size_t	subkeycnt = pSubKeys->size();
	char**	pskeyarray;
	if(NULL == (pskeyarray = reinterpret_cast<char**>(calloc(subkeycnt + 1UL, sizeof(char*))))){
		ERR_DKCPRN("Could not allocate memory.");
		return -1;
	}

	// copy
	int	setpos = 0;
	for(K2HSubKeys::iterator iter = pSubKeys->begin(); iter != pSubKeys->end(); iter++){
		if(0UL == iter->length){
			WAN_DKCPRN("Subkey is empty.");
			continue;
		}
		if(NULL == (pskeyarray[setpos] = reinterpret_cast<char*>(malloc(iter->length + 1UL)))){
			ERR_DKCPRN("Could not allocate memory.");
			DKC_FREE_KEYARRAY(pskeyarray);
			return -1;
		}
		memcpy(pskeyarray[setpos], iter->pSubKey, iter->length);
		(pskeyarray[setpos])[iter->length] = '\0';						// for safe
		setpos++;
	}
	if(0 == setpos){
		MSG_DKCPRN("Not have subkeys.");
		DKC_FREE_KEYARRAY(pskeyarray);
	}else{
		*ppskeyarray = pskeyarray;
	}
	return setpos;
}

bool k2hdkc_get_subkeys(const char* config, const unsigned char* pkey, size_t keylength, PK2HDKCKEYPCK* ppskeypck, int* pskeypckcnt)
{
	return k2hdkc_ex_get_subkeys(config, CHM_INVALID_PORT, false, false, pkey, keylength, ppskeypck, pskeypckcnt);
}

PK2HDKCKEYPCK k2hdkc_get_direct_subkeys(const char* config, const unsigned char* pkey, size_t keylength, int* pskeypckcnt)
{
	return k2hdkc_ex_get_direct_subkeys(config, CHM_INVALID_PORT, false, false, pkey, keylength, pskeypckcnt);
}

int k2hdkc_get_str_subkeys(const char* config, const char* pkey, char*** ppskeyarray)
{
	return k2hdkc_ex_get_str_subkeys(config, CHM_INVALID_PORT, false, false, pkey, ppskeyarray);
}

char** k2hdkc_get_str_direct_subkeys(const char* config, const char* pkey)
{
	return k2hdkc_ex_get_str_direct_subkeys(config, CHM_INVALID_PORT, false, false, pkey);
}

bool k2hdkc_get_subkeys_np(const char* config, const unsigned char* pkey, size_t keylength, PK2HDKCKEYPCK* ppskeypck, int* pskeypckcnt)
{
	return k2hdkc_ex_get_subkeys_np(config, CHM_INVALID_PORT, false, false, pkey, keylength, ppskeypck, pskeypckcnt);
}

PK2HDKCKEYPCK k2hdkc_get_direct_subkeys_np(const char* config, const unsigned char* pkey, size_t keylength, int* pskeypckcnt)
{
	return k2hdkc_ex_get_direct_subkeys_np(config, CHM_INVALID_PORT, false, false, pkey, keylength, pskeypckcnt);
}

int k2hdkc_get_str_subkeys_np(const char* config, const char* pkey, char*** ppskeyarray)
{
	return k2hdkc_ex_get_str_subkeys_np(config, CHM_INVALID_PORT, false, false, pkey, ppskeyarray);
}

char** k2hdkc_get_str_direct_subkeys_np(const char* config, const char* pkey)
{
	return k2hdkc_ex_get_str_direct_subkeys_np(config, CHM_INVALID_PORT, false, false, pkey);
}

bool k2hdkc_ex_get_subkeys(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, PK2HDKCKEYPCK* ppskeypck, int* pskeypckcnt)
{
	K2hdkcComGetSubkeys*	pComObj = GetOtSlaveK2hdkcComGetSubkeys(config, ctlport, is_auto_rejoin, no_giveup_rejoin);
	K2HSubKeys*				pSubKeys= NULL;
	bool					result	= K2hdkcFullGetSubkeys(pComObj, pkey, keylength, true, &pSubKeys);
	if(!result){
		ERR_DKCPRN("Failed to get subkeys.");
		return false;
	}else{
		if(false == (result = K2hdkcCvtSubkeysToPack(pSubKeys, ppskeypck, pskeypckcnt))){
			ERR_DKCPRN("Failed to convert subkeys to k2hkeypck array.");
		}
	}
	DKC_DELETE(pSubKeys);
	DKC_DELETE(pComObj);

	return result;
}

PK2HDKCKEYPCK k2hdkc_ex_get_direct_subkeys(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, int* pskeypckcnt)
{
	PK2HDKCKEYPCK	pskeypck = NULL;
	if(!k2hdkc_ex_get_subkeys(config, ctlport, is_auto_rejoin, no_giveup_rejoin, pkey, keylength, &pskeypck, pskeypckcnt)){
		return NULL;
	}
	return pskeypck;
}

int k2hdkc_ex_get_str_subkeys(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, char*** ppskeyarray)
{
	K2hdkcComGetSubkeys*	pComObj = GetOtSlaveK2hdkcComGetSubkeys(config, ctlport, is_auto_rejoin, no_giveup_rejoin);
	K2HSubKeys*				pSubKeys= NULL;
	int						rescnt	= -1;
	bool					result	= K2hdkcFullGetSubkeys(pComObj, reinterpret_cast<const unsigned char*>(pkey), (pkey ? strlen(pkey) + 1 : 0), true, &pSubKeys);
	if(!result){
		ERR_DKCPRN("Failed to get subkeys.");
	}else{
		if(-1 == (rescnt = K2hdkcCvtSubkeysToStringArray(pSubKeys, ppskeyarray))){
			ERR_DKCPRN("Failed to convert subkeys to string array.");
		}
	}
	DKC_DELETE(pSubKeys);
	DKC_DELETE(pComObj);

	return rescnt;
}

char** k2hdkc_ex_get_str_direct_subkeys(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey)
{
	char**	pskeyarray = NULL;
	if(-1 == k2hdkc_ex_get_str_subkeys(config, ctlport, is_auto_rejoin, no_giveup_rejoin, pkey, &pskeyarray)){
		return NULL;
	}
	return pskeyarray;
}

bool k2hdkc_ex_get_subkeys_np(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, PK2HDKCKEYPCK* ppskeypck, int* pskeypckcnt)
{
	K2hdkcComGetSubkeys*	pComObj = GetOtSlaveK2hdkcComGetSubkeys(config, ctlport, is_auto_rejoin, no_giveup_rejoin);
	K2HSubKeys*				pSubKeys= NULL;
	bool					result	= K2hdkcFullGetSubkeys(pComObj, pkey, keylength, false, &pSubKeys);
	if(!result){
		ERR_DKCPRN("Failed to get subkeys.");
		return false;
	}else{
		if(false == (result = K2hdkcCvtSubkeysToPack(pSubKeys, ppskeypck, pskeypckcnt))){
			ERR_DKCPRN("Failed to convert subkeys to k2hkeypck array.");
		}
	}
	DKC_DELETE(pSubKeys);
	DKC_DELETE(pComObj);

	return result;
}

PK2HDKCKEYPCK k2hdkc_ex_get_direct_subkeys_np(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, int* pskeypckcnt)
{
	PK2HDKCKEYPCK	pskeypck = NULL;
	if(!k2hdkc_ex_get_subkeys_np(config, ctlport, is_auto_rejoin, no_giveup_rejoin, pkey, keylength, &pskeypck, pskeypckcnt)){
		return NULL;
	}
	return pskeypck;
}

int k2hdkc_ex_get_str_subkeys_np(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, char*** ppskeyarray)
{
	K2hdkcComGetSubkeys*	pComObj = GetOtSlaveK2hdkcComGetSubkeys(config, ctlport, is_auto_rejoin, no_giveup_rejoin);
	K2HSubKeys*				pSubKeys= NULL;
	int						rescnt	= -1;
	bool					result	= K2hdkcFullGetSubkeys(pComObj, reinterpret_cast<const unsigned char*>(pkey), (pkey ? strlen(pkey) + 1 : 0), false, &pSubKeys);
	if(!result){
		ERR_DKCPRN("Failed to get subkeys.");
	}else{
		if(-1 == (rescnt = K2hdkcCvtSubkeysToStringArray(pSubKeys, ppskeyarray))){
			ERR_DKCPRN("Failed to convert subkeys to string array.");
		}
	}
	DKC_DELETE(pSubKeys);
	DKC_DELETE(pComObj);

	return rescnt;
}

char** k2hdkc_ex_get_str_direct_subkeys_np(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey)
{
	char**	pskeyarray = NULL;
	if(-1 == k2hdkc_ex_get_str_subkeys_np(config, ctlport, is_auto_rejoin, no_giveup_rejoin, pkey, &pskeyarray)){
		return NULL;
	}
	return pskeyarray;
}

bool k2hdkc_pm_get_subkeys(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, PK2HDKCKEYPCK* ppskeypck, int* pskeypckcnt)
{
	if(K2HDKC_INVALID_HANDLE == handle){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}
	K2hdkcSlave*	pSlave = reinterpret_cast<K2hdkcSlave*>(handle);
	if(!pSlave){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}
	K2hdkcComGetSubkeys*	pComObj = GetPmSlaveK2hdkcComGetSubkeys(pSlave);
	K2HSubKeys*				pSubKeys= NULL;
	bool					result	= K2hdkcFullGetSubkeys(pComObj, pkey, keylength, true, &pSubKeys);
	if(!result){
		ERR_DKCPRN("Failed to get subkeys.");
	}else{
		if(false == (result = K2hdkcCvtSubkeysToPack(pSubKeys, ppskeypck, pskeypckcnt))){
			ERR_DKCPRN("Failed to convert subkeys to k2hkeypck array.");
		}
	}
	DKC_DELETE(pSubKeys);
	DKC_DELETE(pComObj);

	return result;
}

PK2HDKCKEYPCK k2hdkc_pm_get_direct_subkeys(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, int* pskeypckcnt)
{
	PK2HDKCKEYPCK	pskeypck = NULL;
	if(!k2hdkc_pm_get_subkeys(handle, pkey, keylength, &pskeypck, pskeypckcnt)){
		return NULL;
	}
	return pskeypck;
}

int k2hdkc_pm_get_str_subkeys(k2hdkc_chmpx_h handle, const char* pkey, char*** ppskeyarray)
{
	if(K2HDKC_INVALID_HANDLE == handle){
		ERR_DKCPRN("Parameter handle is invalid.");
		return -1;
	}
	K2hdkcSlave*	pSlave = reinterpret_cast<K2hdkcSlave*>(handle);
	if(!pSlave){
		ERR_DKCPRN("Parameter handle is invalid.");
		return -1;
	}
	K2hdkcComGetSubkeys*	pComObj = GetPmSlaveK2hdkcComGetSubkeys(pSlave);
	K2HSubKeys*				pSubKeys= NULL;
	int						rescnt	= -1;
	bool					result	= K2hdkcFullGetSubkeys(pComObj, reinterpret_cast<const unsigned char*>(pkey), (pkey ? strlen(pkey) + 1 : 0), true, &pSubKeys);
	if(!result){
		ERR_DKCPRN("Failed to get subkeys.");
		return -1;
	}else{
		if(-1 == (rescnt = K2hdkcCvtSubkeysToStringArray(pSubKeys, ppskeyarray))){
			ERR_DKCPRN("Failed to convert subkeys to string array.");
		}
	}
	DKC_DELETE(pSubKeys);
	DKC_DELETE(pComObj);

	return rescnt;
}

char** k2hdkc_pm_get_str_direct_subkeys(k2hdkc_chmpx_h handle, const char* pkey)
{
	char**	pskeyarray = NULL;
	if(-1 == k2hdkc_pm_get_str_subkeys(handle, pkey, &pskeyarray)){
		return NULL;
	}
	return pskeyarray;
}

bool k2hdkc_pm_get_subkeys_np(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, PK2HDKCKEYPCK* ppskeypck, int* pskeypckcnt)
{
	if(K2HDKC_INVALID_HANDLE == handle){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}
	K2hdkcSlave*	pSlave = reinterpret_cast<K2hdkcSlave*>(handle);
	if(!pSlave){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}
	K2hdkcComGetSubkeys*	pComObj = GetPmSlaveK2hdkcComGetSubkeys(pSlave);
	K2HSubKeys*				pSubKeys= NULL;
	bool					result	= K2hdkcFullGetSubkeys(pComObj, pkey, keylength, false, &pSubKeys);
	if(!result){
		ERR_DKCPRN("Failed to get subkeys.");
	}else{
		if(false == (result = K2hdkcCvtSubkeysToPack(pSubKeys, ppskeypck, pskeypckcnt))){
			ERR_DKCPRN("Failed to convert subkeys to k2hkeypck array.");
		}
	}
	DKC_DELETE(pSubKeys);
	DKC_DELETE(pComObj);

	return result;
}

PK2HDKCKEYPCK k2hdkc_pm_get_direct_subkeys_np(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, int* pskeypckcnt)
{
	PK2HDKCKEYPCK	pskeypck = NULL;
	if(!k2hdkc_pm_get_subkeys_np(handle, pkey, keylength, &pskeypck, pskeypckcnt)){
		return NULL;
	}
	return pskeypck;
}

int k2hdkc_pm_get_str_subkeys_np(k2hdkc_chmpx_h handle, const char* pkey, char*** ppskeyarray)
{
	if(K2HDKC_INVALID_HANDLE == handle){
		ERR_DKCPRN("Parameter handle is invalid.");
		return -1;
	}
	K2hdkcSlave*	pSlave = reinterpret_cast<K2hdkcSlave*>(handle);
	if(!pSlave){
		ERR_DKCPRN("Parameter handle is invalid.");
		return -1;
	}
	K2hdkcComGetSubkeys*	pComObj = GetPmSlaveK2hdkcComGetSubkeys(pSlave);
	K2HSubKeys*				pSubKeys= NULL;
	int						rescnt	= -1;
	bool					result	= K2hdkcFullGetSubkeys(pComObj, reinterpret_cast<const unsigned char*>(pkey), (pkey ? strlen(pkey) + 1 : 0), false, &pSubKeys);
	if(!result){
		ERR_DKCPRN("Failed to get subkeys.");
	}else{
		if(-1 == (rescnt = K2hdkcCvtSubkeysToStringArray(pSubKeys, ppskeyarray))){
			ERR_DKCPRN("Failed to convert subkeys to string array.");
		}
	}
	DKC_DELETE(pSubKeys);
	DKC_DELETE(pComObj);

	return rescnt;
}

char** k2hdkc_pm_get_str_direct_subkeys_np(k2hdkc_chmpx_h handle, const char* pkey)
{
	char**	pskeyarray = NULL;
	if(-1 == k2hdkc_pm_get_str_subkeys_np(handle, pkey, &pskeyarray)){
		return NULL;
	}
	return pskeyarray;
}

//---------------------------------------------------------
// Functions - Get Attrs
//---------------------------------------------------------
static bool K2hdkcFullGetAttrs(K2hdkcComGetAttrs* pcomobj, const unsigned char* pkey, size_t keylength, K2HAttrs** ppAttrsObj)
{
	if(!pcomobj || !pkey || 0 == keylength || !ppAttrsObj){
		ERR_DKCPRN("Parameters are wrong.");
		return false;
	}
	return pcomobj->CommandSend(pkey, keylength, ppAttrsObj, &Last_Response_Code);
}

// [NOTE]
// This function hes not been declared in header, but has been called from k2hdkc tools.
//
bool K2hdkcCvtAttrsToPack(K2HAttrs* pAttrs, PK2HDKCATTRPCK* ppattrspck, int* pattrspckcnt)
{
	if(!pAttrs || !ppattrspck || !pattrspckcnt){
		ERR_DKCPRN("Parameters are wrong.");
		return false;
	}
	*ppattrspck	= NULL;
	*pattrspckcnt= 0UL;

	if(0 == pAttrs->size()){
		return true;
	}
	size_t			attrscnt = pAttrs->size();
	PK2HDKCATTRPCK	pattrspck;
	if(NULL == (pattrspck = reinterpret_cast<PK2HDKCATTRPCK>(calloc(attrscnt, sizeof(K2HDKCATTRPCK))))){
		ERR_DKCPRN("Could not allocate memory.");
		return false;
	}
	// copy
	int	setpos = 0;
	for(K2HAttrs::iterator iter = pAttrs->begin(); iter != pAttrs->end(); iter++){
		if(0UL == iter->keylength){
			WAN_DKCPRN("Attr key is empty.");
			continue;
		}
		if(NULL == (pattrspck[setpos].pkey = reinterpret_cast<unsigned char*>(malloc(iter->keylength)))){
			ERR_DKCPRN("Could not allocate memory.");
			DKC_FREE_ATTRPACK(pattrspck, attrscnt);
			return false;
		}
		memcpy(pattrspck[setpos].pkey, iter->pkey, iter->keylength);
		pattrspck[setpos].keylength = iter->keylength;

		if(NULL == (pattrspck[setpos].pval = reinterpret_cast<unsigned char*>(malloc(iter->vallength)))){
			ERR_DKCPRN("Could not allocate memory.");
			DKC_FREE_ATTRPACK(pattrspck, attrscnt);
			return false;
		}
		memcpy(pattrspck[setpos].pval, iter->pval, iter->vallength);
		pattrspck[setpos].vallength = iter->vallength;

		setpos++;
	}
	if(0 == setpos){
		MSG_DKCPRN("Not have attrss.");
		DKC_FREE_ATTRPACK(pattrspck, attrscnt);
	}else{
		*ppattrspck		= pattrspck;
		*pattrspckcnt	= setpos;
	}
	return true;
}

bool k2hdkc_get_attrs(const char* config, const unsigned char* pkey, size_t keylength, PK2HDKCATTRPCK* ppattrspck, int* pattrspckcnt)
{
	return k2hdkc_ex_get_attrs(config, CHM_INVALID_PORT, false, false, pkey, keylength, ppattrspck, pattrspckcnt);
}

PK2HDKCATTRPCK k2hdkc_get_direct_attrs(const char* config, const unsigned char* pkey, size_t keylength, int* pattrspckcnt)
{
	return k2hdkc_ex_get_direct_attrs(config, CHM_INVALID_PORT, false, false, pkey, keylength, pattrspckcnt);
}

PK2HDKCATTRPCK k2hdkc_get_str_direct_attrs(const char* config, const char* pkey, int* pattrspckcnt)
{
	return k2hdkc_ex_get_str_direct_attrs(config, CHM_INVALID_PORT, false, false, pkey, pattrspckcnt);
}

bool k2hdkc_ex_get_attrs(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, PK2HDKCATTRPCK* ppattrspck, int* pattrspckcnt)
{
	K2hdkcComGetAttrs*	pComObj = GetOtSlaveK2hdkcComGetAttrs(config, ctlport, is_auto_rejoin, no_giveup_rejoin);
	K2HAttrs*			pAttrs	= NULL;
	bool				result	= K2hdkcFullGetAttrs(pComObj, pkey, keylength, &pAttrs);
	if(!result){
		ERR_DKCPRN("Failed to get attrs.");
	}else{
		if(false == (result = K2hdkcCvtAttrsToPack(pAttrs, ppattrspck, pattrspckcnt))){
			ERR_DKCPRN("Failed to convert attrss to k2hattrspck array.");
		}
	}
	DKC_DELETE(pAttrs);
	DKC_DELETE(pComObj);

	return result;
}

PK2HDKCATTRPCK k2hdkc_ex_get_direct_attrs(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, int* pattrspckcnt)
{
	PK2HDKCATTRPCK	pattrspck = NULL;
	if(!k2hdkc_ex_get_attrs(config, ctlport, is_auto_rejoin, no_giveup_rejoin, pkey, keylength, &pattrspck, pattrspckcnt)){
		return NULL;
	}
	return pattrspck;
}

PK2HDKCATTRPCK k2hdkc_ex_get_str_direct_attrs(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, int* pattrspckcnt)
{
	PK2HDKCATTRPCK	pattrspck = NULL;
	if(!k2hdkc_ex_get_attrs(config, ctlport, is_auto_rejoin, no_giveup_rejoin, reinterpret_cast<const unsigned char*>(pkey), (pkey ? strlen(pkey) + 1 : 0), &pattrspck, pattrspckcnt)){
		return NULL;
	}
	return pattrspck;
}

bool k2hdkc_pm_get_attrs(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, PK2HDKCATTRPCK* ppattrspck, int* pattrspckcnt)
{
	if(K2HDKC_INVALID_HANDLE == handle){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}
	K2hdkcSlave*	pSlave = reinterpret_cast<K2hdkcSlave*>(handle);
	if(!pSlave){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}

	K2hdkcComGetAttrs*	pComObj = GetPmSlaveK2hdkcComGetAttrs(pSlave);
	K2HAttrs*			pAttrs	= NULL;
	bool				result	= K2hdkcFullGetAttrs(pComObj, pkey, keylength, &pAttrs);
	if(!result){
		ERR_DKCPRN("Failed to get attrs.");
	}else{
		if(false == (result = K2hdkcCvtAttrsToPack(pAttrs, ppattrspck, pattrspckcnt))){
			ERR_DKCPRN("Failed to convert attrss to k2hattrspck array.");
		}
	}
	DKC_DELETE(pAttrs);
	DKC_DELETE(pComObj);

	return result;
}

PK2HDKCATTRPCK k2hdkc_pm_get_direct_attrs(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, int* pattrspckcnt)
{
	PK2HDKCATTRPCK	pattrspck = NULL;
	if(!k2hdkc_pm_get_attrs(handle, pkey, keylength, &pattrspck, pattrspckcnt)){
		return NULL;
	}
	return pattrspck;
}

PK2HDKCATTRPCK k2hdkc_pm_get_str_direct_attrs(k2hdkc_chmpx_h handle, const char* pkey, int* pattrspckcnt)
{
	PK2HDKCATTRPCK	pattrspck = NULL;
	if(!k2hdkc_pm_get_attrs(handle, reinterpret_cast<const unsigned char*>(pkey), (pkey ? strlen(pkey) + 1 : 0), &pattrspck, pattrspckcnt)){
		return NULL;
	}
	return pattrspck;
}

//---------------------------------------------------------
// Functions - Set Value
//---------------------------------------------------------
static bool K2hdkcFullSet(K2hdkcComSet* pcomobj, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, bool rmsubkeylist, const char* encpass, const time_t* expire)
{
	if(!pcomobj || !pkey || 0 == keylength){
		ERR_DKCPRN("Parameters are wrong.");
		return false;
	}
	return pcomobj->CommandSend(pkey, keylength, pval, vallength, rmsubkeylist, encpass, expire, &Last_Response_Code);
}

bool k2hdkc_set_value(const char* config, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength)
{
	return k2hdkc_set_value_wa(config, pkey, keylength, pval, vallength, false, NULL, NULL);
}

bool k2hdkc_set_str_value(const char* config, const char* pkey, const char* pval)
{
	return k2hdkc_set_str_value_wa(config, pkey, pval, false, NULL, NULL);
}

bool k2hdkc_ex_set_value(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength)
{
	return k2hdkc_ex_set_value_wa(config, ctlport, is_auto_rejoin, no_giveup_rejoin, pkey, keylength, pval, vallength, false, NULL, NULL);
}

bool k2hdkc_ex_set_str_value(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, const char* pval)
{
	return k2hdkc_ex_set_str_value_wa(config, ctlport, is_auto_rejoin, no_giveup_rejoin, pkey, pval, false, NULL, NULL);
}

bool k2hdkc_pm_set_value(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength)
{
	return k2hdkc_pm_set_value_wa(handle, pkey, keylength, pval, vallength, false, NULL, NULL);
}

bool k2hdkc_pm_set_str_value(k2hdkc_chmpx_h handle, const char* pkey, const char* pval)
{
	return k2hdkc_pm_set_str_value_wa(handle, pkey, pval, false, NULL, NULL);
}

bool k2hdkc_set_value_wa(const char* config, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, bool rmsubkeylist, const char* encpass, const time_t* expire)
{
	return k2hdkc_ex_set_value_wa(config, CHM_INVALID_PORT, false, false, pkey, keylength, pval, vallength, rmsubkeylist, encpass, expire);
}

bool k2hdkc_set_str_value_wa(const char* config, const char* pkey, const char* pval, bool rmsubkeylist, const char* encpass, const time_t* expire)
{
	return k2hdkc_ex_set_str_value_wa(config, CHM_INVALID_PORT, false, false, pkey, pval, rmsubkeylist, encpass, expire);
}

bool k2hdkc_ex_set_value_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, bool rmsubkeylist, const char* encpass, const time_t* expire)
{
	K2hdkcComSet*	pComObj = GetOtSlaveK2hdkcComSet(config, ctlport, is_auto_rejoin, no_giveup_rejoin);
	bool			result	= K2hdkcFullSet(pComObj, pkey, keylength, pval, vallength, rmsubkeylist, encpass, expire);
	if(!result){
		ERR_DKCPRN("Failed to set value.");
	}
	DKC_DELETE(pComObj);
	return result;
}

bool k2hdkc_ex_set_str_value_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, const char* pval, bool rmsubkeylist, const char* encpass, const time_t* expire)
{
	return k2hdkc_ex_set_value_wa(config, ctlport, is_auto_rejoin, no_giveup_rejoin, reinterpret_cast<const unsigned char*>(pkey), (pkey ? strlen(pkey) + 1 : 0), reinterpret_cast<const unsigned char*>(pval), (pval ? strlen(pval) + 1 : 0), rmsubkeylist, encpass, expire);
}

bool k2hdkc_pm_set_value_wa(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, bool rmsubkeylist, const char* encpass, const time_t* expire)
{
	if(K2HDKC_INVALID_HANDLE == handle){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}
	K2hdkcSlave*	pSlave = reinterpret_cast<K2hdkcSlave*>(handle);
	if(!pSlave){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}
	K2hdkcComSet*	pComObj = GetPmSlaveK2hdkcComSet(pSlave);
	bool			result	= K2hdkcFullSet(pComObj, pkey, keylength, pval, vallength, rmsubkeylist, encpass, expire);
	if(!result){
		ERR_DKCPRN("Failed to set value.");
	}
	DKC_DELETE(pComObj);
	return result;
}

bool k2hdkc_pm_set_str_value_wa(k2hdkc_chmpx_h handle, const char* pkey, const char* pval, bool rmsubkeylist, const char* encpass, const time_t* expire)
{
	return k2hdkc_pm_set_value_wa(handle, reinterpret_cast<const unsigned char*>(pkey), (pkey ? strlen(pkey) + 1 : 0), reinterpret_cast<const unsigned char*>(pval), (pval ? strlen(pval) + 1 : 0), rmsubkeylist, encpass, expire);
}

//---------------------------------------------------------
// Functions - Set Direct Access(offset) Value
//---------------------------------------------------------
static bool K2hdkcFullSetDirect(K2hdkcComSetDirect* pcomobj, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, const off_t setpos)
{
	if(!pcomobj || !pkey || 0 == keylength){
		ERR_DKCPRN("Parameters are wrong.");
		return false;
	}
	return pcomobj->CommandSend(pkey, keylength, pval, vallength, setpos, &Last_Response_Code);
}

bool k2hdkc_da_set_value(const char* config, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, const off_t setpos)
{
	return k2hdkc_ex_da_set_value(config, CHM_INVALID_PORT, false, false, pkey, keylength, pval, vallength, setpos);
}

bool k2hdkc_da_set_str_value(const char* config, const char* pkey, const char* pval, const off_t setpos)
{
	return k2hdkc_ex_da_set_str_value(config, CHM_INVALID_PORT, false, false, pkey, pval, setpos);
}

bool k2hdkc_ex_da_set_value(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, const off_t setpos)
{
	K2hdkcComSetDirect*	pComObj = GetOtSlaveK2hdkcComSetDirect(config, ctlport, is_auto_rejoin, no_giveup_rejoin);
	bool				result	= K2hdkcFullSetDirect(pComObj, pkey, keylength, pval, vallength, setpos);
	if(!result){
		ERR_DKCPRN("Failed to set value directly.");
	}
	DKC_DELETE(pComObj);
	return result;
}

bool k2hdkc_ex_da_set_str_value(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, const char* pval, const off_t setpos)
{
	return k2hdkc_ex_da_set_value(config, ctlport, is_auto_rejoin, no_giveup_rejoin, reinterpret_cast<const unsigned char*>(pkey), (pkey ? strlen(pkey) + 1 : 0), reinterpret_cast<const unsigned char*>(pval), (pval ? strlen(pval) + 1 : 0), setpos);
}

bool k2hdkc_pm_da_set_value(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, const off_t setpos)
{
	if(K2HDKC_INVALID_HANDLE == handle){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}
	K2hdkcSlave*	pSlave = reinterpret_cast<K2hdkcSlave*>(handle);
	if(!pSlave){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}
	K2hdkcComSetDirect*	pComObj = GetPmSlaveK2hdkcComSetDirect(pSlave);
	bool				result	= K2hdkcFullSetDirect(pComObj, pkey, keylength, pval, vallength, setpos);
	if(!result){
		ERR_DKCPRN("Failed to set value direclty.");
	}
	DKC_DELETE(pComObj);
	return result;
}

bool k2hdkc_pm_da_set_str_value(k2hdkc_chmpx_h handle, const char* pkey, const char* pval, const off_t setpos)
{
	return k2hdkc_pm_da_set_value(handle, reinterpret_cast<const unsigned char*>(pkey), (pkey ? strlen(pkey) + 1 : 0), reinterpret_cast<const unsigned char*>(pval), (pval ? strlen(pval) + 1 : 0), setpos);
}

//---------------------------------------------------------
// Functions - Set/Clear Subkeys
//---------------------------------------------------------
static bool K2hdkcFullSetSubkeys(K2hdkcComSetSubkeys* pcomobj, const unsigned char* pkey, size_t keylength, const unsigned char* psubkeys, size_t subkeyslength)
{
	if(!pcomobj || !pkey || 0 == keylength){
		ERR_DKCPRN("Parameters are wrong.");
		return false;
	}
	if(!psubkeys || 0 == subkeyslength){
		return pcomobj->ClearSubkeysCommandSend(pkey, keylength, &Last_Response_Code);
	}else{
		return pcomobj->CommandSend(pkey, keylength, psubkeys, subkeyslength, &Last_Response_Code);
	}
}

// [NOTE]
// This function hes not been declared in header, but has been called from k2hdkc tools.
//
bool K2hdkcCvtPackToSubkeys(const PK2HDKCKEYPCK pskeypck, int skeypckcnt, unsigned char** ppsubkeys, size_t* psubkeyslength)
{
	if(!ppsubkeys || !psubkeyslength){
		ERR_K2HPRN("Parameter is wrong.");
		return false;
	}
	*ppsubkeys		= NULL;
	*psubkeyslength	= 0UL;

	if(!pskeypck || skeypckcnt <= 0){
		return true;
	}

	K2HSubKeys	Subkeys;
	for(int cnt = 0; cnt < skeypckcnt; cnt++){
		if(Subkeys.end() == Subkeys.insert(pskeypck[cnt].pkey, pskeypck[cnt].length)){
			ERR_K2HPRN("Subkeys array is something wrong.");
			return false;
		}
	}
	if(0UL < Subkeys.size()){
		if(!Subkeys.Serialize(ppsubkeys, (*psubkeyslength))){
			ERR_K2HPRN("Could not make subkeys binary array.");
			return false;
		}
	}
	return true;
}

static bool K2hdkcCvtStringArrayToSubkeys(const char** pskeyarray, unsigned char** ppsubkeys, size_t* psubkeyslength)
{
	if(!ppsubkeys || !psubkeyslength){
		ERR_K2HPRN("Parameter is wrong.");
		return false;
	}
	*ppsubkeys		= NULL;
	*psubkeyslength	= 0UL;

	if(!pskeyarray){
		return true;
	}

	K2HSubKeys	Subkeys;
	for(; pskeyarray && *pskeyarray; pskeyarray++){
		if(Subkeys.end() == Subkeys.insert(*pskeyarray)){
			ERR_K2HPRN("Subkeys array is something wrong.");
			return false;
		}
	}
	if(0UL < Subkeys.size()){
		if(!Subkeys.Serialize(ppsubkeys, (*psubkeyslength))){
			ERR_K2HPRN("Could not make subkeys binary array.");
			return false;
		}
	}
	return true;
}

bool k2hdkc_set_subkeys(const char* config, const unsigned char* pkey, size_t keylength, const PK2HDKCKEYPCK pskeypck, int skeypckcnt)
{
	return k2hdkc_ex_set_subkeys(config, CHM_INVALID_PORT, false, false, pkey, keylength, pskeypck, skeypckcnt);
}

bool k2hdkc_set_str_subkeys(const char* config, const char* pkey, const char** pskeyarray)
{
	return k2hdkc_ex_set_str_subkeys(config, CHM_INVALID_PORT, false, false, pkey, pskeyarray);
}

bool k2hdkc_ex_set_subkeys(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, const PK2HDKCKEYPCK pskeypck, int skeypckcnt)
{
	unsigned char*	psubkeys		= NULL;
	size_t			subkeyslength	= 0UL;
	if(!K2hdkcCvtPackToSubkeys(pskeypck, skeypckcnt, &psubkeys, &subkeyslength)){
		ERR_DKCPRN("Could not convert subkey pack to binary data.");
		return false;
	}
	K2hdkcComSetSubkeys*	pComObj = GetOtSlaveK2hdkcComSetSubkeys(config, ctlport, is_auto_rejoin, no_giveup_rejoin);
	bool					result	= K2hdkcFullSetSubkeys(pComObj, pkey, keylength, psubkeys, subkeyslength);
	if(!result){
		ERR_DKCPRN("Failed to set bukeys.");
	}
	DKC_FREE(psubkeys);
	DKC_DELETE(pComObj);
	return result;
}

bool k2hdkc_ex_set_str_subkeys(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, const char** pskeyarray)
{
	unsigned char*	psubkeys		= NULL;
	size_t			subkeyslength	= 0UL;
	if(!K2hdkcCvtStringArrayToSubkeys(pskeyarray, &psubkeys, &subkeyslength)){
		ERR_DKCPRN("Could not convert subkey pack to binary data.");
		return false;
	}
	K2hdkcComSetSubkeys*	pComObj = GetOtSlaveK2hdkcComSetSubkeys(config, ctlport, is_auto_rejoin, no_giveup_rejoin);
	bool					result	= K2hdkcFullSetSubkeys(pComObj, reinterpret_cast<const unsigned char*>(pkey), (pkey ? strlen(pkey) + 1 : 0), psubkeys, subkeyslength);
	if(!result){
		ERR_DKCPRN("Failed to set bukeys.");
	}
	DKC_FREE(psubkeys);
	DKC_DELETE(pComObj);
	return result;
}

bool k2hdkc_pm_set_subkeys(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, const PK2HDKCKEYPCK pskeypck, int skeypckcnt)
{
	if(K2HDKC_INVALID_HANDLE == handle){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}
	K2hdkcSlave*	pSlave = reinterpret_cast<K2hdkcSlave*>(handle);
	if(!pSlave){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}

	unsigned char*	psubkeys		= NULL;
	size_t			subkeyslength	= 0UL;
	if(!K2hdkcCvtPackToSubkeys(pskeypck, skeypckcnt, &psubkeys, &subkeyslength)){
		ERR_DKCPRN("Could not convert subkey pack to binary data.");
		return false;
	}
	K2hdkcComSetSubkeys*	pComObj = GetPmSlaveK2hdkcComSetSubkeys(pSlave);
	bool					result	= K2hdkcFullSetSubkeys(pComObj, pkey, keylength, psubkeys, subkeyslength);
	if(!result){
		ERR_DKCPRN("Failed to set bukeys.");
	}
	DKC_FREE(psubkeys);
	DKC_DELETE(pComObj);
	return result;
}

bool k2hdkc_pm_set_str_subkeys(k2hdkc_chmpx_h handle, const char* pkey, const char** pskeyarray)
{
	if(K2HDKC_INVALID_HANDLE == handle){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}
	K2hdkcSlave*	pSlave = reinterpret_cast<K2hdkcSlave*>(handle);
	if(!pSlave){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}

	unsigned char*	psubkeys		= NULL;
	size_t			subkeyslength	= 0UL;
	if(!K2hdkcCvtStringArrayToSubkeys(pskeyarray, &psubkeys, &subkeyslength)){
		ERR_DKCPRN("Could not convert subkey pack to binary data.");
		return false;
	}
	K2hdkcComSetSubkeys*	pComObj = GetPmSlaveK2hdkcComSetSubkeys(pSlave);
	bool					result	= K2hdkcFullSetSubkeys(pComObj, reinterpret_cast<const unsigned char*>(pkey), (pkey ? strlen(pkey) + 1 : 0), psubkeys, subkeyslength);
	if(!result){
		ERR_DKCPRN("Failed to set bukeys.");
	}
	DKC_FREE(psubkeys);
	DKC_DELETE(pComObj);
	return result;
}

bool k2hdkc_clear_subkeys(const char* config, const unsigned char* pkey, size_t keylength)
{
	return k2hdkc_set_subkeys(config, pkey, keylength, NULL, 0);
}

bool k2hdkc_clear_str_subkeys(const char* config, const char* pkey)
{
	return k2hdkc_set_str_subkeys(config, pkey, NULL);
}

bool k2hdkc_ex_clear_subkeys(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength)
{
	return k2hdkc_ex_set_subkeys(config, ctlport, is_auto_rejoin, no_giveup_rejoin, pkey, keylength, NULL, 0);
}

bool k2hdkc_ex_clear_str_subkeys(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey)
{
	return k2hdkc_ex_set_str_subkeys(config, ctlport, is_auto_rejoin, no_giveup_rejoin, pkey, NULL);
}

bool k2hdkc_pm_clear_subkeys(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength)
{
	return k2hdkc_pm_set_subkeys(handle, pkey, keylength, NULL, 0);
}

bool k2hdkc_pm_clear_str_subkeys(k2hdkc_chmpx_h handle, const char* pkey)
{
	return k2hdkc_pm_set_str_subkeys(handle, pkey, NULL);
}

//---------------------------------------------------------
// Functions - Add Subkey
//---------------------------------------------------------
static bool K2hdkcFullAddSubkey(K2hdkcComAddSubkey* pcomobj, const unsigned char* pkey, size_t keylength, const unsigned char* psubkey, size_t subkeylength, const unsigned char* pskeyval, size_t skeyvallength, bool checkattr, const char* encpass, const time_t* expire)
{
	if(!pcomobj || !pkey || 0 == keylength || !psubkey || 0 == subkeylength){
		ERR_DKCPRN("Parameters are wrong.");
		return false;
	}
	return pcomobj->CommandSend(pkey, keylength, psubkey, subkeylength, pskeyval, skeyvallength, checkattr, encpass, expire, &Last_Response_Code);
}

bool k2hdkc_set_subkey(const char* config, const unsigned char* pkey, size_t keylength, const unsigned char* psubkey, size_t subkeylength, const unsigned char* pskeyval, size_t skeyvallength)
{
	return k2hdkc_set_subkey_wa(config, pkey, keylength, psubkey, subkeylength, pskeyval, skeyvallength, true, NULL, NULL);
}

bool k2hdkc_set_str_subkey(const char* config, const char* pkey, const char* psubkey, const char* pskeyval)
{
	return k2hdkc_set_str_subkey_wa(config, pkey, psubkey, pskeyval, true, NULL, NULL);
}

bool k2hdkc_ex_set_subkey(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, const unsigned char* psubkey, size_t subkeylength, const unsigned char* pskeyval, size_t skeyvallength)
{
	return k2hdkc_ex_set_subkey_wa(config, ctlport, is_auto_rejoin, no_giveup_rejoin, pkey, keylength, psubkey, subkeylength, pskeyval, skeyvallength, true, NULL, NULL);
}

bool k2hdkc_ex_set_str_subkey(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, const char* psubkey, const char* pskeyval)
{
	return k2hdkc_ex_set_str_subkey_wa(config, ctlport, is_auto_rejoin, no_giveup_rejoin, pkey, psubkey, pskeyval, true, NULL, NULL);
}

bool k2hdkc_pm_set_subkey(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, const unsigned char* psubkey, size_t subkeylength, const unsigned char* pskeyval, size_t skeyvallength)
{
	return k2hdkc_pm_set_subkey_wa(handle, pkey, keylength, psubkey, subkeylength, pskeyval, skeyvallength, true, NULL, NULL);
}

bool k2hdkc_pm_set_str_subkey(k2hdkc_chmpx_h handle, const char* pkey, const char* psubkey, const char* pskeyval)
{
	return k2hdkc_pm_set_str_subkey_wa(handle, pkey, psubkey, pskeyval, true, NULL, NULL);
}

bool k2hdkc_set_subkey_wa(const char* config, const unsigned char* pkey, size_t keylength, const unsigned char* psubkey, size_t subkeylength, const unsigned char* pskeyval, size_t skeyvallength, bool checkattr, const char* encpass, const time_t* expire)
{
	return k2hdkc_ex_set_subkey_wa(config, CHM_INVALID_PORT, false, false, pkey, keylength, psubkey, subkeylength, pskeyval, skeyvallength, checkattr, encpass, expire);
}

bool k2hdkc_set_str_subkey_wa(const char* config, const char* pkey, const char* psubkey, const char* pskeyval, bool checkattr, const char* encpass, const time_t* expire)
{
	return k2hdkc_ex_set_str_subkey_wa(config, CHM_INVALID_PORT, false, false, pkey, psubkey, pskeyval, checkattr, encpass, expire);
}

bool k2hdkc_ex_set_subkey_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, const unsigned char* psubkey, size_t subkeylength, const unsigned char* pskeyval, size_t skeyvallength, bool checkattr, const char* encpass, const time_t* expire)
{
	K2hdkcComAddSubkey*	pComObj = GetOtSlaveK2hdkcComAddSubkey(config, ctlport, is_auto_rejoin, no_giveup_rejoin);
	bool				result	= K2hdkcFullAddSubkey(pComObj, pkey, keylength, psubkey, subkeylength, pskeyval, skeyvallength, checkattr, encpass, expire);
	if(!result){
		ERR_DKCPRN("Failed to set subkey with value.");
	}
	DKC_DELETE(pComObj);
	return result;
}

bool k2hdkc_ex_set_str_subkey_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, const char* psubkey, const char* pskeyval, bool checkattr, const char* encpass, const time_t* expire)
{
	return k2hdkc_ex_set_subkey_wa(config, ctlport, is_auto_rejoin, no_giveup_rejoin, reinterpret_cast<const unsigned char*>(pkey), (pkey ? strlen(pkey) + 1 : 0), reinterpret_cast<const unsigned char*>(psubkey), (psubkey ? strlen(psubkey) + 1 : 0), reinterpret_cast<const unsigned char*>(pskeyval), (pskeyval ? strlen(pskeyval) + 1 : 0), checkattr, encpass, expire);
}

bool k2hdkc_pm_set_subkey_wa(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, const unsigned char* psubkey, size_t subkeylength, const unsigned char* pskeyval, size_t skeyvallength, bool checkattr, const char* encpass, const time_t* expire)
{
	if(K2HDKC_INVALID_HANDLE == handle){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}
	K2hdkcSlave*	pSlave = reinterpret_cast<K2hdkcSlave*>(handle);
	if(!pSlave){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}
	K2hdkcComAddSubkey*	pComObj = GetPmSlaveK2hdkcComAddSubkey(pSlave);
	bool				result	= K2hdkcFullAddSubkey(pComObj, pkey, keylength, psubkey, subkeylength, pskeyval, skeyvallength, checkattr, encpass, expire);
	if(!result){
		ERR_DKCPRN("Failed to set subkey with value.");
	}
	DKC_DELETE(pComObj);
	return result;
}

bool k2hdkc_pm_set_str_subkey_wa(k2hdkc_chmpx_h handle, const char* pkey, const char* psubkey, const char* pskeyval, bool checkattr, const char* encpass, const time_t* expire)
{
	return k2hdkc_pm_set_subkey_wa(handle, reinterpret_cast<const unsigned char*>(pkey), (pkey ? strlen(pkey) + 1 : 0), reinterpret_cast<const unsigned char*>(psubkey), (psubkey ? strlen(psubkey) + 1 : 0), reinterpret_cast<const unsigned char*>(pskeyval), (pskeyval ? strlen(pskeyval) + 1 : 0), checkattr, encpass, expire);
}

//---------------------------------------------------------
// Functions - Set All
//---------------------------------------------------------
static bool K2hdkcFullSetAll(K2hdkcComSetAll* pcomobj, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, const unsigned char* psubkeys, size_t subkeyslength)
{
	if(!pcomobj || !pkey || 0 == keylength){
		ERR_DKCPRN("Parameters are wrong.");
		return false;
	}
	return pcomobj->CommandSend(pkey, keylength, pval, vallength, psubkeys, subkeyslength, NULL, 0UL, &Last_Response_Code);	// [NOTE] Not set(replace) attributes
}

bool k2hdkc_set_all(const char* config, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, const PK2HDKCKEYPCK pskeypck, int skeypckcnt)
{
	return k2hdkc_ex_set_all(config, CHM_INVALID_PORT, false, false, pkey, keylength, pval, vallength, pskeypck, skeypckcnt);
}

bool k2hdkc_set_str_all(const char* config, const char* pkey, const char* pval, const char** pskeyarray)
{
	return k2hdkc_ex_set_str_all(config, CHM_INVALID_PORT, false, false, pkey, pval, pskeyarray);
}

bool k2hdkc_ex_set_all(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, const PK2HDKCKEYPCK pskeypck, int skeypckcnt)
{
	unsigned char*	psubkeys		= NULL;
	size_t			subkeyslength	= 0UL;
	if(!K2hdkcCvtPackToSubkeys(pskeypck, skeypckcnt, &psubkeys, &subkeyslength)){
		ERR_DKCPRN("Could not convert subkey pack to binary data.");
		return false;
	}
	K2hdkcComSetAll*	pComObj = GetOtSlaveK2hdkcComSetAll(config, ctlport, is_auto_rejoin, no_giveup_rejoin);
	bool				result	= K2hdkcFullSetAll(pComObj, pkey, keylength, pval, vallength, psubkeys, subkeyslength);
	if(!result){
		ERR_DKCPRN("Failed to set bukeys.");
	}
	DKC_FREE(psubkeys);
	DKC_DELETE(pComObj);
	return result;
}

bool k2hdkc_ex_set_str_all(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, const char* pval, const char** pskeyarray)
{
	unsigned char*	psubkeys		= NULL;
	size_t			subkeyslength	= 0UL;
	if(!K2hdkcCvtStringArrayToSubkeys(pskeyarray, &psubkeys, &subkeyslength)){
		ERR_DKCPRN("Could not convert subkey pack to binary data.");
		return false;
	}
	K2hdkcComSetAll*	pComObj = GetOtSlaveK2hdkcComSetAll(config, ctlport, is_auto_rejoin, no_giveup_rejoin);
	bool				result	= K2hdkcFullSetAll(pComObj, reinterpret_cast<const unsigned char*>(pkey), (pkey ? strlen(pkey) + 1 : 0), reinterpret_cast<const unsigned char*>(pval), (pval ? strlen(pval) + 1 : 0), psubkeys, subkeyslength);
	if(!result){
		ERR_DKCPRN("Failed to set bukeys.");
	}
	DKC_FREE(psubkeys);
	DKC_DELETE(pComObj);
	return result;
}

bool k2hdkc_pm_set_all(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, const PK2HDKCKEYPCK pskeypck, int skeypckcnt)
{
	if(K2HDKC_INVALID_HANDLE == handle){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}
	K2hdkcSlave*	pSlave = reinterpret_cast<K2hdkcSlave*>(handle);
	if(!pSlave){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}

	unsigned char*	psubkeys		= NULL;
	size_t			subkeyslength	= 0UL;
	if(!K2hdkcCvtPackToSubkeys(pskeypck, skeypckcnt, &psubkeys, &subkeyslength)){
		ERR_DKCPRN("Could not convert subkey pack to binary data.");
		return false;
	}
	K2hdkcComSetAll*	pComObj = GetPmSlaveK2hdkcComSetAll(pSlave);
	bool				result	= K2hdkcFullSetAll(pComObj, pkey, keylength, pval, vallength, psubkeys, subkeyslength);
	if(!result){
		ERR_DKCPRN("Failed to set bukeys.");
	}
	DKC_FREE(psubkeys);
	DKC_DELETE(pComObj);
	return result;
}

bool k2hdkc_pm_set_str_all(k2hdkc_chmpx_h handle, const char* pkey, const char* pval, const char** pskeyarray)
{
	if(K2HDKC_INVALID_HANDLE == handle){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}
	K2hdkcSlave*	pSlave = reinterpret_cast<K2hdkcSlave*>(handle);
	if(!pSlave){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}

	unsigned char*	psubkeys		= NULL;
	size_t			subkeyslength	= 0UL;
	if(!K2hdkcCvtStringArrayToSubkeys(pskeyarray, &psubkeys, &subkeyslength)){
		ERR_DKCPRN("Could not convert subkey pack to binary data.");
		return false;
	}
	K2hdkcComSetAll*	pComObj = GetPmSlaveK2hdkcComSetAll(pSlave);
	bool				result	= K2hdkcFullSetAll(pComObj, reinterpret_cast<const unsigned char*>(pkey), (pkey ? strlen(pkey) + 1 : 0), reinterpret_cast<const unsigned char*>(pval), (pval ? strlen(pval) + 1 : 0), psubkeys, subkeyslength);
	if(!result){
		ERR_DKCPRN("Failed to set bukeys.");
	}
	DKC_FREE(psubkeys);
	DKC_DELETE(pComObj);
	return result;
}

bool k2hdkc_set_all_wa(const char* config, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, const PK2HDKCKEYPCK pskeypck, int skeypckcnt, const char* encpass, const time_t* expire)
{
	return k2hdkc_ex_set_all_wa(config, CHM_INVALID_PORT, false, false, pkey, keylength, pval, vallength, pskeypck, skeypckcnt, encpass, expire);
}

bool k2hdkc_set_str_all_wa(const char* config, const char* pkey, const char* pval, const char** pskeyarray, const char* encpass, const time_t* expire)
{
	return k2hdkc_ex_set_str_all_wa(config, CHM_INVALID_PORT, false, false, pkey, pval, pskeyarray, encpass, expire);
}

bool k2hdkc_ex_set_all_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, const PK2HDKCKEYPCK pskeypck, int skeypckcnt, const char* encpass, const time_t* expire)
{
	k2hdkc_chmpx_h	handle = k2hdkc_open_chmpx_ex(config, ctlport, is_auto_rejoin, no_giveup_rejoin, true);
	if(K2HDKC_INVALID_HANDLE == handle){
		ERR_DKCPRN("Could not open(join) shmpx msgid on slave node.");
		return false;
	}
	bool	result;
	if(false == (result = k2hdkc_pm_set_all_wa(handle, pkey, keylength, pval, vallength, pskeypck, skeypckcnt, encpass, expire))){
		ERR_DKCPRN("Failed to set key-value with subkeys.");
	}
	if(!k2hdkc_close_chmpx(handle)){
		ERR_DKCPRN("Failed to close chmpx slave node, but continue...");
	}
	return result;
}

bool k2hdkc_ex_set_str_all_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, const char* pval, const char** pskeyarray, const char* encpass, const time_t* expire)
{
	k2hdkc_chmpx_h	handle = k2hdkc_open_chmpx_ex(config, ctlport, is_auto_rejoin, no_giveup_rejoin, true);
	if(K2HDKC_INVALID_HANDLE == handle){
		ERR_DKCPRN("Could not open(join) shmpx msgid on slave node.");
		return false;
	}
	bool	result;
	if(false == (result = k2hdkc_pm_set_str_all_wa(handle, pkey, pval, pskeyarray, encpass, expire))){
		ERR_DKCPRN("Failed to set key-value with subkeys.");
	}
	if(!k2hdkc_close_chmpx(handle)){
		ERR_DKCPRN("Failed to close chmpx slave node, but continue...");
	}
	return result;
}

bool k2hdkc_pm_set_all_wa(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, const PK2HDKCKEYPCK pskeypck, int skeypckcnt, const char* encpass, const time_t* expire)
{
	if(!k2hdkc_pm_set_value_wa(handle, pkey, keylength, pval, true, vallength, encpass, expire)){
		ERR_DKCPRN("Could not set key-value before setting subkeys.");
		return false;
	}
	if(!k2hdkc_pm_set_subkeys(handle, pkey, keylength, pskeypck, skeypckcnt)){
		ERR_DKCPRN("Could not set subkeys after setting key-value.");
		return false;
	}
	return true;
}

bool k2hdkc_pm_set_str_all_wa(k2hdkc_chmpx_h handle, const char* pkey, const char* pval, const char** pskeyarray, const char* encpass, const time_t* expire)
{
	if(!k2hdkc_pm_set_str_value_wa(handle, pkey, pval, true, encpass, expire)){
		ERR_DKCPRN("Could not set key-value before setting subkeys.");
		return false;
	}
	if(!k2hdkc_pm_set_str_subkeys(handle, pkey, pskeyarray)){
		ERR_DKCPRN("Could not set subkeys after setting key-value.");
		return false;
	}
	return true;
}

//---------------------------------------------------------
// Functions - Remove All
//---------------------------------------------------------
static bool K2hdkcFullDel(K2hdkcComDel* pcomobj, const unsigned char* pkey, size_t keylength, bool is_subkeys)
{
	if(!pcomobj || !pkey || 0 == keylength){
		ERR_DKCPRN("Parameters are wrong.");
		return false;
	}
	return pcomobj->CommandSend(pkey, keylength, is_subkeys, true, &Last_Response_Code);
}

bool k2hdkc_remove_all(const char* config, const unsigned char* pkey, size_t keylength)
{
	return k2hdkc_ex_remove_all(config, CHM_INVALID_PORT, false, false, pkey, keylength);
}

bool k2hdkc_remove_str_all(const char* config, const char* pkey)
{
	return k2hdkc_ex_remove_str_all(config, CHM_INVALID_PORT, false, false, pkey);
}

bool k2hdkc_ex_remove_all(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength)
{
	K2hdkcComDel*	pComObj = GetOtSlaveK2hdkcComDel(config, ctlport, is_auto_rejoin, no_giveup_rejoin);
	bool			result	= K2hdkcFullDel(pComObj, pkey, keylength, true);
	if(!result){
		ERR_DKCPRN("Failed to remove all.");
	}
	DKC_DELETE(pComObj);
	return result;
}

bool k2hdkc_ex_remove_str_all(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey)
{
	return k2hdkc_ex_remove_all(config, ctlport, is_auto_rejoin, no_giveup_rejoin, reinterpret_cast<const unsigned char*>(pkey), (pkey ? strlen(pkey) + 1 : 0));
}

bool k2hdkc_pm_remove_all(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength)
{
	if(K2HDKC_INVALID_HANDLE == handle){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}
	K2hdkcSlave*	pSlave = reinterpret_cast<K2hdkcSlave*>(handle);
	if(!pSlave){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}

	K2hdkcComDel*	pComObj = GetPmSlaveK2hdkcComDel(pSlave);
	bool			result	= K2hdkcFullDel(pComObj, pkey, keylength, true);
	if(!result){
		ERR_DKCPRN("Failed to remove all.");
	}
	DKC_DELETE(pComObj);
	return result;
}

bool k2hdkc_pm_remove_str_all(k2hdkc_chmpx_h handle, const char* pkey)
{
	return k2hdkc_pm_remove_all(handle, reinterpret_cast<const unsigned char*>(pkey), (pkey ? strlen(pkey) + 1 : 0));
}

bool k2hdkc_remove(const char* config, const unsigned char* pkey, size_t keylength)
{
	return k2hdkc_ex_remove(config, CHM_INVALID_PORT, false, false, pkey, keylength);
}

bool k2hdkc_remove_str(const char* config, const char* pkey)
{
	return k2hdkc_ex_remove_str(config, CHM_INVALID_PORT, false, false, pkey);
}

bool k2hdkc_ex_remove(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength)
{
	K2hdkcComDel*	pComObj = GetOtSlaveK2hdkcComDel(config, ctlport, is_auto_rejoin, no_giveup_rejoin);
	bool			result	= K2hdkcFullDel(pComObj, pkey, keylength, false);
	if(!result){
		ERR_DKCPRN("Failed to remove key.");
	}
	DKC_DELETE(pComObj);
	return result;
}

bool k2hdkc_ex_remove_str(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey)
{
	return k2hdkc_ex_remove(config, ctlport, is_auto_rejoin, no_giveup_rejoin, reinterpret_cast<const unsigned char*>(pkey), (pkey ? strlen(pkey) + 1 : 0));
}

bool k2hdkc_pm_remove(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength)
{
	if(K2HDKC_INVALID_HANDLE == handle){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}
	K2hdkcSlave*	pSlave = reinterpret_cast<K2hdkcSlave*>(handle);
	if(!pSlave){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}

	K2hdkcComDel*	pComObj = GetPmSlaveK2hdkcComDel(pSlave);
	bool			result	= K2hdkcFullDel(pComObj, pkey, keylength, false);
	if(!result){
		ERR_DKCPRN("Failed to remove key.");
	}
	DKC_DELETE(pComObj);
	return result;
}

bool k2hdkc_pm_remove_str(k2hdkc_chmpx_h handle, const char* pkey)
{
	return k2hdkc_pm_remove(handle, reinterpret_cast<const unsigned char*>(pkey), (pkey ? strlen(pkey) + 1 : 0));
}

//---------------------------------------------------------
// Functions - Remove Subkey
//---------------------------------------------------------
static bool K2hdkcFullDelSubkey(K2hdkcComDelSubkey* pcomobj, const unsigned char* pkey, size_t keylength, const unsigned char* psubkey, size_t subkeylength, bool is_nest, bool checkattr)
{
	if(!pcomobj || !pkey || 0 == keylength || !psubkey || 0 == subkeylength){
		ERR_DKCPRN("Parameters are wrong.");
		return false;
	}
	return pcomobj->CommandSend(pkey, keylength, psubkey, subkeylength, is_nest, checkattr, &Last_Response_Code);
}

bool k2hdkc_remove_subkey(const char* config, const unsigned char* pkey, size_t keylength, const unsigned char* psubkey, size_t subkeylength, bool is_nest)
{
	return k2hdkc_ex_remove_subkey(config, CHM_INVALID_PORT, false, false, pkey, keylength, psubkey, subkeylength, is_nest);
}

bool k2hdkc_remove_str_subkey(const char* config, const char* pkey, const char* psubkey, bool is_nest)
{
	return k2hdkc_ex_remove_str_subkey(config, CHM_INVALID_PORT, false, false, pkey, psubkey, is_nest);
}

bool k2hdkc_ex_remove_subkey(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, const unsigned char* psubkey, size_t subkeylength, bool is_nest)
{
	K2hdkcComDelSubkey*	pComObj = GetOtSlaveK2hdkcComDelSubkey(config, ctlport, is_auto_rejoin, no_giveup_rejoin);
	bool				result	= K2hdkcFullDelSubkey(pComObj, pkey, keylength, psubkey, subkeylength, is_nest, true);
	if(!result){
		ERR_DKCPRN("Failed to remove sbukey.");
	}
	DKC_DELETE(pComObj);
	return result;
}

bool k2hdkc_ex_remove_str_subkey(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, const char* psubkey, bool is_nest)
{
	return k2hdkc_ex_remove_subkey(config, ctlport, is_auto_rejoin, no_giveup_rejoin, reinterpret_cast<const unsigned char*>(pkey), (pkey ? strlen(pkey) + 1 : 0), reinterpret_cast<const unsigned char*>(psubkey), (psubkey ? strlen(psubkey) + 1 : 0), is_nest);
}

bool k2hdkc_pm_remove_subkey(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, const unsigned char* psubkey, size_t subkeylength, bool is_nest)
{
	if(K2HDKC_INVALID_HANDLE == handle){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}
	K2hdkcSlave*	pSlave = reinterpret_cast<K2hdkcSlave*>(handle);
	if(!pSlave){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}

	K2hdkcComDelSubkey*	pComObj = GetPmSlaveK2hdkcComDelSubkey(pSlave);
	bool				result	= K2hdkcFullDelSubkey(pComObj, pkey, keylength, psubkey, subkeylength, is_nest, true);
	if(!result){
		ERR_DKCPRN("Failed to remove sbukey.");
	}
	DKC_DELETE(pComObj);
	return result;
}

bool k2hdkc_pm_remove_str_subkey(k2hdkc_chmpx_h handle, const char* pkey, const char* psubkey, size_t subkeylength, bool is_nest)
{
	return k2hdkc_pm_remove_subkey(handle, reinterpret_cast<const unsigned char*>(pkey), (pkey ? strlen(pkey) + 1 : 0), reinterpret_cast<const unsigned char*>(psubkey), (psubkey ? strlen(psubkey) + 1 : 0), is_nest);
}

bool k2hdkc_remove_subkey_np(const char* config, const unsigned char* pkey, size_t keylength, const unsigned char* psubkey, size_t subkeylength, bool is_nest)
{
	return k2hdkc_ex_remove_subkey_np(config, CHM_INVALID_PORT, false, false, pkey, keylength, psubkey, subkeylength, is_nest);
}

bool k2hdkc_remove_str_subkey_np(const char* config, const char* pkey, const char* psubkey, bool is_nest)
{
	return k2hdkc_ex_remove_str_subkey_np(config, CHM_INVALID_PORT, false, false, pkey, psubkey, is_nest);
}

bool k2hdkc_ex_remove_subkey_np(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, const unsigned char* psubkey, size_t subkeylength, bool is_nest)
{
	K2hdkcComDelSubkey*	pComObj = GetOtSlaveK2hdkcComDelSubkey(config, ctlport, is_auto_rejoin, no_giveup_rejoin);
	bool				result	= K2hdkcFullDelSubkey(pComObj, pkey, keylength, psubkey, subkeylength, is_nest, false);
	if(!result){
		ERR_DKCPRN("Failed to remove sbukey.");
	}
	DKC_DELETE(pComObj);
	return result;
}

bool k2hdkc_ex_remove_str_subkey_np(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, const char* psubkey, bool is_nest)
{
	return k2hdkc_ex_remove_subkey_np(config, ctlport, is_auto_rejoin, no_giveup_rejoin, reinterpret_cast<const unsigned char*>(pkey), (pkey ? strlen(pkey) + 1 : 0), reinterpret_cast<const unsigned char*>(psubkey), (psubkey ? strlen(psubkey) + 1 : 0), is_nest);
}

bool k2hdkc_pm_remove_subkey_np(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, const unsigned char* psubkey, size_t subkeylength, bool is_nest)
{
	if(K2HDKC_INVALID_HANDLE == handle){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}
	K2hdkcSlave*	pSlave = reinterpret_cast<K2hdkcSlave*>(handle);
	if(!pSlave){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}

	K2hdkcComDelSubkey*	pComObj = GetPmSlaveK2hdkcComDelSubkey(pSlave);
	bool				result	= K2hdkcFullDelSubkey(pComObj, pkey, keylength, psubkey, subkeylength, is_nest, false);
	if(!result){
		ERR_DKCPRN("Failed to remove sbukey.");
	}
	DKC_DELETE(pComObj);
	return result;
}

bool k2hdkc_pm_remove_str_subkey_np(k2hdkc_chmpx_h handle, const char* pkey, const char* psubkey, size_t subkeylength, bool is_nest)
{
	return k2hdkc_pm_remove_subkey_np(handle, reinterpret_cast<const unsigned char*>(pkey), (pkey ? strlen(pkey) + 1 : 0), reinterpret_cast<const unsigned char*>(psubkey), (psubkey ? strlen(psubkey) + 1 : 0), is_nest);
}

//---------------------------------------------------------
// Functions - Rename key
//---------------------------------------------------------
static bool K2hdkcFullRen(K2hdkcComRen* pcomobj, const unsigned char* poldkey, size_t oldkeylength, const unsigned char* pnewkey, size_t newkeylength, const unsigned char* pparentkey, size_t parentkeylength, bool checkattr, const char* encpass, const time_t* expire)
{
	if(!pcomobj || !poldkey || 0 == oldkeylength || !pnewkey || 0 == newkeylength){
		ERR_DKCPRN("Parameters are wrong.");
		return false;
	}
	return pcomobj->CommandSend(poldkey, oldkeylength, pnewkey, newkeylength, pparentkey, parentkeylength, checkattr, encpass, expire, &Last_Response_Code);
}

bool k2hdkc_rename(const char* config, const unsigned char* poldkey, size_t oldkeylength, const unsigned char* pnewkey, size_t newkeylength)
{
	return k2hdkc_rename_with_parent(config, poldkey, oldkeylength, pnewkey, newkeylength, NULL, 0UL);
}

bool k2hdkc_rename_str(const char* config, const char* poldkey, const char* pnewkey)
{
	return k2hdkc_rename_with_parent_str(config, poldkey, pnewkey, NULL);
}

bool k2hdkc_ex_rename(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* poldkey, size_t oldkeylength, const unsigned char* pnewkey, size_t newkeylength)
{
	return k2hdkc_ex_rename_with_parent(config, ctlport, is_auto_rejoin, no_giveup_rejoin, poldkey, oldkeylength, pnewkey, newkeylength, NULL, 0UL);
}

bool k2hdkc_ex_rename_str(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* poldkey, const char* pnewkey)
{
	return k2hdkc_ex_rename_with_parent_str(config, ctlport, is_auto_rejoin, no_giveup_rejoin, poldkey, pnewkey, NULL);
}

bool k2hdkc_pm_rename(k2hdkc_chmpx_h handle, const unsigned char* poldkey, size_t oldkeylength, const unsigned char* pnewkey, size_t newkeylength)
{
	return k2hdkc_pm_rename_with_parent(handle, poldkey, oldkeylength, pnewkey, newkeylength, NULL, 0UL);
}

bool k2hdkc_pm_rename_str(k2hdkc_chmpx_h handle, const char* poldkey, const char* pnewkey)
{
	return k2hdkc_pm_rename_with_parent_str(handle, poldkey, pnewkey, NULL);
}

bool k2hdkc_rename_wa(const char* config, const unsigned char* poldkey, size_t oldkeylength, const unsigned char* pnewkey, size_t newkeylength, bool checkattr, const char* encpass, const time_t* expire)
{
	return k2hdkc_rename_with_parent_wa(config, poldkey, oldkeylength, pnewkey, newkeylength, NULL, 0UL, checkattr, encpass, expire);
}

bool k2hdkc_rename_str_wa(const char* config, const char* poldkey, const char* pnewkey, bool checkattr, const char* encpass, const time_t* expire)
{
	return k2hdkc_rename_with_parent_str_wa(config, poldkey, pnewkey, NULL, checkattr, encpass, expire);
}

bool k2hdkc_ex_rename_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* poldkey, size_t oldkeylength, const unsigned char* pnewkey, size_t newkeylength, bool checkattr, const char* encpass, const time_t* expire)
{
	return k2hdkc_ex_rename_with_parent_wa(config, ctlport, is_auto_rejoin, no_giveup_rejoin, poldkey, oldkeylength, pnewkey, newkeylength, NULL, 0UL, checkattr, encpass, expire);
}

bool k2hdkc_ex_rename_str_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* poldkey, const char* pnewkey, bool checkattr, const char* encpass, const time_t* expire)
{
	return k2hdkc_ex_rename_with_parent_str_wa(config, ctlport, is_auto_rejoin, no_giveup_rejoin, poldkey, pnewkey, NULL, checkattr, encpass, expire);
}

bool k2hdkc_pm_rename_wa(k2hdkc_chmpx_h handle, const unsigned char* poldkey, size_t oldkeylength, const unsigned char* pnewkey, size_t newkeylength, bool checkattr, const char* encpass, const time_t* expire)

{
	return k2hdkc_pm_rename_with_parent_wa(handle, poldkey, oldkeylength, pnewkey, newkeylength, NULL, 0UL, checkattr, encpass, expire);
}

bool k2hdkc_pm_rename_str_wa(k2hdkc_chmpx_h handle, const char* poldkey, const char* pnewkey, bool checkattr, const char* encpass, const time_t* expire)
{
	return k2hdkc_pm_rename_with_parent_str_wa(handle, poldkey, pnewkey, NULL, checkattr, encpass, expire);
}

bool k2hdkc_rename_with_parent(const char* config, const unsigned char* poldkey, size_t oldkeylength, const unsigned char* pnewkey, size_t newkeylength, const unsigned char* pparentkey, size_t parentkeylength)
{
	return k2hdkc_rename_with_parent_wa(config, poldkey, oldkeylength, pnewkey, newkeylength, pparentkey, parentkeylength, true, NULL, NULL);
}

bool k2hdkc_rename_with_parent_str(const char* config, const char* poldkey, const char* pnewkey, const char* pparentkey)
{
	return k2hdkc_rename_with_parent_str_wa(config, poldkey, pnewkey, pparentkey, true, NULL, NULL);
}

bool k2hdkc_ex_rename_with_parent(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* poldkey, size_t oldkeylength, const unsigned char* pnewkey, size_t newkeylength, const unsigned char* pparentkey, size_t parentkeylength)
{
	return k2hdkc_ex_rename_with_parent_wa(config, ctlport, is_auto_rejoin, no_giveup_rejoin, poldkey, oldkeylength, pnewkey, newkeylength, pparentkey, parentkeylength, true, NULL, NULL);
}

bool k2hdkc_ex_rename_with_parent_str(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* poldkey, const char* pnewkey, const char* pparentkey)
{
	return k2hdkc_ex_rename_with_parent_str_wa(config, ctlport, is_auto_rejoin, no_giveup_rejoin, poldkey, pnewkey, pparentkey, true, NULL, NULL);
}

bool k2hdkc_pm_rename_with_parent(k2hdkc_chmpx_h handle, const unsigned char* poldkey, size_t oldkeylength, const unsigned char* pnewkey, size_t newkeylength, const unsigned char* pparentkey, size_t parentkeylength)
{
	return k2hdkc_pm_rename_with_parent_wa(handle, poldkey, oldkeylength, pnewkey, newkeylength, pparentkey, parentkeylength, true, NULL, NULL);
}

bool k2hdkc_pm_rename_with_parent_str(k2hdkc_chmpx_h handle, const char* poldkey, const char* pnewkey, const char* pparentkey)
{
	return k2hdkc_pm_rename_with_parent_str_wa(handle, poldkey, pnewkey, pparentkey, true, NULL, NULL);
}

bool k2hdkc_rename_with_parent_wa(const char* config, const unsigned char* poldkey, size_t oldkeylength, const unsigned char* pnewkey, size_t newkeylength, const unsigned char* pparentkey, size_t parentkeylength, bool checkattr, const char* encpass, const time_t* expire)
{
	return k2hdkc_ex_rename_with_parent_wa(config, CHM_INVALID_PORT, false, false, poldkey, oldkeylength, pnewkey, newkeylength, pparentkey, parentkeylength, checkattr, encpass, expire);
}

bool k2hdkc_rename_with_parent_str_wa(const char* config, const char* poldkey, const char* pnewkey, const char* pparentkey, bool checkattr, const char* encpass, const time_t* expire)
{
	return k2hdkc_ex_rename_with_parent_str_wa(config, CHM_INVALID_PORT, false, false, poldkey, pnewkey, pparentkey, checkattr, encpass, expire);
}

bool k2hdkc_ex_rename_with_parent_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* poldkey, size_t oldkeylength, const unsigned char* pnewkey, size_t newkeylength, const unsigned char* pparentkey, size_t parentkeylength, bool checkattr, const char* encpass, const time_t* expire)
{
	K2hdkcComRen*	pComObj = GetOtSlaveK2hdkcComRen(config, ctlport, is_auto_rejoin, no_giveup_rejoin);
	bool			result	= K2hdkcFullRen(pComObj, poldkey, oldkeylength, pnewkey, newkeylength, pparentkey, parentkeylength, checkattr, encpass, expire);
	if(!result){
		ERR_DKCPRN("Failed to rename key.");
	}
	DKC_DELETE(pComObj);
	return result;
}

bool k2hdkc_ex_rename_with_parent_str_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* poldkey, const char* pnewkey, const char* pparentkey, bool checkattr, const char* encpass, const time_t* expire)
{
	return k2hdkc_ex_rename_with_parent_wa(config, ctlport, is_auto_rejoin, no_giveup_rejoin, reinterpret_cast<const unsigned char*>(poldkey), (poldkey ? strlen(poldkey) + 1 : 0), reinterpret_cast<const unsigned char*>(pnewkey), (pnewkey ? strlen(pnewkey) + 1 : 0), reinterpret_cast<const unsigned char*>(pparentkey), (pparentkey ? strlen(pparentkey) + 1 : 0), checkattr, encpass, expire);
}

bool k2hdkc_pm_rename_with_parent_wa(k2hdkc_chmpx_h handle, const unsigned char* poldkey, size_t oldkeylength, const unsigned char* pnewkey, size_t newkeylength, const unsigned char* pparentkey, size_t parentkeylength, bool checkattr, const char* encpass, const time_t* expire)
{
	if(K2HDKC_INVALID_HANDLE == handle){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}
	K2hdkcSlave*	pSlave = reinterpret_cast<K2hdkcSlave*>(handle);
	if(!pSlave){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}

	K2hdkcComRen*	pComObj = GetPmSlaveK2hdkcComRen(pSlave);
	bool			result	= K2hdkcFullRen(pComObj, poldkey, oldkeylength, pnewkey, newkeylength, pparentkey, parentkeylength, checkattr, encpass, expire);
	if(!result){
		ERR_DKCPRN("Failed to rename key.");
	}
	DKC_DELETE(pComObj);
	return result;
}

bool k2hdkc_pm_rename_with_parent_str_wa(k2hdkc_chmpx_h handle, const char* poldkey, const char* pnewkey, const char* pparentkey, bool checkattr, const char* encpass, const time_t* expire)
{
	return k2hdkc_pm_rename_with_parent_wa(handle, reinterpret_cast<const unsigned char*>(poldkey), (poldkey ? strlen(poldkey) + 1 : 0), reinterpret_cast<const unsigned char*>(pnewkey), (pnewkey ? strlen(pnewkey) + 1 : 0), reinterpret_cast<const unsigned char*>(pparentkey), (pparentkey ? strlen(pparentkey) + 1 : 0), checkattr, encpass, expire);
}

//---------------------------------------------------------
// Functions - Queue
//---------------------------------------------------------
//
// Push
//
static bool K2hdkcFullQPush(K2hdkcComQPush* pcomobj, const unsigned char* pprefix, size_t prefixlength, const unsigned char* pval, size_t vallength, bool is_fifo, bool checkattr, const char* encpass, const time_t* expire)
{
	if(!pcomobj || !pprefix || 0 == prefixlength){
		ERR_DKCPRN("Parameters are wrong.");
		return false;
	}
	return pcomobj->QueueCommandSend(pprefix, prefixlength, pval, vallength, is_fifo, NULL, 0UL, checkattr, encpass, expire, &Last_Response_Code);				// [NOTE] not set attributes
}

static bool K2hdkcFullKeyQPush(K2hdkcComQPush* pcomobj, const unsigned char* pprefix, size_t prefixlength, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, bool is_fifo, bool checkattr, const char* encpass, const time_t* expire)
{
	if(!pcomobj || !pprefix || 0 == prefixlength || !pkey || 0 == keylength){
		ERR_DKCPRN("Parameters are wrong.");
		return false;
	}
	return pcomobj->KeyQueueCommandSend(pprefix, prefixlength, pkey, keylength, pval, vallength, is_fifo, NULL, 0UL, checkattr, encpass, expire, &Last_Response_Code);	// [NOTE] not set attributes
}

bool k2hdkc_q_push(const char* config, const unsigned char* pprefix, size_t prefixlength, const unsigned char* pval, size_t vallength, bool is_fifo)
{
	return k2hdkc_q_push_wa(config, pprefix, prefixlength, pval, vallength, is_fifo, true, NULL, NULL);
}

bool k2hdkc_q_str_push(const char* config, const char* pprefix, const char* pval, bool is_fifo)
{
	return k2hdkc_q_str_push_wa(config, pprefix, pval, is_fifo, true, NULL, NULL);
}

bool k2hdkc_ex_q_push(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pprefix, size_t prefixlength, const unsigned char* pval, size_t vallength, bool is_fifo)
{
	return k2hdkc_ex_q_push_wa(config, ctlport, is_auto_rejoin, no_giveup_rejoin, pprefix, prefixlength, pval, vallength, is_fifo, true, NULL, NULL);
}

bool k2hdkc_ex_q_str_push(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pprefix, const char* pval, bool is_fifo)
{
	return k2hdkc_ex_q_str_push_wa(config, ctlport, is_auto_rejoin, no_giveup_rejoin, pprefix, pval, is_fifo, true, NULL, NULL);
}

bool k2hdkc_pm_q_push(k2hdkc_chmpx_h handle, const unsigned char* pprefix, size_t prefixlength, const unsigned char* pval, size_t vallength, bool is_fifo)
{
	return k2hdkc_pm_q_push_wa(handle, pprefix, prefixlength, pval, vallength, is_fifo, true, NULL, NULL);
}

bool k2hdkc_pm_q_str_push(k2hdkc_chmpx_h handle, const char* pprefix, const char* pval, bool is_fifo)
{
	return k2hdkc_pm_q_str_push_wa(handle, pprefix, pval, is_fifo, true, NULL, NULL);
}

bool k2hdkc_q_push_wa(const char* config, const unsigned char* pprefix, size_t prefixlength, const unsigned char* pval, size_t vallength, bool is_fifo, bool checkattr, const char* encpass, const time_t* expire)
{
	return k2hdkc_ex_q_push_wa(config, CHM_INVALID_PORT, false, false, pprefix, prefixlength, pval, vallength, is_fifo, checkattr, encpass, expire);
}

bool k2hdkc_q_str_push_wa(const char* config, const char* pprefix, const char* pval, bool is_fifo, bool checkattr, const char* encpass, const time_t* expire)
{
	return k2hdkc_ex_q_str_push_wa(config, CHM_INVALID_PORT, false, false, pprefix, pval, is_fifo, checkattr, encpass, expire);
}

bool k2hdkc_ex_q_push_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pprefix, size_t prefixlength, const unsigned char* pval, size_t vallength, bool is_fifo, bool checkattr, const char* encpass, const time_t* expire)
{
	K2hdkcComQPush*	pComObj = GetOtSlaveK2hdkcComQPush(config, ctlport, is_auto_rejoin, no_giveup_rejoin);
	bool			result	= K2hdkcFullQPush(pComObj, pprefix, prefixlength, pval, vallength, is_fifo, checkattr, encpass, expire);
	if(!result){
		ERR_DKCPRN("Failed to push queue.");
	}
	DKC_DELETE(pComObj);
	return result;
}

bool k2hdkc_ex_q_str_push_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pprefix, const char* pval, bool is_fifo, bool checkattr, const char* encpass, const time_t* expire)
{
	return k2hdkc_ex_q_push_wa(config, ctlport, is_auto_rejoin, no_giveup_rejoin, reinterpret_cast<const unsigned char*>(pprefix), (pprefix ? strlen(pprefix) + 1 : 0), reinterpret_cast<const unsigned char*>(pval), (pval ? strlen(pval) + 1 : 0), is_fifo, checkattr, encpass, expire);
}

bool k2hdkc_pm_q_push_wa(k2hdkc_chmpx_h handle, const unsigned char* pprefix, size_t prefixlength, const unsigned char* pval, size_t vallength, bool is_fifo, bool checkattr, const char* encpass, const time_t* expire)
{
	if(K2HDKC_INVALID_HANDLE == handle){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}
	K2hdkcSlave*	pSlave = reinterpret_cast<K2hdkcSlave*>(handle);
	if(!pSlave){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}

	K2hdkcComQPush*	pComObj = GetPmSlaveK2hdkcComQPush(pSlave);
	bool			result	= K2hdkcFullQPush(pComObj, pprefix, prefixlength, pval, vallength, is_fifo, checkattr, encpass, expire);
	if(!result){
		ERR_DKCPRN("Failed to push queue.");
	}
	DKC_DELETE(pComObj);
	return result;
}

bool k2hdkc_pm_q_str_push_wa(k2hdkc_chmpx_h handle, const char* pprefix, const char* pval, bool is_fifo, bool checkattr, const char* encpass, const time_t* expire)
{
	return k2hdkc_pm_q_push_wa(handle, reinterpret_cast<const unsigned char*>(pprefix), (pprefix ? strlen(pprefix) + 1 : 0), reinterpret_cast<const unsigned char*>(pval), (pval ? strlen(pval) + 1 : 0), is_fifo, checkattr, encpass, expire);
}

bool k2hdkc_keyq_push(const char* config, const unsigned char* pprefix, size_t prefixlength, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, bool is_fifo)
{
	return k2hdkc_keyq_push_wa(config, pprefix, prefixlength, pkey, keylength, pval, vallength, is_fifo, true, NULL, NULL);
}

bool k2hdkc_keyq_str_push(const char* config, const char* pprefix, const char* pkey, const char* pval, bool is_fifo)
{
	return k2hdkc_keyq_str_push_wa(config, pprefix, pkey, pval, is_fifo, true, NULL, NULL);
}

bool k2hdkc_ex_keyq_push(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pprefix, size_t prefixlength, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, bool is_fifo)
{
	return k2hdkc_ex_keyq_push_wa(config, ctlport, is_auto_rejoin, no_giveup_rejoin, pprefix, prefixlength, pkey, keylength, pval, vallength, is_fifo, true, NULL, NULL);
}

bool k2hdkc_ex_keyq_str_push(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pprefix, const char* pkey, const char* pval, bool is_fifo)
{
	return k2hdkc_ex_keyq_str_push_wa(config, ctlport, is_auto_rejoin, no_giveup_rejoin, pprefix, pkey, pval, is_fifo, true, NULL, NULL);
}

bool k2hdkc_pm_keyq_push(k2hdkc_chmpx_h handle, const unsigned char* pprefix, size_t prefixlength, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, bool is_fifo)
{
	return k2hdkc_pm_keyq_push_wa(handle, pprefix, prefixlength, pkey, keylength, pval, vallength, is_fifo, true, NULL, NULL);
}

bool k2hdkc_pm_keyq_str_push(k2hdkc_chmpx_h handle, const char* pprefix, const char* pkey, const char* pval, bool is_fifo)
{
	return k2hdkc_pm_keyq_str_push_wa(handle, pprefix, pkey, pval, is_fifo, true, NULL, NULL);
}

bool k2hdkc_keyq_push_wa(const char* config, const unsigned char* pprefix, size_t prefixlength, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, bool is_fifo, bool checkattr, const char* encpass, const time_t* expire)
{
	return k2hdkc_ex_keyq_push_wa(config, CHM_INVALID_PORT, false, false, pprefix, prefixlength, pkey, keylength, pval, vallength, is_fifo, checkattr, encpass, expire);
}

bool k2hdkc_keyq_str_push_wa(const char* config, const char* pprefix, const char* pkey, const char* pval, bool is_fifo, bool checkattr, const char* encpass, const time_t* expire)
{
	return k2hdkc_ex_keyq_str_push_wa(config, CHM_INVALID_PORT, false, false, pprefix, pkey, pval, is_fifo, checkattr, encpass, expire);
}

bool k2hdkc_ex_keyq_push_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pprefix, size_t prefixlength, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, bool is_fifo, bool checkattr, const char* encpass, const time_t* expire)
{
	K2hdkcComQPush*	pComObj = GetOtSlaveK2hdkcComQPush(config, ctlport, is_auto_rejoin, no_giveup_rejoin);
	bool			result	= K2hdkcFullKeyQPush(pComObj, pprefix, prefixlength, pkey, keylength, pval, vallength, is_fifo, checkattr, encpass, expire);
	if(!result){
		ERR_DKCPRN("Failed to push key queue.");
	}
	DKC_DELETE(pComObj);
	return result;
}

bool k2hdkc_ex_keyq_str_push_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pprefix, const char* pkey, const char* pval, bool is_fifo, bool checkattr, const char* encpass, const time_t* expire)
{
	return k2hdkc_ex_keyq_push_wa(config, ctlport, is_auto_rejoin, no_giveup_rejoin, reinterpret_cast<const unsigned char*>(pprefix), (pprefix ? strlen(pprefix) + 1 : 0), reinterpret_cast<const unsigned char*>(pkey), (pkey ? strlen(pkey) + 1 : 0), reinterpret_cast<const unsigned char*>(pval), (pval ? strlen(pval) + 1 : 0), is_fifo, checkattr, encpass, expire);
}

bool k2hdkc_pm_keyq_push_wa(k2hdkc_chmpx_h handle, const unsigned char* pprefix, size_t prefixlength, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, bool is_fifo, bool checkattr, const char* encpass, const time_t* expire)
{
	if(K2HDKC_INVALID_HANDLE == handle){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}
	K2hdkcSlave*	pSlave = reinterpret_cast<K2hdkcSlave*>(handle);
	if(!pSlave){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}

	K2hdkcComQPush*	pComObj = GetPmSlaveK2hdkcComQPush(pSlave);
	bool			result	= K2hdkcFullKeyQPush(pComObj, pprefix, prefixlength, pkey, keylength, pval, vallength, is_fifo, checkattr, encpass, expire);
	if(!result){
		ERR_DKCPRN("Failed to push key queue.");
	}
	DKC_DELETE(pComObj);
	return result;
}

bool k2hdkc_pm_keyq_str_push_wa(k2hdkc_chmpx_h handle, const char* pprefix, const char* pkey, const char* pval, bool is_fifo, bool checkattr, const char* encpass, const time_t* expire)
{
	return k2hdkc_pm_keyq_push_wa(handle, reinterpret_cast<const unsigned char*>(pprefix), (pprefix ? strlen(pprefix) + 1 : 0), reinterpret_cast<const unsigned char*>(pkey), (pkey ? strlen(pkey) + 1 : 0), reinterpret_cast<const unsigned char*>(pval), (pval ? strlen(pval) + 1 : 0), is_fifo, checkattr, encpass, expire);
}

//
// Pop
//
static bool K2hdkcFullQPop(K2hdkcComQPop* pcomobj, const unsigned char* pprefix, size_t prefixlength, bool is_fifo, const char* encpass, unsigned char** ppval, size_t* pvallength)
{
	if(!pcomobj || !pprefix || 0 == prefixlength || !ppval || !pvallength){
		ERR_DKCPRN("Parameters are wrong.");
		return false;
	}
	const unsigned char*	pvaltmp		= NULL;
	size_t					valtmplen	= 0L;
	bool					result		= pcomobj->QueueCommandSend(pprefix, prefixlength, is_fifo, true, encpass, &pvaltmp, &valtmplen, &Last_Response_Code);	// [NOTE] always check attributes
	if(result){
		if(pvaltmp && 0 < valtmplen){
			*ppval		= k2hbindup(pvaltmp, valtmplen);
			*pvallength	= valtmplen;
		}else{
			*ppval		= NULL;
			*pvallength	= 0L;
		}
	}
	return result;
}

static bool K2hdkcFullKeyQPop(K2hdkcComQPop* pcomobj, const unsigned char* pprefix, size_t prefixlength, bool is_fifo, const char* encpass, unsigned char** ppkey, size_t* pkeylength, unsigned char** ppval, size_t* pvallength)
{
	if(!pcomobj || !pprefix || 0 == prefixlength || !ppkey || !pkeylength || !ppval || !pvallength){
		ERR_DKCPRN("Parameters are wrong.");
		return false;
	}
	const unsigned char*	pkeytmp		= NULL;
	size_t					keytmplen	= 0L;
	const unsigned char*	pvaltmp		= NULL;
	size_t					valtmplen	= 0L;
	bool					result		= pcomobj->KeyQueueCommandSend(pprefix, prefixlength, is_fifo, true, encpass, &pkeytmp, &keytmplen, &pvaltmp, &valtmplen, &Last_Response_Code);	// [NOTE] always check attributes
	if(result){
		if(pvaltmp && 0 < valtmplen){
			*ppkey		= k2hbindup(pkeytmp, keytmplen);
			*pkeylength	= keytmplen;
			*ppval		= k2hbindup(pvaltmp, valtmplen);
			*pvallength	= valtmplen;
		}else{
			*ppkey		= NULL;
			*pkeylength	= 0L;
			*ppval		= NULL;
			*pvallength	= 0L;
		}
	}
	return result;
}

bool k2hdkc_q_pop(const char* config, const unsigned char* pprefix, size_t prefixlength, bool is_fifo, unsigned char** ppval, size_t* pvallength)
{
	return k2hdkc_q_pop_wp(config, pprefix, prefixlength, is_fifo, NULL, ppval, pvallength);
}

bool k2hdkc_q_str_pop(const char* config, const char* pprefix, bool is_fifo, const char** ppval)
{
	return k2hdkc_q_str_pop_wp(config, pprefix, is_fifo, NULL, ppval);
}

bool k2hdkc_ex_q_pop(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pprefix, size_t prefixlength, bool is_fifo, unsigned char** ppval, size_t* pvallength)
{
	return k2hdkc_ex_q_pop_wp(config, ctlport, is_auto_rejoin, no_giveup_rejoin, pprefix, prefixlength, is_fifo, NULL, ppval, pvallength);
}

bool k2hdkc_ex_q_str_pop(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pprefix, bool is_fifo, const char** ppval)
{
	return k2hdkc_ex_q_str_pop_wp(config, ctlport, is_auto_rejoin, no_giveup_rejoin, pprefix, is_fifo, NULL, ppval);
}

bool k2hdkc_pm_q_pop(k2hdkc_chmpx_h handle, const unsigned char* pprefix, size_t prefixlength, bool is_fifo, unsigned char** ppval, size_t* pvallength)
{
	return k2hdkc_pm_q_pop_wp(handle, pprefix, prefixlength, is_fifo, NULL, ppval, pvallength);
}

bool k2hdkc_pm_q_str_pop(k2hdkc_chmpx_h handle, const char* pprefix, bool is_fifo, const char** ppval)
{
	return k2hdkc_pm_q_str_pop_wp(handle, pprefix, is_fifo, NULL, ppval);
}

bool k2hdkc_q_pop_wp(const char* config, const unsigned char* pprefix, size_t prefixlength, bool is_fifo, const char* encpass, unsigned char** ppval, size_t* pvallength)
{
	return k2hdkc_ex_q_pop_wp(config, CHM_INVALID_PORT, false, false, pprefix, prefixlength, is_fifo, encpass, ppval, pvallength);
}

bool k2hdkc_q_str_pop_wp(const char* config, const char* pprefix, bool is_fifo, const char* encpass, const char** ppval)
{
	return k2hdkc_ex_q_str_pop_wp(config, CHM_INVALID_PORT, false, false, pprefix, is_fifo, encpass, ppval);
}

bool k2hdkc_ex_q_pop_wp(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pprefix, size_t prefixlength, bool is_fifo, const char* encpass, unsigned char** ppval, size_t* pvallength)
{
	K2hdkcComQPop*	pComObj = GetOtSlaveK2hdkcComQPop(config, ctlport, is_auto_rejoin, no_giveup_rejoin);
	bool			result	= K2hdkcFullQPop(pComObj, pprefix, prefixlength, is_fifo, encpass, ppval, pvallength);
	if(!result){
		ERR_DKCPRN("Failed to pop queue.");
	}
	DKC_DELETE(pComObj);
	return result;
}

bool k2hdkc_ex_q_str_pop_wp(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pprefix, bool is_fifo, const char* encpass, const char** ppval)
{
	if(!ppval){
		ERR_DKCPRN("Parameter is wrong.");
		return false;
	}
	unsigned char*	pval		= NULL;
	size_t			vallength	= 0UL;
	if(!k2hdkc_ex_q_pop_wp(config, ctlport, is_auto_rejoin, no_giveup_rejoin, reinterpret_cast<const unsigned char*>(pprefix), (pprefix ? strlen(pprefix) + 1 : 0), is_fifo, encpass, &pval, &vallength)){
		return false;
	}
	if(pval && 0 < vallength){
		*ppval	= reinterpret_cast<const char*>(pval);
	}else{
		*ppval	= NULL;
	}
	return true;
}

bool k2hdkc_pm_q_pop_wp(k2hdkc_chmpx_h handle, const unsigned char* pprefix, size_t prefixlength, bool is_fifo, const char* encpass, unsigned char** ppval, size_t* pvallength)
{
	if(K2HDKC_INVALID_HANDLE == handle){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}
	K2hdkcSlave*	pSlave = reinterpret_cast<K2hdkcSlave*>(handle);
	if(!pSlave){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}

	K2hdkcComQPop*	pComObj 	= GetPmSlaveK2hdkcComQPop(pSlave);
	bool			result		= K2hdkcFullQPop(pComObj, pprefix, prefixlength, is_fifo, encpass, ppval, pvallength);
	if(!result){
		ERR_DKCPRN("Failed to pop queue.");
	}
	DKC_DELETE(pComObj);
	return result;
}

bool k2hdkc_pm_q_str_pop_wp(k2hdkc_chmpx_h handle, const char* pprefix, bool is_fifo, const char* encpass, const char** ppval)
{
	if(!ppval){
		ERR_DKCPRN("Parameter is wrong.");
		return false;
	}
	unsigned char*	pval		= NULL;
	size_t			vallength	= 0UL;
	if(!k2hdkc_pm_q_pop_wp(handle, reinterpret_cast<const unsigned char*>(pprefix), (pprefix ? strlen(pprefix) + 1 : 0), is_fifo, encpass, &pval, &vallength)){
		return false;
	}
	if(pval && 0 < vallength){
		*ppval	= reinterpret_cast<const char*>(pval);
	}else{
		*ppval	= NULL;
	}
	return true;
}

bool k2hdkc_keyq_pop(const char* config, const unsigned char* pprefix, size_t prefixlength, bool is_fifo, unsigned char** ppkey, size_t* pkeylength, unsigned char** ppval, size_t* pvallength)
{
	return k2hdkc_keyq_pop_wp(config, pprefix, prefixlength, is_fifo, NULL, ppkey, pkeylength, ppval, pvallength);
}

bool k2hdkc_keyq_str_pop(const char* config, const char* pprefix, bool is_fifo, const char** ppkey, const char** ppval)
{
	return k2hdkc_keyq_str_pop_wp(config, pprefix, is_fifo, NULL, ppkey, ppval);
}

bool k2hdkc_ex_keyq_pop(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pprefix, size_t prefixlength, bool is_fifo, unsigned char** ppkey, size_t* pkeylength, unsigned char** ppval, size_t* pvallength)
{
	return k2hdkc_ex_keyq_pop_wp(config, ctlport, is_auto_rejoin, no_giveup_rejoin, pprefix, prefixlength, is_fifo, NULL, ppkey, pkeylength, ppval, pvallength);
}

bool k2hdkc_ex_keyq_str_pop(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pprefix, bool is_fifo, const char** ppkey, const char** ppval)
{
	return k2hdkc_ex_keyq_str_pop_wp(config, ctlport, is_auto_rejoin, no_giveup_rejoin, pprefix, is_fifo, NULL, ppkey, ppval);
}

bool k2hdkc_pm_keyq_pop(k2hdkc_chmpx_h handle, const unsigned char* pprefix, size_t prefixlength, bool is_fifo, unsigned char** ppkey, size_t* pkeylength, unsigned char** ppval, size_t* pvallength)
{
	return k2hdkc_pm_keyq_pop_wp(handle, pprefix, prefixlength, is_fifo, NULL, ppkey, pkeylength, ppval, pvallength);
}

bool k2hdkc_pm_keyq_str_pop(k2hdkc_chmpx_h handle, const char* pprefix, bool is_fifo, const char** ppkey, const char** ppval)
{
	return k2hdkc_pm_keyq_str_pop_wp(handle, pprefix, is_fifo, NULL, ppkey, ppval);
}

bool k2hdkc_keyq_pop_wp(const char* config, const unsigned char* pprefix, size_t prefixlength, bool is_fifo, const char* encpass, unsigned char** ppkey, size_t* pkeylength, unsigned char** ppval, size_t* pvallength)
{
	return k2hdkc_ex_keyq_pop_wp(config, CHM_INVALID_PORT, false, false, pprefix, prefixlength, is_fifo, encpass, ppkey, pkeylength, ppval, pvallength);
}

bool k2hdkc_keyq_str_pop_wp(const char* config, const char* pprefix, bool is_fifo, const char* encpass, const char** ppkey, const char** ppval)
{
	return k2hdkc_ex_keyq_str_pop_wp(config, CHM_INVALID_PORT, false, false, pprefix, is_fifo, encpass, ppkey, ppval);
}

bool k2hdkc_ex_keyq_pop_wp(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pprefix, size_t prefixlength, bool is_fifo, const char* encpass, unsigned char** ppkey, size_t* pkeylength, unsigned char** ppval, size_t* pvallength)
{
	K2hdkcComQPop*	pComObj = GetOtSlaveK2hdkcComQPop(config, ctlport, is_auto_rejoin, no_giveup_rejoin);
	bool			result	= K2hdkcFullKeyQPop(pComObj, pprefix, prefixlength, is_fifo, encpass, ppkey, pkeylength, ppval, pvallength);
	if(!result){
		ERR_DKCPRN("Failed to pop key queue.");
	}
	DKC_DELETE(pComObj);
	return result;
}

bool k2hdkc_ex_keyq_str_pop_wp(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pprefix, bool is_fifo, const char* encpass, const char** ppkey, const char** ppval)
{
	if(!ppkey || !ppval){
		ERR_DKCPRN("Parameter is wrong.");
		return false;
	}
	unsigned char*	pkey		= NULL;
	size_t			keylength	= 0UL;
	unsigned char*	pval		= NULL;
	size_t			vallength	= 0UL;
	if(!k2hdkc_ex_keyq_pop_wp(config, ctlport, is_auto_rejoin, no_giveup_rejoin, reinterpret_cast<const unsigned char*>(pprefix), (pprefix ? strlen(pprefix) + 1 : 0), is_fifo, encpass, &pkey, &keylength, &pval, &vallength)){
		return false;
	}
	if(pkey && 0 < keylength){
		*ppkey	= reinterpret_cast<const char*>(pkey);
	}else{
		*ppkey	= NULL;
	}
	if(pval && 0 < vallength){
		*ppval	= reinterpret_cast<const char*>(pval);
	}else{
		*ppval	= NULL;
	}
	return true;
}

bool k2hdkc_pm_keyq_pop_wp(k2hdkc_chmpx_h handle, const unsigned char* pprefix, size_t prefixlength, bool is_fifo, const char* encpass, unsigned char** ppkey, size_t* pkeylength, unsigned char** ppval, size_t* pvallength)
{
	if(K2HDKC_INVALID_HANDLE == handle){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}
	K2hdkcSlave*	pSlave = reinterpret_cast<K2hdkcSlave*>(handle);
	if(!pSlave){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}

	K2hdkcComQPop*	pComObj 	= GetPmSlaveK2hdkcComQPop(pSlave);
	bool			result		= K2hdkcFullKeyQPop(pComObj, pprefix, prefixlength, is_fifo, encpass, ppkey, pkeylength, ppval, pvallength);
	if(!result){
		ERR_DKCPRN("Failed to pop key queue.");
	}
	DKC_DELETE(pComObj);
	return result;
}

bool k2hdkc_pm_keyq_str_pop_wp(k2hdkc_chmpx_h handle, const char* pprefix, bool is_fifo, const char* encpass, const char** ppkey, const char** ppval)
{
	if(!ppkey || !ppval){
		ERR_DKCPRN("Parameter is wrong.");
		return false;
	}
	unsigned char*	pkey		= NULL;
	size_t			keylength	= 0UL;
	unsigned char*	pval		= NULL;
	size_t			vallength	= 0UL;
	if(!k2hdkc_pm_keyq_pop_wp(handle, reinterpret_cast<const unsigned char*>(pprefix), (pprefix ? strlen(pprefix) + 1 : 0), is_fifo, encpass, &pkey, &keylength, &pval, &vallength)){
		return false;
	}
	if(pkey && 0 < keylength){
		*ppkey	= reinterpret_cast<const char*>(pkey);
	}else{
		*ppkey	= NULL;
	}
	if(pval && 0 < vallength){
		*ppval	= reinterpret_cast<const char*>(pval);
	}else{
		*ppval	= NULL;
	}
	return true;
}

//
// Remove
//
static bool K2hdkcFullQDel(K2hdkcComQDel* pcomobj, const unsigned char* pprefix, size_t prefixlength, int count, bool is_fifo, const char* encpass)
{
	if(!pcomobj || !pprefix || 0 == prefixlength){
		ERR_DKCPRN("Parameters are wrong.");
		return false;
	}
	return pcomobj->QueueCommandSend(pprefix, prefixlength, count, is_fifo, true, encpass, &Last_Response_Code);	// [NOTE] always check attributes
}

static bool K2hdkcFullKeyQDel(K2hdkcComQDel* pcomobj, const unsigned char* pprefix, size_t prefixlength, int count, bool is_fifo, const char* encpass)
{
	if(!pcomobj || !pprefix || 0 == prefixlength){
		ERR_DKCPRN("Parameters are wrong.");
		return false;
	}
	return pcomobj->KeyQueueCommandSend(pprefix, prefixlength, count, is_fifo, true, encpass, &Last_Response_Code);	// [NOTE] always check attributes
}

bool k2hdkc_q_remove(const char* config, const unsigned char* pprefix, size_t prefixlength, int count, bool is_fifo)
{
	return k2hdkc_q_remove_wp(config, pprefix, prefixlength, count, is_fifo, NULL);
}

bool k2hdkc_q_str_remove(const char* config, const char* pprefix, int count, bool is_fifo)
{
	return k2hdkc_q_str_remove_wp(config, pprefix, count, is_fifo, NULL);
}

bool k2hdkc_ex_q_remove(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pprefix, size_t prefixlength, int count, bool is_fifo)
{
	return k2hdkc_ex_q_remove_wp(config, ctlport, is_auto_rejoin, no_giveup_rejoin, pprefix, prefixlength, count, is_fifo, NULL);
}

bool k2hdkc_ex_q_str_remove(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pprefix, int count, bool is_fifo)
{
	return k2hdkc_ex_q_str_remove_wp(config, ctlport, is_auto_rejoin, no_giveup_rejoin, pprefix, count, is_fifo, NULL);
}

bool k2hdkc_pm_q_remove(k2hdkc_chmpx_h handle, const unsigned char* pprefix, size_t prefixlength, int count, bool is_fifo)
{
	return k2hdkc_pm_q_remove_wp(handle, pprefix, prefixlength, count, is_fifo, NULL);
}

bool k2hdkc_pm_q_str_remove(k2hdkc_chmpx_h handle, const char* pprefix, int count, bool is_fifo)
{
	return k2hdkc_pm_q_str_remove_wp(handle, pprefix, count, is_fifo, NULL);
}

bool k2hdkc_q_remove_wp(const char* config, const unsigned char* pprefix, size_t prefixlength, int count, bool is_fifo, const char* encpass)
{
	return k2hdkc_ex_q_remove_wp(config, CHM_INVALID_PORT, false, false, pprefix, prefixlength, count, is_fifo, encpass);
}

bool k2hdkc_q_str_remove_wp(const char* config, const char* pprefix, int count, bool is_fifo, const char* encpass)
{
	return k2hdkc_ex_q_str_remove_wp(config, CHM_INVALID_PORT, false, false, pprefix, count, is_fifo, encpass);
}

bool k2hdkc_ex_q_remove_wp(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pprefix, size_t prefixlength, int count, bool is_fifo, const char* encpass)
{
	K2hdkcComQDel*	pComObj = GetOtSlaveK2hdkcComQDel(config, ctlport, is_auto_rejoin, no_giveup_rejoin);
	bool			result	= K2hdkcFullQDel(pComObj, pprefix, prefixlength, count, is_fifo, encpass);
	if(!result){
		ERR_DKCPRN("Failed to remove queue.");
	}
	DKC_DELETE(pComObj);
	return result;
}

bool k2hdkc_ex_q_str_remove_wp(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pprefix, int count, bool is_fifo, const char* encpass)
{
	return k2hdkc_ex_q_remove_wp(config, ctlport, is_auto_rejoin, no_giveup_rejoin, reinterpret_cast<const unsigned char*>(pprefix), (pprefix ? strlen(pprefix) + 1 : 0), count, is_fifo, encpass);
}

bool k2hdkc_pm_q_remove_wp(k2hdkc_chmpx_h handle, const unsigned char* pprefix, size_t prefixlength, int count, bool is_fifo, const char* encpass)
{
	if(K2HDKC_INVALID_HANDLE == handle){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}
	K2hdkcSlave*	pSlave = reinterpret_cast<K2hdkcSlave*>(handle);
	if(!pSlave){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}

	K2hdkcComQDel*	pComObj = GetPmSlaveK2hdkcComQDel(pSlave);
	bool			result	= K2hdkcFullQDel(pComObj, pprefix, prefixlength, count, is_fifo, encpass);
	if(!result){
		ERR_DKCPRN("Failed to remove key queue.");
	}
	DKC_DELETE(pComObj);
	return result;
}

bool k2hdkc_pm_q_str_remove_wp(k2hdkc_chmpx_h handle, const char* pprefix, int count, bool is_fifo, const char* encpass)
{
	return k2hdkc_pm_q_remove_wp(handle, reinterpret_cast<const unsigned char*>(pprefix), (pprefix ? strlen(pprefix) + 1 : 0), count, is_fifo, encpass);
}

bool k2hdkc_keyq_remove(const char* config, const unsigned char* pprefix, size_t prefixlength, int count, bool is_fifo)
{
	return k2hdkc_keyq_remove_wp(config, pprefix, prefixlength, count, is_fifo, NULL);
}

bool k2hdkc_keyq_str_remove(const char* config, const char* pprefix, int count, bool is_fifo)
{
	return k2hdkc_keyq_str_remove_wp(config, pprefix, count, is_fifo, NULL);
}

bool k2hdkc_ex_keyq_remove(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pprefix, size_t prefixlength, int count, bool is_fifo)
{
	return k2hdkc_ex_keyq_remove_wp(config, ctlport, is_auto_rejoin, no_giveup_rejoin, pprefix, prefixlength, count, is_fifo, NULL);
}

bool k2hdkc_ex_keyq_str_remove(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pprefix, int count, bool is_fifo)
{
	return k2hdkc_ex_keyq_str_remove_wp(config, ctlport, is_auto_rejoin, no_giveup_rejoin, pprefix, count, is_fifo, NULL);
}

bool k2hdkc_pm_keyq_remove(k2hdkc_chmpx_h handle, const unsigned char* pprefix, size_t prefixlength, int count, bool is_fifo)
{
	return k2hdkc_pm_keyq_remove_wp(handle, pprefix, prefixlength, count, is_fifo, NULL);
}

bool k2hdkc_pm_keyq_str_remove(k2hdkc_chmpx_h handle, const char* pprefix, int count, bool is_fifo)
{
	return k2hdkc_pm_keyq_str_remove_wp(handle, pprefix, count, is_fifo, NULL);
}

bool k2hdkc_keyq_remove_wp(const char* config, const unsigned char* pprefix, size_t prefixlength, int count, bool is_fifo, const char* encpass)
{
	return k2hdkc_ex_keyq_remove_wp(config, CHM_INVALID_PORT, false, false, pprefix, prefixlength, count, is_fifo, encpass);
}

bool k2hdkc_keyq_str_remove_wp(const char* config, const char* pprefix, int count, bool is_fifo, const char* encpass)
{
	return k2hdkc_ex_keyq_str_remove_wp(config, CHM_INVALID_PORT, false, false, pprefix, count, is_fifo, encpass);
}

bool k2hdkc_ex_keyq_remove_wp(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pprefix, size_t prefixlength, int count, bool is_fifo, const char* encpass)
{
	K2hdkcComQDel*	pComObj = GetOtSlaveK2hdkcComQDel(config, ctlport, is_auto_rejoin, no_giveup_rejoin);
	bool			result	= K2hdkcFullKeyQDel(pComObj, pprefix, prefixlength, count, is_fifo, encpass);
	if(!result){
		ERR_DKCPRN("Failed to remove key queue.");
	}
	DKC_DELETE(pComObj);
	return result;
}

bool k2hdkc_ex_keyq_str_remove_wp(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pprefix, int count, bool is_fifo, const char* encpass)
{
	return k2hdkc_ex_keyq_remove_wp(config, ctlport, is_auto_rejoin, no_giveup_rejoin, reinterpret_cast<const unsigned char*>(pprefix), (pprefix ? strlen(pprefix) + 1 : 0), count, is_fifo, encpass);
}

bool k2hdkc_pm_keyq_remove_wp(k2hdkc_chmpx_h handle, const unsigned char* pprefix, size_t prefixlength, int count, bool is_fifo, const char* encpass)
{
	if(K2HDKC_INVALID_HANDLE == handle){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}
	K2hdkcSlave*	pSlave = reinterpret_cast<K2hdkcSlave*>(handle);
	if(!pSlave){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}

	K2hdkcComQDel*	pComObj = GetPmSlaveK2hdkcComQDel(pSlave);
	bool			result	= K2hdkcFullKeyQDel(pComObj, pprefix, prefixlength, count, is_fifo, encpass);
	if(!result){
		ERR_DKCPRN("Failed to remove key queue.");
	}
	DKC_DELETE(pComObj);
	return result;
}

bool k2hdkc_pm_keyq_str_remove_wp(k2hdkc_chmpx_h handle, const char* pprefix, int count, bool is_fifo, const char* encpass)
{
	return k2hdkc_pm_keyq_remove_wp(handle, reinterpret_cast<const unsigned char*>(pprefix), (pprefix ? strlen(pprefix) + 1 : 0), count, is_fifo, encpass);
}

//---------------------------------------------------------
// Functions - CAS(Compare and Swap)
//---------------------------------------------------------
//
// Init
//
static bool K2hdkcFullCasInit(K2hdkcComCasInit* pcomobj, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, const char* encpass, const time_t* expire)
{
	if(!pkey || 0 == keylength || !pval || 0 == vallength){
		ERR_DKCPRN("Parameters are wrong.");
		return false;
	}
	return pcomobj->CommandSend(pkey, keylength, pval, vallength, encpass, expire, &Last_Response_Code);
}

bool k2hdkc_cas64_init(const char* config, const unsigned char* pkey, size_t keylength, uint64_t val)
{
	return k2hdkc_cas64_init_wa(config, pkey, keylength, val, NULL, NULL);
}

bool k2hdkc_cas64_str_init(const char* config, const char* pkey, uint64_t val)
{
	return k2hdkc_cas64_str_init_wa(config, pkey, val, NULL, NULL);
}

bool k2hdkc_ex_cas64_init(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint64_t val)
{
	return k2hdkc_ex_cas64_init_wa(config, ctlport, is_auto_rejoin, no_giveup_rejoin, pkey, keylength, val, NULL, NULL);
}

bool k2hdkc_ex_cas64_str_init(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint64_t val)
{
	return k2hdkc_ex_cas64_str_init_wa(config, ctlport, is_auto_rejoin, no_giveup_rejoin, pkey, val, NULL, NULL);
}

bool k2hdkc_pm_cas64_init(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint64_t val)
{
	return k2hdkc_pm_cas64_init_wa(handle, pkey, keylength, val, NULL, NULL);
}

bool k2hdkc_pm_cas64_str_init(k2hdkc_chmpx_h handle, const char* pkey, uint64_t val)
{
	return k2hdkc_pm_cas64_str_init_wa(handle, pkey, val, NULL, NULL);
}

bool k2hdkc_cas64_init_wa(const char* config, const unsigned char* pkey, size_t keylength, uint64_t val, const char* encpass, const time_t* expire)
{
	return k2hdkc_ex_cas64_init_wa(config, CHM_INVALID_PORT, false, false, pkey, keylength, val, encpass, expire);
}

bool k2hdkc_cas64_str_init_wa(const char* config, const char* pkey, uint64_t val, const char* encpass, const time_t* expire)
{
	return k2hdkc_ex_cas64_str_init_wa(config, CHM_INVALID_PORT, false, false, pkey, val, encpass, expire);
}

bool k2hdkc_ex_cas64_init_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint64_t val, const char* encpass, const time_t* expire)
{
	K2hdkcComCasInit*	pComObj = GetOtSlaveK2hdkcComCasInit(config, ctlport, is_auto_rejoin, no_giveup_rejoin);
	bool				result	= K2hdkcFullCasInit(pComObj, pkey, keylength, reinterpret_cast<const unsigned char*>(&val), sizeof(uint64_t), encpass, expire);
	if(!result){
		ERR_DKCPRN("Failed to initialize cas value.");
	}
	DKC_DELETE(pComObj);
	return result;
}

bool k2hdkc_ex_cas64_str_init_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint64_t val, const char* encpass, const time_t* expire)
{
	return k2hdkc_ex_cas64_init_wa(config, ctlport, is_auto_rejoin, no_giveup_rejoin, reinterpret_cast<const unsigned char*>(pkey), (pkey ? strlen(pkey) + 1 : 0), val, encpass, expire);
}

bool k2hdkc_pm_cas64_init_wa(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint64_t val, const char* encpass, const time_t* expire)
{
	if(K2HDKC_INVALID_HANDLE == handle){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}
	K2hdkcSlave*	pSlave = reinterpret_cast<K2hdkcSlave*>(handle);
	if(!pSlave){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}

	K2hdkcComCasInit*	pComObj = GetPmSlaveK2hdkcComCasInit(pSlave);
	bool				result	= K2hdkcFullCasInit(pComObj, pkey, keylength, reinterpret_cast<const unsigned char*>(&val), sizeof(uint64_t), encpass, expire);
	if(!result){
		ERR_DKCPRN("Failed to initialize cas value.");
	}
	DKC_DELETE(pComObj);
	return result;
}

bool k2hdkc_pm_cas64_str_init_wa(k2hdkc_chmpx_h handle, const char* pkey, uint64_t val, const char* encpass, const time_t* expire)
{
	return k2hdkc_pm_cas64_init_wa(handle, reinterpret_cast<const unsigned char*>(pkey), (pkey ? strlen(pkey) + 1 : 0), val, encpass, expire);
}

bool k2hdkc_cas32_init(const char* config, const unsigned char* pkey, size_t keylength, uint32_t val)
{
	return k2hdkc_cas32_init_wa(config, pkey, keylength, val, NULL, NULL);
}

bool k2hdkc_cas32_str_init(const char* config, const char* pkey, uint32_t val)
{
	return k2hdkc_cas32_str_init_wa(config, pkey, val, NULL, NULL);
}

bool k2hdkc_ex_cas32_init(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint32_t val)
{
	return k2hdkc_ex_cas32_init_wa(config, ctlport, is_auto_rejoin, no_giveup_rejoin, pkey, keylength, val, NULL, NULL);
}

bool k2hdkc_ex_cas32_str_init(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint32_t val)
{
	return k2hdkc_ex_cas32_str_init_wa(config, ctlport, is_auto_rejoin, no_giveup_rejoin, pkey, val, NULL, NULL);
}

bool k2hdkc_pm_cas32_init(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint32_t val)
{
	return k2hdkc_pm_cas32_init_wa(handle, pkey, keylength, val, NULL, NULL);
}

bool k2hdkc_pm_cas32_str_init(k2hdkc_chmpx_h handle, const char* pkey, uint32_t val)
{
	return k2hdkc_pm_cas32_str_init_wa(handle, pkey, val, NULL, NULL);
}

bool k2hdkc_cas32_init_wa(const char* config, const unsigned char* pkey, size_t keylength, uint32_t val, const char* encpass, const time_t* expire)
{
	return k2hdkc_ex_cas32_init_wa(config, CHM_INVALID_PORT, false, false, pkey, keylength, val, encpass, expire);
}

bool k2hdkc_cas32_str_init_wa(const char* config, const char* pkey, uint32_t val, const char* encpass, const time_t* expire)
{
	return k2hdkc_ex_cas32_str_init_wa(config, CHM_INVALID_PORT, false, false, pkey, val, encpass, expire);
}

bool k2hdkc_ex_cas32_init_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint32_t val, const char* encpass, const time_t* expire)
{
	K2hdkcComCasInit*	pComObj = GetOtSlaveK2hdkcComCasInit(config, ctlport, is_auto_rejoin, no_giveup_rejoin);
	bool				result	= K2hdkcFullCasInit(pComObj, pkey, keylength, reinterpret_cast<const unsigned char*>(&val), sizeof(uint32_t), encpass, expire);
	if(!result){
		ERR_DKCPRN("Failed to initialize cas value.");
	}
	DKC_DELETE(pComObj);
	return result;
}

bool k2hdkc_ex_cas32_str_init_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint32_t val, const char* encpass, const time_t* expire)
{
	return k2hdkc_ex_cas32_init_wa(config, ctlport, is_auto_rejoin, no_giveup_rejoin, reinterpret_cast<const unsigned char*>(pkey), (pkey ? strlen(pkey) + 1 : 0), val, encpass, expire);
}

bool k2hdkc_pm_cas32_init_wa(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint32_t val, const char* encpass, const time_t* expire)
{
	if(K2HDKC_INVALID_HANDLE == handle){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}
	K2hdkcSlave*	pSlave = reinterpret_cast<K2hdkcSlave*>(handle);
	if(!pSlave){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}

	K2hdkcComCasInit*	pComObj = GetPmSlaveK2hdkcComCasInit(pSlave);
	bool				result	= K2hdkcFullCasInit(pComObj, pkey, keylength, reinterpret_cast<const unsigned char*>(&val), sizeof(uint32_t), encpass, expire);
	if(!result){
		ERR_DKCPRN("Failed to initialize cas value.");
	}
	DKC_DELETE(pComObj);
	return result;
}

bool k2hdkc_pm_cas32_str_init_wa(k2hdkc_chmpx_h handle, const char* pkey, uint32_t val, const char* encpass, const time_t* expire)
{
	return k2hdkc_pm_cas32_init_wa(handle, reinterpret_cast<const unsigned char*>(pkey), (pkey ? strlen(pkey) + 1 : 0), val, encpass, expire);
}

bool k2hdkc_cas16_init(const char* config, const unsigned char* pkey, size_t keylength, uint16_t val)
{
	return k2hdkc_cas16_init_wa(config, pkey, keylength, val, NULL, NULL);
}

bool k2hdkc_cas16_str_init(const char* config, const char* pkey, uint16_t val)
{
	return k2hdkc_cas16_str_init_wa(config, pkey, val, NULL, NULL);
}

bool k2hdkc_ex_cas16_init(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint16_t val)
{
	return k2hdkc_ex_cas16_init_wa(config, ctlport, is_auto_rejoin, no_giveup_rejoin, pkey, keylength, val, NULL, NULL);
}

bool k2hdkc_ex_cas16_str_init(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint16_t val)
{
	return k2hdkc_ex_cas16_str_init_wa(config, ctlport, is_auto_rejoin, no_giveup_rejoin, pkey, val, NULL, NULL);
}

bool k2hdkc_pm_cas16_init(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint16_t val)
{
	return k2hdkc_pm_cas16_init_wa(handle, pkey, keylength, val, NULL, NULL);
}

bool k2hdkc_pm_cas16_str_init(k2hdkc_chmpx_h handle, const char* pkey, uint16_t val)
{
	return k2hdkc_pm_cas16_str_init_wa(handle, pkey, val, NULL, NULL);
}

bool k2hdkc_cas16_init_wa(const char* config, const unsigned char* pkey, size_t keylength, uint16_t val, const char* encpass, const time_t* expire)
{
	return k2hdkc_ex_cas16_init_wa(config, CHM_INVALID_PORT, false, false, pkey, keylength, val, encpass, expire);
}

bool k2hdkc_cas16_str_init_wa(const char* config, const char* pkey, uint16_t val, const char* encpass, const time_t* expire)
{
	return k2hdkc_ex_cas16_str_init_wa(config, CHM_INVALID_PORT, false, false, pkey, val, encpass, expire);
}

bool k2hdkc_ex_cas16_init_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint16_t val, const char* encpass, const time_t* expire)
{
	K2hdkcComCasInit*	pComObj = GetOtSlaveK2hdkcComCasInit(config, ctlport, is_auto_rejoin, no_giveup_rejoin);
	bool				result	= K2hdkcFullCasInit(pComObj, pkey, keylength, reinterpret_cast<const unsigned char*>(&val), sizeof(uint16_t), encpass, expire);
	if(!result){
		ERR_DKCPRN("Failed to initialize cas value.");
	}
	DKC_DELETE(pComObj);
	return result;
}

bool k2hdkc_ex_cas16_str_init_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint16_t val, const char* encpass, const time_t* expire)
{
	return k2hdkc_ex_cas16_init_wa(config, ctlport, is_auto_rejoin, no_giveup_rejoin, reinterpret_cast<const unsigned char*>(pkey), (pkey ? strlen(pkey) + 1 : 0), val, encpass, expire);
}

bool k2hdkc_pm_cas16_init_wa(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint16_t val, const char* encpass, const time_t* expire)
{
	if(K2HDKC_INVALID_HANDLE == handle){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}
	K2hdkcSlave*	pSlave = reinterpret_cast<K2hdkcSlave*>(handle);
	if(!pSlave){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}

	K2hdkcComCasInit*	pComObj = GetPmSlaveK2hdkcComCasInit(pSlave);
	bool				result	= K2hdkcFullCasInit(pComObj, pkey, keylength, reinterpret_cast<const unsigned char*>(&val), sizeof(uint16_t), encpass, expire);
	if(!result){
		ERR_DKCPRN("Failed to initialize cas value.");
	}
	DKC_DELETE(pComObj);
	return result;
}

bool k2hdkc_pm_cas16_str_init_wa(k2hdkc_chmpx_h handle, const char* pkey, uint16_t val, const char* encpass, const time_t* expire)
{
	return k2hdkc_pm_cas16_init_wa(handle, reinterpret_cast<const unsigned char*>(pkey), (pkey ? strlen(pkey) + 1 : 0), val, encpass, expire);
}

bool k2hdkc_cas8_init(const char* config, const unsigned char* pkey, size_t keylength, uint8_t val)
{
	return k2hdkc_cas8_init_wa(config, pkey, keylength, val, NULL, NULL);
}

bool k2hdkc_cas8_str_init(const char* config, const char* pkey, uint8_t val)
{
	return k2hdkc_cas8_str_init_wa(config, pkey, val, NULL, NULL);
}

bool k2hdkc_ex_cas8_init(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint8_t val)
{
	return k2hdkc_ex_cas8_init_wa(config, ctlport, is_auto_rejoin, no_giveup_rejoin, pkey, keylength, val, NULL, NULL);
}

bool k2hdkc_ex_cas8_str_init(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint8_t val)
{
	return k2hdkc_ex_cas8_str_init_wa(config, ctlport, is_auto_rejoin, no_giveup_rejoin, pkey, val, NULL, NULL);
}

bool k2hdkc_pm_cas8_init(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint8_t val)
{
	return k2hdkc_pm_cas8_init_wa(handle, pkey, keylength, val, NULL, NULL);
}

bool k2hdkc_pm_cas8_str_init(k2hdkc_chmpx_h handle, const char* pkey, uint8_t val)
{
	return k2hdkc_pm_cas8_str_init_wa(handle, pkey, val, NULL, NULL);
}

bool k2hdkc_cas8_init_wa(const char* config, const unsigned char* pkey, size_t keylength, uint8_t val, const char* encpass, const time_t* expire)
{
	return k2hdkc_ex_cas8_init_wa(config, CHM_INVALID_PORT, false, false, pkey, keylength, val, encpass, expire);
}

bool k2hdkc_cas8_str_init_wa(const char* config, const char* pkey, uint8_t val, const char* encpass, const time_t* expire)
{
	return k2hdkc_ex_cas8_str_init_wa(config, CHM_INVALID_PORT, false, false, pkey, val, encpass, expire);
}

bool k2hdkc_ex_cas8_init_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint8_t val, const char* encpass, const time_t* expire)
{
	K2hdkcComCasInit*	pComObj = GetOtSlaveK2hdkcComCasInit(config, ctlport, is_auto_rejoin, no_giveup_rejoin);
	bool				result	= K2hdkcFullCasInit(pComObj, pkey, keylength, reinterpret_cast<const unsigned char*>(&val), sizeof(uint8_t), encpass, expire);
	if(!result){
		ERR_DKCPRN("Failed to initialize cas value.");
	}
	DKC_DELETE(pComObj);
	return result;
}

bool k2hdkc_ex_cas8_str_init_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint8_t val, const char* encpass, const time_t* expire)
{
	return k2hdkc_ex_cas8_init_wa(config, ctlport, is_auto_rejoin, no_giveup_rejoin, reinterpret_cast<const unsigned char*>(pkey), (pkey ? strlen(pkey) + 1 : 0), val, encpass, expire);
}

bool k2hdkc_pm_cas8_init_wa(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint8_t val, const char* encpass, const time_t* expire)
{
	if(K2HDKC_INVALID_HANDLE == handle){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}
	K2hdkcSlave*	pSlave = reinterpret_cast<K2hdkcSlave*>(handle);
	if(!pSlave){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}

	K2hdkcComCasInit*	pComObj = GetPmSlaveK2hdkcComCasInit(pSlave);
	bool				result	= K2hdkcFullCasInit(pComObj, pkey, keylength, reinterpret_cast<const unsigned char*>(&val), sizeof(uint8_t), encpass, expire);
	if(!result){
		ERR_DKCPRN("Failed to initialize cas value.");
	}
	DKC_DELETE(pComObj);
	return result;
}

bool k2hdkc_pm_cas8_str_init_wa(k2hdkc_chmpx_h handle, const char* pkey, uint8_t val, const char* encpass, const time_t* expire)
{
	return k2hdkc_pm_cas8_init_wa(handle, reinterpret_cast<const unsigned char*>(pkey), (pkey ? strlen(pkey) + 1 : 0), val, encpass, expire);
}

//
// Get
//
static bool K2hdkcFullCasGet(K2hdkcComCasGet* pcomobj, const unsigned char* pkey, size_t keylength, const char* encpass, unsigned char* pval, size_t reqvallength)
{
	if(!pkey || 0 == keylength || !pval || 0 == reqvallength){
		ERR_DKCPRN("Parameters are wrong.");
		return false;
	}
	const unsigned char*	pvaltmp		= NULL;
	size_t					valtmplen	= 0L;
	bool					result		= pcomobj->CommandSend(pkey, keylength, true, encpass, &pvaltmp, &valtmplen, &Last_Response_Code);
	if(result){
		if(valtmplen == reqvallength){
			memcpy(pval, pvaltmp, valtmplen);
		}else{
			ERR_DKCPRN("Cas key value length is not as same as request size.");
			result = false;
		}
	}
	return result;
}

bool k2hdkc_cas64_get(const char* config, const unsigned char* pkey, size_t keylength, uint64_t* pval)
{
	return k2hdkc_cas64_get_wa(config, pkey, keylength, NULL, pval);
}

bool k2hdkc_cas64_str_get(const char* config, const char* pkey, uint64_t* pval)
{
	return k2hdkc_cas64_str_get_wa(config, pkey, NULL, pval);
}

bool k2hdkc_ex_cas64_get(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint64_t* pval)
{
	return k2hdkc_ex_cas64_get_wa(config, ctlport, is_auto_rejoin, no_giveup_rejoin, pkey, keylength, NULL, pval);
}

bool k2hdkc_ex_cas64_str_get(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint64_t* pval)
{
	return k2hdkc_ex_cas64_str_get_wa(config, ctlport, is_auto_rejoin, no_giveup_rejoin, pkey, NULL, pval);
}

bool k2hdkc_pm_cas64_get(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint64_t* pval)
{
	return k2hdkc_pm_cas64_get_wa(handle, pkey, keylength, NULL, pval);
}

bool k2hdkc_pm_cas64_str_get(k2hdkc_chmpx_h handle, const char* pkey, uint64_t* pval)
{
	return k2hdkc_pm_cas64_str_get_wa(handle, pkey, NULL, pval);
}

bool k2hdkc_cas64_get_wa(const char* config, const unsigned char* pkey, size_t keylength, const char* encpass, uint64_t* pval)
{
	return k2hdkc_ex_cas64_get_wa(config, CHM_INVALID_PORT, false, false, pkey, keylength, encpass, pval);
}

bool k2hdkc_cas64_str_get_wa(const char* config, const char* pkey, const char* encpass, uint64_t* pval)
{
	return k2hdkc_ex_cas64_str_get_wa(config, CHM_INVALID_PORT, false, false, pkey, encpass, pval);
}

bool k2hdkc_ex_cas64_get_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, const char* encpass, uint64_t* pval)
{
	K2hdkcComCasGet*	pComObj = GetOtSlaveK2hdkcComCasGet(config, ctlport, is_auto_rejoin, no_giveup_rejoin);
	bool				result	= K2hdkcFullCasGet(pComObj, pkey, keylength, encpass, reinterpret_cast<unsigned char*>(pval), sizeof(uint64_t));
	if(!result){
		ERR_DKCPRN("Failed to get cas value.");
	}
	DKC_DELETE(pComObj);
	return result;
}

bool k2hdkc_ex_cas64_str_get_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, const char* encpass, uint64_t* pval)
{
	return k2hdkc_ex_cas64_get_wa(config, ctlport, is_auto_rejoin, no_giveup_rejoin, reinterpret_cast<const unsigned char*>(pkey), (pkey ? strlen(pkey) + 1 : 0), encpass, pval);
}

bool k2hdkc_pm_cas64_get_wa(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, const char* encpass, uint64_t* pval)
{
	if(K2HDKC_INVALID_HANDLE == handle){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}
	K2hdkcSlave*	pSlave = reinterpret_cast<K2hdkcSlave*>(handle);
	if(!pSlave){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}

	K2hdkcComCasGet*	pComObj = GetPmSlaveK2hdkcComCasGet(pSlave);
	bool				result	= K2hdkcFullCasGet(pComObj, pkey, keylength, encpass, reinterpret_cast<unsigned char*>(pval), sizeof(uint64_t));
	if(!result){
		ERR_DKCPRN("Failed to get cas value.");
	}
	DKC_DELETE(pComObj);
	return result;
}

bool k2hdkc_pm_cas64_str_get_wa(k2hdkc_chmpx_h handle, const char* pkey, const char* encpass, uint64_t* pval)
{
	return k2hdkc_pm_cas64_get_wa(handle, reinterpret_cast<const unsigned char*>(pkey), (pkey ? strlen(pkey) + 1 : 0), encpass, pval);
}

bool k2hdkc_cas32_get(const char* config, const unsigned char* pkey, size_t keylength, uint32_t* pval)
{
	return k2hdkc_cas32_get_wa(config, pkey, keylength, NULL, pval);
}

bool k2hdkc_cas32_str_get(const char* config, const char* pkey, uint32_t* pval)
{
	return k2hdkc_cas32_str_get_wa(config, pkey, NULL, pval);
}

bool k2hdkc_ex_cas32_get(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint32_t* pval)
{
	return k2hdkc_ex_cas32_get_wa(config, ctlport, is_auto_rejoin, no_giveup_rejoin, pkey, keylength, NULL, pval);
}

bool k2hdkc_ex_cas32_str_get(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint32_t* pval)
{
	return k2hdkc_ex_cas32_str_get_wa(config, ctlport, is_auto_rejoin, no_giveup_rejoin, pkey, NULL, pval);
}

bool k2hdkc_pm_cas32_get(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint32_t* pval)
{
	return k2hdkc_pm_cas32_get_wa(handle, pkey, keylength, NULL, pval);
}

bool k2hdkc_pm_cas32_str_get(k2hdkc_chmpx_h handle, const char* pkey, uint32_t* pval)
{
	return k2hdkc_pm_cas32_str_get_wa(handle, pkey, NULL, pval);
}

bool k2hdkc_cas32_get_wa(const char* config, const unsigned char* pkey, size_t keylength, const char* encpass, uint32_t* pval)
{
	return k2hdkc_ex_cas32_get_wa(config, CHM_INVALID_PORT, false, false, pkey, keylength, encpass, pval);
}

bool k2hdkc_cas32_str_get_wa(const char* config, const char* pkey, const char* encpass, uint32_t* pval)
{
	return k2hdkc_ex_cas32_str_get_wa(config, CHM_INVALID_PORT, false, false, pkey, encpass, pval);
}

bool k2hdkc_ex_cas32_get_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, const char* encpass, uint32_t* pval)
{
	K2hdkcComCasGet*	pComObj = GetOtSlaveK2hdkcComCasGet(config, ctlport, is_auto_rejoin, no_giveup_rejoin);
	bool				result	= K2hdkcFullCasGet(pComObj, pkey, keylength, encpass, reinterpret_cast<unsigned char*>(pval), sizeof(uint32_t));
	if(!result){
		ERR_DKCPRN("Failed to get cas value.");
	}
	DKC_DELETE(pComObj);
	return result;
}

bool k2hdkc_ex_cas32_str_get_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, const char* encpass, uint32_t* pval)
{
	return k2hdkc_ex_cas32_get_wa(config, ctlport, is_auto_rejoin, no_giveup_rejoin, reinterpret_cast<const unsigned char*>(pkey), (pkey ? strlen(pkey) + 1 : 0), encpass, pval);
}

bool k2hdkc_pm_cas32_get_wa(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, const char* encpass, uint32_t* pval)
{
	if(K2HDKC_INVALID_HANDLE == handle){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}
	K2hdkcSlave*	pSlave = reinterpret_cast<K2hdkcSlave*>(handle);
	if(!pSlave){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}

	K2hdkcComCasGet*	pComObj = GetPmSlaveK2hdkcComCasGet(pSlave);
	bool				result	= K2hdkcFullCasGet(pComObj, pkey, keylength, encpass, reinterpret_cast<unsigned char*>(pval), sizeof(uint32_t));
	if(!result){
		ERR_DKCPRN("Failed to get cas value.");
	}
	DKC_DELETE(pComObj);
	return result;
}

bool k2hdkc_pm_cas32_str_get_wa(k2hdkc_chmpx_h handle, const char* pkey, const char* encpass, uint32_t* pval)
{
	return k2hdkc_pm_cas32_get_wa(handle, reinterpret_cast<const unsigned char*>(pkey), (pkey ? strlen(pkey) + 1 : 0), encpass, pval);
}

bool k2hdkc_cas16_get(const char* config, const unsigned char* pkey, size_t keylength, uint16_t* pval)
{
	return k2hdkc_cas16_get_wa(config, pkey, keylength, NULL, pval);
}

bool k2hdkc_cas16_str_get(const char* config, const char* pkey, uint16_t* pval)
{
	return k2hdkc_cas16_str_get_wa(config, pkey, NULL, pval);
}

bool k2hdkc_ex_cas16_get(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint16_t* pval)
{
	return k2hdkc_ex_cas16_get_wa(config, ctlport, is_auto_rejoin, no_giveup_rejoin, pkey, keylength, NULL, pval);
}

bool k2hdkc_ex_cas16_str_get(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint16_t* pval)
{
	return k2hdkc_ex_cas16_str_get_wa(config, ctlport, is_auto_rejoin, no_giveup_rejoin, pkey, NULL, pval);
}

bool k2hdkc_pm_cas16_get(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint16_t* pval)
{
	return k2hdkc_pm_cas16_get_wa(handle, pkey, keylength, NULL, pval);
}

bool k2hdkc_pm_cas16_str_get(k2hdkc_chmpx_h handle, const char* pkey, uint16_t* pval)
{
	return k2hdkc_pm_cas16_str_get_wa(handle, pkey, NULL, pval);
}

bool k2hdkc_cas16_get_wa(const char* config, const unsigned char* pkey, size_t keylength, const char* encpass, uint16_t* pval)
{
	return k2hdkc_ex_cas16_get_wa(config, CHM_INVALID_PORT, false, false, pkey, keylength, encpass, pval);
}

bool k2hdkc_cas16_str_get_wa(const char* config, const char* pkey, const char* encpass, uint16_t* pval)
{
	return k2hdkc_ex_cas16_str_get_wa(config, CHM_INVALID_PORT, false, false, pkey, encpass, pval);
}

bool k2hdkc_ex_cas16_get_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, const char* encpass, uint16_t* pval)
{
	K2hdkcComCasGet*	pComObj = GetOtSlaveK2hdkcComCasGet(config, ctlport, is_auto_rejoin, no_giveup_rejoin);
	bool				result	= K2hdkcFullCasGet(pComObj, pkey, keylength, encpass, reinterpret_cast<unsigned char*>(pval), sizeof(uint16_t));
	if(!result){
		ERR_DKCPRN("Failed to get cas value.");
	}
	DKC_DELETE(pComObj);
	return result;
}

bool k2hdkc_ex_cas16_str_get_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, const char* encpass, uint16_t* pval)
{
	return k2hdkc_ex_cas16_get_wa(config, ctlport, is_auto_rejoin, no_giveup_rejoin, reinterpret_cast<const unsigned char*>(pkey), (pkey ? strlen(pkey) + 1 : 0), encpass, pval);
}

bool k2hdkc_pm_cas16_get_wa(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, const char* encpass, uint16_t* pval)
{
	if(K2HDKC_INVALID_HANDLE == handle){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}
	K2hdkcSlave*	pSlave = reinterpret_cast<K2hdkcSlave*>(handle);
	if(!pSlave){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}

	K2hdkcComCasGet*	pComObj = GetPmSlaveK2hdkcComCasGet(pSlave);
	bool				result	= K2hdkcFullCasGet(pComObj, pkey, keylength, encpass, reinterpret_cast<unsigned char*>(pval), sizeof(uint16_t));
	if(!result){
		ERR_DKCPRN("Failed to get cas value.");
	}
	DKC_DELETE(pComObj);
	return result;
}

bool k2hdkc_pm_cas16_str_get_wa(k2hdkc_chmpx_h handle, const char* pkey, const char* encpass, uint16_t* pval)
{
	return k2hdkc_pm_cas16_get_wa(handle, reinterpret_cast<const unsigned char*>(pkey), (pkey ? strlen(pkey) + 1 : 0), encpass, pval);
}

bool k2hdkc_cas8_get(const char* config, const unsigned char* pkey, size_t keylength, uint8_t* pval)
{
	return k2hdkc_cas8_get_wa(config, pkey, keylength, NULL, pval);
}

bool k2hdkc_cas8_str_get(const char* config, const char* pkey, uint8_t* pval)
{
	return k2hdkc_cas8_str_get_wa(config, pkey, NULL, pval);
}

bool k2hdkc_ex_cas8_get(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint8_t* pval)
{
	return k2hdkc_ex_cas8_get_wa(config, ctlport, is_auto_rejoin, no_giveup_rejoin, pkey, keylength, NULL, pval);
}

bool k2hdkc_ex_cas8_str_get(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint8_t* pval)
{
	return k2hdkc_ex_cas8_str_get_wa(config, ctlport, is_auto_rejoin, no_giveup_rejoin, pkey, NULL, pval);
}

bool k2hdkc_pm_cas8_get(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint8_t* pval)
{
	return k2hdkc_pm_cas8_get_wa(handle, pkey, keylength, NULL, pval);
}

bool k2hdkc_pm_cas8_str_get(k2hdkc_chmpx_h handle, const char* pkey, uint8_t* pval)
{
	return k2hdkc_pm_cas8_str_get_wa(handle, pkey, NULL, pval);
}

bool k2hdkc_cas8_get_wa(const char* config, const unsigned char* pkey, size_t keylength, const char* encpass, uint8_t* pval)
{
	return k2hdkc_ex_cas8_get_wa(config, CHM_INVALID_PORT, false, false, pkey, keylength, encpass, pval);
}

bool k2hdkc_cas8_str_get_wa(const char* config, const char* pkey, const char* encpass, uint8_t* pval)
{
	return k2hdkc_ex_cas8_str_get_wa(config, CHM_INVALID_PORT, false, false, pkey, encpass, pval);
}

bool k2hdkc_ex_cas8_get_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, const char* encpass, uint8_t* pval)
{
	K2hdkcComCasGet*	pComObj = GetOtSlaveK2hdkcComCasGet(config, ctlport, is_auto_rejoin, no_giveup_rejoin);
	bool				result	= K2hdkcFullCasGet(pComObj, pkey, keylength, encpass, reinterpret_cast<unsigned char*>(pval), sizeof(uint8_t));
	if(!result){
		ERR_DKCPRN("Failed to get cas value.");
	}
	DKC_DELETE(pComObj);
	return result;
}

bool k2hdkc_ex_cas8_str_get_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, const char* encpass, uint8_t* pval)
{
	return k2hdkc_ex_cas8_get_wa(config, ctlport, is_auto_rejoin, no_giveup_rejoin, reinterpret_cast<const unsigned char*>(pkey), (pkey ? strlen(pkey) + 1 : 0), encpass, pval);
}

bool k2hdkc_pm_cas8_get_wa(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, const char* encpass, uint8_t* pval)
{
	if(K2HDKC_INVALID_HANDLE == handle){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}
	K2hdkcSlave*	pSlave = reinterpret_cast<K2hdkcSlave*>(handle);
	if(!pSlave){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}

	K2hdkcComCasGet*	pComObj = GetPmSlaveK2hdkcComCasGet(pSlave);
	bool				result	= K2hdkcFullCasGet(pComObj, pkey, keylength, encpass, reinterpret_cast<unsigned char*>(pval), sizeof(uint8_t));
	if(!result){
		ERR_DKCPRN("Failed to get cas value.");
	}
	DKC_DELETE(pComObj);
	return result;
}

bool k2hdkc_pm_cas8_str_get_wa(k2hdkc_chmpx_h handle, const char* pkey, const char* encpass, uint8_t* pval)
{
	return k2hdkc_pm_cas8_get_wa(handle, reinterpret_cast<const unsigned char*>(pkey), (pkey ? strlen(pkey) + 1 : 0), encpass, pval);
}

//
// Set
//
static bool K2hdkcFullCasSet(K2hdkcComCasSet* pcomobj, const unsigned char* pkey, size_t keylength, const unsigned char* poldval, size_t oldvallength, const unsigned char* pnewval, size_t newvallength, const char* encpass, const time_t* expire)
{
	if(!pkey || 0 == keylength || !poldval || 0 == oldvallength || !pnewval || 0 == newvallength){
		ERR_DKCPRN("Parameters are wrong.");
		return false;
	}
	return pcomobj->CommandSend(pkey, keylength, poldval, oldvallength, pnewval, newvallength, true, encpass, expire, &Last_Response_Code);
}

bool k2hdkc_cas64_set(const char* config, const unsigned char* pkey, size_t keylength, uint64_t oldval, uint64_t newval)
{
	return k2hdkc_cas64_set_wa(config, pkey, keylength, oldval, newval, NULL, NULL);
}

bool k2hdkc_cas64_str_set(const char* config, const char* pkey, uint64_t oldval, uint64_t newval)
{
	return k2hdkc_cas64_str_set_wa(config, pkey, oldval, newval, NULL, NULL);
}

bool k2hdkc_ex_cas64_set(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint64_t oldval, uint64_t newval)
{
	return k2hdkc_ex_cas64_set_wa(config, ctlport, is_auto_rejoin, no_giveup_rejoin, pkey, keylength, oldval, newval, NULL, NULL);
}

bool k2hdkc_ex_cas64_str_set(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint64_t oldval, uint64_t newval)
{
	return k2hdkc_ex_cas64_str_set_wa(config, ctlport, is_auto_rejoin, no_giveup_rejoin, pkey, oldval, newval, NULL, NULL);
}

bool k2hdkc_pm_cas64_set(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint64_t oldval, uint64_t newval)
{
	return k2hdkc_pm_cas64_set_wa(handle, pkey, keylength, oldval, newval, NULL, NULL);
}

bool k2hdkc_pm_cas64_str_set(k2hdkc_chmpx_h handle, const char* pkey, uint64_t oldval, uint64_t newval)
{
	return k2hdkc_pm_cas64_str_set_wa(handle, pkey, oldval, newval, NULL, NULL);
}

bool k2hdkc_cas64_set_wa(const char* config, const unsigned char* pkey, size_t keylength, uint64_t oldval, uint64_t newval, const char* encpass, const time_t* expire)
{
	return k2hdkc_ex_cas64_set_wa(config, CHM_INVALID_PORT, false, false, pkey, keylength, oldval, newval, encpass, expire);
}

bool k2hdkc_cas64_str_set_wa(const char* config, const char* pkey, uint64_t oldval, uint64_t newval, const char* encpass, const time_t* expire)
{
	return k2hdkc_ex_cas64_str_set_wa(config, CHM_INVALID_PORT, false, false, pkey, oldval, newval, encpass, expire);
}

bool k2hdkc_ex_cas64_set_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint64_t oldval, uint64_t newval, const char* encpass, const time_t* expire)
{
	K2hdkcComCasSet*	pComObj = GetOtSlaveK2hdkcComCasSet(config, ctlport, is_auto_rejoin, no_giveup_rejoin);
	bool				result	= K2hdkcFullCasSet(pComObj, pkey, keylength, reinterpret_cast<const unsigned char*>(&oldval), sizeof(uint64_t), reinterpret_cast<const unsigned char*>(&newval), sizeof(uint64_t), encpass, expire);
	if(!result){
		ERR_DKCPRN("Failed to set cas value.");
	}
	DKC_DELETE(pComObj);
	return result;
}

bool k2hdkc_ex_cas64_str_set_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint64_t oldval, uint64_t newval, const char* encpass, const time_t* expire)
{
	return k2hdkc_ex_cas64_set_wa(config, ctlport, is_auto_rejoin, no_giveup_rejoin, reinterpret_cast<const unsigned char*>(pkey), (pkey ? strlen(pkey) + 1 : 0), oldval, newval, encpass, expire);
}

bool k2hdkc_pm_cas64_set_wa(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint64_t oldval, uint64_t newval, const char* encpass, const time_t* expire)
{
	if(K2HDKC_INVALID_HANDLE == handle){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}
	K2hdkcSlave*	pSlave = reinterpret_cast<K2hdkcSlave*>(handle);
	if(!pSlave){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}

	K2hdkcComCasSet*	pComObj = GetPmSlaveK2hdkcComCasSet(pSlave);
	bool				result	= K2hdkcFullCasSet(pComObj, pkey, keylength, reinterpret_cast<const unsigned char*>(&oldval), sizeof(uint64_t), reinterpret_cast<const unsigned char*>(&newval), sizeof(uint64_t), encpass, expire);
	if(!result){
		ERR_DKCPRN("Failed to set cas value.");
	}
	DKC_DELETE(pComObj);
	return result;
}

bool k2hdkc_pm_cas64_str_set_wa(k2hdkc_chmpx_h handle, const char* pkey, uint64_t oldval, uint64_t newval, const char* encpass, const time_t* expire)
{
	return k2hdkc_pm_cas64_set_wa(handle, reinterpret_cast<const unsigned char*>(pkey), (pkey ? strlen(pkey) + 1 : 0), oldval, newval, encpass, expire);
}

bool k2hdkc_cas32_set(const char* config, const unsigned char* pkey, size_t keylength, uint32_t oldval, uint32_t newval)
{
	return k2hdkc_cas32_set_wa(config, pkey, keylength, oldval, newval, NULL, NULL);
}

bool k2hdkc_cas32_str_set(const char* config, const char* pkey, uint32_t oldval, uint32_t newval)
{
	return k2hdkc_cas32_str_set_wa(config, pkey, oldval, newval, NULL, NULL);
}

bool k2hdkc_ex_cas32_set(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint32_t oldval, uint32_t newval)
{
	return k2hdkc_ex_cas32_set_wa(config, ctlport, is_auto_rejoin, no_giveup_rejoin, pkey, keylength, oldval, newval, NULL, NULL);
}

bool k2hdkc_ex_cas32_str_set(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint32_t oldval, uint32_t newval)
{
	return k2hdkc_ex_cas32_str_set_wa(config, ctlport, is_auto_rejoin, no_giveup_rejoin, pkey, oldval, newval, NULL, NULL);
}

bool k2hdkc_pm_cas32_set(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint32_t oldval, uint32_t newval)
{
	return k2hdkc_pm_cas32_set_wa(handle, pkey, keylength, oldval, newval, NULL, NULL);
}

bool k2hdkc_pm_cas32_str_set(k2hdkc_chmpx_h handle, const char* pkey, uint32_t oldval, uint32_t newval)
{
	return k2hdkc_pm_cas32_str_set_wa(handle, pkey, oldval, newval, NULL, NULL);
}

bool k2hdkc_cas32_set_wa(const char* config, const unsigned char* pkey, size_t keylength, uint32_t oldval, uint32_t newval, const char* encpass, const time_t* expire)
{
	return k2hdkc_ex_cas32_set_wa(config, CHM_INVALID_PORT, false, false, pkey, keylength, oldval, newval, encpass, expire);
}

bool k2hdkc_cas32_str_set_wa(const char* config, const char* pkey, uint32_t oldval, uint32_t newval, const char* encpass, const time_t* expire)
{
	return k2hdkc_ex_cas32_str_set_wa(config, CHM_INVALID_PORT, false, false, pkey, oldval, newval, encpass, expire);
}

bool k2hdkc_ex_cas32_set_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint32_t oldval, uint32_t newval, const char* encpass, const time_t* expire)
{
	K2hdkcComCasSet*	pComObj = GetOtSlaveK2hdkcComCasSet(config, ctlport, is_auto_rejoin, no_giveup_rejoin);
	bool				result	= K2hdkcFullCasSet(pComObj, pkey, keylength, reinterpret_cast<const unsigned char*>(&oldval), sizeof(uint32_t), reinterpret_cast<const unsigned char*>(&newval), sizeof(uint32_t), encpass, expire);
	if(!result){
		ERR_DKCPRN("Failed to set cas value.");
	}
	DKC_DELETE(pComObj);
	return result;
}

bool k2hdkc_ex_cas32_str_set_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint32_t oldval, uint32_t newval, const char* encpass, const time_t* expire)
{
	return k2hdkc_ex_cas32_set_wa(config, ctlport, is_auto_rejoin, no_giveup_rejoin, reinterpret_cast<const unsigned char*>(pkey), (pkey ? strlen(pkey) + 1 : 0), oldval, newval, encpass, expire);
}

bool k2hdkc_pm_cas32_set_wa(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint32_t oldval, uint32_t newval, const char* encpass, const time_t* expire)
{
	if(K2HDKC_INVALID_HANDLE == handle){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}
	K2hdkcSlave*	pSlave = reinterpret_cast<K2hdkcSlave*>(handle);
	if(!pSlave){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}

	K2hdkcComCasSet*	pComObj = GetPmSlaveK2hdkcComCasSet(pSlave);
	bool				result	= K2hdkcFullCasSet(pComObj, pkey, keylength, reinterpret_cast<const unsigned char*>(&oldval), sizeof(uint32_t), reinterpret_cast<const unsigned char*>(&newval), sizeof(uint32_t), encpass, expire);
	if(!result){
		ERR_DKCPRN("Failed to set cas value.");
	}
	DKC_DELETE(pComObj);
	return result;
}

bool k2hdkc_pm_cas32_str_set_wa(k2hdkc_chmpx_h handle, const char* pkey, uint32_t oldval, uint32_t newval, const char* encpass, const time_t* expire)
{
	return k2hdkc_pm_cas32_set_wa(handle, reinterpret_cast<const unsigned char*>(pkey), (pkey ? strlen(pkey) + 1 : 0), oldval, newval, encpass, expire);
}

bool k2hdkc_cas16_set(const char* config, const unsigned char* pkey, size_t keylength, uint16_t oldval, uint16_t newval)
{
	return k2hdkc_cas16_set_wa(config, pkey, keylength, oldval, newval, NULL, NULL);
}

bool k2hdkc_cas16_str_set(const char* config, const char* pkey, uint16_t oldval, uint16_t newval)
{
	return k2hdkc_cas16_str_set_wa(config, pkey, oldval, newval, NULL, NULL);
}

bool k2hdkc_ex_cas16_set(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint16_t oldval, uint16_t newval)
{
	return k2hdkc_ex_cas16_set_wa(config, ctlport, is_auto_rejoin, no_giveup_rejoin, pkey, keylength, oldval, newval, NULL, NULL);
}

bool k2hdkc_ex_cas16_str_set(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint16_t oldval, uint16_t newval)
{
	return k2hdkc_ex_cas16_str_set_wa(config, ctlport, is_auto_rejoin, no_giveup_rejoin, pkey, oldval, newval, NULL, NULL);
}

bool k2hdkc_pm_cas16_set(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint16_t oldval, uint16_t newval)
{
	return k2hdkc_pm_cas16_set_wa(handle, pkey, keylength, oldval, newval, NULL, NULL);
}

bool k2hdkc_pm_cas16_str_set(k2hdkc_chmpx_h handle, const char* pkey, uint16_t oldval, uint16_t newval)
{
	return k2hdkc_pm_cas16_str_set_wa(handle, pkey, oldval, newval, NULL, NULL);
}

bool k2hdkc_cas16_set_wa(const char* config, const unsigned char* pkey, size_t keylength, uint16_t oldval, uint16_t newval, const char* encpass, const time_t* expire)
{
	return k2hdkc_ex_cas16_set_wa(config, CHM_INVALID_PORT, false, false, pkey, keylength, oldval, newval, encpass, expire);
}

bool k2hdkc_cas16_str_set_wa(const char* config, const char* pkey, uint16_t oldval, uint16_t newval, const char* encpass, const time_t* expire)
{
	return k2hdkc_ex_cas16_str_set_wa(config, CHM_INVALID_PORT, false, false, pkey, oldval, newval, encpass, expire);
}

bool k2hdkc_ex_cas16_set_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint16_t oldval, uint16_t newval, const char* encpass, const time_t* expire)
{
	K2hdkcComCasSet*	pComObj = GetOtSlaveK2hdkcComCasSet(config, ctlport, is_auto_rejoin, no_giveup_rejoin);
	bool				result	= K2hdkcFullCasSet(pComObj, pkey, keylength, reinterpret_cast<const unsigned char*>(&oldval), sizeof(uint16_t), reinterpret_cast<const unsigned char*>(&newval), sizeof(uint16_t), encpass, expire);
	if(!result){
		ERR_DKCPRN("Failed to set cas value.");
	}
	DKC_DELETE(pComObj);
	return result;
}

bool k2hdkc_ex_cas16_str_set_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint16_t oldval, uint16_t newval, const char* encpass, const time_t* expire)
{
	return k2hdkc_ex_cas16_set_wa(config, ctlport, is_auto_rejoin, no_giveup_rejoin, reinterpret_cast<const unsigned char*>(pkey), (pkey ? strlen(pkey) + 1 : 0), oldval, newval, encpass, expire);
}

bool k2hdkc_pm_cas16_set_wa(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint16_t oldval, uint16_t newval, const char* encpass, const time_t* expire)
{
	if(K2HDKC_INVALID_HANDLE == handle){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}
	K2hdkcSlave*	pSlave = reinterpret_cast<K2hdkcSlave*>(handle);
	if(!pSlave){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}

	K2hdkcComCasSet*	pComObj = GetPmSlaveK2hdkcComCasSet(pSlave);
	bool				result	= K2hdkcFullCasSet(pComObj, pkey, keylength, reinterpret_cast<const unsigned char*>(&oldval), sizeof(uint16_t), reinterpret_cast<const unsigned char*>(&newval), sizeof(uint16_t), encpass, expire);
	if(!result){
		ERR_DKCPRN("Failed to set cas value.");
	}
	DKC_DELETE(pComObj);
	return result;
}

bool k2hdkc_pm_cas16_str_set_wa(k2hdkc_chmpx_h handle, const char* pkey, uint16_t oldval, uint16_t newval, const char* encpass, const time_t* expire)
{
	return k2hdkc_pm_cas16_set_wa(handle, reinterpret_cast<const unsigned char*>(pkey), (pkey ? strlen(pkey) + 1 : 0), oldval, newval, encpass, expire);
}

bool k2hdkc_cas8_set(const char* config, const unsigned char* pkey, size_t keylength, uint8_t oldval, uint8_t newval)
{
	return k2hdkc_cas8_set_wa(config, pkey, keylength, oldval, newval, NULL, NULL);
}

bool k2hdkc_cas8_str_set(const char* config, const char* pkey, uint8_t oldval, uint8_t newval)
{
	return k2hdkc_cas8_str_set_wa(config, pkey, oldval, newval, NULL, NULL);
}

bool k2hdkc_ex_cas8_set(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint8_t oldval, uint8_t newval)
{
	return k2hdkc_ex_cas8_set_wa(config, ctlport, is_auto_rejoin, no_giveup_rejoin, pkey, keylength, oldval, newval, NULL, NULL);
}

bool k2hdkc_ex_cas8_str_set(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint8_t oldval, uint8_t newval)
{
	return k2hdkc_ex_cas8_str_set_wa(config, ctlport, is_auto_rejoin, no_giveup_rejoin, pkey, oldval, newval, NULL, NULL);
}

bool k2hdkc_pm_cas8_set(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint8_t oldval, uint8_t newval)
{
	return k2hdkc_pm_cas8_set_wa(handle, pkey, keylength, oldval, newval, NULL, NULL);
}

bool k2hdkc_pm_cas8_str_set(k2hdkc_chmpx_h handle, const char* pkey, uint8_t oldval, uint8_t newval)
{
	return k2hdkc_pm_cas8_str_set_wa(handle, pkey, oldval, newval, NULL, NULL);
}

bool k2hdkc_cas8_set_wa(const char* config, const unsigned char* pkey, size_t keylength, uint8_t oldval, uint8_t newval, const char* encpass, const time_t* expire)
{
	return k2hdkc_ex_cas8_set_wa(config, CHM_INVALID_PORT, false, false, pkey, keylength, oldval, newval, encpass, expire);
}

bool k2hdkc_cas8_str_set_wa(const char* config, const char* pkey, uint8_t oldval, uint8_t newval, const char* encpass, const time_t* expire)
{
	return k2hdkc_ex_cas8_str_set_wa(config, CHM_INVALID_PORT, false, false, pkey, oldval, newval, encpass, expire);
}

bool k2hdkc_ex_cas8_set_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint8_t oldval, uint8_t newval, const char* encpass, const time_t* expire)
{
	K2hdkcComCasSet*	pComObj = GetOtSlaveK2hdkcComCasSet(config, ctlport, is_auto_rejoin, no_giveup_rejoin);
	bool				result	= K2hdkcFullCasSet(pComObj, pkey, keylength, reinterpret_cast<const unsigned char*>(&oldval), sizeof(uint8_t), reinterpret_cast<const unsigned char*>(&newval), sizeof(uint8_t), encpass, expire);
	if(!result){
		ERR_DKCPRN("Failed to set cas value.");
	}
	DKC_DELETE(pComObj);
	return result;
}

bool k2hdkc_ex_cas8_str_set_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint8_t oldval, uint8_t newval, const char* encpass, const time_t* expire)
{
	return k2hdkc_ex_cas8_set_wa(config, ctlport, is_auto_rejoin, no_giveup_rejoin, reinterpret_cast<const unsigned char*>(pkey), (pkey ? strlen(pkey) + 1 : 0), oldval, newval, encpass, expire);
}

bool k2hdkc_pm_cas8_set_wa(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint8_t oldval, uint8_t newval, const char* encpass, const time_t* expire)
{
	if(K2HDKC_INVALID_HANDLE == handle){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}
	K2hdkcSlave*	pSlave = reinterpret_cast<K2hdkcSlave*>(handle);
	if(!pSlave){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}

	K2hdkcComCasSet*	pComObj = GetPmSlaveK2hdkcComCasSet(pSlave);
	bool				result	= K2hdkcFullCasSet(pComObj, pkey, keylength, reinterpret_cast<const unsigned char*>(&oldval), sizeof(uint8_t), reinterpret_cast<const unsigned char*>(&newval), sizeof(uint8_t), encpass, expire);
	if(!result){
		ERR_DKCPRN("Failed to set cas value.");
	}
	DKC_DELETE(pComObj);
	return result;
}

bool k2hdkc_pm_cas8_str_set_wa(k2hdkc_chmpx_h handle, const char* pkey, uint8_t oldval, uint8_t newval, const char* encpass, const time_t* expire)
{
	return k2hdkc_pm_cas8_set_wa(handle, reinterpret_cast<const unsigned char*>(pkey), (pkey ? strlen(pkey) + 1 : 0), oldval, newval, encpass, expire);
}

//
// Increment/Decrement
//
static bool K2hdkcFullCasIncDec(K2hdkcComCasIncDec* pcomobj, const unsigned char* pkey, size_t keylength, bool is_increment, const char* encpass, const time_t* expire)
{
	if(!pkey || 0 == keylength){
		ERR_DKCPRN("Parameters are wrong.");
		return false;
	}
	return pcomobj->CommandSend(pkey, keylength, is_increment, true, encpass, expire, &Last_Response_Code);
}

bool k2hdkc_cas_increment(const char* config, const unsigned char* pkey, size_t keylength)
{
	return k2hdkc_cas_increment_wa(config, pkey, keylength, NULL, NULL);
}

bool k2hdkc_cas_str_increment(const char* config, const char* pkey)
{
	return k2hdkc_cas_str_increment_wa(config, pkey, NULL, NULL);
}

bool k2hdkc_ex_cas_increment(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength)
{
	return k2hdkc_ex_cas_increment_wa(config, ctlport, is_auto_rejoin, no_giveup_rejoin, pkey, keylength, NULL, NULL);
}

bool k2hdkc_ex_cas_str_increment(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey)
{
	return k2hdkc_ex_cas_str_increment_wa(config, ctlport, is_auto_rejoin, no_giveup_rejoin, pkey, NULL, NULL);
}

bool k2hdkc_pm_cas_increment(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength)
{
	return k2hdkc_pm_cas_increment_wa(handle, pkey, keylength, NULL, NULL);
}

bool k2hdkc_pm_cas_str_increment(k2hdkc_chmpx_h handle, const char* pkey)
{
	return k2hdkc_pm_cas_str_increment_wa(handle, pkey, NULL, NULL);
}

bool k2hdkc_cas_increment_wa(const char* config, const unsigned char* pkey, size_t keylength, const char* encpass, const time_t* expire)
{
	return k2hdkc_ex_cas_increment_wa(config, CHM_INVALID_PORT, false, false, pkey, keylength, encpass, expire);
}

bool k2hdkc_cas_str_increment_wa(const char* config, const char* pkey, const char* encpass, const time_t* expire)
{
	return k2hdkc_ex_cas_str_increment_wa(config, CHM_INVALID_PORT, false, false, pkey, encpass, expire);
}

bool k2hdkc_ex_cas_increment_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, const char* encpass, const time_t* expire)
{
	K2hdkcComCasIncDec*	pComObj = GetOtSlaveK2hdkcComCasIncDec(config, ctlport, is_auto_rejoin, no_giveup_rejoin);
	bool				result	= K2hdkcFullCasIncDec(pComObj, pkey, keylength, true, encpass, expire);
	if(!result){
		ERR_DKCPRN("Failed to increment cas value.");
	}
	DKC_DELETE(pComObj);
	return result;
}

bool k2hdkc_ex_cas_str_increment_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, const char* encpass, const time_t* expire)
{
	return k2hdkc_ex_cas_increment_wa(config, ctlport, is_auto_rejoin, no_giveup_rejoin, reinterpret_cast<const unsigned char*>(pkey), (pkey ? strlen(pkey) + 1 : 0), encpass, expire);
}

bool k2hdkc_pm_cas_increment_wa(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, const char* encpass, const time_t* expire)
{
	if(K2HDKC_INVALID_HANDLE == handle){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}
	K2hdkcSlave*	pSlave = reinterpret_cast<K2hdkcSlave*>(handle);
	if(!pSlave){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}

	K2hdkcComCasIncDec*	pComObj = GetPmSlaveK2hdkcComCasIncDec(pSlave);
	bool				result	= K2hdkcFullCasIncDec(pComObj, pkey, keylength, true, encpass, expire);
	if(!result){
		ERR_DKCPRN("Failed to increment cas value.");
	}
	DKC_DELETE(pComObj);
	return result;
}

bool k2hdkc_pm_cas_str_increment_wa(k2hdkc_chmpx_h handle, const char* pkey, const char* encpass, const time_t* expire)
{
	return k2hdkc_pm_cas_increment_wa(handle, reinterpret_cast<const unsigned char*>(pkey), (pkey ? strlen(pkey) + 1 : 0), encpass, expire);
}

bool k2hdkc_cas_decrement(const char* config, const unsigned char* pkey, size_t keylength)
{
	return k2hdkc_cas_decrement_wa(config, pkey, keylength, NULL, NULL);
}

bool k2hdkc_cas_str_decrement(const char* config, const char* pkey)
{
	return k2hdkc_cas_str_decrement_wa(config, pkey, NULL, NULL);
}

bool k2hdkc_ex_cas_decrement(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength)
{
	return k2hdkc_ex_cas_decrement_wa(config, ctlport, is_auto_rejoin, no_giveup_rejoin, pkey, keylength, NULL, NULL);
}

bool k2hdkc_ex_cas_str_decrement(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey)
{
	return k2hdkc_ex_cas_str_decrement_wa(config, ctlport, is_auto_rejoin, no_giveup_rejoin, pkey, NULL, NULL);
}

bool k2hdkc_pm_cas_decrement(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength)
{
	return k2hdkc_pm_cas_decrement_wa(handle, pkey, keylength, NULL, NULL);
}

bool k2hdkc_pm_cas_str_decrement(k2hdkc_chmpx_h handle, const char* pkey)
{
	return k2hdkc_pm_cas_str_decrement_wa(handle, pkey, NULL, NULL);
}

bool k2hdkc_cas_decrement_wa(const char* config, const unsigned char* pkey, size_t keylength, const char* encpass, const time_t* expire)
{
	return k2hdkc_ex_cas_decrement_wa(config, CHM_INVALID_PORT, false, false, pkey, keylength, encpass, expire);
}

bool k2hdkc_cas_str_decrement_wa(const char* config, const char* pkey, const char* encpass, const time_t* expire)
{
	return k2hdkc_ex_cas_str_decrement_wa(config, CHM_INVALID_PORT, false, false, pkey, encpass, expire);
}

bool k2hdkc_ex_cas_decrement_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, const char* encpass, const time_t* expire)
{
	K2hdkcComCasIncDec*	pComObj = GetOtSlaveK2hdkcComCasIncDec(config, ctlport, is_auto_rejoin, no_giveup_rejoin);
	bool				result	= K2hdkcFullCasIncDec(pComObj, pkey, keylength, false, encpass, expire);
	if(!result){
		ERR_DKCPRN("Failed to decrement cas value.");
	}
	DKC_DELETE(pComObj);
	return result;
}

bool k2hdkc_ex_cas_str_decrement_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, const char* encpass, const time_t* expire)
{
	return k2hdkc_ex_cas_decrement_wa(config, ctlport, is_auto_rejoin, no_giveup_rejoin, reinterpret_cast<const unsigned char*>(pkey), (pkey ? strlen(pkey) + 1 : 0), encpass, expire);
}

bool k2hdkc_pm_cas_decrement_wa(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, const char* encpass, const time_t* expire)
{
	if(K2HDKC_INVALID_HANDLE == handle){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}
	K2hdkcSlave*	pSlave = reinterpret_cast<K2hdkcSlave*>(handle);
	if(!pSlave){
		ERR_DKCPRN("Parameter handle is invalid.");
		return false;
	}

	K2hdkcComCasIncDec*	pComObj = GetPmSlaveK2hdkcComCasIncDec(pSlave);
	bool				result	= K2hdkcFullCasIncDec(pComObj, pkey, keylength, false, encpass, expire);
	if(!result){
		ERR_DKCPRN("Failed to decrement cas value.");
	}
	DKC_DELETE(pComObj);
	return result;
}

bool k2hdkc_pm_cas_str_decrement_wa(k2hdkc_chmpx_h handle, const char* pkey, const char* encpass, const time_t* expire)
{
	return k2hdkc_pm_cas_decrement_wa(handle, reinterpret_cast<const unsigned char*>(pkey), (pkey ? strlen(pkey) + 1 : 0), encpass, expire);
}

//---------------------------------------------------------
// Functions - Version
//---------------------------------------------------------
extern char k2hdkc_commit_hash[];

void k2hdkc_print_version(FILE* stream)
{
	static const char format[] =
		"\n"
		"K2hdkc Version %s (commit: %s)\n"
		"\n"
		"Copyright 2016 Yahoo! JAPAN corporation.\n"
		"\n"
		"K2HDKC is k2hash based distributed KVS cluster.\n"
		"K2HDKC uses K2HASH, CHMPX, FULLOCK libraries. K2HDKC supports\n"
		"distributed KVS cluster server program and client libraries.\n"
		"\n";

	if(!stream){
		stream = stdout;
	}
	fprintf(stream, format, VERSION, k2hdkc_commit_hash);

	k2h_print_version(stream);
	chmpx_print_version(stream);
}

/*
 * VIM modelines
 *
 * vim:set ts=4 fenc=utf-8:
 */
