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

#ifndef	K2HDKCDBG_H
#define K2HDKCDBG_H

#include "k2hdkccommon.h"
#include "k2hdkccomnum.h"

DECL_EXTERN_C_START

//---------------------------------------------------------
// Debug
//---------------------------------------------------------
typedef enum k2hdkc_dbg_mode{
	DKCDBG_SILENT	= 0,
	DKCDBG_ERR		= 1,
	DKCDBG_WARN		= 3,
	DKCDBG_MSG		= 7,
	DKCDBG_DUMP		= 15
}K2hdkcDbgMode;

extern K2hdkcDbgMode	k2hdkc_debug_mode;		// Do not use directly this variable.
extern FILE*			k2hdkc_dbg_fp;

K2hdkcDbgMode SetK2hdkcDbgMode(K2hdkcDbgMode mode);
K2hdkcDbgMode BumpupK2hdkcDbgMode(void);
K2hdkcDbgMode GetK2hdkcDbgMode(void);
bool LoadK2hdkcDbgEnv(void);
bool SetK2hdkcDbgFile(const char* filepath);
bool UnsetK2hdkcDbgFile(void);
const char* GetK2hdkcDbgFile(void);
void SetK2hdkcComLog(bool enable);
void EnableK2hdkcComLog(void);
void DsableK2hdkcComLog(void);
bool IsK2hdkcComLog(void);

//---------------------------------------------------------
// Debugging Macros
//---------------------------------------------------------
#define	K2hdkcDbgMode_STR(mode)		(	DKCDBG_SILENT	== mode ? "SLT" : \
										DKCDBG_ERR		== mode ? "ERR" : \
										DKCDBG_WARN		== mode ? "WAN" : \
										DKCDBG_MSG		== mode ? "MSG" : \
										DKCDBG_DUMP		== mode ? "DMP" : "")

#define	K2hdkcDbgMode_FULLSTR(mode)	(	DKCDBG_SILENT	== mode ? "silent"	: \
										DKCDBG_ERR		== mode ? "error"	: \
										DKCDBG_WARN		== mode ? "warning"	: \
										DKCDBG_MSG		== mode ? "message"	: \
										DKCDBG_DUMP		== mode ? "dump"	: "")

#define	LOW_K2HDKCPRINT(mode, fmt, ...) \
		fprintf((k2hdkc_dbg_fp ? k2hdkc_dbg_fp : stderr), "[DKC-%s] %s(%d) : " fmt "%s\n", K2hdkcDbgMode_STR(mode), __func__, __LINE__, __VA_ARGS__);

#define	K2HDKCPRINT(mode, ...) \
		if((k2hdkc_debug_mode & mode) == mode){ \
			LOW_K2HDKCPRINT(mode, __VA_ARGS__); \
		}

#define	RAW_K2HDKCPRINT(mode, fmt, ...) \
		if((k2hdkc_debug_mode & mode) == mode){ \
			fprintf((k2hdkc_dbg_fp ? k2hdkc_dbg_fp : stderr), fmt "%s\n", __VA_ARGS__); \
		}

#define	IS_DKCDBG_DUMP() 		((k2hdkc_debug_mode & DKCDBG_DUMP) == DKCDBG_DUMP)

#define	SLT_DKCPRN(...)			K2HDKCPRINT(DKCDBG_SILENT,	##__VA_ARGS__, "")	// This means nothing...
#define	ERR_DKCPRN(...)			K2HDKCPRINT(DKCDBG_ERR,		##__VA_ARGS__, "")
#define	WAN_DKCPRN(...)			K2HDKCPRINT(DKCDBG_WARN,	##__VA_ARGS__, "")
#define	MSG_DKCPRN(...)			K2HDKCPRINT(DKCDBG_MSG,		##__VA_ARGS__, "")
#define	DMP_DKCPRN(...)			K2HDKCPRINT(DKCDBG_DUMP,	##__VA_ARGS__, "")
#define	RAW_DKCPRN(...)			RAW_K2HDKCPRINT(DKCDBG_DUMP,##__VA_ARGS__, "")

#define	LOW_K2HDKCCOMLOG_PRINT(dispnum, comnum, fmt, ...) \
		fprintf((k2hdkc_dbg_fp ? k2hdkc_dbg_fp : stderr), "[DKC-COM(%016" PRIx64 " - %016" PRIx64 ")] " fmt "%s\n", dispnum, comnum, __VA_ARGS__);

#define	COMLOG_DMP_PRN(dispnum, comnum, ...) \
		if(DKCDBG_DUMP == (k2hdkc_debug_mode & DKCDBG_DUMP)){\
			LOW_K2HDKCCOMLOG_PRINT(dispnum, comnum, ##__VA_ARGS__, ""); \
		}

#define	COMLOG_PRN(dispnum, comnum, ...) \
		if(K2hdkcComNumber::IsEnable()){\
			LOW_K2HDKCCOMLOG_PRINT(dispnum, comnum, ##__VA_ARGS__, ""); \
		}

DECL_EXTERN_C_END

#endif	// K2HDKCDBG_H

/*
 * VIM modelines
 *
 * vim:set ts=4 fenc=utf-8:
 */
