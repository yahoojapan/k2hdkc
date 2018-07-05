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
		bool		enable;
		uint64_t	number;

	public:
		static const uint64_t	INIT_NUMBER = 0;

	protected:
		static K2hdkcComNumber& GetSingleton(void);

		K2hdkcComNumber(void);
		virtual ~K2hdkcComNumber(void) {}

		uint64_t Increment(void);

	public:
		static void Enable(void);
		static void Disable(void);
		static bool IsEnable(void);
		static uint64_t Get(void);
};

#endif	// K2HDKCCOMNUM_H

/*
 * VIM modelines
 *
 * vim:set ts=4 fenc=utf-8:
 */
