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
// Class K2hdkcDbgControl
//---------------------------------------------------------
class K2hdkcDbgControl
{
	protected:
		static const char*		DBGENVNAME;
		static const char*		DBGENVFILE;
		static K2hdkcDbgControl	singleton;

	public:
		static bool LoadEnv(void);
		static bool LoadEnvName(void);
		static bool LoadEnvFile(void);

		K2hdkcDbgControl();
		virtual ~K2hdkcDbgControl();
};

//---------------------------------------------------------
// Class valiables
//---------------------------------------------------------
const char*			K2hdkcDbgControl::DBGENVNAME = "DKCDBGMODE";
const char*			K2hdkcDbgControl::DBGENVFILE = "DKCDBGFILE";
K2hdkcDbgControl	K2hdkcDbgControl::singleton;

//---------------------------------------------------------
// Class Methods
//---------------------------------------------------------
bool K2hdkcDbgControl::LoadEnv(void)
{
	if(!K2hdkcDbgControl::LoadEnvName() || !K2hdkcDbgControl::LoadEnvFile()){
		return false;
	}
	return true;
}

bool K2hdkcDbgControl::LoadEnvName(void)
{
	string	value;
	if(!k2h_getenv(K2hdkcDbgControl::DBGENVNAME, value)){
		MSG_DKCPRN("%s ENV is not set.", K2hdkcDbgControl::DBGENVNAME);
		return true;
	}
	if(0 == strcasecmp(value.c_str(), "SLT") || 0 == strcasecmp(value.c_str(), "SILENT")){
		SetK2hdkcDbgMode(DKCDBG_SILENT);
	}else if(0 == strcasecmp(value.c_str(), "ERR") || 0 == strcasecmp(value.c_str(), "ERROR")){
		SetK2hdkcDbgMode(DKCDBG_ERR);
	}else if(0 == strcasecmp(value.c_str(), "WAN") || 0 == strcasecmp(value.c_str(), "WARNING")){
		SetK2hdkcDbgMode(DKCDBG_WARN);
	}else if(0 == strcasecmp(value.c_str(), "MSG") || 0 == strcasecmp(value.c_str(), "INFO")){
		SetK2hdkcDbgMode(DKCDBG_MSG);
	}else if(0 == strcasecmp(value.c_str(), "DMP") || 0 == strcasecmp(value.c_str(), "DUMP")){
		SetK2hdkcDbgMode(DKCDBG_DUMP);
	}else{
		MSG_DKCPRN("%s ENV is not unknown string(%s).", K2hdkcDbgControl::DBGENVNAME, value.c_str());
		return false;
	}
	return true;
}

bool K2hdkcDbgControl::LoadEnvFile(void)
{
	string	value;
	if(!k2h_getenv(K2hdkcDbgControl::DBGENVFILE, value)){
		MSG_DKCPRN("%s ENV is not set.", K2hdkcDbgControl::DBGENVFILE);
		return true;
	}
	if(!SetK2hdkcDbgFile(value.c_str())){
		MSG_DKCPRN("%s ENV is unsafe string(%s).", K2hdkcDbgControl::DBGENVFILE, value.c_str());
		return false;
	}
	return true;
}

//---------------------------------------------------------
// Constructor / Destructor
//---------------------------------------------------------
K2hdkcDbgControl::K2hdkcDbgControl()
{
	K2hdkcDbgControl::LoadEnv();
}
K2hdkcDbgControl::~K2hdkcDbgControl()
{
}

//---------------------------------------------------------
// Global Variable
//---------------------------------------------------------
K2hdkcDbgMode	k2hdkc_debug_mode	= DKCDBG_SILENT;
FILE*			k2hdkc_dbg_fp		= NULL;

//---------------------------------------------------------
// Static Variable
//---------------------------------------------------------
static string	k2hdkc_dbg_file("");

//---------------------------------------------------------
// Global Function
//---------------------------------------------------------
K2hdkcDbgMode SetK2hdkcDbgMode(K2hdkcDbgMode mode)
{
	K2hdkcDbgMode oldmode	= k2hdkc_debug_mode;
	k2hdkc_debug_mode		= mode;
	return oldmode;
}

K2hdkcDbgMode BumpupK2hdkcDbgMode(void)
{
	K2hdkcDbgMode	mode = GetK2hdkcDbgMode();

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
	return ::SetK2hdkcDbgMode(mode);
}

K2hdkcDbgMode GetK2hdkcDbgMode(void)
{
	return k2hdkc_debug_mode;
}

bool LoadK2hdkcDbgEnv(void)
{
	return K2hdkcDbgControl::LoadEnv();
}

bool SetK2hdkcDbgFile(const char* filepath)
{
	if(DKCEMPTYSTR(filepath)){
		ERR_DKCPRN("Parameter is wrong.");
		return false;
	}
	if(!UnsetK2hdkcDbgFile()){
		return false;
	}
	FILE*	newfp;
	if(NULL == (newfp = fopen(filepath, "a+"))){
		ERR_DKCPRN("Could not open debug file(%s). errno = %d", filepath, errno);
		return false;
	}
	k2hdkc_dbg_fp	= newfp;
	k2hdkc_dbg_file	= filepath;
	return true;
}

bool UnsetK2hdkcDbgFile(void)
{
	if(k2hdkc_dbg_fp){
		if(0 != fclose(k2hdkc_dbg_fp)){
			ERR_DKCPRN("Could not close debug file. errno = %d", errno);
			k2hdkc_dbg_fp = NULL;		// On this case, k2hdkc_dbg_fp is not correct pointer after error...
			return false;
		}
		k2hdkc_dbg_fp = NULL;
	}
	k2hdkc_dbg_file.clear();
	return true;
}

const char* GetK2hdkcDbgFile(void)
{
	if(k2hdkc_dbg_fp && !k2hdkc_dbg_file.empty()){
		return k2hdkc_dbg_file.c_str();
	}
	return NULL;
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
 * VIM modelines
 *
 * vim:set ts=4 fenc=utf-8:
 */
