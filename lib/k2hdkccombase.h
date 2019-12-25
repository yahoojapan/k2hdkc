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
 * CREATE:   Fri Jul 15 2016
 * REVISION:
 * 
 */

#ifndef	K2HDKCCOMBASE_H
#define	K2HDKCCOMBASE_H

#include <pthread.h>

#include <k2hash/k2hshm.h>
#include <chmpx/chmcntrl.h>

#include "k2hdkccommon.h"
#include "k2hdkccomstructure.h"
#include "k2hdkccomnum.h"
#include "k2hdkcslave.h"
#include "k2hdkcdbg.h"

class K2hdkcCommand;

//---------------------------------------------------------
// Prototype function
//---------------------------------------------------------
typedef bool (*dkccom_setwait_fp)(uint64_t comnum, K2hdkcCommand* pobj, void* pparam);
typedef bool (*dkccom_unsetwait_fp)(uint64_t comnum, K2hdkcCommand* pobj, void* pparam);

//---------------------------------------------------------
// K2hdkcCommand Class
//---------------------------------------------------------
class K2hdkcCommand
{
	protected:
		static const long			DEFAULT_RCV_TIMEOUT_MS	= 1000;	// 1000ms
		static long					RcvTimeout;						// timeout ns for receiving
		static dkccom_setwait_fp	WaitFp;							// registered callback function
		static dkccom_unsetwait_fp	UnWaitFp;						// registered callback function
		static void*				pWaitFpParam;					// parameters for callback function

		std::string					strClassName;					// class name string
		K2HShm*						pK2hObj;						// k2hash object
		ChmCntrl*					pChmObj;						// chmpx object
		bool						IsServerNode;					// process joins server node
		uint64_t					ComNumber;						// Number for Ancestry Communication for debugging
		uint64_t					DispComNumber;					// Number for Disposable Communication for debugging
		bool						IsNeedReply;					// whether need to reply
		bool						WithoutSelfOnServer;			// sending command without self server chmpx node when client on server node
		bool						IsWaitResOnServer;				// whether wait response on server node
		bool						RoutingOnServer;				// using routing mode at sending command from server node
		bool						RoutingOnSlave;					// using routing mode at sending command from slave node( default true )
		bool						ReplicateSend;					// using replicate api for sending command( default false )
		bool						TriggerResponse;				// flag for releasing of the stand-by response
		dkccom_type_t				MainCommandType;				// main command type

		PDKCCOM_ALL					pRcvComAll;						// Command data for received command
		size_t						RcvComLength;					// received command length
		PCOMPKT						pRcvComPkt;						// chmpx ComPkt for received command
		pthread_mutex_t				cond_mutex;						// mutex for condition object
		pthread_cond_t				cond_val;						// condition object for waiting response

		PDKCCOM_ALL					pSendComAll;					// Command data for send(response) data
		size_t						SendLength;						// send(response) data length
		msgid_t						SendMsgid;						// msgid on chmpx slave node for sending
		chmhash_t					SendHash;						// hash code for sending(responding)
		bool						IsLocalMsgid;					// msgid is opened by object, so need to close it before destructing
		bool						IsCreateSlave;					// whether creating slave command object
		K2hdkcSlave*				pSlaveObj;						// slave command object pointer

	protected:
		static chmhash_t MakeChmpxHash(unsigned char* byptr, size_t length);

		const char* GetClassName(void) const { return strClassName.c_str(); }

		virtual bool Clean(void);
		bool CleanSlave(void);

		virtual bool SetSendData(PDKCCOM_ALL pComAll, chmhash_t hash);

		virtual bool SetResponseData(PDKCCOM_ALL pComAll);
		virtual bool SetErrorResponseData(dkcres_type_t subcode = DKC_RES_SUBCODE_NOTHING, dkcres_type_t errcode = DKC_RES_ERROR);
		virtual PDKCCOM_ALL MakeResponseOnlyHeadData(dkcres_type_t subcode = DKC_RES_SUBCODE_NOTHING, dkcres_type_t errcode = DKC_RES_ERROR);
		virtual bool ReplyResponse(void);

		virtual bool CommandProcessing(void);

		virtual bool CommandSending(void);
		virtual bool CommandReplicate(const unsigned char* pkey, size_t keylength, const struct timespec ts);

		void DumpComAll(const char* pprefix, const PDKCCOM_ALL pComAll) const;
		virtual void RawDumpComAll(const PDKCCOM_ALL pComAll) const;

