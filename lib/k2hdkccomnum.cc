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

#include <stdlib.h>
#include <stdio.h>

#include <k2hash/k2hutil.h>

#include "k2hdkccommon.h"
#include "k2hdkccomnum.h"

//---------------------------------------------------------
// Class Variables
//---------------------------------------------------------
K2hdkcComNumber	K2hdkcComNumber::singleton;
const uint64_t	K2hdkcComNumber::INIT_NUMBER;

//---------------------------------------------------------
// Methods
//---------------------------------------------------------
K2hdkcComNumber::K2hdkcComNumber(void) : enable(false), number(0)
{
	int	seed = static_cast<int>((gettid() & 0xFFFF) << 16) | static_cast<int>(time(NULL) & 0xFFFF);
	srandom(seed);
	number = (static_cast<uint64_t>(random()) << 32) & 0xFFFFFFFF00000000;
}

/*
 * VIM modelines
 *
 * vim:set ts=4 fenc=utf-8:
 */
