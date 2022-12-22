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

#ifndef	K2HDKCCOMQPOP_H
#define	K2HDKCCOMQPOP_H

#include "k2hdkccombase.h"

//---------------------------------------------------------
// K2hdkcComQPop Class
//---------------------------------------------------------
class K2hdkcComQPop : public K2hdkcCommand
{
	using	K2hdkcCommand::SetResponseData;
	using	K2hdkcCommand::CommandSend;
	using	K2hdkcCommand::GetResponseData;

	protected:
		virtual PDKCCOM_ALL MakeResponseOnlyHeadData(dkcres_type_t subcode = DKC_RES_SUBCODE_NOTHING, dkcres_type_t errcode = DKC_RES_ERROR);
		bool SetResponseData(const unsigned char* pKey, size_t KeyLen, const unsigned char* pValue, size_t ValLen);

		bool CommandSend(const unsigned char* pprefix, size_t prefixlength, bool is_fifo, bool is_keyqueue, bool checkattr, const char* encpass);
		bool CommandSend(const unsigned char* pprefix, size_t prefixlength, bool is_fifo, bool is_keyqueue, bool checkattr, const char* encpass, const unsigned char** ppkey, size_t* pkeylength, const unsigned char** ppval, size_t* pvallength, dkcres_type_t* prescode);

		bool GetResponseData(const unsigned char** ppkey, size_t* pkeylength, const unsigned char** ppval, size_t* pvallength, dkcres_type_t* prescode) const;
		bool IsSuccessResponse(void) const;

		virtual void RawDumpComAll(const PDKCCOM_ALL pComAll) const;

	public:
		explicit K2hdkcComQPop(K2HShm* pk2hash = NULL, ChmCntrl* pchmcntrl = NULL, uint64_t comnum = K2hdkcComNumber::INIT_NUMBER, bool without_self = true, bool is_routing_on_server = true, bool is_wait_on_server = false);
		virtual ~K2hdkcComQPop(void);

		virtual bool CommandProcessing(void);

		bool KeyQueueCommandSend(const unsigned char* pprefix, size_t prefixlength, bool is_fifo, bool checkattr = true, const char* encpass = NULL) { return CommandSend(pprefix, prefixlength, is_fifo, true, checkattr, encpass); }
		bool KeyQueueCommandSend(const unsigned char* pprefix, size_t prefixlength, bool is_fifo, bool checkattr, const char* encpass, const unsigned char** ppkey, size_t* pkeylength, const unsigned char** ppval, size_t* pvallength, dkcres_type_t* prescode) { return CommandSend(pprefix, prefixlength, is_fifo, true, checkattr, encpass, ppkey, pkeylength, ppval, pvallength, prescode); }
		bool QueueCommandSend(const unsigned char* pprefix, size_t prefixlength, bool is_fifo, bool checkattr = true, const char* encpass = NULL) { return CommandSend(pprefix, prefixlength, is_fifo, false, checkattr, encpass); }
		bool QueueCommandSend(const unsigned char* pprefix, size_t prefixlength, bool is_fifo, bool checkattr, const char* encpass, const unsigned char** ppval, size_t* pvallength, dkcres_type_t* prescode) { return CommandSend(pprefix, prefixlength, is_fifo, false, checkattr, encpass, NULL, NULL, ppval, pvallength, prescode); }

		bool GetKeyQueueResponseData(const unsigned char** ppkey, size_t* pkeylength, const unsigned char** ppval, size_t* pvallength, dkcres_type_t* prescode) const { return GetResponseData(ppkey, pkeylength, ppval, pvallength, prescode); }
		bool GetQueueResponseData(const unsigned char** ppval, size_t* pvallength, dkcres_type_t* prescode) const { return GetResponseData(NULL, NULL, ppval, pvallength, prescode); }
};

#endif	// K2HDKCCOMQPOP_H

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noexpandtab sw=4 ts=4 fdm=marker
 * vim<600: noexpandtab sw=4 ts=4
 */
