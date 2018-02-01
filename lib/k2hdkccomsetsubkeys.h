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
 * CREATE:   Wed Jul 20 2016
 * REVISION:
 * 
 */

#ifndef	K2HDKCCOMSETSUBKEYS_H
#define	K2HDKCCOMSETSUBKEYS_H

#include "k2hdkccombase.h"

//---------------------------------------------------------
// K2hdkcComSetSubkeys Class
//---------------------------------------------------------
class K2hdkcComSetSubkeys : public K2hdkcCommand
{
	using	K2hdkcCommand::SetResponseData;
	using	K2hdkcCommand::CommandSend;
	using	K2hdkcCommand::GetResponseData;

	protected:
		virtual PDKCCOM_ALL MakeResponseOnlyHeadData(dkcres_type_t subcode = DKC_RES_SUBCODE_NOTHING, dkcres_type_t errcode = DKC_RES_ERROR);
		bool SetSucceedResponseData(void);

		virtual void RawDumpComAll(const PDKCCOM_ALL pComAll) const;

	public:
		K2hdkcComSetSubkeys(K2HShm* pk2hash = NULL, ChmCntrl* pchmcntrl = NULL, uint64_t comnum = K2hdkcComNumber::INIT_NUMBER, bool without_self = true, bool is_routing_on_server = true, bool is_wait_on_server = false);
		virtual ~K2hdkcComSetSubkeys(void);

		virtual bool CommandProcessing(void);

		bool CommandSend(const unsigned char* pkey, size_t keylength, const unsigned char* psubkeys, size_t subkeyslength);
		bool CommandSend(const unsigned char* pkey, size_t keylength, const unsigned char* psubkeys, size_t subkeyslength, dkcres_type_t* prescode);
		bool GetResponseData(dkcres_type_t* prescode) const;

		bool ClearSubkeysCommandSend(const unsigned char* pkey, size_t keylength) { return CommandSend(pkey, keylength, NULL, 0); }
		bool ClearSubkeysCommandSend(const unsigned char* pkey, size_t keylength, dkcres_type_t* prescode) { return CommandSend(pkey, keylength, NULL, 0, prescode); }
};

#endif	// K2HDKCCOMSETSUBKEYS_H

/*
 * VIM modelines
 *
 * vim:set ts=4 fenc=utf-8:
 */
