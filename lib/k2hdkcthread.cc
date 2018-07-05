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

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <fullock/fullock.h>
#include <fullock/flckstructure.h>
#include <fullock/flckbaselist.tcc>

#include "k2hdkccommon.h"
#include "k2hdkcthread.h"
#include "k2hdkcutil.h"
#include "k2hdkcdbg.h"

using namespace	std;

//---------------------------------------------------------
// Symbols
//---------------------------------------------------------
#ifndef CLOCK_REALTIME_COARSE
#define	CLOCK_REALTIME_COARSE		CLOCK_REALTIME
#endif

//---------------------------------------------------------
// Utility
//---------------------------------------------------------
inline void increment_counter(int64_t& counter)
{
	int64_t	beforeval;
	do{
		beforeval = counter;
	}while(beforeval != __sync_val_compare_and_swap(&counter, beforeval, beforeval + 1) && -1 <= sched_yield());
}

inline void decrement_counter(int64_t& counter)
{
	int64_t	beforeval;
	do{
		beforeval = counter;
	}while(beforeval != __sync_val_compare_and_swap(&counter, beforeval, beforeval - 1) && -1 <= sched_yield());
}

//---------------------------------------------------------
// Class Variable
//---------------------------------------------------------
const size_t	K2hdkcThread::MIN_THREAD_COUNT;
const size_t	K2hdkcThread::MAX_THREAD_COUNT;
const size_t	K2hdkcThread::DEFAULT_THREAD_COUNT;
const long		K2hdkcThread::WAITEXIT_THREAD;
const long		K2hdkcThread::WAITEXIT_THREAD_TRYCNT;
const long		K2hdkcThread::WAITCOND_TIMEOUT_NS;
const time_t	K2hdkcThread::DEFAULT_REDUCE_TIMEOUT;
const long		K2hdkcThread::DEFAULT_REDUCE_IDLE_COUNT;

