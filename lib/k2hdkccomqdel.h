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

#ifndef	K2HDKCCOMQDEL_H
#define	K2HDKCCOMQDEL_H

#include "k2hdkccombase.h"

//---------------------------------------------------------
// K2hdkcComQDel Class
//---------------------------------------------------------
class K2hdkcComQDel : public K2hdkcCommand
{
	using	K2hdkcCommand::SetResponseData;
	using	K2hdkcCommand::CommandSend;
	using	K2hdkcCommand::GetResponseData;

	protected:
		virtual PDKCCOM_ALL MakeResponseOnlyHeadData(dkcres_type_t subcode = DKC_RES_SUBCODE_NOTHING, dkcres_type_t errcode = DKC_RES_ERROR);
		virtual bool SetErrorResponseData(dkcres_type_t subcode = DKC_RES_SUBCODE_NOTHING, dkcres_type_t errcode = DKC_RES_ERROR);
		bool SetSucceedResponseData(void);

		bool CommandSend(const unsigned char* pprefix, size_t prefixlength, int count, bool is_keyqueue, bool is_fifo, bool checkattr, const char* encpass);
		bool CommandSend(const unsigned char* pprefix, size_t prefixlength, int count, bool is_keyqueue, bool is_fifo, bool checkattr, const char* encpass, dkcres_type_t* prescode);

		virtual void RawDumpComAll(const PDKCCOM_ALL pComAll) const;

	public:
		explicit K2hdkcComQDel(K2HShm* pk2hash = NULL, ChmCntrl* pchmcntrl = NULL, uint64_t comnum = K2hdkcComNumber::INIT_NUMBER, bool without_self = true, bool is_routing_on_server = true, bool is_wait_on_server = false);
		virtual ~K2hdkcComQDel(void);

		virtual bool CommandProcessing(void);

		bool KeyQueueCommandSend(const unsigned char* pprefix, size_t prefixlength, int count, bool is_fifo, bool checkattr = true, const char* encpass = NULL) { return CommandSend(pprefix, prefixlength, count, true, is_fifo, checkattr, encpass); }
		bool KeyQueueCommandSend(const unsigned char* pprefix, size_t prefixlength, int count, bool is_fifo, bool checkattr, const char* encpass, dkcres_type_t* prescode) { return CommandSend(pprefix, prefixlength, count, true, is_fifo, checkattr, encpass, prescode); }
		bool QueueCommandSend(const unsigned char* pprefix, size_t prefixlength, int count, bool is_fifo, bool checkattr, const char* encpass) { return CommandSend(pprefix, prefixlength, count, false, is_fifo, checkattr, encpass); }
		bool QueueCommandSend(const unsigned char* pprefix, size_t prefixlength, int count, bool is_fifo, bool checkattr, const char* encpass, dkcres_type_t* prescode) { return CommandSend(pprefix, prefixlength, count, false, is_fifo, checkattr, encpass, prescode); }

		bool GetResponseData(dkcres_type_t* prescode) const;
};

#endif	// K2HDKCCOMQDEL_H

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noexpandtab sw=4 ts=4 fdm=marker
 * vim<600: noexpandtab sw=4 ts=4
 */
