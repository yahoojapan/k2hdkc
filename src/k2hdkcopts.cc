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

#include "k2hdkccommon.h"
#include "k2hdkcopts.h"
#include "k2hdkcdbg.h"

using namespace	std;

//---------------------------------------------------------
// Symbols
//---------------------------------------------------------
#define	OPT_PARAM_SEP			"\r\n"

//---------------------------------------------------------
// Constructor/Destructor
//---------------------------------------------------------
K2hdkcOpts::K2hdkcOpts(int argc, char** argv)
{
	if(argv && 0 < argc){
		Initialize(argc, argv);
	}
}

K2hdkcOpts::~K2hdkcOpts(void)
{
}

//---------------------------------------------------------
// Methods
//---------------------------------------------------------
bool K2hdkcOpts::Initialize(int argc, char** argv)
{
	if(!argv || 0 == argc){
		ERR_DKCPRN("Parameters are wrong.");
		return false;
	}
	optmap.clear();

	for(int cnt = 0; cnt < argc; cnt++){
		string		option;
		strlst_t	paramlist;

		if('-' != argv[cnt][0]){
			option = "";
		}else{
			option = &(argv[cnt][1]);
		}
		option = upper(option);

		paramlist.clear();
		int	pcnt;
		for(pcnt = 0; (cnt + pcnt + 1) < argc; pcnt++){
			if('-' == argv[cnt + pcnt + 1][0]){
				break;
			}
			str_paeser(argv[cnt + pcnt + 1], paramlist, OPT_PARAM_SEP);
		}
		optmap[option] = paramlist;
		cnt += pcnt;
	}
	return true;
}

bool K2hdkcOpts::Get(const char* popt, string& param)
{
	string	stropt;
	if(DKCEMPTYSTR(popt)){
		stropt = "";
	}else{
		stropt = upper(string(popt));
	}
	if(optmap.end() == optmap.find(stropt)){
		return false;
	}
	if(optmap[stropt].empty()){
		param = "";
	}else{
		param = optmap[stropt].front();
	}
	return true;
}

bool K2hdkcOpts::Get(const char* popt, strlst_t& params)
{
	string	stropt;
	if(DKCEMPTYSTR(popt)){
		stropt = "";
	}else{
		stropt = upper(string(popt));
	}
	if(optmap.end() == optmap.find(stropt)){
		return false;
	}
	params = optmap[stropt];
	return true;
}

bool K2hdkcOpts::Find(const char* popt) const
{
	if(DKCEMPTYSTR(popt)){
		return false;
	}
	string	stropt = upper(string(popt));
	if(optmap.end() == optmap.find(stropt)){
		return false;
	}
	return true;
}

/*
 * VIM modelines
 *
 * vim:set ts=4 fenc=utf-8:
 */
