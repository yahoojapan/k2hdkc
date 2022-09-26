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
 * CREATE:   Thu Mar 31 2016
 * REVISION:
 * 
 */

#ifndef	K2HDKCOPTS_H
#define	K2HDKCOPTS_H

#include <chmpx/chmutil.h>

//---------------------------------------------------------
// K2hdkcOpts Class
//---------------------------------------------------------
class K2hdkcOpts
{
	protected:
		strlstmap_t	optmap;

	public:
		explicit K2hdkcOpts(int argc = 0, char** argv = NULL);
		virtual ~K2hdkcOpts(void);

		bool Initialize(int argc, char** argv);
		bool Get(const char* popt, std::string& param);
		bool Get(const char* popt, strlst_t& params);
		bool Find(const char* popt) const;
		long Count(void) const { return static_cast<long>(optmap.size()); }
};

#endif	// K2HDKCOPTS_H

/*
 * VIM modelines
 *
 * vim:set ts=4 fenc=utf-8:
 */
