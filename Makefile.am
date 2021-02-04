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

SUBDIRS=docs lib src tests buildutils

EXTRA_DIST=RELEASE_VERSION

CPPCHECK_CMD=	cppcheck
CPPCHECK_OPT=	--quiet \
				--error-exitcode=1 \
				--inline-suppr \
				-j 4 \
				--std=c++03 \
				--xml \
				--enable=warning,style,information,missingInclude
CPPCHECK_IGN=

# [NOTE]
# Building chmpx on Github Actions(CI) is running in several OS.
# Depending on the OS(CentOS, ubuntu, debian, fedora), different
# cppcheck versions are installed. Certain versions of cppcheck
# are incompatible with the chmpx template source code and are
# very poor performance on the VM or container(CI).
# In the problematic version, cppcheck runs very slowly. To avoid
# this we skip only specific versions of cppcheck. Even if we
# skip a specific version of the cppcheck, it will be tested
# with a combination of other versions and OS.
# When the process runs on Github Actions, the CI environment is
# set true.
# 
# Currently, old debian and centos use cppcheck 1.7x(or 1.6x),
# there is a problem with these old version and skipping these.
# If the other will support 1.8x or later versions in the
# future, this skipped process will be unnecessary.
#
CPPCHECK_NGVER=	-e 1\\.7 -e 1\\.6

cppcheck:
	echo "$(CPPCHECK_CMD) $(CPPCHECK_OPT) $(CPPCHECK_IGN) $(SUBDIRS)"
	if test "X$$CI" = "Xtrue"; then \
		if ($(CPPCHECK_CMD) --version | grep $(CPPCHECK_NGVER)) >/dev/null 2>&1; then \
			echo " --> Skip, because this old $(CPPCHECK_CMD) version is very poor performance on VM(CI)"; \
		else \
			$(CPPCHECK_CMD) $(CPPCHECK_OPT) $(CPPCHECK_IGN) $(SUBDIRS); \
		fi; \
	else \
		$(CPPCHECK_CMD) $(CPPCHECK_OPT) $(CPPCHECK_IGN) $(SUBDIRS); \
	fi

#
# VIM modelines
#
# vim:set ts=4 fenc=utf-8:
#
