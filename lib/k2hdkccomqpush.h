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

#ifndef	K2HDKCCOMQPUSH_H
#define	K2HDKCCOMQPUSH_H

#include "k2hdkccombase.h"

//---------------------------------------------------------
// K2hdkcComQPush Class
//---------------------------------------------------------
class K2hdkcComQPush : public K2hdkcCommand
{
	using	K2hdkcCommand::SetResponseData;
	using	K2hdkcCommand::CommandSend;
	using	K2hdkcCommand::GetResponseData;

	protected:
		virtual PDKCCOM_ALL MakeResponseOnlyHeadData(dkcres_type_t subcode = DKC_RES_SUBCODE_NOTHING, dkcres_type_t errcode = DKC_RES_ERROR);
		bool SetSucceedResponseData(void);

		bool CommandSend(const unsigned char* pprefix, size_t prefixlength, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, bool is_fifo, const unsigned char* pattrs, size_t attrslength, bool checkattr, const char* encpass, const time_t* expire);
		bool CommandSend(const unsigned char* pprefix, size_t prefixlength, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, bool is_fifo, const unsigned char* pattrs, size_t attrslength, bool checkattr, const char* encpass, const time_t* expire, dkcres_type_t* prescode);

		virtual void RawDumpComAll(const PDKCCOM_ALL pComAll) const;

	public:
		K2hdkcComQPush(K2HShm* pk2hash = NULL, ChmCntrl* pchmcntrl = NULL, uint64_t comnum = K2hdkcComNumber::INIT_NUMBER, bool without_self = true, bool is_routing_on_server = true, bool is_wait_on_server = false);
		virtual ~K2hdkcComQPush(void);

		virtual bool CommandProcessing(void);

		bool KeyQueueCommandSend(const unsigned char* pprefix, size_t prefixlength, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, bool is_fifo, const unsigned char* pattrs = NULL, size_t attrslength = 0L, bool checkattr = true, const char* encpass = NULL, const time_t* expire = NULL) { return CommandSend(pprefix, prefixlength, pkey, keylength, pval, vallength, is_fifo, pattrs, attrslength, checkattr, encpass, expire); }
		bool KeyQueueCommandSend(const unsigned char* pprefix, size_t prefixlength, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, bool is_fifo, const unsigned char* pattrs, size_t attrslength, bool checkattr, const char* encpass, const time_t* expire, dkcres_type_t* prescode) { return CommandSend(pprefix, prefixlength, pkey, keylength, pval, vallength, is_fifo, pattrs, attrslength, checkattr, encpass, expire, prescode); }

		bool QueueCommandSend(const unsigned char* pprefix, size_t prefixlength, const unsigned char* pval, size_t vallength, bool is_fifo, const unsigned char* pattrs = NULL, size_t attrslength = 0L, bool checkattr = true, const char* encpass = NULL, const time_t* expire = NULL) { return CommandSend(pprefix, prefixlength, NULL, 0L, pval, vallength, is_fifo, pattrs, attrslength, checkattr, encpass, expire); }
		bool QueueCommandSend(const unsigned char* pprefix, size_t prefixlength, const unsigned char* pval, size_t vallength, bool is_fifo, const unsigned char* pattrs, size_t attrslength, bool checkattr, const char* encpass, const time_t* expire, dkcres_type_t* prescode) { return CommandSend(pprefix, prefixlength, NULL, 0L, pval, vallength, is_fifo, pattrs, attrslength, checkattr, encpass, expire, prescode); }

		bool GetResponseData(dkcres_type_t* prescode) const;
};

#endif	// K2HDKCCOMQPUSH_H

/*
 * VIM modelines
 *
 * vim:set ts=4 fenc=utf-8:
 */
