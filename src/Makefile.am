#
# K2HDKC
# 
# Copyright 2016 Yahoo Japan Corporation.
# 
# K2HDKC is k2hash based distributed KVS cluster.
# K2HDKC uses K2HASH, CHMPX, FULLOCK libraries. K2HDKC supports
# distributed KVS cluster server program and client libraries.
# 
# For the full copyright and license information, please view
# the license file that was distributed with this source code.
#
# AUTHOR:   Takeshi Nakatani
# CREATE:   Thu Mar 31 2016
# REVISION:
#

bin_PROGRAMS	= k2hdkc

k2hdkc_SOURCES	= k2hdkcmain.cc k2hdkccntrl.h k2hdkccntrl.cc k2hdkcconfparser.h k2hdkcconfparser.cc k2hdkcopts.h k2hdkcopts.cc
k2hdkc_LDADD	= -L../lib/.libs -lk2hdkc $(fullock_LIBS) $(k2hash_LIBS) $(chmpx_LIBS) -lyaml

ACLOCAL_AMFLAGS	= -I m4
AM_CFLAGS		= -I../lib $(fullock_CFLAGS) $(k2hash_CFLAGS) $(chmpx_CFLAGS)
AM_CPPFLAGS		= -I../lib $(fullock_CFLAGS) $(k2hash_CFLAGS) $(chmpx_CFLAGS)

#
# Local variables:
# tab-width: 4
# c-basic-offset: 4
# End:
# vim600: noexpandtab sw=4 ts=4 fdm=marker
# vim<600: noexpandtab sw=4 ts=4
#
