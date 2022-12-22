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
 * CREATE:   Mon Aug 15 2016
 * REVISION:
 * 
 */

#ifndef	K2HDKCCOMUTIL_H
#define	K2HDKCCOMUTIL_H

#include "k2hdkccombase.h"
#include "k2hdkccomaddsubkey.h"
#include "k2hdkccomaddsubkeys.h"
#include "k2hdkccomcasget.h"
#include "k2hdkccomcasincdec.h"
#include "k2hdkccomcasinit.h"
#include "k2hdkccomcasset.h"
#include "k2hdkccomdel.h"
#include "k2hdkccomdelsubkey.h"
#include "k2hdkccomdelsubkeys.h"
#include "k2hdkccomgetattr.h"
#include "k2hdkccomgetattrs.h"
#include "k2hdkccomgetdirect.h"
#include "k2hdkccomget.h"
#include "k2hdkccomgetsubkeys.h"
#include "k2hdkccommon.h"
#include "k2hdkccomnum.h"
#include "k2hdkccomqdel.h"
#include "k2hdkccomqpop.h"
#include "k2hdkccomqpush.h"
#include "k2hdkccomren.h"
#include "k2hdkccomreplkey.h"
#include "k2hdkccomsetall.h"
#include "k2hdkccomsetdirect.h"
#include "k2hdkccomset.h"
#include "k2hdkccomsetsubkeys.h"
#include "k2hdkccomk2hstate.h"
#include "k2hdkccomstate.h"

