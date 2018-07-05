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

bin_PROGRAMS	= k2hdkclinetool
noinst_PROGRAMS = 

k2hdkclinetool_SOURCES= k2hdkclinetool.cc
k2hdkclinetool_LDADD	= -L../lib/.libs -lk2hdkc $(fullock_LIBS) $(k2hash_LIBS) $(chmpx_LIBS) -lyaml

ACLOCAL_AMFLAGS	= -I m4
AM_CFLAGS		= -I../lib $(fullock_CFLAGS) $(k2hash_CFLAGS) $(chmpx_CFLAGS)
AM_CPPFLAGS		= -I../lib $(fullock_CFLAGS) $(k2hash_CFLAGS) $(chmpx_CFLAGS)

TESTS = test.sh

EXTRA_DIST =	test.sh \
				k2hdkclinetool.cmd \
				k2hdkclinetool_exit.cmd \
				k2hdkclinetool.log \
				test_server.ini \
				test_slave.ini \
				test_trans_server.ini \
				test_trans_slave.ini \
				test_server.yaml \
				test_slave.yaml \
				test_trans_server.yaml \
				test_trans_slave.yaml \
				test_server.json \
				test_slave.json \
				test_trans_server.json \
				test_trans_slave.json \
				test_json_string.data

#
# VIM modelines
#
# vim:set ts=4 fenc=utf-8:
#
