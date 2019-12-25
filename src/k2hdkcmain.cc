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
 * CREATE:   Thu Mar 31 2016
 * REVISION:
 * 
 */

#include <stdlib.h>
#include <stdio.h>

#include <chmpx/chmconfutil.h>

#include <iostream>

#include "k2hdkccommon.h"
#include "k2hdkccntrl.h"
#include "k2hdkcconfparser.h"
#include "k2hdkcopts.h"
#include "k2hdkcutil.h"
#include "k2hdkc.h"
#include "k2hdkcdbg.h"

using namespace	std;

//---------------------------------------------------------
// Symbols
//---------------------------------------------------------
#define	OPT_HELP1				"H"
#define	OPT_HELP2				"HELP"
#define	OPT_VERSION1			"V"
#define	OPT_VERSION2			"VER"
#define	OPT_VERSION3			"VERSION"
#define	OPT_CONFPATH			"CONF"
#define	OPT_JSON				"JSON"
#define	OPT_DBG					"D"
#define	OPT_DBGFILEPATH			"DFILE"
#define	OPT_CTLPORT				"CTLPORT"
#define	OPT_CUK					"CUK"
#define	OPT_COMDBG				"COMLOG"
#define	OPT_NO_GIVEUP_REJOIN	"NO_GIVEUP_REJOIN"

#define	OPT_DBG_PARAM_SLT		"SLT"
#define	OPT_DBG_PARAM_SLIENT	"SILENT"
#define	OPT_DBG_PARAM_ERR		"ERR"
#define	OPT_DBG_PARAM_ERROR		"ERROR"
#define	OPT_DBG_PARAM_WAN		"WAN"
#define	OPT_DBG_PARAM_WARNING	"WARNING"
#define	OPT_DBG_PARAM_MSG		"MSG"
#define	OPT_DBG_PARAM_MESSAGE	"MESSAGE"
#define	OPT_DBG_PARAM_INFO		"INFO"
#define	OPT_DBG_PARAM_DUMP		"DUMP"

//---------------------------------------------------------
// Functions
//---------------------------------------------------------
//	[Usage]
//	k2hdkc -conf <configuration file path> [-ctlport <port>] [-comlog] [-no_giveup_rejoin] [-d [silent|err|wan|msg|dump]] [-dfile <debug file path>]
//
static bool PrintUsage(const char* prgname)
{
	cout << "[Usage]" << endl;
	cout << (prgname ? prgname : "prgname") << " [-conf <file path> | -json <json string>] [-ctlport <port>] [-cuk <cuk>] [-comlog] [-no_giveup_rejoin] [-d [silent|err|wan|msg|dump]] [-dfile <file path>]"	<< endl;
	cout << (prgname ? prgname : "prgname") << " [ -h | -v ]"										<< endl;
	cout << "[option]"																				<< endl;
	cout << "  -conf <path>         specify the configuration file(.ini .yaml .json) path"			<< endl;
	cout << "  -json <string>       specify the configuration json string"							<< endl;
	cout << "  -ctlport <port>      specify the self control port(*)"								<< endl;
	cout << "  -cuk <cuk string>    specify the self CUK(*)"										<< endl;
	cout << "  -no_giveup_rejoin    not give up rejoining chmpx"									<< endl;
	cout << "  -comlog              enable logging communication command"							<< endl;
	cout << "  -d <param>           specify the debugging output mode:"								<< endl;
	cout << "                        silent - no output"											<< endl;
	cout << "                        err    - output error level"									<< endl;
	cout << "                        wan    - output warning level"									<< endl;
	cout << "                        msg    - output debug(message) level"							<< endl;
	cout << "                        dump   - output communication debug level"						<< endl;
	cout << "  -dfile <path>        specify the file path which is put output"						<< endl;
	cout << "  -h(help)             display this usage."											<< endl;
	cout << "  -v(version)          display version."												<< endl;
	cout << endl;
	cout << "[environment]"																			<< endl;
	cout << "  K2HDKCCONFFILE       specify the configuration file(.ini .yaml .json) path"			<< endl;
	cout << "  K2HDKCJSONCONF       specify the configuration json string"							<< endl;
	cout << endl;
	cout << "(*) you can use environment DKCDBGMODE and DKCDBGFILE instead of -d/-dfile options."	<< endl;
	cout << "(*) if ctlport and cuk option is specified, chmpx searches same ctlport/cuk"			<< endl;
	cout << "    in configuration file and ignores \"CTLPORT\" or \"CUK\" directive in"				<< endl;
	cout << "    \"GLOBAL\" section. and chmpx will start in the mode indicated by the"				<< endl;
	cout << "    server entry that has been detected."												<< endl;

	return true;
}

