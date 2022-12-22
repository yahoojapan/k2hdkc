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

#ifndef	K2HDKCCOMCASSET_H
#define	K2HDKCCOMCASSET_H

#include "k2hdkccombase.h"

//---------------------------------------------------------
// K2hdkcComCasSet Class
//---------------------------------------------------------
class K2hdkcComCasSet : public K2hdkcCommand
{
	using	K2hdkcCommand::SetResponseData;
	using	K2hdkcCommand::CommandSend;
	using	K2hdkcCommand::GetResponseData;

	protected:
		virtual PDKCCOM_ALL MakeResponseOnlyHeadData(dkcres_type_t subcode = DKC_RES_SUBCODE_NOTHING, dkcres_type_t errcode = DKC_RES_ERROR);
		bool SetSucceedResponseData(void);

		virtual void RawDumpComAll(const PDKCCOM_ALL pComAll) const;

	public:
		explicit K2hdkcComCasSet(K2HShm* pk2hash = NULL, ChmCntrl* pchmcntrl = NULL, uint64_t comnum = K2hdkcComNumber::INIT_NUMBER, bool without_self = true, bool is_routing_on_server = true, bool is_wait_on_server = false);
		virtual ~K2hdkcComCasSet(void);

		virtual bool CommandProcessing(void);

		bool CommandSend(const unsigned char* pkey, size_t keylength, const unsigned char* poldval, size_t oldvallength, const unsigned char* pnewval, size_t newvallength, bool checkattr = true, const char* encpass = NULL, const time_t* expire = NULL);
		bool CommandSend(const unsigned char* pkey, size_t keylength, uint8_t oldval, uint8_t newval, bool checkattr = true, const char* encpass = NULL, const time_t* expire = NULL) { return CommandSend(pkey, keylength, reinterpret_cast<const unsigned char*>(&oldval), sizeof(uint8_t), reinterpret_cast<const unsigned char*>(&newval), sizeof(uint8_t), checkattr, encpass, expire); }
		bool CommandSend(const unsigned char* pkey, size_t keylength, uint16_t oldval, uint16_t newval, bool checkattr = true, const char* encpass = NULL, const time_t* expire = NULL) { return CommandSend(pkey, keylength, reinterpret_cast<const unsigned char*>(&oldval), sizeof(uint16_t), reinterpret_cast<const unsigned char*>(&newval), sizeof(uint16_t), checkattr, encpass, expire); }
		bool CommandSend(const unsigned char* pkey, size_t keylength, uint32_t oldval, uint32_t newval, bool checkattr = true, const char* encpass = NULL, const time_t* expire = NULL) { return CommandSend(pkey, keylength, reinterpret_cast<const unsigned char*>(&oldval), sizeof(uint32_t), reinterpret_cast<const unsigned char*>(&newval), sizeof(uint32_t), checkattr, encpass, expire); }
		bool CommandSend(const unsigned char* pkey, size_t keylength, uint64_t oldval, uint64_t newval, bool checkattr = true, const char* encpass = NULL, const time_t* expire = NULL) { return CommandSend(pkey, keylength, reinterpret_cast<const unsigned char*>(&oldval), sizeof(uint64_t), reinterpret_cast<const unsigned char*>(&newval), sizeof(uint64_t), checkattr, encpass, expire); }

		bool CommandSend(const unsigned char* pkey, size_t keylength, const unsigned char* poldval, size_t oldvallength, const unsigned char* pnewval, size_t newvallength, bool checkattr, const char* encpass, const time_t* expire, dkcres_type_t* prescode);
		bool CommandSend(const unsigned char* pkey, size_t keylength, uint8_t oldval, uint8_t newval, bool checkattr, const char* encpass, const time_t* expire, dkcres_type_t* prescode) { return CommandSend(pkey, keylength, reinterpret_cast<const unsigned char*>(&oldval), sizeof(uint8_t), reinterpret_cast<const unsigned char*>(&newval), sizeof(uint8_t), checkattr, encpass, expire, prescode); }
		bool CommandSend(const unsigned char* pkey, size_t keylength, uint16_t oldval, uint16_t newval, bool checkattr, const char* encpass, const time_t* expire, dkcres_type_t* prescode) { return CommandSend(pkey, keylength, reinterpret_cast<const unsigned char*>(&oldval), sizeof(uint16_t), reinterpret_cast<const unsigned char*>(&newval), sizeof(uint16_t), checkattr, encpass, expire, prescode); }
		bool CommandSend(const unsigned char* pkey, size_t keylength, uint32_t oldval, uint32_t newval, bool checkattr, const char* encpass, const time_t* expire, dkcres_type_t* prescode) { return CommandSend(pkey, keylength, reinterpret_cast<const unsigned char*>(&oldval), sizeof(uint32_t), reinterpret_cast<const unsigned char*>(&newval), sizeof(uint32_t), checkattr, encpass, expire, prescode); }
		bool CommandSend(const unsigned char* pkey, size_t keylength, uint64_t oldval, uint64_t newval, bool checkattr, const char* encpass, const time_t* expire, dkcres_type_t* prescode) { return CommandSend(pkey, keylength, reinterpret_cast<const unsigned char*>(&oldval), sizeof(uint64_t), reinterpret_cast<const unsigned char*>(&newval), sizeof(uint64_t), checkattr, encpass, expire, prescode); }

		bool GetResponseData(dkcres_type_t* prescode) const;
};

#endif	// K2HDKCCOMCASSET_H

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noexpandtab sw=4 ts=4 fdm=marker
 * vim<600: noexpandtab sw=4 ts=4
 */
