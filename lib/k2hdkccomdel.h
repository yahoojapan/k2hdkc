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
 * CREATE:   Thu Jul 28 2016
 * REVISION:
 * 
 */

#ifndef	K2HDKCCOMDEL_H
#define	K2HDKCCOMDEL_H

#include "k2hdkccombase.h"

//---------------------------------------------------------
// K2hdkcComDel Class
//---------------------------------------------------------
class K2hdkcComDel : public K2hdkcCommand
{
	using	K2hdkcCommand::SetResponseData;
	using	K2hdkcCommand::CommandSend;
	using	K2hdkcCommand::GetResponseData;

	protected:
		virtual PDKCCOM_ALL MakeResponseOnlyHeadData(dkcres_type_t subcode = DKC_RES_SUBCODE_NOTHING, dkcres_type_t errcode = DKC_RES_ERROR);
		bool SetSucceedResponseData(void);

		virtual bool CommandReplicate(const unsigned char* pkey, size_t keylength, const struct timespec ts);
		virtual void RawDumpComAll(const PDKCCOM_ALL pComAll) const;

		bool IsAllNodesSafe(void);

	public:
		explicit K2hdkcComDel(K2HShm* pk2hash = NULL, ChmCntrl* pchmcntrl = NULL, uint64_t comnum = K2hdkcComNumber::INIT_NUMBER, bool without_self = true, bool is_routing_on_server = true, bool is_wait_on_server = false);
		virtual ~K2hdkcComDel(void);

		virtual bool CommandProcessing(void);

		bool CommandSend(const unsigned char* pkey, size_t keylength, bool is_subkeys, bool checkattr = true);
		bool CommandSend(const unsigned char* pkey, size_t keylength, bool is_subkeys, bool checkattr, dkcres_type_t* prescode);
		bool GetResponseData(dkcres_type_t* prescode) const;
};

#endif	// K2HDKCCOMDEL_H

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noexpandtab sw=4 ts=4 fdm=marker
 * vim<600: noexpandtab sw=4 ts=4
 */
