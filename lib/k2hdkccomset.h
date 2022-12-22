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
 * CREATE:   Wed Jul 20 2016
 * REVISION:
 * 
 */

#ifndef	K2HDKCCOMSET_H
#define	K2HDKCCOMSET_H

#include "k2hdkccombase.h"

//---------------------------------------------------------
// K2hdkcComSet Class
//---------------------------------------------------------
class K2hdkcComSet : public K2hdkcCommand
{
	using	K2hdkcCommand::SetResponseData;
	using	K2hdkcCommand::CommandSend;
	using	K2hdkcCommand::GetResponseData;

	protected:
		virtual PDKCCOM_ALL MakeResponseOnlyHeadData(dkcres_type_t subcode = DKC_RES_SUBCODE_NOTHING, dkcres_type_t errcode = DKC_RES_ERROR);
		bool SetSucceedResponseData(void);

		bool CommandSendEx(const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, bool rm_subkeylist, dkc_qtype_t queue_type, const unsigned char* pattrs, size_t attrslength, const char* encpass, const time_t* expire);
		bool CommandSendEx(const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, bool rm_subkeylist, dkc_qtype_t queue_type, const unsigned char* pattrs, size_t attrslength, const char* encpass, const time_t* expire, dkcres_type_t* prescode);

		virtual void RawDumpComAll(const PDKCCOM_ALL pComAll) const;

	public:
		explicit K2hdkcComSet(K2HShm* pk2hash = NULL, ChmCntrl* pchmcntrl = NULL, uint64_t comnum = K2hdkcComNumber::INIT_NUMBER, bool without_self = true, bool is_routing_on_server = true, bool is_wait_on_server = false);
		virtual ~K2hdkcComSet(void);

		virtual bool CommandProcessing(void);

		// for normal
		bool CommandSend(const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, bool rm_subkeylist = false, const char* encpass = NULL, const time_t* expire = NULL) { return CommandSendEx(pkey, keylength, pval, vallength, rm_subkeylist, DKC_QUEUE_TYPE_NOTQUEUE, NULL, 0L, encpass, expire); }
		bool CommandSend(const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, bool rm_subkeylist, const char* encpass, const time_t* expire, dkcres_type_t* prescode) { return CommandSendEx(pkey, keylength, pval, vallength, rm_subkeylist, DKC_QUEUE_TYPE_NOTQUEUE, NULL, 0L, encpass, expire, prescode); }

		// for only operation queue internally
		bool CommandSend(const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, dkc_qtype_t queue_type, const unsigned char* pattrs = NULL, size_t attrslength = 0L, const char* encpass = NULL, const time_t* expire = NULL) { if(!IS_DKC_QUEUE_TYPE(queue_type)){ return false; }else{ return CommandSendEx(pkey, keylength, pval, vallength, true, queue_type, pattrs, attrslength, encpass, expire); } }
		bool CommandSend(const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, dkc_qtype_t queue_type, const unsigned char* pattrs, size_t attrslength, const char* encpass, const time_t* expire, dkcres_type_t* prescode) { if(!IS_DKC_QUEUE_TYPE(queue_type)){ return false; }else{ return CommandSendEx(pkey, keylength, pval, vallength, true, queue_type, pattrs, attrslength, encpass, expire, prescode); } }

		bool GetResponseData(dkcres_type_t* prescode) const;
};

#endif	// K2HDKCCOMSET_H

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noexpandtab sw=4 ts=4 fdm=marker
 * vim<600: noexpandtab sw=4 ts=4
 */
