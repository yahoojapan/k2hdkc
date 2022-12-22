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
#include <strings.h>
#include <errno.h>
#include <k2hash/k2hutil.h>

#include <string>

#include "k2hdkccommon.h"
#include "k2hdkcdbg.h"

using namespace	std;

//---------------------------------------------------------
// Global Variable
//---------------------------------------------------------
K2hdkcDbgMode	k2hdkc_debug_mode	= DKCDBG_SILENT;
FILE*			k2hdkc_dbg_fp		= NULL;

//---------------------------------------------------------
// Class K2hdkcDbgControl
//---------------------------------------------------------
class K2hdkcDbgControl
{
	protected:
		static const char*	DBGENVNAME;
		static const char*	DBGENVFILE;

		K2hdkcDbgMode*		pk2hdkc_debug_mode;
		FILE**				pk2hdkc_dbg_fp;
		string				k2hdkc_dbg_file;

	protected:
		K2hdkcDbgControl() : pk2hdkc_debug_mode(&k2hdkc_debug_mode), pk2hdkc_dbg_fp(&k2hdkc_dbg_fp), k2hdkc_dbg_file("")
		{
			*pk2hdkc_debug_mode	= DKCDBG_SILENT;
			*pk2hdkc_dbg_fp		= NULL;
			DbgCtrlLoadEnv();
		}
		virtual ~K2hdkcDbgControl()
		{
		}

		bool DbgCtrlLoadEnvName(void);
		bool DbgCtrlLoadEnvFile(void);

	public:
		static K2hdkcDbgControl& GetK2hdkcDbgCtrl(void)
		{
			static K2hdkcDbgControl	singleton;			// singleton
			return singleton;
		}

		bool DbgCtrlLoadEnv(void);
		K2hdkcDbgMode SetDbgCtrlMode(K2hdkcDbgMode mode);
		K2hdkcDbgMode BumpupDbgCtrlMode(void);
		K2hdkcDbgMode GetDbgCtrlMode(void);
		bool SetDbgCtrlFile(const char* filepath);
		bool UnsetDbgCtrlFile(void);
		const char* GetDbgCtrlFile(void);
};

//---------------------------------------------------------
// Class variables
//---------------------------------------------------------
const char*	K2hdkcDbgControl::DBGENVNAME = "DKCDBGMODE";
const char*	K2hdkcDbgControl::DBGENVFILE = "DKCDBGFILE";

//---------------------------------------------------------
// Methods
//---------------------------------------------------------
bool K2hdkcDbgControl::DbgCtrlLoadEnv(void)
{
	if(!DbgCtrlLoadEnvName() || !DbgCtrlLoadEnvFile()){
		return false;
	}
	return true;
}

bool K2hdkcDbgControl::DbgCtrlLoadEnvName(void)
{
	string	value;
	if(!k2h_getenv(K2hdkcDbgControl::DBGENVNAME, value)){
		MSG_DKCPRN("%s ENV is not set.", K2hdkcDbgControl::DBGENVNAME);
		return true;
	}
	if(0 == strcasecmp(value.c_str(), "SLT") || 0 == strcasecmp(value.c_str(), "SILENT")){
		SetDbgCtrlMode(DKCDBG_SILENT);
	}else if(0 == strcasecmp(value.c_str(), "ERR") || 0 == strcasecmp(value.c_str(), "ERROR")){
		SetDbgCtrlMode(DKCDBG_ERR);
	}else if(0 == strcasecmp(value.c_str(), "WAN") || 0 == strcasecmp(value.c_str(), "WARNING")){
		SetDbgCtrlMode(DKCDBG_WARN);
	}else if(0 == strcasecmp(value.c_str(), "MSG") || 0 == strcasecmp(value.c_str(), "INFO")){
		SetDbgCtrlMode(DKCDBG_MSG);
	}else if(0 == strcasecmp(value.c_str(), "DMP") || 0 == strcasecmp(value.c_str(), "DUMP")){
		SetDbgCtrlMode(DKCDBG_DUMP);
	}else{
		MSG_DKCPRN("%s ENV is not unknown string(%s).", K2hdkcDbgControl::DBGENVNAME, value.c_str());
		return false;
	}
	return true;
}

bool K2hdkcDbgControl::DbgCtrlLoadEnvFile(void)
{
	string	value;
	if(!k2h_getenv(K2hdkcDbgControl::DBGENVFILE, value)){
		MSG_DKCPRN("%s ENV is not set.", K2hdkcDbgControl::DBGENVFILE);
		return true;
	}
	if(!SetDbgCtrlFile(value.c_str())){
		MSG_DKCPRN("%s ENV is unsafe string(%s).", K2hdkcDbgControl::DBGENVFILE, value.c_str());
		return false;
	}
	return true;
}