//---------------------------------------------------------
// Utility Macros for Class Factory
//---------------------------------------------------------
//
// Class factory Interface macro for common command class
//
// GetCommonK2hdkcComXXXXX(...)
//
//		K2HShm*		pk2hash
//		ChmCntrl*	pchmcntrl
//		msgid_t		msgid					(default = CHM_INVALID_MSGID)
//		uint64_t	comnum					(default = K2hdkcComNumber::INIT_NUMBER)
//		bool		without_self			(default = true)
//		bool		is_routing_on_server	(default = true)
//		bool		is_wait_on_server		(default = false)
//
#define GetCommonK2hdkcComGet(pk2hash, pchmcntrl, ...)			reinterpret_cast<K2hdkcComGet*>(		K2hdkcCommand::GetCommandSendObject(pk2hash, pchmcntrl, DKC_COM_GET,		__VA_ARGS__))
#define GetCommonK2hdkcComGetDirect(pk2hash, pchmcntrl, ...)	reinterpret_cast<K2hdkcComGetDirect*>(	K2hdkcCommand::GetCommandSendObject(pk2hash, pchmcntrl, DKC_COM_GET_DIRECT,	__VA_ARGS__))
#define GetCommonK2hdkcComGetSubkeys(pk2hash, pchmcntrl, ...)	reinterpret_cast<K2hdkcComGetSubkeys*>(	K2hdkcCommand::GetCommandSendObject(pk2hash, pchmcntrl, DKC_COM_GET_SUBKEYS,__VA_ARGS__))
#define GetCommonK2hdkcComGetAttrs(pk2hash, pchmcntrl, ...)		reinterpret_cast<K2hdkcComGetAttrs*>(	K2hdkcCommand::GetCommandSendObject(pk2hash, pchmcntrl, DKC_COM_GET_ATTRS,	__VA_ARGS__))
#define GetCommonK2hdkcComGetAttr(pk2hash, pchmcntrl, ...)		reinterpret_cast<K2hdkcComGetAttr*>(	K2hdkcCommand::GetCommandSendObject(pk2hash, pchmcntrl, DKC_COM_GET_ATTR,	__VA_ARGS__))
#define GetCommonK2hdkcComSet(pk2hash, pchmcntrl, ...)			reinterpret_cast<K2hdkcComSet*>(		K2hdkcCommand::GetCommandSendObject(pk2hash, pchmcntrl, DKC_COM_SET,		__VA_ARGS__))
#define GetCommonK2hdkcComSetDirect(pk2hash, pchmcntrl, ...)	reinterpret_cast<K2hdkcComSetDirect*>(	K2hdkcCommand::GetCommandSendObject(pk2hash, pchmcntrl, DKC_COM_SET_DIRECT,	__VA_ARGS__))
#define GetCommonK2hdkcComSetSubkeys(pk2hash, pchmcntrl, ...)	reinterpret_cast<K2hdkcComSetSubkeys*>(	K2hdkcCommand::GetCommandSendObject(pk2hash, pchmcntrl, DKC_COM_SET_SUBKEYS,__VA_ARGS__))
#define GetCommonK2hdkcComSetAll(pk2hash, pchmcntrl, ...)		reinterpret_cast<K2hdkcComSetAll*>(		K2hdkcCommand::GetCommandSendObject(pk2hash, pchmcntrl, DKC_COM_SET_ALL,	__VA_ARGS__))
#define GetCommonK2hdkcComAddSubkeys(pk2hash, pchmcntrl, ...)	reinterpret_cast<K2hdkcComAddSubkeys*>(	K2hdkcCommand::GetCommandSendObject(pk2hash, pchmcntrl, DKC_COM_ADD_SUBKEYS,__VA_ARGS__))
#define GetCommonK2hdkcComAddSubkey(pk2hash, pchmcntrl, ...)	reinterpret_cast<K2hdkcComAddSubkey*>(	K2hdkcCommand::GetCommandSendObject(pk2hash, pchmcntrl, DKC_COM_ADD_SUBKEY,	__VA_ARGS__))
#define GetCommonK2hdkcComDel(pk2hash, pchmcntrl, ...)			reinterpret_cast<K2hdkcComDel*>(		K2hdkcCommand::GetCommandSendObject(pk2hash, pchmcntrl, DKC_COM_DEL,		__VA_ARGS__))
#define GetCommonK2hdkcComDelSubkeys(pk2hash, pchmcntrl, ...)	reinterpret_cast<K2hdkcComDelSubkeys*>(	K2hdkcCommand::GetCommandSendObject(pk2hash, pchmcntrl, DKC_COM_DEL_SUBKEYS,__VA_ARGS__))
#define GetCommonK2hdkcComDelSubkey(pk2hash, pchmcntrl, ...)	reinterpret_cast<K2hdkcComDelSubkey*>(	K2hdkcCommand::GetCommandSendObject(pk2hash, pchmcntrl, DKC_COM_DEL_SUBKEY,	__VA_ARGS__))
#define GetCommonK2hdkcComRen(pk2hash, pchmcntrl, ...)			reinterpret_cast<K2hdkcComRen*>(		K2hdkcCommand::GetCommandSendObject(pk2hash, pchmcntrl, DKC_COM_REN,		__VA_ARGS__))
#define GetCommonK2hdkcComQPush(pk2hash, pchmcntrl, ...)		reinterpret_cast<K2hdkcComQPush*>(		K2hdkcCommand::GetCommandSendObject(pk2hash, pchmcntrl, DKC_COM_QPUSH,		__VA_ARGS__))
#define GetCommonK2hdkcComQPop(pk2hash, pchmcntrl, ...)			reinterpret_cast<K2hdkcComQPop*>(		K2hdkcCommand::GetCommandSendObject(pk2hash, pchmcntrl, DKC_COM_QPOP,		__VA_ARGS__))
#define GetCommonK2hdkcComQDel(pk2hash, pchmcntrl, ...)			reinterpret_cast<K2hdkcComQDel*>(		K2hdkcCommand::GetCommandSendObject(pk2hash, pchmcntrl, DKC_COM_QDEL,		__VA_ARGS__))
#define GetCommonK2hdkcComCasInit(pk2hash, pchmcntrl, ...)		reinterpret_cast<K2hdkcComCasInit*>(	K2hdkcCommand::GetCommandSendObject(pk2hash, pchmcntrl, DKC_COM_CAS_INIT,	__VA_ARGS__))
#define GetCommonK2hdkcComCasGet(pk2hash, pchmcntrl, ...)		reinterpret_cast<K2hdkcComCasGet*>(		K2hdkcCommand::GetCommandSendObject(pk2hash, pchmcntrl, DKC_COM_CAS_GET,	__VA_ARGS__))
#define GetCommonK2hdkcComCasSet(pk2hash, pchmcntrl, ...)		reinterpret_cast<K2hdkcComCasSet*>(		K2hdkcCommand::GetCommandSendObject(pk2hash, pchmcntrl, DKC_COM_CAS_SET,	__VA_ARGS__))
#define GetCommonK2hdkcComCasIncDec(pk2hash, pchmcntrl, ...)	reinterpret_cast<K2hdkcComCasIncDec*>(	K2hdkcCommand::GetCommandSendObject(pk2hash, pchmcntrl, DKC_COM_CAS_INCDEC,	__VA_ARGS__))
#define GetCommonK2hdkcComReplKey(pk2hash, pchmcntrl, ...)		reinterpret_cast<K2hdkcComReplKey*>(	K2hdkcCommand::GetCommandSendObject(pk2hash, pchmcntrl, DKC_COM_REPL_KEY,	__VA_ARGS__))
#define GetCommonK2hdkcComK2hState(pk2hash, pchmcntrl, ...)		reinterpret_cast<K2hdkcComK2hState*>(	K2hdkcCommand::GetCommandSendObject(pk2hash, pchmcntrl, DKC_COM_K2HSTATE,	__VA_ARGS__))
#define GetCommonK2hdkcComState(pk2hash, pchmcntrl, ...)		reinterpret_cast<K2hdkcComState*>(		K2hdkcCommand::GetCommandSendObject(pk2hash, pchmcntrl, DKC_COM_STATE,		__VA_ARGS__))
#define GetCommonK2hdkcComReplDel(pk2hash, pchmcntrl, ...)		reinterpret_cast<K2hdkcComReplDel*>(	K2hdkcCommand::GetCommandSendObject(pk2hash, pchmcntrl, DKC_COM_REPL_DEL,	__VA_ARGS__))

