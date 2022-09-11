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
# CREATE:   Mon Jul 11 2016
# REVISION:
#

pkginclude_HEADERS	=	k2hdkccommon.h \
						k2hdkctypes.h \
						k2hdkcdbg.h \
						k2hdkccomnum.h \
						k2hdkcutil.h \
						k2hdkc.h \
						k2hdkccom.h \
						k2hdkccomstructure.h \
						k2hdkccomdbg.h \
						k2hdkccombase.h \
						k2hdkccomget.h \
						k2hdkccomgetdirect.h \
						k2hdkccomgetsubkeys.h \
						k2hdkccomgetattrs.h \
						k2hdkccomgetattr.h \
						k2hdkccomset.h \
						k2hdkccomsetdirect.h \
						k2hdkccomsetsubkeys.h \
						k2hdkccomsetall.h \
						k2hdkccomaddsubkey.h \
						k2hdkccomaddsubkeys.h \
						k2hdkccomdel.h \
						k2hdkccomdelsubkeys.h \
						k2hdkccomdelsubkey.h \
						k2hdkccomqpush.h \
						k2hdkccomqpop.h \
						k2hdkccomqdel.h \
						k2hdkccomren.h \
						k2hdkccomcasinit.h \
						k2hdkccomcasget.h \
						k2hdkccomcasset.h \
						k2hdkccomcasincdec.h \
						k2hdkccomreplkey.h \
						k2hdkccomk2hstate.h \
						k2hdkccomstate.h \
						k2hdkccomrepldel.h \
						k2hdkcthread.h \
						k2hdkcslave.h \
						k2hdkccomutil.h

pkgincludedir		= $(includedir)/k2hdkc

EXTRA_DIST			=

pkgconfigdir		= $(libdir)/pkgconfig
pkgconfig_DATA		= libk2hdkc.pc
DISTCLEANFILES		= $(pkgconfig_DATA)

lib_LTLIBRARIES		= libk2hdkc.la
libk2hdkc_la_SOURCES=	k2hdkcdbg.cc \
						k2hdkccomnum.cc \
						k2hdkcutil.cc \
						k2hdkc.cc \
						k2hdkccombase.cc \
						k2hdkccomget.cc \
						k2hdkccomgetdirect.cc \
						k2hdkccomgetsubkeys.cc \
						k2hdkccomgetattrs.cc \
						k2hdkccomgetattr.cc \
						k2hdkccomset.cc \
						k2hdkccomsetdirect.cc \
						k2hdkccomsetsubkeys.cc \
						k2hdkccomsetall.cc \
						k2hdkccomaddsubkey.cc \
						k2hdkccomaddsubkeys.cc \
						k2hdkccomdel.cc \
						k2hdkccomdelsubkeys.cc \
						k2hdkccomdelsubkey.cc \
						k2hdkccomqpush.cc \
						k2hdkccomqpop.cc \
						k2hdkccomqdel.cc \
						k2hdkccomren.cc \
						k2hdkccomcasinit.cc \
						k2hdkccomcasget.cc \
						k2hdkccomcasset.cc \
						k2hdkccomcasincdec.cc \
						k2hdkccomreplkey.cc \
						k2hdkccomk2hstate.cc \
						k2hdkccomstate.cc \
						k2hdkccomrepldel.cc \
						k2hdkcthread.cc \
						k2hdkcslave.cc \
						k2hdkcversion.cc

libk2hdkc_la_LDFLAGS= -version-info $(LIB_VERSION_INFO)
libk2hdkc_la_LIBADD	= $(fullock_LIBS) $(k2hash_LIBS) $(chmpx_LIBS)

ACLOCAL_AMFLAGS		= -I m4

# [NOTE]
# "-Waddress-of-packed-member" optsion was introduced by default
# from GCC 9.
# Knowing that packed structure is CPU architecture dependent,
# this program ignores this warning.
#
AM_CFLAGS			= $(fullock_CFLAGS) $(k2hash_CFLAGS) $(chmpx_CFLAGS) -Wno-address-of-packed-member
AM_CPPFLAGS			= $(fullock_CFLAGS) $(k2hash_CFLAGS) $(chmpx_CFLAGS) -Wno-address-of-packed-member

### version(commit hash)
.PHONY:	k2hdkcxversion

k2hdkcversion.cc:	k2hdkcxversion
	@../buildutils/make_commit_hash_source.sh k2hdkcversion.cc k2hdkc_commit_hash

#
# Local variables:
# tab-width: 4
# c-basic-offset: 4
# End:
# vim600: noexpandtab sw=4 ts=4 fdm=marker
# vim<600: noexpandtab sw=4 ts=4
#
