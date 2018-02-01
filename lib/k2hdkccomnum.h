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
 * CREATE:   Wed Jul 27 2016
 * REVISION:
 * 
 */

#ifndef	K2HDKCCOMNUM_H
#define K2HDKCCOMNUM_H

//---------------------------------------------------------
// K2hdkcComNumber Class
//---------------------------------------------------------
class K2hdkcComNumber
{
	protected:
		static K2hdkcComNumber	singleton;

		bool		enable;
		uint64_t	number;

	public:
		static const uint64_t	INIT_NUMBER = 0;

	protected:
		K2hdkcComNumber(void);
		virtual ~K2hdkcComNumber(void) {}

		uint64_t Increment(void)
		{
			uint64_t	oldval;
			uint64_t	newval;
			uint64_t	resultval;
			do{
				oldval = K2hdkcComNumber::singleton.number;
				newval = oldval + 1;
			}while(oldval != (resultval = __sync_val_compare_and_swap(&(K2hdkcComNumber::singleton.number), oldval, newval)));
			return newval;
		}

	public:
		static void Enable(void) { K2hdkcComNumber::singleton.enable = true; }
		static void Disable(void) { K2hdkcComNumber::singleton.enable = false; }
		static bool IsEnable(void) { return K2hdkcComNumber::singleton.enable; }
		static uint64_t Get(void)
		{
			if(!K2hdkcComNumber::singleton.enable){
				return 0;
			}
			return K2hdkcComNumber::singleton.Increment();
		}
};

#endif	// K2HDKCCOMNUM_H

/*
 * VIM modelines
 *
 * vim:set ts=4 fenc=utf-8:
 */