	public:
		static void SetReceiveWaitFunc(dkccom_setwait_fp setfp, dkccom_unsetwait_fp unsetfp, void* pdata) { WaitFp = setfp; UnWaitFp = unsetfp; pWaitFpParam = pdata; }
		static void SetReceiveTimeout(long timeout_ms) { RcvTimeout = timeout_ms; }
		static long GetReceiveTimeout(void) { return RcvTimeout; }
		static bool GetResponseCommandType(const unsigned char* pbody, size_t length, dkccom_type_t& comtype, dkcres_type_t& restype, uint64_t& comnum, uint64_t& dispnum);

		// class factory
		static K2hdkcCommand* GetCommandReceiveObject(K2HShm* pk2hash, ChmCntrl* pchmcntrl, PCOMPKT pComPkt, unsigned char* pbody, size_t length);
		static K2hdkcCommand* GetCommandSendObject(K2HShm* pk2hash, ChmCntrl* pchmcntrl, dkccom_type_t comtype, msgid_t msgid = CHM_INVALID_MSGID, uint64_t comnum = K2hdkcComNumber::INIT_NUMBER, bool without_self = true, bool is_routing_on_server = true, bool is_wait_on_server = false);
		static K2hdkcCommand* GetSlaveCommandSendObject(dkccom_type_t comtype, const char* config, short ctlport = CHM_INVALID_PORT, const char* cuk = NULL, bool is_auto_rejoin = false, bool no_giveup_rejoin = false, uint64_t comnum = K2hdkcComNumber::INIT_NUMBER);

		K2hdkcCommand(K2HShm* pk2hash = NULL, ChmCntrl* pchmcntrl = NULL, uint64_t comnum = K2hdkcComNumber::INIT_NUMBER, bool without_self = true, bool is_routing_on_server = true, bool is_wait_on_server = false, dkccom_type_t comtype = DKC_COM_UNKNOWN);
		virtual ~K2hdkcCommand(void);

		// parameters for command
		uint64_t GetComNumber(void) const		{ return ComNumber; }
		uint64_t GetDispComNumber(void) const	{ return DispComNumber; }
		bool IsSendWithoutSelf(void) const		{ return WithoutSelfOnServer; }
		void SetSendWithoutSelf(void)			{ WithoutSelfOnServer = true; }
		void SetSendWithSelf(void)				{ WithoutSelfOnServer = false; }
		bool IsRoutingOnServer(void) const		{ return RoutingOnServer; }
		void SetRoutingOnServer(void)			{ RoutingOnServer = true; }
		void UnsetRoutingOnServer(void)			{ RoutingOnServer = false; }
		bool IsRoutingOnSlave(void) const		{ return RoutingOnSlave; }
		void SetRoutingOnSlave(void)			{ RoutingOnSlave = true; }
		void UnsetRoutingOnSlave(void)			{ RoutingOnSlave = false; }
		bool IsReplicateSend(void) const		{ return ReplicateSend; }
		void SetReplicateSend(void)				{ ReplicateSend = true; }
		void UnsetReplicateSend(void)			{ ReplicateSend = false; }
		dkccom_type_t GetCommandType(void) const{ return MainCommandType; }

		// do not use this method, allow only directly setting result
		virtual bool SetReceiveData(PCOMPKT pComPkt, PDKCCOM_ALL pComAll);

		// for receiving command and processing on server node
		virtual bool CommandProcess(void);

		// for sending command on client node
		virtual bool CommandSend(void);
		virtual bool GetResponseData(PDKCCOM_ALL* ppcomall, size_t* plength, dkcres_type_t* prescode) const;

		// received data type/code
		bool GetResponseCode(dkcres_type_t& rescode) const;
		dkcres_type_t GetResponseCode(void) const { dkcres_type_t rescode = DKC_NORESTYPE; GetResponseCode(rescode); return rescode; }
		bool IsReceivedCommandType(void) const { return (DKC_NORESTYPE == GetResponseCode()); }
		bool IsReceivedResposeType(void) const { return (DKC_NORESTYPE != GetResponseCode()); }
		bool SetPermanentSlave(K2hdkcSlave* pSlave);

		// for debug
		void Dump(void) const;
};

#endif	// K2HDKCCOMBASE_H

/*
 * VIM modelines
 *
 * vim:set ts=4 fenc=utf-8:
 */