//---------------------------------------------------------
// Utility Macros for Class Factory
//---------------------------------------------------------
//
// Class factory Interface macro for onetime slave command class
//
// GetOtSlaveK2hdkcComXXXXX(...)
//
//		const char*		config
//		short			ctlport				(default = CHM_INVALID_PORT)
//		const char*		cuk					(default = NULL)
//		bool			is_auto_rejoin		(default = false)
//		bool			no_giveup_rejoin	(default = false)
//		uint64_t		comnum				(default = K2hdkcComNumber::INIT_NUMBER)
//
#define	GetOtSlaveK2hdkcComGet(...)								reinterpret_cast<K2hdkcComGet*>(		K2hdkcCommand::GetSlaveCommandSendObject(DKC_COM_GET,			__VA_ARGS__))
#define	GetOtSlaveK2hdkcComGetDirect(...)						reinterpret_cast<K2hdkcComGetDirect*>(	K2hdkcCommand::GetSlaveCommandSendObject(DKC_COM_GET_DIRECT,	__VA_ARGS__))
#define	GetOtSlaveK2hdkcComGetSubkeys(...)						reinterpret_cast<K2hdkcComGetSubkeys*>(	K2hdkcCommand::GetSlaveCommandSendObject(DKC_COM_GET_SUBKEYS,	__VA_ARGS__))
#define	GetOtSlaveK2hdkcComGetAttrs(...)						reinterpret_cast<K2hdkcComGetAttrs*>(	K2hdkcCommand::GetSlaveCommandSendObject(DKC_COM_GET_ATTRS,		__VA_ARGS__))
#define	GetOtSlaveK2hdkcComGetAttr(...)							reinterpret_cast<K2hdkcComGetAttr*>(	K2hdkcCommand::GetSlaveCommandSendObject(DKC_COM_GET_ATTR,		__VA_ARGS__))
#define	GetOtSlaveK2hdkcComSet(...)								reinterpret_cast<K2hdkcComSet*>(		K2hdkcCommand::GetSlaveCommandSendObject(DKC_COM_SET,			__VA_ARGS__))
#define	GetOtSlaveK2hdkcComSetDirect(...)						reinterpret_cast<K2hdkcComSetDirect*>(	K2hdkcCommand::GetSlaveCommandSendObject(DKC_COM_SET_DIRECT,	__VA_ARGS__))
#define	GetOtSlaveK2hdkcComSetSubkeys(...)						reinterpret_cast<K2hdkcComSetSubkeys*>(	K2hdkcCommand::GetSlaveCommandSendObject(DKC_COM_SET_SUBKEYS,	__VA_ARGS__))
#define	GetOtSlaveK2hdkcComSetAll(...)							reinterpret_cast<K2hdkcComSetAll*>(		K2hdkcCommand::GetSlaveCommandSendObject(DKC_COM_SET_ALL,		__VA_ARGS__))
#define	GetOtSlaveK2hdkcComAddSubkeys(...)						reinterpret_cast<K2hdkcComAddSubkeys*>(	K2hdkcCommand::GetSlaveCommandSendObject(DKC_COM_ADD_SUBKEYS,	__VA_ARGS__))
#define	GetOtSlaveK2hdkcComAddSubkey(...)						reinterpret_cast<K2hdkcComAddSubkey*>(	K2hdkcCommand::GetSlaveCommandSendObject(DKC_COM_ADD_SUBKEY,	__VA_ARGS__))
#define	GetOtSlaveK2hdkcComDel(...)								reinterpret_cast<K2hdkcComDel*>(		K2hdkcCommand::GetSlaveCommandSendObject(DKC_COM_DEL,			__VA_ARGS__))
#define	GetOtSlaveK2hdkcComDelSubkeys(...)						reinterpret_cast<K2hdkcComDelSubkeys*>(	K2hdkcCommand::GetSlaveCommandSendObject(DKC_COM_DEL_SUBKEYS,	__VA_ARGS__))
#define	GetOtSlaveK2hdkcComDelSubkey(...)						reinterpret_cast<K2hdkcComDelSubkey*>(	K2hdkcCommand::GetSlaveCommandSendObject(DKC_COM_DEL_SUBKEY,	__VA_ARGS__))
#define	GetOtSlaveK2hdkcComRen(...)								reinterpret_cast<K2hdkcComRen*>(		K2hdkcCommand::GetSlaveCommandSendObject(DKC_COM_REN,			__VA_ARGS__))
#define	GetOtSlaveK2hdkcComQPush(...)							reinterpret_cast<K2hdkcComQPush*>(		K2hdkcCommand::GetSlaveCommandSendObject(DKC_COM_QPUSH,			__VA_ARGS__))
#define	GetOtSlaveK2hdkcComQPop(...)							reinterpret_cast<K2hdkcComQPop*>(		K2hdkcCommand::GetSlaveCommandSendObject(DKC_COM_QPOP,			__VA_ARGS__))
#define	GetOtSlaveK2hdkcComQDel(...)							reinterpret_cast<K2hdkcComQDel*>(		K2hdkcCommand::GetSlaveCommandSendObject(DKC_COM_QDEL,			__VA_ARGS__))
#define	GetOtSlaveK2hdkcComCasInit(...)							reinterpret_cast<K2hdkcComCasInit*>(	K2hdkcCommand::GetSlaveCommandSendObject(DKC_COM_CAS_INIT,		__VA_ARGS__))
#define	GetOtSlaveK2hdkcComCasGet(...)							reinterpret_cast<K2hdkcComCasGet*>(		K2hdkcCommand::GetSlaveCommandSendObject(DKC_COM_CAS_GET,		__VA_ARGS__))
#define	GetOtSlaveK2hdkcComCasSet(...)							reinterpret_cast<K2hdkcComCasSet*>(		K2hdkcCommand::GetSlaveCommandSendObject(DKC_COM_CAS_SET,		__VA_ARGS__))
#define	GetOtSlaveK2hdkcComCasIncDec(...)						reinterpret_cast<K2hdkcComCasIncDec*>(	K2hdkcCommand::GetSlaveCommandSendObject(DKC_COM_CAS_INCDEC,	__VA_ARGS__))
#define	GetOtSlaveK2hdkcComReplKey(...)							reinterpret_cast<K2hdkcComReplKey*>(	K2hdkcCommand::GetSlaveCommandSendObject(DKC_COM_REPL_KEY,		__VA_ARGS__))
#define	GetOtSlaveK2hdkcComK2hState(...)						reinterpret_cast<K2hdkcComK2hState*>(	K2hdkcCommand::GetSlaveCommandSendObject(DKC_COM_K2HSTATE,		__VA_ARGS__))
#define	GetOtSlaveK2hdkcComState(...)							reinterpret_cast<K2hdkcComState*>(		K2hdkcCommand::GetSlaveCommandSendObject(DKC_COM_STATE,			__VA_ARGS__))
#define	GetOtSlaveK2hdkcComReplDel(...)							reinterpret_cast<K2hdkcComReplDel*>(	K2hdkcCommand::GetSlaveCommandSendObject(DKC_COM_REPL_DEL,		__VA_ARGS__))

