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
 * CREATE:   Fri Aug 12 2016
 * REVISION:
 * 
 */

#ifndef	K2HDKCTHREAD_H
#define	K2HDKCTHREAD_H

#include <pthread.h>
#include <vector>
#include <map>

class K2hdkcThread;

//---------------------------------------------------------
// Typedefs & Structures
//---------------------------------------------------------
typedef bool (*dkcth_workproc_fp)(void* pobj, bool* piswork);				// result: true -> continue, false -> exit thread

typedef struct k2hdkcthrad_param{
	K2hdkcThread*		parent_obj;
	pthread_t			threadid;				// thread id
	int					result_code;			// result code
	long				sleep_ns;				// sleep time ns for one waiting
	long				idle_count;				// idle count by sleep time
	volatile bool		exited;					// whether exiting
	volatile bool		stop_req;				// true means thread have to exit assap
	dkcth_workproc_fp	work_proc;

	k2hdkcthrad_param() : parent_obj(NULL), threadid(0), result_code(0), sleep_ns(0), idle_count(0), exited(false), stop_req(false), work_proc(NULL) {}
	~k2hdkcthrad_param() {}

}DKCTH_PARAM, *PDKCTH_PARAM;

typedef std::vector<PDKCTH_PARAM>			k2hdkcthlist_t;
typedef std::map<pthread_t, PDKCTH_PARAM>	k2hdkcthmap_t;

//---------------------------------------------------------
// K2hdkcThread Class
//---------------------------------------------------------
class K2hdkcThread
{
	public:
		static const size_t	MIN_THREAD_COUNT			= 1;
		static const size_t	MAX_THREAD_COUNT			= 100;
		static const size_t	DEFAULT_THREAD_COUNT		= 0;
		static const long	WAITEXIT_THREAD				= 50 * 1000 * 1000;			// 50 ms
		static const long	WAITEXIT_THREAD_TRYCNT		= 30 * 20;					// 600 count (30s / 50 ms)
		static const long	WAITCOND_TIMEOUT_NS			= 1000 * 1000 * 1000;		// 1 s
		static const time_t	DEFAULT_REDUCE_TIMEOUT		= 600;						// 600 s (10 min)
		static const long	DEFAULT_REDUCE_IDLE_COUNT	= 600;						// 600 count (600s / 1s)

	protected:
		void*				pthread_paramobj;			// object pointer for worker proc parameter
		bool				is_init_cond_vals;			// flag for initialized variables
		pthread_mutex_t		cond_mutex;					// mutex for condition object
		pthread_cond_t		cond_val;					// condition object
		k2hdkcthlist_t		thread_list;				// child threads list with their parameter
		k2hdkcthmap_t		run_thread_map;				// only running thread mapping
		volatile bool		exit_notice;				// the flag for notification of exiting threads(using this value without locking)
		volatile int		list_lockval;				// lockval for threads map
		size_t				min_thread_cnt;				// minimum threads count
		size_t				max_thread_cnt;				// maximum threads count
		int64_t				free_thread_count;			// thread pool counter
		long				reduce_idlecnt;				// limit idle count for reducing thread
		dkcth_workproc_fp	auto_run_wpfp;				// worker proc pointer for running automatically

	protected:
		static void MakeLimitTime(long timens, struct timespec& limitts);
		static void* WorkerProc(void* pparam);

		bool Uninitialize(void);
		bool ExitAllThreads(void);
		size_t JoinThreads(bool& is_rest_thread);

	public:
		explicit K2hdkcThread(void* pobj = NULL);
		virtual ~K2hdkcThread();

		bool Initialize(size_t minthcnt = K2hdkcThread::MIN_THREAD_COUNT, size_t maxthcnt = K2hdkcThread::MAX_THREAD_COUNT, time_t reduce_timeout = K2hdkcThread::DEFAULT_REDUCE_TIMEOUT);
		bool Clean(void);

		ssize_t CreateThreads(dkcth_workproc_fp wpfp, size_t runcnt = K2hdkcThread::DEFAULT_THREAD_COUNT, bool set_wpfp_auto = true);
		bool CheckExitThreads(void);
		bool WakeupThreads(void);
};

#endif	// K2HDKCTHREAD_H

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noexpandtab sw=4 ts=4 fdm=marker
 * vim<600: noexpandtab sw=4 ts=4
 */