K2hdkcDbgMode K2hdkcDbgControl::SetDbgCtrlMode(K2hdkcDbgMode mode)
{
	K2hdkcDbgMode oldmode	= *pk2hdkc_debug_mode;
	*pk2hdkc_debug_mode		= mode;
	return oldmode;
}

K2hdkcDbgMode K2hdkcDbgControl::BumpupDbgCtrlMode(void)
{
	K2hdkcDbgMode	mode = GetDbgCtrlMode();

	if(DKCDBG_SILENT == mode){
		mode = DKCDBG_ERR;
	}else if(DKCDBG_ERR == mode){
		mode = DKCDBG_WARN;
	}else if(DKCDBG_WARN == mode){
		mode = DKCDBG_MSG;
	}else if(DKCDBG_MSG == mode){
		mode = DKCDBG_DUMP;
	}else{	// DKCDBG_DUMP == mode
		mode = DKCDBG_SILENT;
	}
	return SetDbgCtrlMode(mode);
}

K2hdkcDbgMode K2hdkcDbgControl::GetDbgCtrlMode(void)
{
	return *pk2hdkc_debug_mode;
}

bool K2hdkcDbgControl::SetDbgCtrlFile(const char* filepath)
{
	if(DKCEMPTYSTR(filepath)){
		ERR_DKCPRN("Parameter is wrong.");
		return false;
	}
	if(!UnsetDbgCtrlFile()){
		return false;
	}
	FILE*	newfp;
	if(NULL == (newfp = fopen(filepath, "a+"))){
		ERR_DKCPRN("Could not open debug file(%s). errno = %d", filepath, errno);
		// cppcheck-suppress resourceLeak
		return false;
	}
	*pk2hdkc_dbg_fp	= newfp;
	k2hdkc_dbg_file	= filepath;
	return true;
}

bool K2hdkcDbgControl::UnsetDbgCtrlFile(void)
{
	k2hdkc_dbg_file.clear();

	if(*pk2hdkc_dbg_fp){
		if(0 != fclose(*pk2hdkc_dbg_fp)){
			ERR_DKCPRN("Could not close debug file. errno = %d", errno);
			*pk2hdkc_dbg_fp = NULL;		// On this case, k2hdkc_dbg_fp is not correct pointer after error...
			return false;
		}
		*pk2hdkc_dbg_fp = NULL;
	}
	return true;
}

const char* K2hdkcDbgControl::GetDbgCtrlFile(void)
{
	if(*pk2hdkc_dbg_fp && !k2hdkc_dbg_file.empty()){
		return k2hdkc_dbg_file.c_str();
	}
	return NULL;
}

//---------------------------------------------------------
// Global Function
//---------------------------------------------------------
K2hdkcDbgMode SetK2hdkcDbgMode(K2hdkcDbgMode mode)
{
	return K2hdkcDbgControl::GetK2hdkcDbgCtrl().SetDbgCtrlMode(mode);
}

K2hdkcDbgMode BumpupK2hdkcDbgMode(void)
{
	return K2hdkcDbgControl::GetK2hdkcDbgCtrl().BumpupDbgCtrlMode();
}

K2hdkcDbgMode GetK2hdkcDbgMode(void)
{
	return K2hdkcDbgControl::GetK2hdkcDbgCtrl().GetDbgCtrlMode();
}

bool LoadK2hdkcDbgEnv(void)
{
	return K2hdkcDbgControl::GetK2hdkcDbgCtrl().DbgCtrlLoadEnv();
}

bool SetK2hdkcDbgFile(const char* filepath)
{
	return K2hdkcDbgControl::GetK2hdkcDbgCtrl().SetDbgCtrlFile(filepath);
}

bool UnsetK2hdkcDbgFile(void)
{
	return K2hdkcDbgControl::GetK2hdkcDbgCtrl().UnsetDbgCtrlFile();
}

const char* GetK2hdkcDbgFile(void)
{
	return K2hdkcDbgControl::GetK2hdkcDbgCtrl().GetDbgCtrlFile();
}

void SetK2hdkcComLog(bool enable)
{
	if(enable){
		K2hdkcComNumber::Enable();
	}else{
		K2hdkcComNumber::Disable();
	}
}

void EnableK2hdkcComLog(void)
{
	SetK2hdkcComLog(true);
}

void DsableK2hdkcComLog(void)
{
	SetK2hdkcComLog(false);
}

bool IsK2hdkcComLog(void)
{
	return K2hdkcComNumber::IsEnable();
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noexpandtab sw=4 ts=4 fdm=marker
 * vim<600: noexpandtab sw=4 ts=4
 */
