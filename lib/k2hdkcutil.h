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

#ifndef	K2HDKCUTIL_H
#define	K2HDKCUTIL_H

#include <time.h>
#include <chmpx/chmutil.h>
#include <string>

#include "k2hdkccommon.h"

//---------------------------------------------------------
// Utility Macros
//---------------------------------------------------------
#define	DKC_FREE(ptr)	\
		{ \
			if(ptr){ \
				free(ptr); \
				ptr = NULL; \
			} \
		}

#define	DKC_DELETE(ptr)	\
		{ \
			if(ptr){ \
				delete ptr; \
				ptr = NULL; \
			} \
		}

#define	DKC_CLOSE(fd)	\
		{ \
			if(DKC_INVALID_HANDLE != fd){ \
				close(fd); \
				fd = DKC_INVALID_HANDLE; \
			} \
		}

//---------------------------------------------------------
// Utility Functions
//---------------------------------------------------------
DECL_EXTERN_C_START

bool path_to_abspath(const char* path, std::string& abspath);
void make_current_timespec_with_mergin(long timens, struct timespec& limitts);

DECL_EXTERN_C_END

std::string bin_to_string(const unsigned char* pdata, size_t length, size_t maxlength = 256);

//---------------------------------------------------------
// Utility Functions - timespec
//---------------------------------------------------------
inline std::string STR_DKC_TIMESPEC(const struct timespec* val)
{
	std::string	result("");
	if(val){
		result += to_string((val)->tv_sec);
		result += "s ";
		result += to_string((val)->tv_nsec / (1000 * 1000));
		result += "ms ";
		result += to_string(((val)->tv_nsec % (1000 * 1000)) / 1000);
		result += "us ";
		result += to_string((val)->tv_nsec % 1000);
		result += "ns";
	}
	return result;
}

inline std::string STR_DKC_TIMESPEC_NS(const struct timespec* val)
{
	std::string	result("");
	if(val){
		result += to_string(((val)->tv_nsec + ((val)->tv_sec * 1000 * 1000 * 1000)));
		result += "ns ";
	}
	return result;
}

inline void INIT_DKC_TIMESPEC(struct timespec& ts)
{
	if(-1 == clock_gettime(CLOCK_REALTIME_COARSE, &ts)){
		ts.tv_sec	= time(NULL);
		ts.tv_nsec	= 0;
	}
}

#endif	// K2HDKCUTIL_H

/*
 * VIM modelines
 *
 * vim:set ts=4 fenc=utf-8:
 */
