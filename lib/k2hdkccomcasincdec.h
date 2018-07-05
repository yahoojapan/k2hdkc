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
 * CREATE:   Tue Aug 2 2016
 * REVISION:
 * 
 */

#ifndef	K2HDKCCOMCASINCDEC_H
#define	K2HDKCCOMCASINCDEC_H

#include "k2hdkccombase.h"

//---------------------------------------------------------
// K2hdkcComCasIncDec Class
//---------------------------------------------------------
class K2hdkcComCasIncDec : public K2hdkcCommand
{
	using	K2hdkcCommand::SetResponseData;
	using	K2hdkcCommand::CommandSend;
	using	K2hdkcCommand::GetResponseData;

	protected:
		virtual PDKCCOM_ALL MakeResponseOnlyHeadData(dkcres_type_t subcode = DKC_RES_SUBCODE_NOTHING, dkcres_type_t errcode = DKC_RES_ERROR);
		bool SetSucceedResponseData(void);

		virtual void RawDumpComAll(const PDKCCOM_ALL pComAll) const;

	public:
		K2hdkcComCasIncDec(K2HShm* pk2hash = NULL, ChmCntrl* pchmcntrl = NULL, uint64_t comnum = K2hdkcComNumber::INIT_NUMBER, bool without_self = true, bool is_routing_on_server = true, bool is_wait_on_server = false);
		virtual ~K2hdkcComCasIncDec(void);

		virtual bool CommandProcessing(void);

		bool CommandSend(const unsigned char* pkey, size_t keylength, bool is_increment, bool checkattr = true, const char* encpass = NULL, const time_t* expire = NULL);
		bool IncrementCommandSend(const unsigned char* pkey, size_t keylength, bool checkattr = true, const char* encpass = NULL, const time_t* expire = NULL) { return CommandSend(pkey, keylength, true, checkattr, encpass, expire); }
		bool DecrementCommandSend(const unsigned char* pkey, size_t keylength, bool checkattr = true, const char* encpass = NULL, const time_t* expire = NULL) { return CommandSend(pkey, keylength, false, checkattr, encpass, expire); }

		bool CommandSend(const unsigned char* pkey, size_t keylength, bool is_increment, bool checkattr, const char* encpass, const time_t* expire, dkcres_type_t* prescode);
		bool IncrementCommandSend(const unsigned char* pkey, size_t keylength, bool checkattr, const char* encpass, const time_t* expire, dkcres_type_t* prescode) { return CommandSend(pkey, keylength, true, checkattr, encpass, expire, prescode); }
		bool DecrementCommandSend(const unsigned char* pkey, size_t keylength, bool checkattr, const char* encpass, const time_t* expire, dkcres_type_t* prescode) { return CommandSend(pkey, keylength, false, checkattr, encpass, expire, prescode); }

		bool GetResponseData(dkcres_type_t* prescode) const;
};

#endif	// K2HDKCCOMCASINCDEC_H

/*
 * VIM modelines
 *
 * vim:set ts=4 fenc=utf-8:
 */
