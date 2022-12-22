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
 * CREATE:   Tue Sep 6 2016
 * REVISION:
 * 
 */

#ifndef	K2HDKCCOMSTATE_H
#define	K2HDKCCOMSTATE_H

#include <map>

#include "k2hdkccombase.h"

//---------------------------------------------------------
// Typedefs
//---------------------------------------------------------
typedef std::map<chmpxid_t, chmhash_t>	dkcchmpxhashmap_t;

//---------------------------------------------------------
// K2hdkcComState Class
//---------------------------------------------------------
class K2hdkcComState : public K2hdkcCommand
{
	using	K2hdkcCommand::SetResponseData;
	using	K2hdkcCommand::CommandSend;
	using	K2hdkcCommand::GetResponseData;

	protected:
		virtual PDKCCOM_ALL MakeResponseOnlyHeadData(dkcres_type_t subcode = DKC_RES_SUBCODE_NOTHING, dkcres_type_t errcode = DKC_RES_ERROR);
		PDKCCOM_ALL AllocateResponseData(size_t svrnodecnt);

		virtual void RawDumpComAll(const PDKCCOM_ALL pComAll) const;

		size_t GetServerNodeMapList(dkcchmpxhashmap_t& idhashmap);

	public:
		explicit K2hdkcComState(K2HShm* pk2hash = NULL, ChmCntrl* pchmcntrl = NULL, uint64_t comnum = K2hdkcComNumber::INIT_NUMBER, bool without_self = true, bool is_routing_on_server = false, bool is_wait_on_server = false);
		virtual ~K2hdkcComState(void);

		virtual bool CommandProcessing(void);

		virtual bool CommandSend(void);
		bool CommandSend(const DKC_NODESTATE** ppStates, size_t* pstatecount, dkcres_type_t* prescode);
		bool GetResponseData(const DKC_NODESTATE** ppStates, size_t* pstatecount, dkcres_type_t* prescode) const;
};

#endif	// K2HDKCCOMSTATE_H

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noexpandtab sw=4 ts=4 fdm=marker
 * vim<600: noexpandtab sw=4 ts=4
 */