//---------------------------------------------------------
// Class Method
//---------------------------------------------------------
//
// Worker proc
//
void* K2hdkcThread::WorkerProc(void* pparam)
{
	PDKCTH_PARAM	pthparam = reinterpret_cast<PDKCTH_PARAM>(pparam);

	// check
	if(!pthparam || !pthparam->parent_obj || !pthparam->parent_obj->pthread_paramobj || !pthparam->work_proc || pthparam->sleep_ns <= 0){
		ERR_DKCPRN("Could not run worker thread(%lu), some parameter is wrong.", pthread_self());
		pthread_exit(NULL);
	}
	MSG_DKCPRN("Worker thread(%ld) start up now.", pthread_self());

	// loop
	struct timespec	abstimeout;
	int				condres;
	int				unlockres;
	for(pthparam->result_code = 0, pthparam->idle_count = 0; !pthparam->stop_req; ){
		// set new abstimeout
		make_current_timespec_with_mergin(pthparam->sleep_ns, abstimeout);

		// wait condition event
		if(0 != (condres = pthread_mutex_lock(&(pthparam->parent_obj->cond_mutex)))){
			ERR_DKCPRN("Could not lock mutex for condition by error code(%d).", condres);
			pthparam->result_code = condres;
			break;
		}
		condres		= pthread_cond_timedwait(&(pthparam->parent_obj->cond_val), &(pthparam->parent_obj->cond_mutex), &abstimeout);
		unlockres	= pthread_mutex_unlock(&(pthparam->parent_obj->cond_mutex));
		if(0 != unlockres){
			ERR_DKCPRN("Could not unlock mutex for condition by error code(%d).", unlockres);
			pthparam->result_code = unlockres;
			break;
		}

		// check flag, for exiting assap
		if(pthparam->stop_req){
			break;
		}

		// do work/timeout
		if(ETIMEDOUT == condres){
			// timeouted
			++(pthparam->idle_count);

		}else if(EINTR == condres){
			// signal break
			//MSG_DKCPRN("Waiting condition is broken by signal, so continue to wait.");

		}else if(0 != condres){
			// something error occurred, but continue...
			ERR_DKCPRN("Waiting condition is broken by something error occurred(return code=%d), but continue to wait.", condres);
			pthparam->result_code = condres;
			break;

		}else{
			// do work
			decrement_counter(pthparam->parent_obj->free_thread_count);				// decrement free thread count

			bool	is_work = true;
			if(!pthparam->work_proc(pthparam->parent_obj->pthread_paramobj, &is_work)){
				increment_counter(pthparam->parent_obj->free_thread_count);			// increment free thread count
				MSG_DKCPRN("Exit worker proc request after calling worker function(function is %sworking).", is_work ? "" : "not ");
				pthparam->result_code = 0;
				break;
			}
			increment_counter(pthparam->parent_obj->free_thread_count);				// increment free thread count

			//MSG_DKCPRN("Finish calling worker function(function is %sworking).", is_work ? "" : "not ");

			if(is_work){
				pthparam->idle_count= 0;
			}else{
				++(pthparam->idle_count);
			}
		}

		// check idle count
		if(0 < pthparam->idle_count && pthparam->parent_obj->reduce_idlecnt < pthparam->idle_count){
			// [NOTE]
			// Do only check when that can be locked
			//
			if(fullock::flck_trylock_noshared_mutex(&(pthparam->parent_obj->list_lockval))){		// LOCK
				if(pthparam->parent_obj->min_thread_cnt < pthparam->parent_obj->run_thread_map.size() && 1 < pthparam->parent_obj->run_thread_map.size()){
					// remove this
					MSG_DKCPRN("This thread is idle count(%zu), and thread count in parent is over minimum count. so this thread is going to exit now.", pthparam->idle_count);

					pthparam->parent_obj->run_thread_map.erase(pthparam->threadid);
					pthparam->result_code = 0;

					fullock::flck_unlock_noshared_mutex(&(pthparam->parent_obj->list_lockval));		// UNLOCK
					break;
				}
				fullock::flck_unlock_noshared_mutex(&(pthparam->parent_obj->list_lockval));			// UNLOCK
			}
		}
	}
	decrement_counter(pthparam->parent_obj->free_thread_count);						// decrement free thread count

	// set notice for exiting
	while(!fullock::flck_trylock_noshared_mutex(&(pthparam->parent_obj->list_lockval)));	// LOCK

	k2hdkcthmap_t::iterator	miter = pthparam->parent_obj->run_thread_map.find(pthparam->threadid);
	if(miter != pthparam->parent_obj->run_thread_map.end()){
		pthparam->parent_obj->run_thread_map.erase(miter);
	}
	fullock::flck_unlock_noshared_mutex(&(pthparam->parent_obj->list_lockval));				// UNLOCK

	// set status and notification
	pthparam->exited					= true;
	pthparam->parent_obj->exit_notice	= true;												// [NOTE] change without locking

	MSG_DKCPRN("Worker thread(%ld) exit now.", pthread_self());
	pthread_exit(NULL);

	return NULL;
}

//---------------------------------------------------------
// Constructor/Destructor
//---------------------------------------------------------
K2hdkcThread::K2hdkcThread(void* pobj) :
	pthread_paramobj(pobj), is_init_cond_vals(false), list_lockval(FLCK_NOSHARED_MUTEX_VAL_UNLOCKED),
	min_thread_cnt(K2hdkcThread::MIN_THREAD_COUNT), max_thread_cnt(K2hdkcThread::MAX_THREAD_COUNT), free_thread_count(0),
	reduce_idlecnt(K2hdkcThread::DEFAULT_REDUCE_IDLE_COUNT), auto_run_wpfp(NULL)
{
	assert(pthread_paramobj);

	// initliaze condition values
	pthread_mutex_init(&cond_mutex, NULL);

	if(0 != pthread_cond_init(&cond_val, NULL)){
		ERR_DKCPRN("Failed to initialize condition variable.");
	}else{
		is_init_cond_vals = true;
	}
}

K2hdkcThread::~K2hdkcThread(void)
{
	Clean();
}

//---------------------------------------------------------
// Methods
//---------------------------------------------------------
//
// Initialize all variables
//
bool K2hdkcThread::Initialize(size_t minthcnt, size_t maxthcnt, time_t reduce_timeout)
{
	if(minthcnt < K2hdkcThread::MIN_THREAD_COUNT || K2hdkcThread::MAX_THREAD_COUNT < maxthcnt || maxthcnt < minthcnt){
		ERR_DKCPRN("Parameters are wrong.");
		return false;
	}
	if(!is_init_cond_vals){
		ERR_DKCPRN("Sync cond objects variables is not initialized yet, so this is fatal.");
		return false;
	}
	// set thread count
	min_thread_cnt		= minthcnt;
	max_thread_cnt		= maxthcnt;
	free_thread_count	= 0;
	reduce_idlecnt		= static_cast<long>(static_cast<uint64_t>(reduce_timeout * 1000 * 1000 * 1000) / static_cast<uint64_t>(K2hdkcThread::WAITCOND_TIMEOUT_NS));

	return true;
}

