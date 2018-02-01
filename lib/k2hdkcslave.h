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
 * CREATE:   Mon Aug 8 2016
 * REVISION:
 * 
 */

#ifndef	K2HDKCSLAVE_H
#define	K2HDKCSLAVE_H

#include <chmpx/chmcntrl.h>

#include "k2hdkccommon.h"
#include "k2hdkccomstructure.h"

class K2hdkcCommand;

//---------------------------------------------------------
// K2hdkcSlave Class
//---------------------------------------------------------
class K2hdkcSlave
{
	protected:
		std::string		chmpxconfig;
		short			chmpxctlport;
		bool			chmpxautorejoin;
		ChmCntrl		chmobj;
		msgid_t			msgid;
		dkcres_type_t	lastrescode;

	protected:
		bool IsInitialize(void) const { return !chmpxconfig.empty(); }
		bool IsOpen(void) const { return (CHM_INVALID_MSGID != msgid); }

	public:
		static K2hdkcCommand* GetSlaveCommandSendObject(dkccom_type_t comtype, K2hdkcSlave* pslaveobj, uint64_t comnum = K2hdkcComNumber::INIT_NUMBER);

		K2hdkcSlave(void);
		virtual ~K2hdkcSlave(void);

		bool Initialize(const char* config, short ctlport = CHM_INVALID_PORT, bool is_auto_rejoin = false);
		bool Uninitialize(bool is_clean_bup = true) { return Clean(); }
		bool Clean(bool is_clean_bup = true);

		bool Open(bool no_giveup_rejoin = false);
		bool Close(void);

		bool SetResponseCode(dkcres_type_t rescode);
		bool GetResponseCode(dkcres_type_t& rescode) const;
		dkcres_type_t GetResponseCode(void) const { return lastrescode; }

		// call from K2hdkcCommand class
		ChmCntrl* GetChmCntrlObject(void) { return (IsInitialize() ? &chmobj : NULL); }
		msgid_t GetMsgid(void) { return msgid; }
};

#endif	// K2HDKCSLAVE_H

/*
 * VIM modelines
 *
 * vim:set ts=4 fenc=utf-8:
 */
