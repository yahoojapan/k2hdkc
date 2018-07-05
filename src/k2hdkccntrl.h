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
 * CREATE:   Mon Jul 11 2016
 * REVISION:
 * 
 */

#ifndef	K2HDKCCNTRL_H
#define	K2HDKCCNTRL_H

#include <map>
#include <vector>

#include <k2hash/k2hshm.h>
#include <chmpx/chmpx.h>
#include <chmpx/chmcntrl.h>

#include "k2hdkcconfparser.h"
#include "k2hdkcthread.h"
#include "k2hdkccombase.h"

//---------------------------------------------------------
// Typedefs
//---------------------------------------------------------
typedef std::map<dkccom_type_t, K2hdkcCommand*>		k2hdkcwaitcommap_t;		// map: Command type		-> Command Object pointer
typedef std::map<uint64_t, k2hdkcwaitcommap_t*>		k2hdkcwaitresmap_t;		// map: Communication Number-> Command type mapping pointer

typedef struct k2hdkc_rcv_processing_data{
	PCOMPKT			pComPkt;
	unsigned char*	pbody;
	size_t			length;
}DKC_RCVPROC_DATA, *PDKC_RCVPROC_DATA;

typedef std::vector<PDKC_RCVPROC_DATA>				k2hdkcrcvprocdata_t;

//---------------------------------------------------------
// K2hdkcCntrl Class
//---------------------------------------------------------
class K2hdkcCntrl
{
	protected:
		static const int	RECEIVE_WAIT_MS	= 1000;		// 1s
		static K2hdkcCntrl	singleton;

		K2HShm				k2hobj;
		ChmCntrl			chmobj;
		K2hdkcThread		ThreadManager;
		short				chmpxctlport;
		bool				no_giveup_rejoin;
		volatile bool		is_loop;
		volatile int		waitmap_lockval;			// lock valiable for waitmap used by fullock_mutex
		k2hdkcwaitresmap_t	waitmap;
		volatile int		procdatas_lockval;			// lock valiable for proc datas used by fullock_mutex
		k2hdkcrcvprocdata_t	rcvprocdatas;

	protected:
		static void SigUsr1handler(int signum);
		static void SigUsr2handler(int signum);
		static void SigHupHandler(int signum);
		static void SigExitHandler(int signum);
		static bool SetSignalHandler(int signum, void (*handler)(int));
		static bool InitializeSignal(void);

		static bool MergeLastTsCallback(chmpx_h handle, struct timespec* pts);
		static bool MergeGetCallback(chmpx_h handle, const PCHM_MERGE_GETPARAM pparam, chmhash_t* pnexthash, PCHMBIN* ppdatas, size_t* pdatacnt);
		static bool MergeSetCallback(chmpx_h handle, size_t length, const unsigned char* pdata, const struct timespec* pts);

		static bool K2hdkcCommandReceiveWaitSetFunc(uint64_t comnum, K2hdkcCommand* pobj, void* pparam);
		static bool K2hdkcCommandReceiveWaitUnsetFunc(uint64_t comnum, K2hdkcCommand* pobj, void* pparam);

		static bool WorkerProc(void* pobj, bool* piswork);

		K2hdkcCntrl(void);
		virtual ~K2hdkcCntrl(void);

		bool Clean(void);
		bool Processing(PCOMPKT pComPkt, unsigned char* pbody, size_t length, bool& need_free);

		bool SetWaitCommandObject(uint64_t comnum, K2hdkcCommand* pobj);
		bool UnsetWaitCommandObject(uint64_t comnum, K2hdkcCommand* pobj);
		K2hdkcCommand* FindWaitCommandObject(uint64_t comnum, dkccom_type_t comtype);

	public:
		static K2hdkcCntrl* Get(void) { return &K2hdkcCntrl::singleton; }

		bool Initialize(const K2hdkcConfig* pk2hdkcconf, short ctlport = CHM_INVALID_PORT, bool nogiveuprejoin = false);
		bool Run(void);
};

#endif	// K2HDKCCNTRL_H

/*
 * VIM modelines
 *
 * vim:set ts=4 fenc=utf-8:
 */
