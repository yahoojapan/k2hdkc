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

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <k2hash/k2hutil.h>

#include "k2hdkccommon.h"
#include "k2hdkccomnum.h"

//---------------------------------------------------------
// Class Variables
//---------------------------------------------------------
const uint64_t	K2hdkcComNumber::INIT_NUMBER;

//---------------------------------------------------------
// Class Methods
//---------------------------------------------------------
K2hdkcComNumber& K2hdkcComNumber::GetSingleton(void)
{
	static K2hdkcComNumber	singleton;			// singleton
	return singleton;
}

void K2hdkcComNumber::Enable(void)
{
	K2hdkcComNumber::GetSingleton().enable = true;
}

void K2hdkcComNumber::Disable(void)
{
	K2hdkcComNumber::GetSingleton().enable = false;
}

bool K2hdkcComNumber::IsEnable(void)
{
	return K2hdkcComNumber::GetSingleton().enable;
}

uint64_t K2hdkcComNumber::Get(void)
{
	if(!K2hdkcComNumber::GetSingleton().enable){
		return 0;
	}
	return K2hdkcComNumber::GetSingleton().Increment();
}

//---------------------------------------------------------
// Methods
//---------------------------------------------------------
K2hdkcComNumber::K2hdkcComNumber(void) : enable(false), number(0)
{
	int	seed = static_cast<int>((gettid() & 0xFFFF) << 16) | static_cast<int>(time(NULL) & 0xFFFF);
	srandom(seed);
	number = (static_cast<uint64_t>(random()) << 32) & 0xFFFFFFFF00000000;
}

uint64_t K2hdkcComNumber::Increment(void)
{
	uint64_t	oldval;
	uint64_t	newval;
	uint64_t	resultval;
	do{
		oldval = K2hdkcComNumber::GetSingleton().number;
		newval = oldval + 1;
	}while(oldval != (resultval = __sync_val_compare_and_swap(&(K2hdkcComNumber::GetSingleton().number), oldval, newval)));
	return newval;
}

/*
 * VIM modelines
 *
 * vim:set ts=4 fenc=utf-8:
 */