//---------------------------------------------------------
// Utility Macros for Class Factory
//---------------------------------------------------------
//
// Class factory Interface macro for permanent slave command class
//
// GetPmK2hdkcComXXXXX(...)
//
//		K2hdkcSlave*	pslaveobj
//		uint64_t		comnum		(default = K2hdkcComNumber::INIT_NUMBER)
//
#define	GetPmSlaveK2hdkcComGet(...)								reinterpret_cast<K2hdkcComGet*>(		K2hdkcSlave::GetSlaveCommandSendObject(DKC_COM_GET,			__VA_ARGS__))
#define	GetPmSlaveK2hdkcComGetDirect(...)						reinterpret_cast<K2hdkcComGetDirect*>(	K2hdkcSlave::GetSlaveCommandSendObject(DKC_COM_GET_DIRECT,	__VA_ARGS__))
#define	GetPmSlaveK2hdkcComGetSubkeys(...)						reinterpret_cast<K2hdkcComGetSubkeys*>(	K2hdkcSlave::GetSlaveCommandSendObject(DKC_COM_GET_SUBKEYS,	__VA_ARGS__))
#define	GetPmSlaveK2hdkcComGetAttrs(...)						reinterpret_cast<K2hdkcComGetAttrs*>(	K2hdkcSlave::GetSlaveCommandSendObject(DKC_COM_GET_ATTRS,	__VA_ARGS__))
#define	GetPmSlaveK2hdkcComGetAttr(...)							reinterpret_cast<K2hdkcComGetAttr*>(	K2hdkcSlave::GetSlaveCommandSendObject(DKC_COM_GET_ATTR,	__VA_ARGS__))
#define	GetPmSlaveK2hdkcComSet(...)								reinterpret_cast<K2hdkcComSet*>(		K2hdkcSlave::GetSlaveCommandSendObject(DKC_COM_SET,			__VA_ARGS__))
#define	GetPmSlaveK2hdkcComSetDirect(...)						reinterpret_cast<K2hdkcComSetDirect*>(	K2hdkcSlave::GetSlaveCommandSendObject(DKC_COM_SET_DIRECT,	__VA_ARGS__))
#define	GetPmSlaveK2hdkcComSetSubkeys(...)						reinterpret_cast<K2hdkcComSetSubkeys*>(	K2hdkcSlave::GetSlaveCommandSendObject(DKC_COM_SET_SUBKEYS,	__VA_ARGS__))
#define	GetPmSlaveK2hdkcComSetAll(...)							reinterpret_cast<K2hdkcComSetAll*>(		K2hdkcSlave::GetSlaveCommandSendObject(DKC_COM_SET_ALL,		__VA_ARGS__))
#define	GetPmSlaveK2hdkcComAddSubkeys(...)						reinterpret_cast<K2hdkcComAddSubkeys*>(	K2hdkcSlave::GetSlaveCommandSendObject(DKC_COM_ADD_SUBKEYS,	__VA_ARGS__))
#define	GetPmSlaveK2hdkcComAddSubkey(...)						reinterpret_cast<K2hdkcComAddSubkey*>(	K2hdkcSlave::GetSlaveCommandSendObject(DKC_COM_ADD_SUBKEY,	__VA_ARGS__))
#define	GetPmSlaveK2hdkcComDel(...)								reinterpret_cast<K2hdkcComDel*>(		K2hdkcSlave::GetSlaveCommandSendObject(DKC_COM_DEL,			__VA_ARGS__))
#define	GetPmSlaveK2hdkcComDelSubkeys(...)						reinterpret_cast<K2hdkcComDelSubkeys*>(	K2hdkcSlave::GetSlaveCommandSendObject(DKC_COM_DEL_SUBKEYS,	__VA_ARGS__))
#define	GetPmSlaveK2hdkcComDelSubkey(...)						reinterpret_cast<K2hdkcComDelSubkey*>(	K2hdkcSlave::GetSlaveCommandSendObject(DKC_COM_DEL_SUBKEY,	__VA_ARGS__))
#define	GetPmSlaveK2hdkcComRen(...)								reinterpret_cast<K2hdkcComRen*>(		K2hdkcSlave::GetSlaveCommandSendObject(DKC_COM_REN,			__VA_ARGS__))
#define	GetPmSlaveK2hdkcComQPush(...)							reinterpret_cast<K2hdkcComQPush*>(		K2hdkcSlave::GetSlaveCommandSendObject(DKC_COM_QPUSH,		__VA_ARGS__))
#define	GetPmSlaveK2hdkcComQPop(...)							reinterpret_cast<K2hdkcComQPop*>(		K2hdkcSlave::GetSlaveCommandSendObject(DKC_COM_QPOP,		__VA_ARGS__))
#define	GetPmSlaveK2hdkcComQDel(...)							reinterpret_cast<K2hdkcComQDel*>(		K2hdkcSlave::GetSlaveCommandSendObject(DKC_COM_QDEL,		__VA_ARGS__))
#define	GetPmSlaveK2hdkcComCasInit(...)							reinterpret_cast<K2hdkcComCasInit*>(	K2hdkcSlave::GetSlaveCommandSendObject(DKC_COM_CAS_INIT,	__VA_ARGS__))
#define	GetPmSlaveK2hdkcComCasGet(...)							reinterpret_cast<K2hdkcComCasGet*>(		K2hdkcSlave::GetSlaveCommandSendObject(DKC_COM_CAS_GET,		__VA_ARGS__))
#define	GetPmSlaveK2hdkcComCasSet(...)							reinterpret_cast<K2hdkcComCasSet*>(		K2hdkcSlave::GetSlaveCommandSendObject(DKC_COM_CAS_SET,		__VA_ARGS__))
#define	GetPmSlaveK2hdkcComCasIncDec(...)						reinterpret_cast<K2hdkcComCasIncDec*>(	K2hdkcSlave::GetSlaveCommandSendObject(DKC_COM_CAS_INCDEC,	__VA_ARGS__))
#define	GetPmSlaveK2hdkcComReplKey(...)							reinterpret_cast<K2hdkcComReplKey*>(	K2hdkcSlave::GetSlaveCommandSendObject(DKC_COM_REPL_KEY,	__VA_ARGS__))
#define	GetPmSlaveK2hdkcComK2hState(...)						reinterpret_cast<K2hdkcComK2hState*>(	K2hdkcSlave::GetSlaveCommandSendObject(DKC_COM_K2HSTATE,	__VA_ARGS__))
#define	GetPmSlaveK2hdkcComState(...)							reinterpret_cast<K2hdkcComState*>(		K2hdkcSlave::GetSlaveCommandSendObject(DKC_COM_STATE,		__VA_ARGS__))
#define	GetPmSlaveK2hdkcComReplDel(...)							reinterpret_cast<K2hdkcComReplDel*>(	K2hdkcSlave::GetSlaveCommandSendObject(DKC_COM_REPL_DEL,	__VA_ARGS__))

#endif	// K2HDKCCOMUTIL_H

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noexpandtab sw=4 ts=4 fdm=marker
 * vim<600: noexpandtab sw=4 ts=4
 */
