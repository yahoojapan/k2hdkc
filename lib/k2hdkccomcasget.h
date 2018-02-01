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
 * CREATE:   Tue Aug 2 2016
 * REVISION:
 * 
 */

#ifndef	K2HDKCCOMCASGET_H
#define	K2HDKCCOMCASGET_H

#include "k2hdkccombase.h"

//---------------------------------------------------------
// K2hdkcComCasGet Class
//---------------------------------------------------------
class K2hdkcComCasGet : public K2hdkcCommand
{
	using	K2hdkcCommand::SetResponseData;
	using	K2hdkcCommand::CommandSend;
	using	K2hdkcCommand::GetResponseData;

	protected:
		virtual PDKCCOM_ALL MakeResponseOnlyHeadData(dkcres_type_t subcode = DKC_RES_SUBCODE_NOTHING, dkcres_type_t errcode = DKC_RES_ERROR);
		bool SetResponseData(const unsigned char* pValue, size_t ValLen);

		bool CommandSend(const unsigned char* pkey, size_t keylength, bool checkattr, const char* encpass, const unsigned char** ppval, size_t reqvallength, dkcres_type_t* prescode);
		bool GetResponseData(const unsigned char** ppval, size_t reqvallength, dkcres_type_t* prescode) const;

		virtual void RawDumpComAll(const PDKCCOM_ALL pComAll) const;

	public:
		K2hdkcComCasGet(K2HShm* pk2hash = NULL, ChmCntrl* pchmcntrl = NULL, uint64_t comnum = K2hdkcComNumber::INIT_NUMBER, bool without_self = true, bool is_routing_on_server = false, bool is_wait_on_server = false);
		virtual ~K2hdkcComCasGet(void);

		virtual bool CommandProcessing(void);

		bool CommandSend(const unsigned char* pkey, size_t keylength, bool checkattr = true, const char* encpass = NULL);

		bool CommandSend(const unsigned char* pkey, size_t keylength, bool checkattr, const char* encpass, const unsigned char** ppval, size_t* pvallength, dkcres_type_t* prescode);
		bool CommandSend(const unsigned char* pkey, size_t keylength, bool checkattr, const char* encpass, const uint8_t** ppval, dkcres_type_t* prescode) { return CommandSend(pkey, keylength, checkattr, encpass, reinterpret_cast<const unsigned char**>(ppval), sizeof(uint8_t), prescode); }
		bool CommandSend(const unsigned char* pkey, size_t keylength, bool checkattr, const char* encpass, const uint16_t** ppval, dkcres_type_t* prescode) { return CommandSend(pkey, keylength, checkattr, encpass, reinterpret_cast<const unsigned char**>(ppval), sizeof(uint16_t), prescode); }
		bool CommandSend(const unsigned char* pkey, size_t keylength, bool checkattr, const char* encpass, const uint32_t** ppval, dkcres_type_t* prescode) { return CommandSend(pkey, keylength, checkattr, encpass, reinterpret_cast<const unsigned char**>(ppval), sizeof(uint32_t), prescode); }
		bool CommandSend(const unsigned char* pkey, size_t keylength, bool checkattr, const char* encpass, const uint64_t** ppval, dkcres_type_t* prescode) { return CommandSend(pkey, keylength, checkattr, encpass, reinterpret_cast<const unsigned char**>(ppval), sizeof(uint64_t), prescode); }

		bool GetResponseData(const unsigned char** ppval, size_t* plength, dkcres_type_t* prescode) const;
		bool GetResponseData(const uint8_t** ppval, dkcres_type_t* prescode) const { return GetResponseData(reinterpret_cast<const unsigned char**>(ppval), sizeof(uint8_t), prescode); }
		bool GetResponseData(const uint16_t** ppval, dkcres_type_t* prescode) const { return GetResponseData(reinterpret_cast<const unsigned char**>(ppval), sizeof(uint16_t), prescode); }
		bool GetResponseData(const uint32_t** ppval, dkcres_type_t* prescode) const { return GetResponseData(reinterpret_cast<const unsigned char**>(ppval), sizeof(uint32_t), prescode); }
		bool GetResponseData(const uint64_t** ppval, dkcres_type_t* prescode) const { return GetResponseData(reinterpret_cast<const unsigned char**>(ppval), sizeof(uint64_t), prescode); }
};

#endif	// K2HDKCCOMCASGET_H

/*
 * VIM modelines
 *
 * vim:set ts=4 fenc=utf-8:
 */
