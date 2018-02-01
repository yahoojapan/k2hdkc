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
 * CREATE:   Fri Jul 22 2016
 * REVISION:
 * 
 */

#ifndef	K2HDKCCOMGETDIRECT_H
#define	K2HDKCCOMGETDIRECT_H

#include "k2hdkccombase.h"

//---------------------------------------------------------
// K2hdkcComGetDirect Class
//---------------------------------------------------------
class K2hdkcComGetDirect : public K2hdkcCommand
{
	using	K2hdkcCommand::SetResponseData;
	using	K2hdkcCommand::CommandSend;
	using	K2hdkcCommand::GetResponseData;

	protected:
		virtual PDKCCOM_ALL MakeResponseOnlyHeadData(dkcres_type_t subcode = DKC_RES_SUBCODE_NOTHING, dkcres_type_t errcode = DKC_RES_ERROR);
		bool SetResponseData(const unsigned char* pValue, size_t ValLen);

		virtual void RawDumpComAll(const PDKCCOM_ALL pComAll) const;

	public:
		K2hdkcComGetDirect(K2HShm* pk2hash = NULL, ChmCntrl* pchmcntrl = NULL, uint64_t comnum = K2hdkcComNumber::INIT_NUMBER, bool without_self = true, bool is_routing_on_server = true, bool is_wait_on_server = false);
		virtual ~K2hdkcComGetDirect(void);

		virtual bool CommandProcessing(void);

		bool CommandSend(const unsigned char* pkey, size_t keylength, off_t val_pos, size_t val_length);
		bool CommandSend(const unsigned char* pkey, size_t keylength, off_t val_pos, size_t val_length, const unsigned char** ppval, size_t* pvallength, dkcres_type_t* prescode);
		bool GetResponseData(const unsigned char** ppdata, size_t* plength, dkcres_type_t* prescode) const;
};

#endif	// K2HDKCCOMGETDIRECT_H

/*
 * VIM modelines
 *
 * vim:set ts=4 fenc=utf-8:
 */