//---------------------------------------------------------
// Main
//---------------------------------------------------------
int main(int argc, char** argv)
{
	if(0 == argc || !argv){
		exit(EXIT_FAILURE);
	}
	string	prgname = basename(argv[0]);
	if(0 == strncmp(prgname.c_str(), "lt-", strlen("lt-"))){
		prgname = prgname.substr(3);
	}

	// load option
	K2hdkcOpts	opts((argc - 1), &argv[1]);

	// help
	if(opts.Find(OPT_HELP1) || opts.Find(OPT_HELP2)){
		PrintUsage(prgname.c_str());
		exit(EXIT_SUCCESS);
	}

	// version
	if(opts.Find(OPT_VERSION1) || opts.Find(OPT_VERSION2) || opts.Find(OPT_VERSION3)){
		k2hdkc_print_version(stdout);
		exit(EXIT_SUCCESS);
	}

	// parameter - debug
	bool			is_dbgopt	= false;
	K2hdkcDbgMode	dbgmode		= DKCDBG_SILENT;
	string			dbgfile		= "";
	{
		string	strDbgMode;
		if(opts.Get(OPT_DBG, strDbgMode)){
			if(strDbgMode.empty()){
				cout << "ERROR: \"-d\" option must be specified with parameter." << endl;
				PrintUsage(prgname.c_str());
				exit(EXIT_FAILURE);
			}
			strDbgMode = upper(strDbgMode);

			if(OPT_DBG_PARAM_SLT == strDbgMode || OPT_DBG_PARAM_SLIENT == strDbgMode){
				dbgmode = DKCDBG_SILENT;
			}else if(OPT_DBG_PARAM_ERR == strDbgMode || OPT_DBG_PARAM_ERROR == strDbgMode){
				dbgmode = DKCDBG_ERR;
			}else if(OPT_DBG_PARAM_WAN == strDbgMode || OPT_DBG_PARAM_WARNING == strDbgMode){
				dbgmode = DKCDBG_WARN;
			}else if(OPT_DBG_PARAM_MSG == strDbgMode || OPT_DBG_PARAM_MESSAGE == strDbgMode || OPT_DBG_PARAM_INFO == strDbgMode){
				dbgmode = DKCDBG_MSG;
			}else if(OPT_DBG_PARAM_DUMP == strDbgMode){
				dbgmode = DKCDBG_DUMP;
			}else{
				cout << "ERROR: Unknown \"-d\" option parameter(" << strDbgMode << ") is specified." << endl;
				PrintUsage(prgname.c_str());
				exit(EXIT_FAILURE);
			}
			is_dbgopt = true;
		}
		opts.Get(OPT_DBGFILEPATH, dbgfile);
	}

	// Set debug mode
	if(!LoadK2hdkcDbgEnv()){
		cout << "WARNING: Something error occured while loading debugging mode/file from environment." << endl;
	}
	if(!dbgfile.empty()){
		if(!SetK2hdkcDbgFile(dbgfile.c_str())){
			cout << "WARNING: Something error occured while dispatching debugging file(" << dbgfile << ")." << endl;
		}
	}
	if(is_dbgopt){
		SetK2hdkcDbgMode(dbgmode);
	}

	// parameter - configuration file path
	string	config;
	if(!opts.Get(OPT_CONFPATH, config)){
		if(!opts.Get(OPT_JSON, config)){
			// over check for environment
			CHMCONFTYPE	conftype = check_chmconf_type_ex(NULL, K2HDKC_CONFFILE_ENV_NAME, K2HDKC_JSONCONF_ENV_NAME, NULL);
			if(CHMCONF_TYPE_UNKNOWN == conftype || CHMCONF_TYPE_NULL == conftype){
				cout << "ERROR: not specify option \"-conf\" or \"-json\", thus checking environments." << endl;
				PrintUsage(prgname.c_str());
				exit(EXIT_FAILURE);
			}
		}
	}

	// parameter - control port
	short	ctlport = CHM_INVALID_PORT;
	string	strctlport;
	if(opts.Get(OPT_CTLPORT, strctlport)){
		ctlport = static_cast<short>(atoi(strctlport.c_str()));
		if(CHM_INVALID_PORT == ctlport || 0 == ctlport){
			cout << "ERROR: option \"-ctlport\" is specified with invalid value." << endl;
			exit(EXIT_FAILURE);
		}
	}

	// parameter - cuk
	string	strcuk;
	if(!opts.Get(OPT_CUK, strcuk)){
		strcuk.clear();
	}

	// parameter - comlog
	if(opts.Find(OPT_COMDBG)){
		K2hdkcComNumber::Enable();
	}

	// parameter - no giveup rejoin
	bool	no_giveup_rejoin = false;
	if(opts.Find(OPT_NO_GIVEUP_REJOIN)){
		no_giveup_rejoin = true;
	}

	// Load configuration file
	K2hdkcConfig	k2hdkcconf;
	if(!k2hdkcconf.Load(config.c_str())){
		cout << "ERROR: Failed to load configuration : \"" << config << "\"" << endl;
		exit(EXIT_FAILURE);
	}

	// initialize K2hdkcCntrl
	K2hdkcCntrl*	pCntrl = K2hdkcCntrl::Get();
	if(!pCntrl->Initialize(&k2hdkcconf, ctlport, (strcuk.empty() ? NULL : strcuk.c_str()), no_giveup_rejoin)){
		cout << "ERROR: Failed to initialize control main object." << endl;
		exit(EXIT_FAILURE);
	}

	// run
	bool	result = pCntrl->Run();
	if(!result){
		cout << "ERROR: Processing returns error." << endl;
	}

	exit(result ? EXIT_SUCCESS : EXIT_FAILURE);
}

/*
 * VIM modelines
 *
 * vim:set ts=4 fenc=utf-8:
 */