//
// Uninitialize sync objects.
//
bool K2hdkcThread::Uninitialize(void)
{
	if(!is_init_cond_vals){
		WAN_DKCPRN("Sync cond objects variables is not initialized or already destroied, so do not need to uninitialize.");
		return true;
	}
	bool	result = true;
	if(0 != pthread_cond_destroy(&cond_val)){
		ERR_DKCPRN("Failed to destroy condition variable, but continue...");
		result = false;
	}
	if(0 != pthread_mutex_destroy(&cond_mutex)){
		ERR_DKCPRN("Failed to destroy condition mutex variable, but continue...");
		result = false;
	}
	is_init_cond_vals = false;

	return result;
}

//
// Clean
//
bool K2hdkcThread::Clean(void)
{
	bool	result = true;

	// stop all threads
	if(!ExitAllThreads()){
		ERR_DKCPRN("Failed to stop all threads, but continue...");
		result = false;
	}
	if(!Uninitialize()){
		ERR_DKCPRN("Failed to uninitialize object, but continue...");
		result = false;
	}
	return result;
}

//
// Create(Adds) threads
//
ssize_t K2hdkcThread::CreateThreads(dkcth_workproc_fp wpfp, size_t runcnt, bool set_wpfp_auto)
{
	if(!wpfp){
		ERR_DKCPRN("Parameters are wrong.");
		return -1;
	}
	if(!is_init_cond_vals){
		ERR_DKCPRN("Sync cond objects variables is not initialized yet, so this is fatal.");
		return false;
	}
	if(set_wpfp_auto){
		auto_run_wpfp = wpfp;
	}

	fullock::flck_lock_noshared_mutex(&list_lockval);			// LOCK

	// check maximum
	size_t	now_thread_cnt = run_thread_map.size();
	if(max_thread_cnt <= now_thread_cnt){
		// already run maximum threads
		WAN_DKCPRN("Already run maximum threads(%zu), so do not run no more threads.", now_thread_cnt);
		fullock::flck_unlock_noshared_mutex(&list_lockval);		// UNLOCK
		return 0;
	}
	if(K2hdkcThread::DEFAULT_THREAD_COUNT == runcnt){
		runcnt = max_thread_cnt - now_thread_cnt;
	}

	// create thread loop
	PDKCTH_PARAM	pthparam;
	size_t			cnt;
	for(cnt = 0; cnt < runcnt && (cnt + now_thread_cnt) < max_thread_cnt; ++cnt){
		// thread parameter
		pthparam					= new DKCTH_PARAM;
		pthparam->parent_obj		= this;
		pthparam->threadid			= 0;
		pthparam->sleep_ns			= K2hdkcThread::WAITCOND_TIMEOUT_NS;
		pthparam->idle_count		= 0;
		pthparam->exited			= false;
		pthparam->stop_req			= false;
		pthparam->work_proc			= wpfp;

		// create thread
		int	result;
		if(0 != (result = pthread_create(&(pthparam->threadid), NULL, K2hdkcThread::WorkerProc, pthparam))){
			ERR_DKCPRN("Failed to create thread(%zu/%zu). return code(error) = %d", cnt, runcnt, result);
			DKC_DELETE(pthparam);
			fullock::flck_unlock_noshared_mutex(&list_lockval);	// UNLOCK
			return static_cast<ssize_t>(cnt);					// break loop
		}
		// add list and map
		thread_list.push_back(pthparam);
		run_thread_map[pthparam->threadid] = pthparam;

		increment_counter(free_thread_count);					// increment free thread count
	}
	fullock::flck_unlock_noshared_mutex(&list_lockval);			// UNLOCK

	return static_cast<ssize_t>(cnt);
}

