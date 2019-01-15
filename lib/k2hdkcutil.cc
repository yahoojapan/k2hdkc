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
#include <limits.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#include "k2hdkcutil.h"
#include "k2hdkccommon.h"
#include "k2hdkcdbg.h"

using namespace std;

//---------------------------------------------------------
// Utilities
//---------------------------------------------------------
bool path_to_abspath(const char* path, string& abspath)
{
	if(DKCEMPTYSTR(path)){
		ERR_DKCPRN("path is empty.");
		return false;
	}
	char*	rpath;
	if(NULL == (rpath = realpath(path, NULL))){
		MSG_DKCPRN("Could not convert path(%s) to real path or there is no such file(errno:%d).", path, errno);
		return false;
	}
	abspath = rpath;
	DKC_FREE(rpath);
	return true;
}

string bin_to_string(const unsigned char* pdata, size_t length, size_t maxlength)
{
	string	strresult("");

	if(pdata && 0 < length && 0 < maxlength){
		size_t	dlength	= min(length, maxlength);
		char*	pbuff	= new char[maxlength + 1];

		memcpy(pbuff, pdata, dlength);
		for(size_t pos = 0; pos < dlength; ++pos){
			if(0 == isprint(pbuff[pos])){
				pbuff[pos] = 0xFF;
			}
		}
		pbuff[dlength]	= '\0';
		strresult		= pbuff;
		delete[] pbuff;

		if(maxlength < length){
			strresult += "...";
		}
	}
	return strresult;
}

void make_current_timespec_with_mergin(long timens, struct timespec& limitts)
{
	if(-1 == clock_gettime(CLOCK_REALTIME_COARSE, &limitts)){
		WAN_DKCPRN("Could not get REALTIME for timespec by errno(%d), so using time function.", errno);
		limitts.tv_sec	= time(NULL) + (timens / (1000 * 1000 * 1000)) + (0 < (timens % (1000 * 1000 * 1000)) ? 1 : 0);
		limitts.tv_nsec	= 0;
	}else{
		limitts.tv_sec	+= timens / (1000 * 1000 * 1000);
		limitts.tv_nsec	+= timens % (1000 * 1000 * 1000);
	}
	if(0 < ((limitts.tv_nsec) / (1000 * 1000 * 1000))){
		++(limitts.tv_sec);
		limitts.tv_nsec = limitts.tv_nsec % (1000 * 1000 * 1000);
	}
}

/*
 * VIM modelines
 *
 * vim:set ts=4 fenc=utf-8:
 */