//
// Force exit all threads
//
bool K2hdkcThread::ExitAllThreads(void)
{
	if(thread_list.empty()){
		MSG_DKCPRN("There is no child thread, so return success assap.");
		return true;
	}

	// loop for all thread exiting
	struct timespec	sleeptime		= {0, K2hdkcThread::WAITEXIT_THREAD};		// 50 ms
	bool 			is_rest_thread	= true;
	for(long cnt = 0; cnt < K2hdkcThread::WAITEXIT_THREAD_TRYCNT && is_rest_thread; ++cnt){
		// set exit request flag to all threads.
		for(k2hdkcthlist_t::iterator iter = thread_list.begin(); iter != thread_list.end(); ++iter){
			if(*iter){
				(*iter)->stop_req = true;
			}
		}

		// wakeup all thread
		int		condres;
		bool	locked_mutex = true;
		if(0 != (condres = pthread_mutex_lock(&cond_mutex))){
			ERR_DKCPRN("Could not lock mutex for condition by error code(%d), but continue...", condres);
			locked_mutex = false;
		}
		if(0 != (condres = pthread_cond_broadcast(&cond_val))){
			ERR_DKCPRN("Could not broadcast cond(return code = %d), but continue...", condres);
		}
		if(locked_mutex){
			pthread_mutex_unlock(&cond_mutex);
		}

		// sleep
		nanosleep(&sleeptime, NULL);

		// join threads
		size_t	exited_count = JoinThreads(is_rest_thread);
		MSG_DKCPRN("Wait loop for thread exiting(%ld count): Exit threads count is %zu, rest threads is %zu", cnt, exited_count, thread_list.size());
	}

	if(0 < thread_list.size()){
		WAN_DKCPRN("Gave up waiting(%ld ms * %ld count) for exiting all threads(rest %zu threads).", (K2hdkcThread::WAITEXIT_THREAD / (1000 * 1000)), K2hdkcThread::WAITEXIT_THREAD_TRYCNT, thread_list.size());
		return false;
	}
	return true;
}

//
// After processing for exiting threads
//
bool K2hdkcThread::CheckExitThreads(void)
{
	if(exit_notice){				// [NOTE] check and set this value without locking
		exit_notice = false;		// reset

		bool	is_rest_thread	= false;
		size_t	exitcnt			= JoinThreads(is_rest_thread);
		MSG_DKCPRN("Exited thread count is %zu, there is %s rest thread after checking.", exitcnt, (is_rest_thread ? "some" : "no"));
	}
	return true;
}

//
// Only join threads if there are exited threads.
//
size_t K2hdkcThread::JoinThreads(bool& is_rest_thread)
{
	size_t	exited_count = 0;

	for(k2hdkcthlist_t::iterator iter = thread_list.begin(); iter != thread_list.end(); ){
		if(!(*iter)){
			// why?
			ERR_DKCPRN("Found null entry in thread list, so remove it.");
			++exited_count;										// OK?
			iter = thread_list.erase(iter);
		}else{
			if((*iter)->exited){
				// found already finished thread
				int	result;
				if(0 != (result = pthread_join((*iter)->threadid, NULL))){
					ERR_DKCPRN("Failed to wait exiting thread(return code = %d), so do not retrive this, and continue...", result);
					++iter;
				}else{
					MSG_DKCPRN("Succeed to exit thread.");
					DKC_DELETE(*iter);
					iter = thread_list.erase(iter);
					++exited_count;
				}
			}else{
				++iter;
			}
		}
	}
	is_rest_thread = !thread_list.empty();

	return exited_count;
}

//
// Wakeup one thread.
//
bool K2hdkcThread::WakeupThreads(void)
{
	if(!is_init_cond_vals){
		ERR_DKCPRN("Sync cond objects variables is not initialized yet, so this is fatal.");
		return false;
	}

	// check free thread count(without locking)
	if(free_thread_count <= 0){
		if(thread_list.size() < max_thread_cnt && auto_run_wpfp){
			// [NOTE]
			// there is no free thread, but running thread count is under maximum thread count.
			// So run new thread here.
			//
			if(1 != CreateThreads(auto_run_wpfp, 1, false)){
				ERR_DKCPRN("Could not run new thread, but continue...");
			}
		}
	}

	// wakeup one thread
	int		condres;
	if(0 != (condres = pthread_mutex_lock(&cond_mutex))){
		ERR_DKCPRN("Could not lock mutex for condition by error code(%d).", condres);
		return false;
	}
	if(0 != (condres = pthread_cond_broadcast(&cond_val))){			// Wakeup all thread
		ERR_DKCPRN("Could not signal cond(return code = %d).", condres);
		pthread_mutex_unlock(&cond_mutex);
		return false;
	}
	pthread_mutex_unlock(&cond_mutex);

	return true;
}

/*
 * VIM modelines
 *
 * vim:set ts=4 fenc=utf-8:
 */
