#!/bin/sh
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
#

###########################################################
## Common Variables
###########################################################
PROGRAM_NAME=`basename $0`
DATE=`date`
PROCID=$$
LOGFILE="/tmp/test_sh_$PROCID.log"
CHMPXLOG="/tmp/test_chmpx_$PROCID"
K2HDKCLOG="/tmp/test_k2hdkc_$PROCID"
LINETOOLLOG="/tmp/test_linetool_$PROCID.log"
LINETOOLEXITLOG="/tmp/test_linetool_exit_$PROCID.log"
LINETOOLERRLOG="/tmp/test_linetool_$PROCID"

###########################################################
## Parameter
###########################################################
CHMPX_DBG_PARAM="-d err"
DKC_DBG_PARAM="-d err"
DKC_COMLOG_PARAM=""
DO_INI_CONF=no
DO_YAML_CONF=no
DO_JSON_CONF=no
DO_JSON_STRING=no
DO_JSON_ENV=no
IS_FOUND_TEST_TYPE_OPT=no
while [ $# -ne 0 ]; do
	if [ "X$1" = "X-h" -o "X$1" = "X-help" ]; then
		echo "Usage: ${PROGRAM_NAME} [-h] [ ini_conf & yaml_conf & json_conf & json_string & json_env ] [-chmpx_dbg_level <err|wan|msg|dump>] [-dkc_dbg_level <err|wan|msg|dump>] [-dkccomlog] [-logfile <filepath>]"
		exit 0

	elif [ "X$1" = "Xini_conf" ]; then
		DO_INI_CONF=yes
		IS_FOUND_TEST_TYPE_OPT=yes

	elif [ "X$1" = "Xyaml_conf" ]; then
		DO_YAML_CONF=yes
		IS_FOUND_TEST_TYPE_OPT=yes

	elif [ "X$1" = "Xjson_conf" ]; then
		DO_JSON_CONF=yes
		IS_FOUND_TEST_TYPE_OPT=yes

	elif [ "X$1" = "Xjson_string" ]; then
		DO_JSON_STRING=yes
		IS_FOUND_TEST_TYPE_OPT=yes

	elif [ "X$1" = "Xjson_env" ]; then
		DO_JSON_ENV=yes
		IS_FOUND_TEST_TYPE_OPT=yes

	elif [ "X$1" = "X-chmpx_dbg_level" ]; then
		shift
		CHMPX_DBG_PARAM="-d $1"

	elif [ "X$1" = "X-dkc_dbg_level" ]; then
		shift
		DKC_DBG_PARAM="-d $1"

	elif [ "X$1" = "X-logfile" ]; then
		shift
		LOGFILE="$1"

	elif [ "X$1" = "X-dkccomlog" ]; then
		DKC_COMLOG_PARAM="-comlog"

	else
		echo "ERROR: Unknown option $1"
		echo "Usage: ${PROGRAM_NAME} [-chmpx_dbg_level <err|wan|msg|dump>] [-dkc_dbg_level <err|wan|msg|dump>] [-dkccomlog] [-logfile <filepath>]"
		exit 1
	fi

	shift
done

if [ "X${IS_FOUND_TEST_TYPE_OPT}" = "Xno" ]; then
	DO_INI_CONF=yes
	DO_YAML_CONF=yes
	DO_JSON_CONF=yes
	DO_JSON_STRING=yes
	DO_JSON_ENV=yes
fi

#
# Always use this parameter for k2hdkclinetool.
#
DKCTOOL_COMLOG_PARAM="-comlog off"
DKCTOOL_DBG_PARAM="-d err"

echo "================= $DATE ====================" > ${LOGFILE}

##############################################################
## library path & programs path
###########################################################
#SCRIPTNAME=$(basename "${0}")
SCRIPTDIR=$(dirname "${0}")
SCRIPTDIR=$(cd "${SCRIPTDIR}" || exit 1; pwd)
SRCTOP=$(cd "${SCRIPTDIR}/.." || exit 1; pwd)
TESTSDIR=$(cd "${SRCTOP}/tests" || exit 1; pwd)
SRCDIR=$(cd "${SRCTOP}/src" || exit 1; pwd)
LIBDIR=$(cd "${SRCTOP}/lib" || exit 1; pwd)

if [ "X${OBJDIR}" = "X" ]; then
	LD_LIBRARY_PATH="${LIBDIR}/.libs"
else
	LD_LIBRARY_PATH="${LIBDIR}/${OBJDIR}"
fi
export LD_LIBRARY_PATH

if [ -f ${SRCDIR}/${PLATFORM_CURRENT}/k2hdkc ]; then
	K2HDKCBIN=${SRCDIR}/${PLATFORM_CURRENT}/k2hdkc
elif [ -f ${SRCDIR}/${PLATFORM_CURRENT}/k2hdkcmain ]; then
	K2HDKCBIN=${SRCDIR}/${PLATFORM_CURRENT}/k2hdkcmain
elif [ -f ${SRCDIR}/${OBJDIR}/k2hdkc ]; then
	K2HDKCBIN=${SRCDIR}/${OBJDIR}/k2hdkc
elif [ -f ${SRCDIR}/${OBJDIR}/k2hdkcmain ]; then
	K2HDKCBIN=${SRCDIR}/${OBJDIR}/k2hdkcmain
else
	echo "ERROR: there is no k2hdkc binary"
	echo "FAILED"
	exit 1
fi
if [ -f ${TESTSDIR}/${PLATFORM_CURRENT}/k2hdkclinetool ]; then
	LINETOOLBIN=${TESTSDIR}/${PLATFORM_CURRENT}/k2hdkclinetool
elif [ -f ${TESTSDIR}/${OBJDIR}/k2hdkclinetool ]; then
	LINETOOLBIN=${TESTSDIR}/${OBJDIR}/k2hdkclinetool
else
	echo "ERROR: there is no k2hdkclinetool binary"
	echo "FAILED"
	exit 1
fi
LINETOOLCMD=${TESTSDIR}/k2hdkclinetool.cmd
LINETOOLEXITCMD=${TESTSDIR}/k2hdkclinetool_exit.cmd
MASTER_LINETOOLLOG=${TESTSDIR}/k2hdkclinetool.log

###########################################################
## Json Parameter
###########################################################
TEST_SERVER_JSON=`grep 'TEST_SERVER_JSON_STR=' ${TESTSDIR}/test_json_string.data 2>/dev/null | sed 's/TEST_SERVER_JSON_STR=//g' 2>/dev/null`
TEST_SLAVE_JSON=`grep 'TEST_SLAVE_JSON_STR=' ${TESTSDIR}/test_json_string.data 2>/dev/null | sed 's/TEST_SLAVE_JSON_STR=//g' 2>/dev/null`
TEST_TRANS_SERVER_JSON=`grep 'TEST_TRANS_SERVER_JSON_STR=' ${TESTSDIR}/test_json_string.data 2>/dev/null | sed 's/TEST_TRANS_SERVER_JSON_STR=//g' 2>/dev/null`
TEST_TRANS_SLAVE_JSON=`grep 'TEST_TRANS_SLAVE_JSON_STR=' ${TESTSDIR}/test_json_string.data 2>/dev/null | sed 's/TEST_TRANS_SLAVE_JSON_STR=//g' 2>/dev/null`

#
# Clean old logs
#
rm -f ${LOGFILE}
rm -f ${LINETOOLLOG}
rm -f ${LINETOOLEXITLOG}
rm -f ${CHMPXLOG}_8021.log
rm -f ${CHMPXLOG}_8023.log
rm -f ${CHMPXLOG}_8025.log
rm -f ${CHMPXLOG}_8027.log
rm -f ${CHMPXLOG}_8031.log
rm -f ${K2HDKCLOG}_8021.log
rm -f ${K2HDKCLOG}_8023.log
rm -f ${K2HDKCLOG}_8025.log
rm -f ${K2HDKCLOG}_8027.log
rm -f ${LINETOOLERRLOG}_C_P.log
rm -f ${LINETOOLERRLOG}_CPP_P.log
rm -f ${LINETOOLERRLOG}_C.log
rm -f ${LINETOOLERRLOG}_CPP.log

#
# Initialize Result
#
TEST_RESULT=0

###########################################################
## Start INI conf file(execution)
###########################################################
if [ ${TEST_RESULT} -eq 0 -a "X${DO_INI_CONF}" = "Xyes" ]; then
	echo ""
	echo "====== START TEST FOR INI CONF FILE (EXECUTION) ============"
	echo ""
	CONF_FILE_EXT=".ini"

	########################################
	#
	# RUN chmpx processes and k2hdkc server processes
	#
	echo "======= [INI CONF(EXECUTION)] RUN CHMPX(server/slave) / K2HDKC" >> ${LOGFILE}

	echo -n "------- CHMPX(server) 8021             : " >> ${LOGFILE}
	chmpx -conf ${TESTSDIR}/test_server${CONF_FILE_EXT} -ctlport 8021 ${CHMPX_DBG_PARAM} > ${CHMPXLOG}_8021.log 2>&1 &
	CHMPX_8021_PID=$!
	echo "${CHMPX_8021_PID}" >> ${LOGFILE}
	sleep 2

	echo -n "------- K2HDKC 8021                    : " >> ${LOGFILE}
	${K2HDKCBIN} -conf ${TESTSDIR}/test_server${CONF_FILE_EXT} -ctlport 8021 ${DKC_DBG_PARAM} ${DKC_COMLOG_PARAM} > ${K2HDKCLOG}_8021.log 2>&1 &
	K2HDKC_8021_PID=$!
	sleep 1
	K2HDKC_LT_8021_PID=`ps ax | grep 'lt-k2hdkc' | grep 8021 | grep -v grep | awk '{print $1}'`
	if [ "X${K2HDKC_LT_8021_PID}" = "X" ]; then
		K2HDKC_LT_8021_PID=${K2HDKC_8021_PID}
	fi
	echo "${K2HDKC_8021_PID}, ${K2HDKC_LT_8021_PID}" >> ${LOGFILE}

	echo -n "------- CHMPX(server) 8023             : " >> ${LOGFILE}
	chmpx -conf ${TESTSDIR}/test_server${CONF_FILE_EXT} -ctlport 8023 ${CHMPX_DBG_PARAM} > ${CHMPXLOG}_8023.log 2>&1 &
	CHMPX_8023_PID=$!
	echo "${CHMPX_8023_PID}" >> ${LOGFILE}
	sleep 2

	echo -n "------- K2HDKC 8023                    : " >> ${LOGFILE}
	${K2HDKCBIN} -conf ${TESTSDIR}/test_server${CONF_FILE_EXT} -ctlport 8023 ${DKC_DBG_PARAM} ${DKC_COMLOG_PARAM} > ${K2HDKCLOG}_8023.log 2>&1 &
	K2HDKC_8023_PID=$!
	sleep 1
	K2HDKC_LT_8023_PID=`ps ax | grep 'lt-k2hdkc' | grep 8023 | grep -v grep | awk '{print $1}'`
	if [ "X${K2HDKC_LT_8023_PID}" = "X" ]; then
		K2HDKC_LT_8023_PID=${K2HDKC_8023_PID}
	fi
	echo "${K2HDKC_8023_PID}, ${K2HDKC_LT_8023_PID}" >> ${LOGFILE}

	echo -n "------- CHMPX(server) 8025             : " >> ${LOGFILE}
	chmpx -conf ${TESTSDIR}/test_server${CONF_FILE_EXT} -ctlport 8025 ${CHMPX_DBG_PARAM} > ${CHMPXLOG}_8025.log 2>&1 &
	CHMPX_8025_PID=$!
	echo "${CHMPX_8025_PID}" >> ${LOGFILE}
	sleep 2

	echo -n "------- K2HDKC 8025                    : " >> ${LOGFILE}
	${K2HDKCBIN} -conf ${TESTSDIR}/test_server${CONF_FILE_EXT} -ctlport 8025 ${DKC_DBG_PARAM} ${DKC_COMLOG_PARAM} > ${K2HDKCLOG}_8025.log 2>&1 &
	K2HDKC_8025_PID=$!
	sleep 1
	K2HDKC_LT_8025_PID=`ps ax | grep 'lt-k2hdkc' | grep 8025 | grep -v grep | awk '{print $1}'`
	if [ "X${K2HDKC_LT_8025_PID}" = "X" ]; then
		K2HDKC_LT_8025_PID=${K2HDKC_8025_PID}
	fi
	echo "${K2HDKC_8025_PID}, ${K2HDKC_LT_8025_PID}" >> ${LOGFILE}

	echo -n "------- CHMPX(server) 8027             : " >> ${LOGFILE}
	chmpx -conf ${TESTSDIR}/test_server${CONF_FILE_EXT} -ctlport 8027 ${CHMPX_DBG_PARAM} > ${CHMPXLOG}_8027.log 2>&1 &
	CHMPX_8027_PID=$!
	echo "${CHMPX_8027_PID}" >> ${LOGFILE}
	sleep 2

	echo -n "------- K2HDKC 8027                    : " >> ${LOGFILE}
	${K2HDKCBIN} -conf ${TESTSDIR}/test_server${CONF_FILE_EXT} -ctlport 8027 ${DKC_DBG_PARAM} ${DKC_COMLOG_PARAM} > ${K2HDKCLOG}_8027.log 2>&1 &
	K2HDKC_8027_PID=$!
	sleep 1
	K2HDKC_LT_8027_PID=`ps ax | grep 'lt-k2hdkc' | grep 8027 | grep -v grep | awk '{print $1}'`
	if [ "X${K2HDKC_LT_8027_PID}" = "X" ]; then
		K2HDKC_LT_8027_PID=${K2HDKC_8027_PID}
	fi
	echo "${K2HDKC_8027_PID}, ${K2HDKC_LT_8027_PID}" >> ${LOGFILE}

	echo -n "------- CHMPX(slave) 8031              : " >> ${LOGFILE}
	chmpx -conf ${TESTSDIR}/test_slave${CONF_FILE_EXT} -ctlport 8031 ${CHMPX_DBG_PARAM} > ${CHMPXLOG}_8031.log 2>&1 &
	CHMPX_8031_PID=$!
	echo "${CHMPX_8031_PID}" >> ${LOGFILE}
	sleep 2

	########################################
	#
	# RUN k2hdkclinetool process
	#
	echo "" >> ${LOGFILE}
	echo "======= [INI CONF(EXECUTION)] RUN K2HDKCLINETOOL" >> ${LOGFILE}

	echo -n "------- [INI CONF(EXECUTION)] K2HDKCLINETOOL      : " >> ${LOGFILE}
	${LINETOOLBIN} -conf ${TESTSDIR}/test_slave${CONF_FILE_EXT} -ctlport 8031 ${DKCTOOL_COMLOG_PARAM} ${DKCTOOL_DBG_PARAM} -rejoin -nogiveup -nocleanup -run ${LINETOOLEXITCMD} > ${LINETOOLEXITLOG} 2>&1
	if [ $? -ne 0 ]; then
		echo "FAILED" >> ${LOGFILE}
		echo "FAILED" >> ${LINETOOLLOG}

		echo ""
		echo "*********** DETAIL LOG : LOGFILE *****************"
		cat ${LOGFILE}

		echo ""
		echo "*********** DETAIL LOG : K2HDKCLINETOOL **********"
		cat ${LINETOOLEXITLOG}

		echo ""
		echo "*********** DETAIL LOG : CHMPX(8021) *************"
		cat ${CHMPXLOG}_8021.log

		echo ""
		echo "*********** DETAIL LOG : CHMPX(8023) *************"
		cat ${CHMPXLOG}_8023.log

		echo ""
		echo "*********** DETAIL LOG : CHMPX(8025) *************"
		cat ${CHMPXLOG}_8025.log

		echo ""
		echo "*********** DETAIL LOG : CHMPX(8027) *************"
		cat ${CHMPXLOG}_8027.log

		echo ""
		echo "*********** DETAIL LOG : K2HDKC(8021) *************"
		cat ${K2HDKCLOG}_8021.log

		echo ""
		echo "*********** DETAIL LOG : K2HDKC(8023) *************"
		cat ${K2HDKCLOG}_8023.log

		echo ""
		echo "*********** DETAIL LOG : K2HDKC(8025) *************"
		cat ${K2HDKCLOG}_8025.log

		echo ""
		echo "*********** DETAIL LOG : K2HDKC(8027) *************"
		cat ${K2HDKCLOG}_8027.log

		TEST_RESULT=1
	else
		echo "SUCCEED" >> ${LOGFILE}
		echo "SUCCEED" >> ${LINETOOLLOG}
	fi

	########################################
	#
	# STOP ALL
	#
	echo "" >> ${LOGFILE}
	echo "======= [INI CONF(EXECUTION)] STOP ALL PROCESSES CHMPX(server/slave) / K2HDKC" >> ${LOGFILE}
	(ps -p ${CHMPX_8031_PID}		> /dev/null 2>&1) && (kill -HUP ${CHMPX_8031_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8027_PID}		> /dev/null 2>&1) && (kill -HUP ${CHMPX_8027_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8027_PID}		> /dev/null 2>&1) && (kill -HUP ${K2HDKC_8027_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8027_PID}	> /dev/null 2>&1) && (kill -HUP ${K2HDKC_LT_8027_PID}	>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8025_PID}		> /dev/null 2>&1) && (kill -HUP ${CHMPX_8025_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8025_PID}		> /dev/null 2>&1) && (kill -HUP ${K2HDKC_8025_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8025_PID}	> /dev/null 2>&1) && (kill -HUP ${K2HDKC_LT_8025_PID}	>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8023_PID}		> /dev/null 2>&1) && (kill -HUP ${CHMPX_8023_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8023_PID}		> /dev/null 2>&1) && (kill -HUP ${K2HDKC_8023_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8023_PID}	> /dev/null 2>&1) && (kill -HUP ${K2HDKC_LT_8023_PID}	>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8021_PID}		> /dev/null 2>&1) && (kill -HUP ${CHMPX_8021_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8021_PID}		> /dev/null 2>&1) && (kill -HUP ${K2HDKC_8021_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8021_PID}	> /dev/null 2>&1) && (kill -HUP ${K2HDKC_LT_8021_PID}	>> ${LOGFILE} 2>&1)
	sleep 10
	(ps -p ${CHMPX_8031_PID}		> /dev/null 2>&1) && (kill -9 ${CHMPX_8031_PID}			>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8027_PID}		> /dev/null 2>&1) && (kill -9 ${CHMPX_8027_PID}			>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8027_PID}		> /dev/null 2>&1) && (kill -9 ${K2HDKC_8027_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8027_PID}	> /dev/null 2>&1) && (kill -9 ${K2HDKC_LT_8027_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8025_PID}		> /dev/null 2>&1) && (kill -9 ${CHMPX_8025_PID}			>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8025_PID}		> /dev/null 2>&1) && (kill -9 ${K2HDKC_8025_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8025_PID}	> /dev/null 2>&1) && (kill -9 ${K2HDKC_LT_8025_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8023_PID}		> /dev/null 2>&1) && (kill -9 ${CHMPX_8023_PID}			>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8023_PID}		> /dev/null 2>&1) && (kill -9 ${K2HDKC_8023_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8023_PID}	> /dev/null 2>&1) && (kill -9 ${K2HDKC_LT_8023_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8021_PID}		> /dev/null 2>&1) && (kill -9 ${CHMPX_8021_PID}			>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8021_PID}		> /dev/null 2>&1) && (kill -9 ${K2HDKC_8021_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8021_PID}	> /dev/null 2>&1) && (kill -9 ${K2HDKC_LT_8021_PID}		>> ${LOGFILE} 2>&1)

	########################################
	#
	# CLEANUP
	#
	if [ ${TEST_RESULT} -eq 0 ]; then
		rm -f ${LOGFILE}
		rm -f ${LINETOOLLOG}
		rm -f ${LINETOOLEXITLOG}
		rm -f ${CHMPXLOG}_8021.log
		rm -f ${CHMPXLOG}_8023.log
		rm -f ${CHMPXLOG}_8025.log
		rm -f ${CHMPXLOG}_8027.log
		rm -f ${CHMPXLOG}_8031.log
		rm -f ${K2HDKCLOG}_8021.log
		rm -f ${K2HDKCLOG}_8023.log
		rm -f ${K2HDKCLOG}_8025.log
		rm -f ${K2HDKCLOG}_8027.log
	fi
fi

###########################################################
## Start YAML conf file(execution)
###########################################################
if [ ${TEST_RESULT} -eq 0 -a "X${DO_YAML_CONF}" = "Xyes" ]; then
	echo ""
	echo "====== START TEST FOR YAML CONF FILE (EXECUTION) ============"
	echo ""
	CONF_FILE_EXT=".yaml"

	########################################
	#
	# RUN chmpx processes and k2hdkc server processes
	#
	echo "======= [YAML CONF(EXECUTION)] RUN CHMPX(server/slave) / K2HDKC" >> ${LOGFILE}

	echo -n "------- CHMPX(server) 8021             : " >> ${LOGFILE}
	chmpx -conf ${TESTSDIR}/test_server${CONF_FILE_EXT} -ctlport 8021 ${CHMPX_DBG_PARAM} > ${CHMPXLOG}_8021.log 2>&1 &
	CHMPX_8021_PID=$!
	echo "${CHMPX_8021_PID}" >> ${LOGFILE}
	sleep 2

	echo -n "------- K2HDKC 8021                    : " >> ${LOGFILE}
	${K2HDKCBIN} -conf ${TESTSDIR}/test_server${CONF_FILE_EXT} -ctlport 8021 ${DKC_DBG_PARAM} ${DKC_COMLOG_PARAM} > ${K2HDKCLOG}_8021.log 2>&1 &
	K2HDKC_8021_PID=$!
	sleep 1
	K2HDKC_LT_8021_PID=`ps ax | grep 'lt-k2hdkc' | grep 8021 | grep -v grep | awk '{print $1}'`
	if [ "X${K2HDKC_LT_8021_PID}" = "X" ]; then
		K2HDKC_LT_8021_PID=${K2HDKC_8021_PID}
	fi
	echo "${K2HDKC_8021_PID}, ${K2HDKC_LT_8021_PID}" >> ${LOGFILE}

	echo -n "------- CHMPX(server) 8023             : " >> ${LOGFILE}
	chmpx -conf ${TESTSDIR}/test_server${CONF_FILE_EXT} -ctlport 8023 ${CHMPX_DBG_PARAM} > ${CHMPXLOG}_8023.log 2>&1 &
	CHMPX_8023_PID=$!
	echo "${CHMPX_8023_PID}" >> ${LOGFILE}
	sleep 2

	echo -n "------- K2HDKC 8023                    : " >> ${LOGFILE}
	${K2HDKCBIN} -conf ${TESTSDIR}/test_server${CONF_FILE_EXT} -ctlport 8023 ${DKC_DBG_PARAM} ${DKC_COMLOG_PARAM} > ${K2HDKCLOG}_8023.log 2>&1 &
	K2HDKC_8023_PID=$!
	sleep 1
	K2HDKC_LT_8023_PID=`ps ax | grep 'lt-k2hdkc' | grep 8023 | grep -v grep | awk '{print $1}'`
	if [ "X${K2HDKC_LT_8023_PID}" = "X" ]; then
		K2HDKC_LT_8023_PID=${K2HDKC_8023_PID}
	fi
	echo "${K2HDKC_8023_PID}, ${K2HDKC_LT_8023_PID}" >> ${LOGFILE}

	echo -n "------- CHMPX(server) 8025             : " >> ${LOGFILE}
	chmpx -conf ${TESTSDIR}/test_server${CONF_FILE_EXT} -ctlport 8025 ${CHMPX_DBG_PARAM} > ${CHMPXLOG}_8025.log 2>&1 &
	CHMPX_8025_PID=$!
	echo "${CHMPX_8025_PID}" >> ${LOGFILE}
	sleep 2

	echo -n "------- K2HDKC 8025                    : " >> ${LOGFILE}
	${K2HDKCBIN} -conf ${TESTSDIR}/test_server${CONF_FILE_EXT} -ctlport 8025 ${DKC_DBG_PARAM} ${DKC_COMLOG_PARAM} > ${K2HDKCLOG}_8025.log 2>&1 &
	K2HDKC_8025_PID=$!
	sleep 1
	K2HDKC_LT_8025_PID=`ps ax | grep 'lt-k2hdkc' | grep 8025 | grep -v grep | awk '{print $1}'`
	if [ "X${K2HDKC_LT_8025_PID}" = "X" ]; then
		K2HDKC_LT_8025_PID=${K2HDKC_8025_PID}
	fi
	echo "${K2HDKC_8025_PID}, ${K2HDKC_LT_8025_PID}" >> ${LOGFILE}

	echo -n "------- CHMPX(server) 8027             : " >> ${LOGFILE}
	chmpx -conf ${TESTSDIR}/test_server${CONF_FILE_EXT} -ctlport 8027 ${CHMPX_DBG_PARAM} > ${CHMPXLOG}_8027.log 2>&1 &
	CHMPX_8027_PID=$!
	echo "${CHMPX_8027_PID}" >> ${LOGFILE}
	sleep 2

	echo -n "------- K2HDKC 8027                    : " >> ${LOGFILE}
	${K2HDKCBIN} -conf ${TESTSDIR}/test_server${CONF_FILE_EXT} -ctlport 8027 ${DKC_DBG_PARAM} ${DKC_COMLOG_PARAM} > ${K2HDKCLOG}_8027.log 2>&1 &
	K2HDKC_8027_PID=$!
	sleep 1
	K2HDKC_LT_8027_PID=`ps ax | grep 'lt-k2hdkc' | grep 8027 | grep -v grep | awk '{print $1}'`
	if [ "X${K2HDKC_LT_8027_PID}" = "X" ]; then
		K2HDKC_LT_8027_PID=${K2HDKC_8027_PID}
	fi
	echo "${K2HDKC_8027_PID}, ${K2HDKC_LT_8027_PID}" >> ${LOGFILE}

	echo -n "------- CHMPX(slave) 8031              : " >> ${LOGFILE}
	chmpx -conf ${TESTSDIR}/test_slave${CONF_FILE_EXT} -ctlport 8031 ${CHMPX_DBG_PARAM} > ${CHMPXLOG}_8031.log 2>&1 &
	CHMPX_8031_PID=$!
	echo "${CHMPX_8031_PID}" >> ${LOGFILE}
	sleep 2

	########################################
	#
	# RUN k2hdkclinetool process
	#
	TEST_RESULT=0

	echo "" >> ${LOGFILE}
	echo "======= [YAML CONF(EXECUTION)] RUN K2HDKCLINETOOL" >> ${LOGFILE}

	echo -n "------- [YAML CONF(EXECUTION)] K2HDKCLINETOOL      : " >> ${LOGFILE}
	${LINETOOLBIN} -conf ${TESTSDIR}/test_slave${CONF_FILE_EXT} -ctlport 8031 ${DKCTOOL_COMLOG_PARAM} ${DKCTOOL_DBG_PARAM} -rejoin -nogiveup -nocleanup -run ${LINETOOLEXITCMD} > ${LINETOOLEXITLOG} 2>&1
	if [ $? -ne 0 ]; then
		echo "FAILED" >> ${LOGFILE}
		echo "FAILED" >> ${LINETOOLLOG}

		echo ""
		echo "*********** DETAIL LOG : LOGFILE *****************"
		cat ${LOGFILE}

		echo ""
		echo "*********** DETAIL LOG : K2HDKCLINETOOL **********"
		cat ${LINETOOLEXITLOG}

		echo ""
		echo "*********** DETAIL LOG : CHMPX(8021) *************"
		cat ${CHMPXLOG}_8021.log

		echo ""
		echo "*********** DETAIL LOG : CHMPX(8023) *************"
		cat ${CHMPXLOG}_8023.log

		echo ""
		echo "*********** DETAIL LOG : CHMPX(8025) *************"
		cat ${CHMPXLOG}_8025.log

		echo ""
		echo "*********** DETAIL LOG : CHMPX(8027) *************"
		cat ${CHMPXLOG}_8027.log

		echo ""
		echo "*********** DETAIL LOG : K2HDKC(8021) *************"
		cat ${K2HDKCLOG}_8021.log

		echo ""
		echo "*********** DETAIL LOG : K2HDKC(8023) *************"
		cat ${K2HDKCLOG}_8023.log

		echo ""
		echo "*********** DETAIL LOG : K2HDKC(8025) *************"
		cat ${K2HDKCLOG}_8025.log

		echo ""
		echo "*********** DETAIL LOG : K2HDKC(8027) *************"
		cat ${K2HDKCLOG}_8027.log

		TEST_RESULT=1
	else
		echo "SUCCEED" >> ${LOGFILE}
		echo "SUCCEED" >> ${LINETOOLLOG}
	fi

	########################################
	#
	# STOP ALL
	#
	echo "" >> ${LOGFILE}
	echo "======= [YAML CONF(EXECUTION)] STOP ALL PROCESSES CHMPX(server/slave) / K2HDKC" >> ${LOGFILE}
	(ps -p ${CHMPX_8031_PID}		> /dev/null 2>&1) && (kill -HUP ${CHMPX_8031_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8027_PID}		> /dev/null 2>&1) && (kill -HUP ${CHMPX_8027_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8027_PID}		> /dev/null 2>&1) && (kill -HUP ${K2HDKC_8027_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8027_PID}	> /dev/null 2>&1) && (kill -HUP ${K2HDKC_LT_8027_PID}	>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8025_PID}		> /dev/null 2>&1) && (kill -HUP ${CHMPX_8025_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8025_PID}		> /dev/null 2>&1) && (kill -HUP ${K2HDKC_8025_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8025_PID}	> /dev/null 2>&1) && (kill -HUP ${K2HDKC_LT_8025_PID}	>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8023_PID}		> /dev/null 2>&1) && (kill -HUP ${CHMPX_8023_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8023_PID}		> /dev/null 2>&1) && (kill -HUP ${K2HDKC_8023_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8023_PID}	> /dev/null 2>&1) && (kill -HUP ${K2HDKC_LT_8023_PID}	>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8021_PID}		> /dev/null 2>&1) && (kill -HUP ${CHMPX_8021_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8021_PID}		> /dev/null 2>&1) && (kill -HUP ${K2HDKC_8021_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8021_PID}	> /dev/null 2>&1) && (kill -HUP ${K2HDKC_LT_8021_PID}	>> ${LOGFILE} 2>&1)
	sleep 10
	(ps -p ${CHMPX_8031_PID}		> /dev/null 2>&1) && (kill -9 ${CHMPX_8031_PID}			>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8027_PID}		> /dev/null 2>&1) && (kill -9 ${CHMPX_8027_PID}			>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8027_PID}		> /dev/null 2>&1) && (kill -9 ${K2HDKC_8027_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8027_PID}	> /dev/null 2>&1) && (kill -9 ${K2HDKC_LT_8027_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8025_PID}		> /dev/null 2>&1) && (kill -9 ${CHMPX_8025_PID}			>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8025_PID}		> /dev/null 2>&1) && (kill -9 ${K2HDKC_8025_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8025_PID}	> /dev/null 2>&1) && (kill -9 ${K2HDKC_LT_8025_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8023_PID}		> /dev/null 2>&1) && (kill -9 ${CHMPX_8023_PID}			>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8023_PID}		> /dev/null 2>&1) && (kill -9 ${K2HDKC_8023_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8023_PID}	> /dev/null 2>&1) && (kill -9 ${K2HDKC_LT_8023_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8021_PID}		> /dev/null 2>&1) && (kill -9 ${CHMPX_8021_PID}			>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8021_PID}		> /dev/null 2>&1) && (kill -9 ${K2HDKC_8021_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8021_PID}	> /dev/null 2>&1) && (kill -9 ${K2HDKC_LT_8021_PID}		>> ${LOGFILE} 2>&1)

	########################################
	#
	# CLEANUP
	#
	if [ ${TEST_RESULT} -eq 0 ]; then
		rm -f ${LOGFILE}
		rm -f ${LINETOOLLOG}
		rm -f ${LINETOOLEXITLOG}
		rm -f ${CHMPXLOG}_8021.log
		rm -f ${CHMPXLOG}_8023.log
		rm -f ${CHMPXLOG}_8025.log
		rm -f ${CHMPXLOG}_8027.log
		rm -f ${CHMPXLOG}_8031.log
		rm -f ${K2HDKCLOG}_8021.log
		rm -f ${K2HDKCLOG}_8023.log
		rm -f ${K2HDKCLOG}_8025.log
		rm -f ${K2HDKCLOG}_8027.log
	fi
fi

###########################################################
## Start JSON conf file(execution)
###########################################################
if [ ${TEST_RESULT} -eq 0 -a "X${DO_JSON_CONF}" = "Xyes" ]; then
	echo ""
	echo "====== START TEST FOR JSON CONF FILE (EXECUTION) ============"
	echo ""
	CONF_FILE_EXT=".json"

	########################################
	#
	# RUN chmpx processes and k2hdkc server processes
	#
	echo "======= [JSON CONF(EXECUTION)] RUN CHMPX(server/slave) / K2HDKC" >> ${LOGFILE}

	echo -n "------- CHMPX(server) 8021             : " >> ${LOGFILE}
	chmpx -conf ${TESTSDIR}/test_server${CONF_FILE_EXT} -ctlport 8021 ${CHMPX_DBG_PARAM} > ${CHMPXLOG}_8021.log 2>&1 &
	CHMPX_8021_PID=$!
	echo "${CHMPX_8021_PID}" >> ${LOGFILE}
	sleep 2

	echo -n "------- K2HDKC 8021                    : " >> ${LOGFILE}
	${K2HDKCBIN} -conf ${TESTSDIR}/test_server${CONF_FILE_EXT} -ctlport 8021 ${DKC_DBG_PARAM} ${DKC_COMLOG_PARAM} > ${K2HDKCLOG}_8021.log 2>&1 &
	K2HDKC_8021_PID=$!
	sleep 1
	K2HDKC_LT_8021_PID=`ps ax | grep 'lt-k2hdkc' | grep 8021 | grep -v grep | awk '{print $1}'`
	if [ "X${K2HDKC_LT_8021_PID}" = "X" ]; then
		K2HDKC_LT_8021_PID=${K2HDKC_8021_PID}
	fi
	echo "${K2HDKC_8021_PID}, ${K2HDKC_LT_8021_PID}" >> ${LOGFILE}

	echo -n "------- CHMPX(server) 8023             : " >> ${LOGFILE}
	chmpx -conf ${TESTSDIR}/test_server${CONF_FILE_EXT} -ctlport 8023 ${CHMPX_DBG_PARAM} > ${CHMPXLOG}_8023.log 2>&1 &
	CHMPX_8023_PID=$!
	echo "${CHMPX_8023_PID}" >> ${LOGFILE}
	sleep 2

	echo -n "------- K2HDKC 8023                    : " >> ${LOGFILE}
	${K2HDKCBIN} -conf ${TESTSDIR}/test_server${CONF_FILE_EXT} -ctlport 8023 ${DKC_DBG_PARAM} ${DKC_COMLOG_PARAM} > ${K2HDKCLOG}_8023.log 2>&1 &
	K2HDKC_8023_PID=$!
	sleep 1
	K2HDKC_LT_8023_PID=`ps ax | grep 'lt-k2hdkc' | grep 8023 | grep -v grep | awk '{print $1}'`
	if [ "X${K2HDKC_LT_8023_PID}" = "X" ]; then
		K2HDKC_LT_8023_PID=${K2HDKC_8023_PID}
	fi
	echo "${K2HDKC_8023_PID}, ${K2HDKC_LT_8023_PID}" >> ${LOGFILE}

	echo -n "------- CHMPX(server) 8025             : " >> ${LOGFILE}
	chmpx -conf ${TESTSDIR}/test_server${CONF_FILE_EXT} -ctlport 8025 ${CHMPX_DBG_PARAM} > ${CHMPXLOG}_8025.log 2>&1 &
	CHMPX_8025_PID=$!
	echo "${CHMPX_8025_PID}" >> ${LOGFILE}
	sleep 2

	echo -n "------- K2HDKC 8025                    : " >> ${LOGFILE}
	${K2HDKCBIN} -conf ${TESTSDIR}/test_server${CONF_FILE_EXT} -ctlport 8025 ${DKC_DBG_PARAM} ${DKC_COMLOG_PARAM} > ${K2HDKCLOG}_8025.log 2>&1 &
	K2HDKC_8025_PID=$!
	sleep 1
	K2HDKC_LT_8025_PID=`ps ax | grep 'lt-k2hdkc' | grep 8025 | grep -v grep | awk '{print $1}'`
	if [ "X${K2HDKC_LT_8025_PID}" = "X" ]; then
		K2HDKC_LT_8025_PID=${K2HDKC_8025_PID}
	fi
	echo "${K2HDKC_8025_PID}, ${K2HDKC_LT_8025_PID}" >> ${LOGFILE}

	echo -n "------- CHMPX(server) 8027             : " >> ${LOGFILE}
	chmpx -conf ${TESTSDIR}/test_server${CONF_FILE_EXT} -ctlport 8027 ${CHMPX_DBG_PARAM} > ${CHMPXLOG}_8027.log 2>&1 &
	CHMPX_8027_PID=$!
	echo "${CHMPX_8027_PID}" >> ${LOGFILE}
	sleep 2

	echo -n "------- K2HDKC 8027                    : " >> ${LOGFILE}
	${K2HDKCBIN} -conf ${TESTSDIR}/test_server${CONF_FILE_EXT} -ctlport 8027 ${DKC_DBG_PARAM} ${DKC_COMLOG_PARAM} > ${K2HDKCLOG}_8027.log 2>&1 &
	K2HDKC_8027_PID=$!
	sleep 1
	K2HDKC_LT_8027_PID=`ps ax | grep 'lt-k2hdkc' | grep 8027 | grep -v grep | awk '{print $1}'`
	if [ "X${K2HDKC_LT_8027_PID}" = "X" ]; then
		K2HDKC_LT_8027_PID=${K2HDKC_8027_PID}
	fi
	echo "${K2HDKC_8027_PID}, ${K2HDKC_LT_8027_PID}" >> ${LOGFILE}

	echo -n "------- CHMPX(slave) 8031              : " >> ${LOGFILE}
	chmpx -conf ${TESTSDIR}/test_slave${CONF_FILE_EXT} -ctlport 8031 ${CHMPX_DBG_PARAM} > ${CHMPXLOG}_8031.log 2>&1 &
	CHMPX_8031_PID=$!
	echo "${CHMPX_8031_PID}" >> ${LOGFILE}
	sleep 2

	########################################
	#
	# RUN k2hdkclinetool process
	#
	TEST_RESULT=0

	echo "" >> ${LOGFILE}
	echo "======= [JSON CONF(EXECUTION)] RUN K2HDKCLINETOOL" >> ${LOGFILE}

	echo -n "------- [JSON CONF(EXECUTION)] K2HDKCLINETOOL     : " >> ${LOGFILE}
	${LINETOOLBIN} -conf ${TESTSDIR}/test_slave${CONF_FILE_EXT} -ctlport 8031 ${DKCTOOL_COMLOG_PARAM} ${DKCTOOL_DBG_PARAM} -rejoin -nogiveup -nocleanup -run ${LINETOOLEXITCMD} > ${LINETOOLEXITLOG} 2>&1
	if [ $? -ne 0 ]; then
		echo "FAILED" >> ${LOGFILE}
		echo "FAILED" >> ${LINETOOLLOG}

		echo ""
		echo "*********** DETAIL LOG : LOGFILE *****************"
		cat ${LOGFILE}

		echo ""
		echo "*********** DETAIL LOG : K2HDKCLINETOOL **********"
		cat ${LINETOOLEXITLOG}

		echo ""
		echo "*********** DETAIL LOG : CHMPX(8021) *************"
		cat ${CHMPXLOG}_8021.log

		echo ""
		echo "*********** DETAIL LOG : CHMPX(8023) *************"
		cat ${CHMPXLOG}_8023.log

		echo ""
		echo "*********** DETAIL LOG : CHMPX(8025) *************"
		cat ${CHMPXLOG}_8025.log

		echo ""
		echo "*********** DETAIL LOG : CHMPX(8027) *************"
		cat ${CHMPXLOG}_8027.log

		echo ""
		echo "*********** DETAIL LOG : K2HDKC(8021) *************"
		cat ${K2HDKCLOG}_8021.log

		echo ""
		echo "*********** DETAIL LOG : K2HDKC(8023) *************"
		cat ${K2HDKCLOG}_8023.log

		echo ""
		echo "*********** DETAIL LOG : K2HDKC(8025) *************"
		cat ${K2HDKCLOG}_8025.log

		echo ""
		echo "*********** DETAIL LOG : K2HDKC(8027) *************"
		cat ${K2HDKCLOG}_8027.log

		TEST_RESULT=1
	else
		echo "SUCCEED" >> ${LOGFILE}
		echo "SUCCEED" >> ${LINETOOLLOG}
	fi

	########################################
	#
	# STOP ALL
	#
	echo "" >> ${LOGFILE}
	echo "======= [JSON CONF(EXECUTION)] STOP ALL PROCESSES CHMPX(server/slave) / K2HDKC" >> ${LOGFILE}
	(ps -p ${CHMPX_8031_PID}		> /dev/null 2>&1) && (kill -HUP ${CHMPX_8031_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8027_PID}		> /dev/null 2>&1) && (kill -HUP ${CHMPX_8027_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8027_PID}		> /dev/null 2>&1) && (kill -HUP ${K2HDKC_8027_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8027_PID}	> /dev/null 2>&1) && (kill -HUP ${K2HDKC_LT_8027_PID}	>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8025_PID}		> /dev/null 2>&1) && (kill -HUP ${CHMPX_8025_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8025_PID}		> /dev/null 2>&1) && (kill -HUP ${K2HDKC_8025_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8025_PID}	> /dev/null 2>&1) && (kill -HUP ${K2HDKC_LT_8025_PID}	>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8023_PID}		> /dev/null 2>&1) && (kill -HUP ${CHMPX_8023_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8023_PID}		> /dev/null 2>&1) && (kill -HUP ${K2HDKC_8023_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8023_PID}	> /dev/null 2>&1) && (kill -HUP ${K2HDKC_LT_8023_PID}	>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8021_PID}		> /dev/null 2>&1) && (kill -HUP ${CHMPX_8021_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8021_PID}		> /dev/null 2>&1) && (kill -HUP ${K2HDKC_8021_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8021_PID}	> /dev/null 2>&1) && (kill -HUP ${K2HDKC_LT_8021_PID}	>> ${LOGFILE} 2>&1)
	sleep 10
	(ps -p ${CHMPX_8031_PID}		> /dev/null 2>&1) && (kill -9 ${CHMPX_8031_PID}			>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8027_PID}		> /dev/null 2>&1) && (kill -9 ${CHMPX_8027_PID}			>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8027_PID}		> /dev/null 2>&1) && (kill -9 ${K2HDKC_8027_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8027_PID}	> /dev/null 2>&1) && (kill -9 ${K2HDKC_LT_8027_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8025_PID}		> /dev/null 2>&1) && (kill -9 ${CHMPX_8025_PID}			>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8025_PID}		> /dev/null 2>&1) && (kill -9 ${K2HDKC_8025_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8025_PID}	> /dev/null 2>&1) && (kill -9 ${K2HDKC_LT_8025_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8023_PID}		> /dev/null 2>&1) && (kill -9 ${CHMPX_8023_PID}			>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8023_PID}		> /dev/null 2>&1) && (kill -9 ${K2HDKC_8023_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8023_PID}	> /dev/null 2>&1) && (kill -9 ${K2HDKC_LT_8023_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8021_PID}		> /dev/null 2>&1) && (kill -9 ${CHMPX_8021_PID}			>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8021_PID}		> /dev/null 2>&1) && (kill -9 ${K2HDKC_8021_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8021_PID}	> /dev/null 2>&1) && (kill -9 ${K2HDKC_LT_8021_PID}		>> ${LOGFILE} 2>&1)

	########################################
	#
	# CLEANUP
	#
	if [ ${TEST_RESULT} -eq 0 ]; then
		rm -f ${LOGFILE}
		rm -f ${LINETOOLLOG}
		rm -f ${LINETOOLEXITLOG}
		rm -f ${CHMPXLOG}_8021.log
		rm -f ${CHMPXLOG}_8023.log
		rm -f ${CHMPXLOG}_8025.log
		rm -f ${CHMPXLOG}_8027.log
		rm -f ${CHMPXLOG}_8031.log
		rm -f ${K2HDKCLOG}_8021.log
		rm -f ${K2HDKCLOG}_8023.log
		rm -f ${K2HDKCLOG}_8025.log
		rm -f ${K2HDKCLOG}_8027.log
	fi
fi

###########################################################
## Start JSON string
###########################################################
if [ ${TEST_RESULT} -eq 0 -a "X${DO_JSON_STRING}" = "Xyes" ]; then
	echo ""
	echo "====== START TEST FOR JSON STRING (EXECUTION) ================="
	echo ""

	########################################
	#
	# RUN chmpx processes and k2hdkc server processes
	#
	echo "======= [JSON STRING(EXECUTION)] RUN CHMPX(server/slave) / K2HDKC" >> ${LOGFILE}

	echo -n "------- CHMPX(server) 8021             : " >> ${LOGFILE}
	chmpx -json "${TEST_SERVER_JSON}" -ctlport 8021 ${CHMPX_DBG_PARAM} > ${CHMPXLOG}_8021.log 2>&1 &
	CHMPX_8021_PID=$!
	echo "${CHMPX_8021_PID}" >> ${LOGFILE}
	sleep 2

	echo -n "------- K2HDKC 8021                    : " >> ${LOGFILE}
	${K2HDKCBIN} -json "${TEST_SERVER_JSON}" -ctlport 8021 ${DKC_DBG_PARAM} ${DKC_COMLOG_PARAM} > ${K2HDKCLOG}_8021.log 2>&1 &
	K2HDKC_8021_PID=$!
	sleep 1
	K2HDKC_LT_8021_PID=`ps ax | grep 'lt-k2hdkc' | grep 8021 | grep -v grep | awk '{print $1}'`
	if [ "X${K2HDKC_LT_8021_PID}" = "X" ]; then
		K2HDKC_LT_8021_PID=${K2HDKC_8021_PID}
	fi
	echo "${K2HDKC_8021_PID}, ${K2HDKC_LT_8021_PID}" >> ${LOGFILE}

	echo -n "------- CHMPX(server) 8023             : " >> ${LOGFILE}
	chmpx -json "${TEST_SERVER_JSON}" -ctlport 8023 ${CHMPX_DBG_PARAM} > ${CHMPXLOG}_8023.log 2>&1 &
	CHMPX_8023_PID=$!
	echo "${CHMPX_8023_PID}" >> ${LOGFILE}
	sleep 2

	echo -n "------- K2HDKC 8023                    : " >> ${LOGFILE}
	${K2HDKCBIN} -json "${TEST_SERVER_JSON}" -ctlport 8023 ${DKC_DBG_PARAM} ${DKC_COMLOG_PARAM} > ${K2HDKCLOG}_8023.log 2>&1 &
	K2HDKC_8023_PID=$!
	sleep 1
	K2HDKC_LT_8023_PID=`ps ax | grep 'lt-k2hdkc' | grep 8023 | grep -v grep | awk '{print $1}'`
	if [ "X${K2HDKC_LT_8023_PID}" = "X" ]; then
		K2HDKC_LT_8023_PID=${K2HDKC_8023_PID}
	fi
	echo "${K2HDKC_8023_PID}, ${K2HDKC_LT_8023_PID}" >> ${LOGFILE}

	echo -n "------- CHMPX(server) 8025             : " >> ${LOGFILE}
	chmpx -json "${TEST_SERVER_JSON}" -ctlport 8025 ${CHMPX_DBG_PARAM} > ${CHMPXLOG}_8025.log 2>&1 &
	CHMPX_8025_PID=$!
	echo "${CHMPX_8025_PID}" >> ${LOGFILE}
	sleep 2

	echo -n "------- K2HDKC 8025                    : " >> ${LOGFILE}
	${K2HDKCBIN} -json "${TEST_SERVER_JSON}" -ctlport 8025 ${DKC_DBG_PARAM} ${DKC_COMLOG_PARAM} > ${K2HDKCLOG}_8025.log 2>&1 &
	K2HDKC_8025_PID=$!
	sleep 1
	K2HDKC_LT_8025_PID=`ps ax | grep 'lt-k2hdkc' | grep 8025 | grep -v grep | awk '{print $1}'`
	if [ "X${K2HDKC_LT_8025_PID}" = "X" ]; then
		K2HDKC_LT_8025_PID=${K2HDKC_8025_PID}
	fi
	echo "${K2HDKC_8025_PID}, ${K2HDKC_LT_8025_PID}" >> ${LOGFILE}

	echo -n "------- CHMPX(server) 8027             : " >> ${LOGFILE}
	chmpx -json "${TEST_SERVER_JSON}" -ctlport 8027 ${CHMPX_DBG_PARAM} > ${CHMPXLOG}_8027.log 2>&1 &
	CHMPX_8027_PID=$!
	echo "${CHMPX_8027_PID}" >> ${LOGFILE}
	sleep 2

	echo -n "------- K2HDKC 8027                    : " >> ${LOGFILE}
	${K2HDKCBIN} -json "${TEST_SERVER_JSON}" -ctlport 8027 ${DKC_DBG_PARAM} ${DKC_COMLOG_PARAM} > ${K2HDKCLOG}_8027.log 2>&1 &
	K2HDKC_8027_PID=$!
	sleep 1
	K2HDKC_LT_8027_PID=`ps ax | grep 'lt-k2hdkc' | grep 8027 | grep -v grep | awk '{print $1}'`
	if [ "X${K2HDKC_LT_8027_PID}" = "X" ]; then
		K2HDKC_LT_8027_PID=${K2HDKC_8027_PID}
	fi
	echo "${K2HDKC_8027_PID}, ${K2HDKC_LT_8027_PID}" >> ${LOGFILE}

	echo -n "------- CHMPX(slave) 8031              : " >> ${LOGFILE}
	chmpx -json "${TEST_SLAVE_JSON}" -ctlport 8031 ${CHMPX_DBG_PARAM} > ${CHMPXLOG}_8031.log 2>&1 &
	CHMPX_8031_PID=$!
	echo "${CHMPX_8031_PID}" >> ${LOGFILE}
	sleep 2

	########################################
	#
	# RUN k2hdkclinetool process
	#
	TEST_RESULT=0

	echo "" >> ${LOGFILE}
	echo "======= [JSON STRING(EXECUTION)] RUN K2HDKCLINETOOL" >> ${LOGFILE}

	echo -n "------- [JSON STRING(EXECUTION)] K2HDKCLINETOOL   : " >> ${LOGFILE}
	${LINETOOLBIN} -json "${TEST_SLAVE_JSON}" -ctlport 8031 ${DKCTOOL_COMLOG_PARAM} ${DKCTOOL_DBG_PARAM} -rejoin -nogiveup -nocleanup -run ${LINETOOLEXITCMD} > ${LINETOOLEXITLOG} 2>&1
	if [ $? -ne 0 ]; then
		echo "FAILED" >> ${LOGFILE}
		echo "FAILED" >> ${LINETOOLLOG}

		echo ""
		echo "*********** DETAIL LOG : LOGFILE *****************"
		cat ${LOGFILE}

		echo ""
		echo "*********** DETAIL LOG : K2HDKCLINETOOL **********"
		cat ${LINETOOLEXITLOG}

		echo ""
		echo "*********** DETAIL LOG : CHMPX(8021) *************"
		cat ${CHMPXLOG}_8021.log

		echo ""
		echo "*********** DETAIL LOG : CHMPX(8023) *************"
		cat ${CHMPXLOG}_8023.log

		echo ""
		echo "*********** DETAIL LOG : CHMPX(8025) *************"
		cat ${CHMPXLOG}_8025.log

		echo ""
		echo "*********** DETAIL LOG : CHMPX(8027) *************"
		cat ${CHMPXLOG}_8027.log

		echo ""
		echo "*********** DETAIL LOG : K2HDKC(8021) *************"
		cat ${K2HDKCLOG}_8021.log

		echo ""
		echo "*********** DETAIL LOG : K2HDKC(8023) *************"
		cat ${K2HDKCLOG}_8023.log

		echo ""
		echo "*********** DETAIL LOG : K2HDKC(8025) *************"
		cat ${K2HDKCLOG}_8025.log

		echo ""
		echo "*********** DETAIL LOG : K2HDKC(8027) *************"
		cat ${K2HDKCLOG}_8027.log

		TEST_RESULT=1
	else
		echo "SUCCEED" >> ${LOGFILE}
		echo "SUCCEED" >> ${LINETOOLLOG}
	fi

	########################################
	#
	# STOP ALL
	#
	echo "" >> ${LOGFILE}
	echo "======= [JSON STRING(EXECUTION)] STOP ALL PROCESSES CHMPX(server/slave) / K2HDKC" >> ${LOGFILE}
	(ps -p ${CHMPX_8031_PID}		> /dev/null 2>&1) && (kill -HUP ${CHMPX_8031_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8027_PID}		> /dev/null 2>&1) && (kill -HUP ${CHMPX_8027_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8027_PID}		> /dev/null 2>&1) && (kill -HUP ${K2HDKC_8027_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8027_PID}	> /dev/null 2>&1) && (kill -HUP ${K2HDKC_LT_8027_PID}	>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8025_PID}		> /dev/null 2>&1) && (kill -HUP ${CHMPX_8025_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8025_PID}		> /dev/null 2>&1) && (kill -HUP ${K2HDKC_8025_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8025_PID}	> /dev/null 2>&1) && (kill -HUP ${K2HDKC_LT_8025_PID}	>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8023_PID}		> /dev/null 2>&1) && (kill -HUP ${CHMPX_8023_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8023_PID}		> /dev/null 2>&1) && (kill -HUP ${K2HDKC_8023_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8023_PID}	> /dev/null 2>&1) && (kill -HUP ${K2HDKC_LT_8023_PID}	>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8021_PID}		> /dev/null 2>&1) && (kill -HUP ${CHMPX_8021_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8021_PID}		> /dev/null 2>&1) && (kill -HUP ${K2HDKC_8021_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8021_PID}	> /dev/null 2>&1) && (kill -HUP ${K2HDKC_LT_8021_PID}	>> ${LOGFILE} 2>&1)
	sleep 10
	(ps -p ${CHMPX_8031_PID}		> /dev/null 2>&1) && (kill -9 ${CHMPX_8031_PID}			>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8027_PID}		> /dev/null 2>&1) && (kill -9 ${CHMPX_8027_PID}			>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8027_PID}		> /dev/null 2>&1) && (kill -9 ${K2HDKC_8027_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8027_PID}	> /dev/null 2>&1) && (kill -9 ${K2HDKC_LT_8027_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8025_PID}		> /dev/null 2>&1) && (kill -9 ${CHMPX_8025_PID}			>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8025_PID}		> /dev/null 2>&1) && (kill -9 ${K2HDKC_8025_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8025_PID}	> /dev/null 2>&1) && (kill -9 ${K2HDKC_LT_8025_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8023_PID}		> /dev/null 2>&1) && (kill -9 ${CHMPX_8023_PID}			>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8023_PID}		> /dev/null 2>&1) && (kill -9 ${K2HDKC_8023_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8023_PID}	> /dev/null 2>&1) && (kill -9 ${K2HDKC_LT_8023_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8021_PID}		> /dev/null 2>&1) && (kill -9 ${CHMPX_8021_PID}			>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8021_PID}		> /dev/null 2>&1) && (kill -9 ${K2HDKC_8021_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8021_PID}	> /dev/null 2>&1) && (kill -9 ${K2HDKC_LT_8021_PID}		>> ${LOGFILE} 2>&1)

	########################################
	#
	# CLEANUP
	#
	if [ ${TEST_RESULT} -eq 0 ]; then
		rm -f ${LOGFILE}
		rm -f ${LINETOOLLOG}
		rm -f ${LINETOOLEXITLOG}
		rm -f ${CHMPXLOG}_8021.log
		rm -f ${CHMPXLOG}_8023.log
		rm -f ${CHMPXLOG}_8025.log
		rm -f ${CHMPXLOG}_8027.log
		rm -f ${CHMPXLOG}_8031.log
		rm -f ${K2HDKCLOG}_8021.log
		rm -f ${K2HDKCLOG}_8023.log
		rm -f ${K2HDKCLOG}_8025.log
		rm -f ${K2HDKCLOG}_8027.log
	fi
fi

###########################################################
## Start JSON environment(execution)
###########################################################
if [ ${TEST_RESULT} -eq 0 -a "X${DO_JSON_ENV}" = "Xyes" ]; then
	echo ""
	echo "====== START TEST FOR JSON ENV (EXECUTION) ================="
	echo ""

	########################################
	#
	# RUN chmpx processes and k2hdkc server processes
	#
	echo "======= [JSON ENV(EXECUTION)] RUN CHMPX(server/slave) / K2HDKC" >> ${LOGFILE}

	echo -n "------- CHMPX(server) 8021             : " >> ${LOGFILE}
	CHMJSONCONF="${TEST_SERVER_JSON}" chmpx -ctlport 8021 ${CHMPX_DBG_PARAM} > ${CHMPXLOG}_8021.log 2>&1 &
	CHMPX_8021_PID=$!
	echo "${CHMPX_8021_PID}" >> ${LOGFILE}
	sleep 2

	echo -n "------- K2HDKC 8021                    : " >> ${LOGFILE}
	K2HDKCJSONCONF="${TEST_SERVER_JSON}" ${K2HDKCBIN} -ctlport 8021 ${DKC_DBG_PARAM} ${DKC_COMLOG_PARAM} > ${K2HDKCLOG}_8021.log 2>&1 &
	K2HDKC_8021_PID=$!
	sleep 1
	K2HDKC_LT_8021_PID=`ps ax | grep 'lt-k2hdkc' | grep 8021 | grep -v grep | awk '{print $1}'`
	if [ "X${K2HDKC_LT_8021_PID}" = "X" ]; then
		K2HDKC_LT_8021_PID=${K2HDKC_8021_PID}
	fi
	echo "${K2HDKC_8021_PID}, ${K2HDKC_LT_8021_PID}" >> ${LOGFILE}

	echo -n "------- CHMPX(server) 8023             : " >> ${LOGFILE}
	CHMJSONCONF="${TEST_SERVER_JSON}" chmpx -ctlport 8023 ${CHMPX_DBG_PARAM} > ${CHMPXLOG}_8023.log 2>&1 &
	CHMPX_8023_PID=$!
	echo "${CHMPX_8023_PID}" >> ${LOGFILE}
	sleep 2

	echo -n "------- K2HDKC 8023                    : " >> ${LOGFILE}
	K2HDKCJSONCONF="${TEST_SERVER_JSON}" ${K2HDKCBIN} -ctlport 8023 ${DKC_DBG_PARAM} ${DKC_COMLOG_PARAM} > ${K2HDKCLOG}_8023.log 2>&1 &
	K2HDKC_8023_PID=$!
	sleep 1
	K2HDKC_LT_8023_PID=`ps ax | grep 'lt-k2hdkc' | grep 8023 | grep -v grep | awk '{print $1}'`
	if [ "X${K2HDKC_LT_8023_PID}" = "X" ]; then
		K2HDKC_LT_8023_PID=${K2HDKC_8023_PID}
	fi
	echo "${K2HDKC_8023_PID}, ${K2HDKC_LT_8023_PID}" >> ${LOGFILE}

	echo -n "------- CHMPX(server) 8025             : " >> ${LOGFILE}
	CHMJSONCONF="${TEST_SERVER_JSON}" chmpx -ctlport 8025 ${CHMPX_DBG_PARAM} > ${CHMPXLOG}_8025.log 2>&1 &
	CHMPX_8025_PID=$!
	echo "${CHMPX_8025_PID}" >> ${LOGFILE}
	sleep 2

	echo -n "------- K2HDKC 8025                    : " >> ${LOGFILE}
	K2HDKCJSONCONF="${TEST_SERVER_JSON}" ${K2HDKCBIN} -ctlport 8025 ${DKC_DBG_PARAM} ${DKC_COMLOG_PARAM} > ${K2HDKCLOG}_8025.log 2>&1 &
	K2HDKC_8025_PID=$!
	sleep 1
	K2HDKC_LT_8025_PID=`ps ax | grep 'lt-k2hdkc' | grep 8025 | grep -v grep | awk '{print $1}'`
	if [ "X${K2HDKC_LT_8025_PID}" = "X" ]; then
		K2HDKC_LT_8025_PID=${K2HDKC_8025_PID}
	fi
	echo "${K2HDKC_8025_PID}, ${K2HDKC_LT_8025_PID}" >> ${LOGFILE}

	echo -n "------- CHMPX(server) 8027             : " >> ${LOGFILE}
	CHMJSONCONF="${TEST_SERVER_JSON}" chmpx -ctlport 8027 ${CHMPX_DBG_PARAM} > ${CHMPXLOG}_8027.log 2>&1 &
	CHMPX_8027_PID=$!
	echo "${CHMPX_8027_PID}" >> ${LOGFILE}
	sleep 2

	echo -n "------- K2HDKC 8027                    : " >> ${LOGFILE}
	K2HDKCJSONCONF="${TEST_SERVER_JSON}" ${K2HDKCBIN} -ctlport 8027 ${DKC_DBG_PARAM} ${DKC_COMLOG_PARAM} > ${K2HDKCLOG}_8027.log 2>&1 &
	K2HDKC_8027_PID=$!
	sleep 1
	K2HDKC_LT_8027_PID=`ps ax | grep 'lt-k2hdkc' | grep 8027 | grep -v grep | awk '{print $1}'`
	if [ "X${K2HDKC_LT_8027_PID}" = "X" ]; then
		K2HDKC_LT_8027_PID=${K2HDKC_8027_PID}
	fi
	echo "${K2HDKC_8027_PID}, ${K2HDKC_LT_8027_PID}" >> ${LOGFILE}

	echo -n "------- CHMPX(slave) 8031              : " >> ${LOGFILE}
	CHMJSONCONF="${TEST_SLAVE_JSON}" chmpx -ctlport 8031 ${CHMPX_DBG_PARAM} > ${CHMPXLOG}_8031.log 2>&1 &
	CHMPX_8031_PID=$!
	echo "${CHMPX_8031_PID}" >> ${LOGFILE}
	sleep 2

	########################################
	#
	# RUN k2hdkclinetool process
	#
	TEST_RESULT=0

	echo "" >> ${LOGFILE}
	echo "======= [JSON ENV(EXECUTION)] RUN K2HDKCLINETOOL" >> ${LOGFILE}

	echo -n "------- [JSON ENV(EXECUTION)] K2HDKCLINETOOL      : " >> ${LOGFILE}
	K2HDKCJSONCONF="${TEST_SLAVE_JSON}" ${LINETOOLBIN} -ctlport 8031 ${DKCTOOL_COMLOG_PARAM} ${DKCTOOL_DBG_PARAM} -rejoin -nogiveup -nocleanup -run ${LINETOOLEXITCMD} > ${LINETOOLEXITLOG} 2>&1
	if [ $? -ne 0 ]; then
		echo "FAILED" >> ${LOGFILE}
		echo "FAILED" >> ${LINETOOLLOG}

		echo ""
		echo "*********** DETAIL LOG : LOGFILE *****************"
		cat ${LOGFILE}

		echo ""
		echo "*********** DETAIL LOG : K2HDKCLINETOOL **********"
		cat ${LINETOOLEXITLOG}

		echo ""
		echo "*********** DETAIL LOG : CHMPX(8021) *************"
		cat ${CHMPXLOG}_8021.log

		echo ""
		echo "*********** DETAIL LOG : CHMPX(8023) *************"
		cat ${CHMPXLOG}_8023.log

		echo ""
		echo "*********** DETAIL LOG : CHMPX(8025) *************"
		cat ${CHMPXLOG}_8025.log

		echo ""
		echo "*********** DETAIL LOG : CHMPX(8027) *************"
		cat ${CHMPXLOG}_8027.log

		echo ""
		echo "*********** DETAIL LOG : K2HDKC(8021) *************"
		cat ${K2HDKCLOG}_8021.log

		echo ""
		echo "*********** DETAIL LOG : K2HDKC(8023) *************"
		cat ${K2HDKCLOG}_8023.log

		echo ""
		echo "*********** DETAIL LOG : K2HDKC(8025) *************"
		cat ${K2HDKCLOG}_8025.log

		echo ""
		echo "*********** DETAIL LOG : K2HDKC(8027) *************"
		cat ${K2HDKCLOG}_8027.log

		TEST_RESULT=1
	else
		echo "SUCCEED" >> ${LOGFILE}
		echo "SUCCEED" >> ${LINETOOLLOG}
	fi

	########################################
	#
	# STOP ALL
	#
	echo "" >> ${LOGFILE}
	echo "======= [JSON ENV(EXECUTION)] STOP ALL PROCESSES CHMPX(server/slave) / K2HDKC" >> ${LOGFILE}
	(ps -p ${CHMPX_8031_PID}		> /dev/null 2>&1) && (kill -HUP ${CHMPX_8031_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8027_PID}		> /dev/null 2>&1) && (kill -HUP ${CHMPX_8027_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8027_PID}		> /dev/null 2>&1) && (kill -HUP ${K2HDKC_8027_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8027_PID}	> /dev/null 2>&1) && (kill -HUP ${K2HDKC_LT_8027_PID}	>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8025_PID}		> /dev/null 2>&1) && (kill -HUP ${CHMPX_8025_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8025_PID}		> /dev/null 2>&1) && (kill -HUP ${K2HDKC_8025_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8025_PID}	> /dev/null 2>&1) && (kill -HUP ${K2HDKC_LT_8025_PID}	>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8023_PID}		> /dev/null 2>&1) && (kill -HUP ${CHMPX_8023_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8023_PID}		> /dev/null 2>&1) && (kill -HUP ${K2HDKC_8023_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8023_PID}	> /dev/null 2>&1) && (kill -HUP ${K2HDKC_LT_8023_PID}	>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8021_PID}		> /dev/null 2>&1) && (kill -HUP ${CHMPX_8021_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8021_PID}		> /dev/null 2>&1) && (kill -HUP ${K2HDKC_8021_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8021_PID}	> /dev/null 2>&1) && (kill -HUP ${K2HDKC_LT_8021_PID}	>> ${LOGFILE} 2>&1)
	sleep 10
	(ps -p ${CHMPX_8031_PID}		> /dev/null 2>&1) && (kill -9 ${CHMPX_8031_PID}			>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8027_PID}		> /dev/null 2>&1) && (kill -9 ${CHMPX_8027_PID}			>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8027_PID}		> /dev/null 2>&1) && (kill -9 ${K2HDKC_8027_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8027_PID}	> /dev/null 2>&1) && (kill -9 ${K2HDKC_LT_8027_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8025_PID}		> /dev/null 2>&1) && (kill -9 ${CHMPX_8025_PID}			>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8025_PID}		> /dev/null 2>&1) && (kill -9 ${K2HDKC_8025_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8025_PID}	> /dev/null 2>&1) && (kill -9 ${K2HDKC_LT_8025_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8023_PID}		> /dev/null 2>&1) && (kill -9 ${CHMPX_8023_PID}			>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8023_PID}		> /dev/null 2>&1) && (kill -9 ${K2HDKC_8023_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8023_PID}	> /dev/null 2>&1) && (kill -9 ${K2HDKC_LT_8023_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8021_PID}		> /dev/null 2>&1) && (kill -9 ${CHMPX_8021_PID}			>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8021_PID}		> /dev/null 2>&1) && (kill -9 ${K2HDKC_8021_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8021_PID}	> /dev/null 2>&1) && (kill -9 ${K2HDKC_LT_8021_PID}		>> ${LOGFILE} 2>&1)

	########################################
	#
	# CLEANUP
	#
	if [ ${TEST_RESULT} -eq 0 ]; then
		rm -f ${LOGFILE}
		rm -f ${LINETOOLLOG}
		rm -f ${LINETOOLEXITLOG}
		rm -f ${CHMPXLOG}_8021.log
		rm -f ${CHMPXLOG}_8023.log
		rm -f ${CHMPXLOG}_8025.log
		rm -f ${CHMPXLOG}_8027.log
		rm -f ${CHMPXLOG}_8031.log
		rm -f ${K2HDKCLOG}_8021.log
		rm -f ${K2HDKCLOG}_8023.log
		rm -f ${K2HDKCLOG}_8025.log
		rm -f ${K2HDKCLOG}_8027.log
	fi
fi

###########################################################
## Test all command(one pattern : default INI)
###########################################################
if [ ${TEST_RESULT} -eq 0 -a "X${DO_INI_CONF}" = "Xyes" ]; then
	###########################################################
	## Start INI conf file
	###########################################################
	echo ""
	echo "====== START TEST FOR INI CONF FILE ========================"
	echo ""
	CONF_FILE_EXT=".ini"

	########################################
	#
	# RUN chmpx processes and k2hdkc server processes
	#
	echo "======= [INI CONF] RUN CHMPX(server) / K2HDKC" >> ${LOGFILE}

	echo -n "------- CHMPX(server) 8021             : " >> ${LOGFILE}
	chmpx -conf ${TESTSDIR}/test_server${CONF_FILE_EXT} -ctlport 8021 ${CHMPX_DBG_PARAM} > ${CHMPXLOG}_8021.log 2>&1 &
	CHMPX_8021_PID=$!
	echo "${CHMPX_8021_PID}" >> ${LOGFILE}
	sleep 2

	echo -n "------- K2HDKC 8021                    : " >> ${LOGFILE}
	${K2HDKCBIN} -conf ${TESTSDIR}/test_server${CONF_FILE_EXT} -ctlport 8021 ${DKC_DBG_PARAM} ${DKC_COMLOG_PARAM} > ${K2HDKCLOG}_8021.log 2>&1 &
	K2HDKC_8021_PID=$!
	sleep 1
	K2HDKC_LT_8021_PID=`ps ax | grep 'lt-k2hdkc' | grep 8021 | grep -v grep | awk '{print $1}'`
	if [ "X${K2HDKC_LT_8021_PID}" = "X" ]; then
		K2HDKC_LT_8021_PID=${K2HDKC_8021_PID}
	fi
	echo "${K2HDKC_8021_PID}, ${K2HDKC_LT_8021_PID}" >> ${LOGFILE}

	echo -n "------- CHMPX(server) 8023             : " >> ${LOGFILE}
	chmpx -conf ${TESTSDIR}/test_server${CONF_FILE_EXT} -ctlport 8023 ${CHMPX_DBG_PARAM} > ${CHMPXLOG}_8023.log 2>&1 &
	CHMPX_8023_PID=$!
	echo "${CHMPX_8023_PID}" >> ${LOGFILE}
	sleep 2

	echo -n "------- K2HDKC 8023                    : " >> ${LOGFILE}
	${K2HDKCBIN} -conf ${TESTSDIR}/test_server${CONF_FILE_EXT} -ctlport 8023 ${DKC_DBG_PARAM} ${DKC_COMLOG_PARAM} > ${K2HDKCLOG}_8023.log 2>&1 &
	K2HDKC_8023_PID=$!
	sleep 1
	K2HDKC_LT_8023_PID=`ps ax | grep 'lt-k2hdkc' | grep 8023 | grep -v grep | awk '{print $1}'`
	if [ "X${K2HDKC_LT_8023_PID}" = "X" ]; then
		K2HDKC_LT_8023_PID=${K2HDKC_8023_PID}
	fi
	echo "${K2HDKC_8023_PID}, ${K2HDKC_LT_8023_PID}" >> ${LOGFILE}

	echo -n "------- CHMPX(server) 8025             : " >> ${LOGFILE}
	chmpx -conf ${TESTSDIR}/test_server${CONF_FILE_EXT} -ctlport 8025 ${CHMPX_DBG_PARAM} > ${CHMPXLOG}_8025.log 2>&1 &
	CHMPX_8025_PID=$!
	echo "${CHMPX_8025_PID}" >> ${LOGFILE}
	sleep 2

	echo -n "------- K2HDKC 8025                    : " >> ${LOGFILE}
	${K2HDKCBIN} -conf ${TESTSDIR}/test_server${CONF_FILE_EXT} -ctlport 8025 ${DKC_DBG_PARAM} ${DKC_COMLOG_PARAM} > ${K2HDKCLOG}_8025.log 2>&1 &
	K2HDKC_8025_PID=$!
	sleep 1
	K2HDKC_LT_8025_PID=`ps ax | grep 'lt-k2hdkc' | grep 8025 | grep -v grep | awk '{print $1}'`
	if [ "X${K2HDKC_LT_8025_PID}" = "X" ]; then
		K2HDKC_LT_8025_PID=${K2HDKC_8025_PID}
	fi
	echo "${K2HDKC_8025_PID}, ${K2HDKC_LT_8025_PID}" >> ${LOGFILE}

	echo -n "------- CHMPX(server) 8027             : " >> ${LOGFILE}
	chmpx -conf ${TESTSDIR}/test_server${CONF_FILE_EXT} -ctlport 8027 ${CHMPX_DBG_PARAM} > ${CHMPXLOG}_8027.log 2>&1 &
	CHMPX_8027_PID=$!
	echo "${CHMPX_8027_PID}" >> ${LOGFILE}
	sleep 2

	echo -n "------- K2HDKC 8027                    : " >> ${LOGFILE}
	${K2HDKCBIN} -conf ${TESTSDIR}/test_server${CONF_FILE_EXT} -ctlport 8027 ${DKC_DBG_PARAM} ${DKC_COMLOG_PARAM} > ${K2HDKCLOG}_8027.log 2>&1 &
	K2HDKC_8027_PID=$!
	sleep 1
	K2HDKC_LT_8027_PID=`ps ax | grep 'lt-k2hdkc' | grep 8027 | grep -v grep | awk '{print $1}'`
	if [ "X${K2HDKC_LT_8027_PID}" = "X" ]; then
		K2HDKC_LT_8027_PID=${K2HDKC_8027_PID}
	fi
	echo "${K2HDKC_8027_PID}, ${K2HDKC_LT_8027_PID}" >> ${LOGFILE}

	########################################
	#
	# RUN k2hdkclinetool process
	#
	echo "" >> ${LOGFILE}
	echo "======= [INI CONF] RUN CHMPX(slave) / K2HDKCLINETOOL" >> ${LOGFILE}

	echo -n "------- CHMPX(slave) 8031              : " >> ${LOGFILE}
	chmpx -conf ${TESTSDIR}/test_slave${CONF_FILE_EXT} -ctlport 8031 ${CHMPX_DBG_PARAM} > ${CHMPXLOG}_8031.log 2>&1 &
	CHMPX_8031_PID=$!
	echo "${CHMPX_8031_PID}" >> ${LOGFILE}
	sleep 2

	echo -n "------- [INI CONF] K2HDKCLINETOOL CAPI/PERM       : " >> ${LOGFILE}
	echo "=========================================================================="	> ${LINETOOLLOG}
	echo "======= CAPI : PERM : REJOIN : NOGIVEUP : NOCLEANUP"							>> ${LINETOOLLOG}
	echo "=========================================================================="	>> ${LINETOOLLOG}
	${LINETOOLBIN} -conf ${TESTSDIR}/test_slave${CONF_FILE_EXT} -ctlport 8031 ${DKCTOOL_COMLOG_PARAM} ${DKCTOOL_DBG_PARAM} -perm -rejoin -nogiveup -nocleanup -run ${LINETOOLCMD} -capi >> ${LINETOOLLOG} 2>> ${LINETOOLERRLOG}_C_P.log
	if [ $? -ne 0 ]; then
		echo "FAILED" >> ${LOGFILE}
		echo "FAILED" >> ${LINETOOLLOG}
		TEST_RESULT=1
	else
		echo "SUCCEED" >> ${LOGFILE}
		echo "SUCCEED" >> ${LINETOOLLOG}
	fi
	sleep 1

	echo -n "------- [INI CONF] K2HDKCLINETOOL CPPAPI/PERM     : " >> ${LOGFILE}
	echo "=========================================================================="	>> ${LINETOOLLOG}
	echo "======= CPPAPI : PERM : REJOIN : NOGIVEUP : NOCLEANUP"						>> ${LINETOOLLOG}
	echo "=========================================================================="	>> ${LINETOOLLOG}
	${LINETOOLBIN} -conf ${TESTSDIR}/test_slave${CONF_FILE_EXT} -ctlport 8031 ${DKCTOOL_COMLOG_PARAM} ${DKCTOOL_DBG_PARAM} -perm -rejoin -nogiveup -nocleanup -run ${LINETOOLCMD} >> ${LINETOOLLOG} 2>> ${LINETOOLERRLOG}_CPP_P.log
	if [ $? -ne 0 ]; then
		echo "FAILED" >> ${LOGFILE}
		echo "FAILED" >> ${LINETOOLLOG}
		TEST_RESULT=1
	else
		echo "SUCCEED" >> ${LOGFILE}
		echo "SUCCEED" >> ${LINETOOLLOG}
	fi
	sleep 1

	echo -n "------- [INI CONF] K2HDKCLINETOOL CAPI/NOT PERM   : " >> ${LOGFILE}
	echo "=========================================================================="	>> ${LINETOOLLOG}
	echo "======= CAPI : NOT PERM : REJOIN : NOGIVEUP : NOCLEANUP"						>> ${LINETOOLLOG}
	echo "=========================================================================="	>> ${LINETOOLLOG}
	${LINETOOLBIN} -conf ${TESTSDIR}/test_slave${CONF_FILE_EXT} -ctlport 8031 ${DKCTOOL_COMLOG_PARAM} ${DKCTOOL_DBG_PARAM} -rejoin -nogiveup -nocleanup -run ${LINETOOLCMD} -capi >> ${LINETOOLLOG} 2>> ${LINETOOLERRLOG}_C.log
	if [ $? -ne 0 ]; then
		echo "FAILED" >> ${LOGFILE}
		echo "FAILED" >> ${LINETOOLLOG}
		TEST_RESULT=1
	else
		echo "SUCCEED" >> ${LOGFILE}
		echo "SUCCEED" >> ${LINETOOLLOG}
	fi
	sleep 1

	echo -n "------- [INI CONF] K2HDKCLINETOOL CPPAPI/NOT PERM : " >> ${LOGFILE}
	echo "=========================================================================="	>> ${LINETOOLLOG}
	echo "======= CPPAPI : NOT PERM : REJOIN : NOGIVEUP : NOCLEANUP"					>> ${LINETOOLLOG}
	echo "=========================================================================="	>> ${LINETOOLLOG}
	${LINETOOLBIN} -conf ${TESTSDIR}/test_slave${CONF_FILE_EXT} -ctlport 8031 ${DKCTOOL_COMLOG_PARAM} ${DKCTOOL_DBG_PARAM} -rejoin -nogiveup -nocleanup -run ${LINETOOLCMD} >> ${LINETOOLLOG} 2>> ${LINETOOLERRLOG}_CPP.log
	if [ $? -ne 0 ]; then
		echo "FAILED" >> ${LOGFILE}
		echo "FAILED" >> ${LINETOOLLOG}
		TEST_RESULT=1
	else
		echo "SUCCEED" >> ${LOGFILE}
		echo "SUCCEED" >> ${LINETOOLLOG}
	fi
	sleep 1

	########################################
	#
	# STOP ALL
	#
	echo "" >> ${LOGFILE}
	echo "======= [INI CONF] STOP ALL PROCESSES CHMPX(server/slave) / K2HDKC" >> ${LOGFILE}
	(ps -p ${CHMPX_8031_PID}		> /dev/null 2>&1) && (kill -HUP ${CHMPX_8031_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8027_PID}		> /dev/null 2>&1) && (kill -HUP ${CHMPX_8027_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8027_PID}		> /dev/null 2>&1) && (kill -HUP ${K2HDKC_8027_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8027_PID}	> /dev/null 2>&1) && (kill -HUP ${K2HDKC_LT_8027_PID}	>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8025_PID}		> /dev/null 2>&1) && (kill -HUP ${CHMPX_8025_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8025_PID}		> /dev/null 2>&1) && (kill -HUP ${K2HDKC_8025_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8025_PID}	> /dev/null 2>&1) && (kill -HUP ${K2HDKC_LT_8025_PID}	>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8023_PID}		> /dev/null 2>&1) && (kill -HUP ${CHMPX_8023_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8023_PID}		> /dev/null 2>&1) && (kill -HUP ${K2HDKC_8023_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8023_PID}	> /dev/null 2>&1) && (kill -HUP ${K2HDKC_LT_8023_PID}	>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8021_PID}		> /dev/null 2>&1) && (kill -HUP ${CHMPX_8021_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8021_PID}		> /dev/null 2>&1) && (kill -HUP ${K2HDKC_8021_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8021_PID}	> /dev/null 2>&1) && (kill -HUP ${K2HDKC_LT_8021_PID}	>> ${LOGFILE} 2>&1)
	sleep 10
	(ps -p ${CHMPX_8031_PID}		> /dev/null 2>&1) && (kill -9 ${CHMPX_8031_PID}			>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8027_PID}		> /dev/null 2>&1) && (kill -9 ${CHMPX_8027_PID}			>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8027_PID}		> /dev/null 2>&1) && (kill -9 ${K2HDKC_8027_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8027_PID}	> /dev/null 2>&1) && (kill -9 ${K2HDKC_LT_8027_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8025_PID}		> /dev/null 2>&1) && (kill -9 ${CHMPX_8025_PID}			>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8025_PID}		> /dev/null 2>&1) && (kill -9 ${K2HDKC_8025_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8025_PID}	> /dev/null 2>&1) && (kill -9 ${K2HDKC_LT_8025_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8023_PID}		> /dev/null 2>&1) && (kill -9 ${CHMPX_8023_PID}			>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8023_PID}		> /dev/null 2>&1) && (kill -9 ${K2HDKC_8023_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8023_PID}	> /dev/null 2>&1) && (kill -9 ${K2HDKC_LT_8023_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8021_PID}		> /dev/null 2>&1) && (kill -9 ${CHMPX_8021_PID}			>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8021_PID}		> /dev/null 2>&1) && (kill -9 ${K2HDKC_8021_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8021_PID}	> /dev/null 2>&1) && (kill -9 ${K2HDKC_LT_8021_PID}		>> ${LOGFILE} 2>&1)

	########################################
	#
	# CHECK RESULT
	#
	echo -n "------- [INI CONF] K2HDKCLINETOOL LOG DIFF RESULT : " >> ${LOGFILE}
	diff ${LINETOOLLOG} ${MASTER_LINETOOLLOG} >/dev/null 2>&1
	if [ $? -ne 0 ]; then
		echo "INI CONF TEST RESULT --> FAILED" >> ${LOGFILE}
		echo "" >> ${LOGFILE}

		echo "======= INI FILE TEST                  : FAILED"	>> ${LOGFILE}
		echo ""													>> ${LOGFILE}
		echo "================= `date` ================"		>> ${LOGFILE}

		echo ""
		echo "*********** K2HDKCTOOL LOG DIFF ******************"
		diff ${LINETOOLLOG} ${MASTER_LINETOOLLOG} 2> /dev/null

		echo ""
		echo "*********** DETAIL LOG : LOGFILE *****************"
		cat ${LOGFILE}

		echo ""
		echo "*********** DETAIL LOG : K2HDKCLINETOOL **********"
		cat ${LINETOOLLOG}

		echo ""
		echo "*********** DETAIL LOG : CHMPX(8021) *************"
		cat ${CHMPXLOG}_8021.log

		echo ""
		echo "*********** DETAIL LOG : CHMPX(8023) *************"
		cat ${CHMPXLOG}_8023.log

		echo ""
		echo "*********** DETAIL LOG : CHMPX(8025) *************"
		cat ${CHMPXLOG}_8025.log

		echo ""
		echo "*********** DETAIL LOG : CHMPX(8027) *************"
		cat ${CHMPXLOG}_8027.log

		echo ""
		echo "*********** DETAIL LOG : K2HDKC(8021) *************"
		cat ${K2HDKCLOG}_8021.log

		echo ""
		echo "*********** DETAIL LOG : K2HDKC(8023) *************"
		cat ${K2HDKCLOG}_8023.log

		echo ""
		echo "*********** DETAIL LOG : K2HDKC(8025) *************"
		cat ${K2HDKCLOG}_8025.log

		echo ""
		echo "*********** DETAIL LOG : K2HDKC(8027) *************"
		cat ${K2HDKCLOG}_8027.log

		echo ""
		echo "*********** DETAIL LOG : K2HDKCLINETOOL(C-PERM) ***"
		cat ${LINETOOLERRLOG}_C_P.log

		echo ""
		echo "*********** DETAIL LOG : K2HDKCLINETOOL(CPP-PERM) *"
		cat ${LINETOOLERRLOG}_CPP_P.log

		echo ""
		echo "*********** DETAIL LOG : K2HDKCLINETOOL(C) ********"
		cat ${LINETOOLERRLOG}_C.log

		echo ""
		echo "*********** DETAIL LOG : K2HDKCLINETOOL(CPP) ******"
		cat ${LINETOOLERRLOG}_CPP.log

		TEST_RESULT=1
	else
		echo "INI CONF TEST RESULT --> SUCCEED" >> ${LOGFILE}
		echo "" >> ${LOGFILE}
	fi

	########################################
	#
	# CLEANUP
	#
	if [ ${TEST_RESULT} -eq 0 ]; then
		rm -f ${LOGFILE}
		rm -f ${LINETOOLLOG}
		rm -f ${CHMPXLOG}_8021.log
		rm -f ${CHMPXLOG}_8023.log
		rm -f ${CHMPXLOG}_8025.log
		rm -f ${CHMPXLOG}_8027.log
		rm -f ${CHMPXLOG}_8031.log
		rm -f ${K2HDKCLOG}_8021.log
		rm -f ${K2HDKCLOG}_8023.log
		rm -f ${K2HDKCLOG}_8025.log
		rm -f ${K2HDKCLOG}_8027.log
		rm -f ${LINETOOLERRLOG}_C_P.log
		rm -f ${LINETOOLERRLOG}_CPP_P.log
		rm -f ${LINETOOLERRLOG}_C.log
		rm -f ${LINETOOLERRLOG}_CPP.log
	fi

elif [ ${TEST_RESULT} -eq 0 -a "X${DO_YAML_CONF}" = "Xyes" ]; then
	###########################################################
	## Start YAML conf file
	###########################################################
	echo ""
	echo "====== START TEST FOR YAML CONF FILE ========================"
	echo ""
	CONF_FILE_EXT=".yaml"

	########################################
	#
	# RUN chmpx processes and k2hdkc server processes
	#
	echo "======= [YAML CONF] RUN CHMPX(server) / K2HDKC" >> ${LOGFILE}

	echo -n "------- CHMPX(server) 8021             : " >> ${LOGFILE}
	chmpx -conf ${TESTSDIR}/test_server${CONF_FILE_EXT} -ctlport 8021 ${CHMPX_DBG_PARAM} > ${CHMPXLOG}_8021.log 2>&1 &
	CHMPX_8021_PID=$!
	echo "${CHMPX_8021_PID}" >> ${LOGFILE}
	sleep 2

	echo -n "------- K2HDKC 8021                    : " >> ${LOGFILE}
	${K2HDKCBIN} -conf ${TESTSDIR}/test_server${CONF_FILE_EXT} -ctlport 8021 ${DKC_DBG_PARAM} ${DKC_COMLOG_PARAM} > ${K2HDKCLOG}_8021.log 2>&1 &
	K2HDKC_8021_PID=$!
	sleep 1
	K2HDKC_LT_8021_PID=`ps ax | grep 'lt-k2hdkc' | grep 8021 | grep -v grep | awk '{print $1}'`
	if [ "X${K2HDKC_LT_8021_PID}" = "X" ]; then
		K2HDKC_LT_8021_PID=${K2HDKC_8021_PID}
	fi
	echo "${K2HDKC_8021_PID}, ${K2HDKC_LT_8021_PID}" >> ${LOGFILE}

	echo -n "------- CHMPX(server) 8023             : " >> ${LOGFILE}
	chmpx -conf ${TESTSDIR}/test_server${CONF_FILE_EXT} -ctlport 8023 ${CHMPX_DBG_PARAM} > ${CHMPXLOG}_8023.log 2>&1 &
	CHMPX_8023_PID=$!
	echo "${CHMPX_8023_PID}" >> ${LOGFILE}
	sleep 2

	echo -n "------- K2HDKC 8023                    : " >> ${LOGFILE}
	${K2HDKCBIN} -conf ${TESTSDIR}/test_server${CONF_FILE_EXT} -ctlport 8023 ${DKC_DBG_PARAM} ${DKC_COMLOG_PARAM} > ${K2HDKCLOG}_8023.log 2>&1 &
	K2HDKC_8023_PID=$!
	sleep 1
	K2HDKC_LT_8023_PID=`ps ax | grep 'lt-k2hdkc' | grep 8023 | grep -v grep | awk '{print $1}'`
	if [ "X${K2HDKC_LT_8023_PID}" = "X" ]; then
		K2HDKC_LT_8023_PID=${K2HDKC_8023_PID}
	fi
	echo "${K2HDKC_8023_PID}, ${K2HDKC_LT_8023_PID}" >> ${LOGFILE}

	echo -n "------- CHMPX(server) 8025             : " >> ${LOGFILE}
	chmpx -conf ${TESTSDIR}/test_server${CONF_FILE_EXT} -ctlport 8025 ${CHMPX_DBG_PARAM} > ${CHMPXLOG}_8025.log 2>&1 &
	CHMPX_8025_PID=$!
	echo "${CHMPX_8025_PID}" >> ${LOGFILE}
	sleep 2

	echo -n "------- K2HDKC 8025                    : " >> ${LOGFILE}
	${K2HDKCBIN} -conf ${TESTSDIR}/test_server${CONF_FILE_EXT} -ctlport 8025 ${DKC_DBG_PARAM} ${DKC_COMLOG_PARAM} > ${K2HDKCLOG}_8025.log 2>&1 &
	K2HDKC_8025_PID=$!
	sleep 1
	K2HDKC_LT_8025_PID=`ps ax | grep 'lt-k2hdkc' | grep 8025 | grep -v grep | awk '{print $1}'`
	if [ "X${K2HDKC_LT_8025_PID}" = "X" ]; then
		K2HDKC_LT_8025_PID=${K2HDKC_8025_PID}
	fi
	echo "${K2HDKC_8025_PID}, ${K2HDKC_LT_8025_PID}" >> ${LOGFILE}

	echo -n "------- CHMPX(server) 8027             : " >> ${LOGFILE}
	chmpx -conf ${TESTSDIR}/test_server${CONF_FILE_EXT} -ctlport 8027 ${CHMPX_DBG_PARAM} > ${CHMPXLOG}_8027.log 2>&1 &
	CHMPX_8027_PID=$!
	echo "${CHMPX_8027_PID}" >> ${LOGFILE}
	sleep 2

	echo -n "------- K2HDKC 8027                    : " >> ${LOGFILE}
	${K2HDKCBIN} -conf ${TESTSDIR}/test_server${CONF_FILE_EXT} -ctlport 8027 ${DKC_DBG_PARAM} ${DKC_COMLOG_PARAM} > ${K2HDKCLOG}_8027.log 2>&1 &
	K2HDKC_8027_PID=$!
	sleep 1
	K2HDKC_LT_8027_PID=`ps ax | grep 'lt-k2hdkc' | grep 8027 | grep -v grep | awk '{print $1}'`
	if [ "X${K2HDKC_LT_8027_PID}" = "X" ]; then
		K2HDKC_LT_8027_PID=${K2HDKC_8027_PID}
	fi
	echo "${K2HDKC_8027_PID}, ${K2HDKC_LT_8027_PID}" >> ${LOGFILE}

	########################################
	#
	# RUN k2hdkclinetool process
	#
	TEST_RESULT=0

	echo "" >> ${LOGFILE}
	echo "======= [YAML CONF] RUN CHMPX(slave) / K2HDKCLINETOOL" >> ${LOGFILE}

	echo -n "------- CHMPX(slave) 8031              : " >> ${LOGFILE}
	chmpx -conf ${TESTSDIR}/test_slave${CONF_FILE_EXT} -ctlport 8031 ${CHMPX_DBG_PARAM} > ${CHMPXLOG}_8031.log 2>&1 &
	CHMPX_8031_PID=$!
	echo "${CHMPX_8031_PID}" >> ${LOGFILE}
	sleep 2

	echo -n "------- [YAML CONF] K2HDKCLINETOOL CAPI/PERM       : " >> ${LOGFILE}
	echo "=========================================================================="	> ${LINETOOLLOG}
	echo "======= CAPI : PERM : REJOIN : NOGIVEUP : NOCLEANUP"							>> ${LINETOOLLOG}
	echo "=========================================================================="	>> ${LINETOOLLOG}
	${LINETOOLBIN} -conf ${TESTSDIR}/test_slave${CONF_FILE_EXT} -ctlport 8031 ${DKCTOOL_COMLOG_PARAM} ${DKCTOOL_DBG_PARAM} -perm -rejoin -nogiveup -nocleanup -run ${LINETOOLCMD} -capi >> ${LINETOOLLOG} 2>> ${LINETOOLERRLOG}_C_P.log
	if [ $? -ne 0 ]; then
		echo "FAILED" >> ${LOGFILE}
		echo "FAILED" >> ${LINETOOLLOG}
		TEST_RESULT=1
	else
		echo "SUCCEED" >> ${LOGFILE}
		echo "SUCCEED" >> ${LINETOOLLOG}
	fi
	sleep 1

	echo -n "------- [YAML CONF] K2HDKCLINETOOL CPPAPI/PERM     : " >> ${LOGFILE}
	echo "=========================================================================="	>> ${LINETOOLLOG}
	echo "======= CPPAPI : PERM : REJOIN : NOGIVEUP : NOCLEANUP"						>> ${LINETOOLLOG}
	echo "=========================================================================="	>> ${LINETOOLLOG}
	${LINETOOLBIN} -conf ${TESTSDIR}/test_slave${CONF_FILE_EXT} -ctlport 8031 ${DKCTOOL_COMLOG_PARAM} ${DKCTOOL_DBG_PARAM} -perm -rejoin -nogiveup -nocleanup -run ${LINETOOLCMD} >> ${LINETOOLLOG} 2>> ${LINETOOLERRLOG}_CPP_P.log
	if [ $? -ne 0 ]; then
		echo "FAILED" >> ${LOGFILE}
		echo "FAILED" >> ${LINETOOLLOG}
		TEST_RESULT=1
	else
		echo "SUCCEED" >> ${LOGFILE}
		echo "SUCCEED" >> ${LINETOOLLOG}
	fi
	sleep 1

	echo -n "------- [YAML CONF] K2HDKCLINETOOL CAPI/NOT PERM   : " >> ${LOGFILE}
	echo "=========================================================================="	>> ${LINETOOLLOG}
	echo "======= CAPI : NOT PERM : REJOIN : NOGIVEUP : NOCLEANUP"						>> ${LINETOOLLOG}
	echo "=========================================================================="	>> ${LINETOOLLOG}
	${LINETOOLBIN} -conf ${TESTSDIR}/test_slave${CONF_FILE_EXT} -ctlport 8031 ${DKCTOOL_COMLOG_PARAM} ${DKCTOOL_DBG_PARAM} -rejoin -nogiveup -nocleanup -run ${LINETOOLCMD} -capi >> ${LINETOOLLOG} 2>> ${LINETOOLERRLOG}_C.log
	if [ $? -ne 0 ]; then
		echo "FAILED" >> ${LOGFILE}
		echo "FAILED" >> ${LINETOOLLOG}
		TEST_RESULT=1
	else
		echo "SUCCEED" >> ${LOGFILE}
		echo "SUCCEED" >> ${LINETOOLLOG}
	fi
	sleep 1

	echo -n "------- [YAML CONF] K2HDKCLINETOOL CPPAPI/NOT PERM : " >> ${LOGFILE}
	echo "=========================================================================="	>> ${LINETOOLLOG}
	echo "======= CPPAPI : NOT PERM : REJOIN : NOGIVEUP : NOCLEANUP"					>> ${LINETOOLLOG}
	echo "=========================================================================="	>> ${LINETOOLLOG}
	${LINETOOLBIN} -conf ${TESTSDIR}/test_slave${CONF_FILE_EXT} -ctlport 8031 ${DKCTOOL_COMLOG_PARAM} ${DKCTOOL_DBG_PARAM} -rejoin -nogiveup -nocleanup -run ${LINETOOLCMD} >> ${LINETOOLLOG} 2>> ${LINETOOLERRLOG}_CPP.log
	if [ $? -ne 0 ]; then
		echo "FAILED" >> ${LOGFILE}
		echo "FAILED" >> ${LINETOOLLOG}
		TEST_RESULT=1
	else
		echo "SUCCEED" >> ${LOGFILE}
		echo "SUCCEED" >> ${LINETOOLLOG}
	fi
	sleep 1

	########################################
	#
	# STOP ALL
	#
	echo "" >> ${LOGFILE}
	echo "======= [YAML CONF] STOP ALL PROCESSES CHMPX(server/slave) / K2HDKC" >> ${LOGFILE}
	(ps -p ${CHMPX_8031_PID}		> /dev/null 2>&1) && (kill -HUP ${CHMPX_8031_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8027_PID}		> /dev/null 2>&1) && (kill -HUP ${CHMPX_8027_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8027_PID}		> /dev/null 2>&1) && (kill -HUP ${K2HDKC_8027_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8027_PID}	> /dev/null 2>&1) && (kill -HUP ${K2HDKC_LT_8027_PID}	>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8025_PID}		> /dev/null 2>&1) && (kill -HUP ${CHMPX_8025_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8025_PID}		> /dev/null 2>&1) && (kill -HUP ${K2HDKC_8025_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8025_PID}	> /dev/null 2>&1) && (kill -HUP ${K2HDKC_LT_8025_PID}	>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8023_PID}		> /dev/null 2>&1) && (kill -HUP ${CHMPX_8023_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8023_PID}		> /dev/null 2>&1) && (kill -HUP ${K2HDKC_8023_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8023_PID}	> /dev/null 2>&1) && (kill -HUP ${K2HDKC_LT_8023_PID}	>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8021_PID}		> /dev/null 2>&1) && (kill -HUP ${CHMPX_8021_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8021_PID}		> /dev/null 2>&1) && (kill -HUP ${K2HDKC_8021_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8021_PID}	> /dev/null 2>&1) && (kill -HUP ${K2HDKC_LT_8021_PID}	>> ${LOGFILE} 2>&1)
	sleep 10
	(ps -p ${CHMPX_8031_PID}		> /dev/null 2>&1) && (kill -9 ${CHMPX_8031_PID}			>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8027_PID}		> /dev/null 2>&1) && (kill -9 ${CHMPX_8027_PID}			>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8027_PID}		> /dev/null 2>&1) && (kill -9 ${K2HDKC_8027_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8027_PID}	> /dev/null 2>&1) && (kill -9 ${K2HDKC_LT_8027_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8025_PID}		> /dev/null 2>&1) && (kill -9 ${CHMPX_8025_PID}			>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8025_PID}		> /dev/null 2>&1) && (kill -9 ${K2HDKC_8025_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8025_PID}	> /dev/null 2>&1) && (kill -9 ${K2HDKC_LT_8025_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8023_PID}		> /dev/null 2>&1) && (kill -9 ${CHMPX_8023_PID}			>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8023_PID}		> /dev/null 2>&1) && (kill -9 ${K2HDKC_8023_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8023_PID}	> /dev/null 2>&1) && (kill -9 ${K2HDKC_LT_8023_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8021_PID}		> /dev/null 2>&1) && (kill -9 ${CHMPX_8021_PID}			>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8021_PID}		> /dev/null 2>&1) && (kill -9 ${K2HDKC_8021_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8021_PID}	> /dev/null 2>&1) && (kill -9 ${K2HDKC_LT_8021_PID}		>> ${LOGFILE} 2>&1)

	########################################
	#
	# CHECK RESULT
	#
	echo -n "------- [YAML CONF] K2HDKCLINETOOL LOG DIFF RESULT : " >> ${LOGFILE}
	diff ${LINETOOLLOG} ${MASTER_LINETOOLLOG} >/dev/null 2>&1
	if [ $? -ne 0 ]; then
		echo "YAML CONF TEST RESULT --> FAILED" >> ${LOGFILE}
		echo "" >> ${LOGFILE}

		echo "======= YAML CONF FILE TEST            : FAILED"	>> ${LOGFILE}
		echo ""													>> ${LOGFILE}
		echo "================= `date` ================"		>> ${LOGFILE}

		echo ""
		echo "*********** K2HDKCTOOL LOG DIFF ******************"
		diff ${LINETOOLLOG} ${MASTER_LINETOOLLOG} 2> /dev/null

		echo ""
		echo "*********** DETAIL LOG : LOGFILE *****************"
		cat ${LOGFILE}

		echo ""
		echo "*********** DETAIL LOG : K2HDKCLINETOOL **********"
		cat ${LINETOOLLOG}

		echo ""
		echo "*********** DETAIL LOG : CHMPX(8021) *************"
		cat ${CHMPXLOG}_8021.log

		echo ""
		echo "*********** DETAIL LOG : CHMPX(8023) *************"
		cat ${CHMPXLOG}_8023.log

		echo ""
		echo "*********** DETAIL LOG : CHMPX(8025) *************"
		cat ${CHMPXLOG}_8025.log

		echo ""
		echo "*********** DETAIL LOG : CHMPX(8027) *************"
		cat ${CHMPXLOG}_8027.log

		echo ""
		echo "*********** DETAIL LOG : K2HDKC(8021) *************"
		cat ${K2HDKCLOG}_8021.log

		echo ""
		echo "*********** DETAIL LOG : K2HDKC(8023) *************"
		cat ${K2HDKCLOG}_8023.log

		echo ""
		echo "*********** DETAIL LOG : K2HDKC(8025) *************"
		cat ${K2HDKCLOG}_8025.log

		echo ""
		echo "*********** DETAIL LOG : K2HDKC(8027) *************"
		cat ${K2HDKCLOG}_8027.log

		echo ""
		echo "*********** DETAIL LOG : K2HDKCLINETOOL(C-PERM) ***"
		cat ${LINETOOLERRLOG}_C_P.log

		echo ""
		echo "*********** DETAIL LOG : K2HDKCLINETOOL(CPP-PERM) *"
		cat ${LINETOOLERRLOG}_CPP_P.log

		echo ""
		echo "*********** DETAIL LOG : K2HDKCLINETOOL(C) ********"
		cat ${LINETOOLERRLOG}_C.log

		echo ""
		echo "*********** DETAIL LOG : K2HDKCLINETOOL(CPP) ******"
		cat ${LINETOOLERRLOG}_CPP.log

		TEST_RESULT=1
	else
		echo "YAML CONF TEST RESULT --> SUCCEED" >> ${LOGFILE}
		echo "" >> ${LOGFILE}
	fi

	########################################
	#
	# CLEANUP
	#
	if [ ${TEST_RESULT} -eq 0 ]; then
		rm -f ${LOGFILE}
		rm -f ${LINETOOLLOG}
		rm -f ${CHMPXLOG}_8021.log
		rm -f ${CHMPXLOG}_8023.log
		rm -f ${CHMPXLOG}_8025.log
		rm -f ${CHMPXLOG}_8027.log
		rm -f ${CHMPXLOG}_8031.log
		rm -f ${K2HDKCLOG}_8021.log
		rm -f ${K2HDKCLOG}_8023.log
		rm -f ${K2HDKCLOG}_8025.log
		rm -f ${K2HDKCLOG}_8027.log
		rm -f ${LINETOOLERRLOG}_C_P.log
		rm -f ${LINETOOLERRLOG}_CPP_P.log
		rm -f ${LINETOOLERRLOG}_C.log
		rm -f ${LINETOOLERRLOG}_CPP.log
	fi

elif [ ${TEST_RESULT} -eq 0 -a "X${DO_JSON_CONF}" = "Xyes" ]; then
	###########################################################
	## Start JSON conf file
	###########################################################
	echo ""
	echo ""
	echo "====== START TEST FOR JSON CONF FILE ========================"
	echo ""
	CONF_FILE_EXT=".json"

	########################################
	#
	# RUN chmpx processes and k2hdkc server processes
	#
	echo "======= [JSON CONF] RUN CHMPX(server) / K2HDKC" >> ${LOGFILE}

	echo -n "------- CHMPX(server) 8021             : " >> ${LOGFILE}
	chmpx -conf ${TESTSDIR}/test_server${CONF_FILE_EXT} -ctlport 8021 ${CHMPX_DBG_PARAM} > ${CHMPXLOG}_8021.log 2>&1 &
	CHMPX_8021_PID=$!
	echo "${CHMPX_8021_PID}" >> ${LOGFILE}
	sleep 2

	echo -n "------- K2HDKC 8021                    : " >> ${LOGFILE}
	${K2HDKCBIN} -conf ${TESTSDIR}/test_server${CONF_FILE_EXT} -ctlport 8021 ${DKC_DBG_PARAM} ${DKC_COMLOG_PARAM} > ${K2HDKCLOG}_8021.log 2>&1 &
	K2HDKC_8021_PID=$!
	sleep 1
	K2HDKC_LT_8021_PID=`ps ax | grep 'lt-k2hdkc' | grep 8021 | grep -v grep | awk '{print $1}'`
	if [ "X${K2HDKC_LT_8021_PID}" = "X" ]; then
		K2HDKC_LT_8021_PID=${K2HDKC_8021_PID}
	fi
	echo "${K2HDKC_8021_PID}, ${K2HDKC_LT_8021_PID}" >> ${LOGFILE}

	echo -n "------- CHMPX(server) 8023             : " >> ${LOGFILE}
	chmpx -conf ${TESTSDIR}/test_server${CONF_FILE_EXT} -ctlport 8023 ${CHMPX_DBG_PARAM} > ${CHMPXLOG}_8023.log 2>&1 &
	CHMPX_8023_PID=$!
	echo "${CHMPX_8023_PID}" >> ${LOGFILE}
	sleep 2

	echo -n "------- K2HDKC 8023                    : " >> ${LOGFILE}
	${K2HDKCBIN} -conf ${TESTSDIR}/test_server${CONF_FILE_EXT} -ctlport 8023 ${DKC_DBG_PARAM} ${DKC_COMLOG_PARAM} > ${K2HDKCLOG}_8023.log 2>&1 &
	K2HDKC_8023_PID=$!
	sleep 1
	K2HDKC_LT_8023_PID=`ps ax | grep 'lt-k2hdkc' | grep 8023 | grep -v grep | awk '{print $1}'`
	if [ "X${K2HDKC_LT_8023_PID}" = "X" ]; then
		K2HDKC_LT_8023_PID=${K2HDKC_8023_PID}
	fi
	echo "${K2HDKC_8023_PID}, ${K2HDKC_LT_8023_PID}" >> ${LOGFILE}

	echo -n "------- CHMPX(server) 8025             : " >> ${LOGFILE}
	chmpx -conf ${TESTSDIR}/test_server${CONF_FILE_EXT} -ctlport 8025 ${CHMPX_DBG_PARAM} > ${CHMPXLOG}_8025.log 2>&1 &
	CHMPX_8025_PID=$!
	echo "${CHMPX_8025_PID}" >> ${LOGFILE}
	sleep 2

	echo -n "------- K2HDKC 8025                    : " >> ${LOGFILE}
	${K2HDKCBIN} -conf ${TESTSDIR}/test_server${CONF_FILE_EXT} -ctlport 8025 ${DKC_DBG_PARAM} ${DKC_COMLOG_PARAM} > ${K2HDKCLOG}_8025.log 2>&1 &
	K2HDKC_8025_PID=$!
	sleep 1
	K2HDKC_LT_8025_PID=`ps ax | grep 'lt-k2hdkc' | grep 8025 | grep -v grep | awk '{print $1}'`
	if [ "X${K2HDKC_LT_8025_PID}" = "X" ]; then
		K2HDKC_LT_8025_PID=${K2HDKC_8025_PID}
	fi
	echo "${K2HDKC_8025_PID}, ${K2HDKC_LT_8025_PID}" >> ${LOGFILE}

	echo -n "------- CHMPX(server) 8027             : " >> ${LOGFILE}
	chmpx -conf ${TESTSDIR}/test_server${CONF_FILE_EXT} -ctlport 8027 ${CHMPX_DBG_PARAM} > ${CHMPXLOG}_8027.log 2>&1 &
	CHMPX_8027_PID=$!
	echo "${CHMPX_8027_PID}" >> ${LOGFILE}
	sleep 2

	echo -n "------- K2HDKC 8027                    : " >> ${LOGFILE}
	${K2HDKCBIN} -conf ${TESTSDIR}/test_server${CONF_FILE_EXT} -ctlport 8027 ${DKC_DBG_PARAM} ${DKC_COMLOG_PARAM} > ${K2HDKCLOG}_8027.log 2>&1 &
	K2HDKC_8027_PID=$!
	sleep 1
	K2HDKC_LT_8027_PID=`ps ax | grep 'lt-k2hdkc' | grep 8027 | grep -v grep | awk '{print $1}'`
	if [ "X${K2HDKC_LT_8027_PID}" = "X" ]; then
		K2HDKC_LT_8027_PID=${K2HDKC_8027_PID}
	fi
	echo "${K2HDKC_8027_PID}, ${K2HDKC_LT_8027_PID}" >> ${LOGFILE}

	########################################
	#
	# RUN k2hdkclinetool process
	#
	TEST_RESULT=0

	echo "" >> ${LOGFILE}
	echo "======= [JSON CONF] RUN CHMPX(slave) / K2HDKCLINETOOL" >> ${LOGFILE}

	echo -n "------- CHMPX(slave) 8031              : " >> ${LOGFILE}
	chmpx -conf ${TESTSDIR}/test_slave${CONF_FILE_EXT} -ctlport 8031 ${CHMPX_DBG_PARAM} > ${CHMPXLOG}_8031.log 2>&1 &
	CHMPX_8031_PID=$!
	echo "${CHMPX_8031_PID}" >> ${LOGFILE}
	sleep 2

	echo -n "------- [JSON CONF] K2HDKCLINETOOL CAPI/PERM       : " >> ${LOGFILE}
	echo "=========================================================================="	> ${LINETOOLLOG}
	echo "======= CAPI : PERM : REJOIN : NOGIVEUP : NOCLEANUP"							>> ${LINETOOLLOG}
	echo "=========================================================================="	>> ${LINETOOLLOG}
	${LINETOOLBIN} -conf ${TESTSDIR}/test_slave${CONF_FILE_EXT} -ctlport 8031 ${DKCTOOL_COMLOG_PARAM} ${DKCTOOL_DBG_PARAM} -perm -rejoin -nogiveup -nocleanup -run ${LINETOOLCMD} -capi >> ${LINETOOLLOG} 2>> ${LINETOOLERRLOG}_C_P.log
	if [ $? -ne 0 ]; then
		echo "FAILED" >> ${LOGFILE}
		echo "FAILED" >> ${LINETOOLLOG}
		TEST_RESULT=1
	else
		echo "SUCCEED" >> ${LOGFILE}
		echo "SUCCEED" >> ${LINETOOLLOG}
	fi
	sleep 1

	echo -n "------- [JSON CONF] K2HDKCLINETOOL CPPAPI/PERM     : " >> ${LOGFILE}
	echo "=========================================================================="	>> ${LINETOOLLOG}
	echo "======= CPPAPI : PERM : REJOIN : NOGIVEUP : NOCLEANUP"						>> ${LINETOOLLOG}
	echo "=========================================================================="	>> ${LINETOOLLOG}
	${LINETOOLBIN} -conf ${TESTSDIR}/test_slave${CONF_FILE_EXT} -ctlport 8031 ${DKCTOOL_COMLOG_PARAM} ${DKCTOOL_DBG_PARAM} -perm -rejoin -nogiveup -nocleanup -run ${LINETOOLCMD} >> ${LINETOOLLOG} 2>> ${LINETOOLERRLOG}_CPP_P.log
	if [ $? -ne 0 ]; then
		echo "FAILED" >> ${LOGFILE}
		echo "FAILED" >> ${LINETOOLLOG}
		TEST_RESULT=1
	else
		echo "SUCCEED" >> ${LOGFILE}
		echo "SUCCEED" >> ${LINETOOLLOG}
	fi
	sleep 1

	echo -n "------- [JSON CONF] K2HDKCLINETOOL CAPI/NOT PERM   : " >> ${LOGFILE}
	echo "=========================================================================="	>> ${LINETOOLLOG}
	echo "======= CAPI : NOT PERM : REJOIN : NOGIVEUP : NOCLEANUP"						>> ${LINETOOLLOG}
	echo "=========================================================================="	>> ${LINETOOLLOG}
	${LINETOOLBIN} -conf ${TESTSDIR}/test_slave${CONF_FILE_EXT} -ctlport 8031 ${DKCTOOL_COMLOG_PARAM} ${DKCTOOL_DBG_PARAM} -rejoin -nogiveup -nocleanup -run ${LINETOOLCMD} -capi >> ${LINETOOLLOG} 2>> ${LINETOOLERRLOG}_C.log
	if [ $? -ne 0 ]; then
		echo "FAILED" >> ${LOGFILE}
		echo "FAILED" >> ${LINETOOLLOG}
		TEST_RESULT=1
	else
		echo "SUCCEED" >> ${LOGFILE}
		echo "SUCCEED" >> ${LINETOOLLOG}
	fi
	sleep 1

	echo -n "------- [JSON CONF] K2HDKCLINETOOL CPPAPI/NOT PERM : " >> ${LOGFILE}
	echo "=========================================================================="	>> ${LINETOOLLOG}
	echo "======= CPPAPI : NOT PERM : REJOIN : NOGIVEUP : NOCLEANUP"					>> ${LINETOOLLOG}
	echo "=========================================================================="	>> ${LINETOOLLOG}
	${LINETOOLBIN} -conf ${TESTSDIR}/test_slave${CONF_FILE_EXT} -ctlport 8031 ${DKCTOOL_COMLOG_PARAM} ${DKCTOOL_DBG_PARAM} -rejoin -nogiveup -nocleanup -run ${LINETOOLCMD} >> ${LINETOOLLOG} 2>> ${LINETOOLERRLOG}_CPP.log
	if [ $? -ne 0 ]; then
		echo "FAILED" >> ${LOGFILE}
		echo "FAILED" >> ${LINETOOLLOG}
		TEST_RESULT=1
	else
		echo "SUCCEED" >> ${LOGFILE}
		echo "SUCCEED" >> ${LINETOOLLOG}
	fi
	sleep 1

	########################################
	#
	# STOP ALL
	#
	echo "" >> ${LOGFILE}
	echo "======= [JSON CONF] STOP ALL PROCESSES CHMPX(server/slave) / K2HDKC" >> ${LOGFILE}
	(ps -p ${CHMPX_8031_PID}		> /dev/null 2>&1) && (kill -HUP ${CHMPX_8031_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8027_PID}		> /dev/null 2>&1) && (kill -HUP ${CHMPX_8027_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8027_PID}		> /dev/null 2>&1) && (kill -HUP ${K2HDKC_8027_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8027_PID}	> /dev/null 2>&1) && (kill -HUP ${K2HDKC_LT_8027_PID}	>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8025_PID}		> /dev/null 2>&1) && (kill -HUP ${CHMPX_8025_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8025_PID}		> /dev/null 2>&1) && (kill -HUP ${K2HDKC_8025_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8025_PID}	> /dev/null 2>&1) && (kill -HUP ${K2HDKC_LT_8025_PID}	>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8023_PID}		> /dev/null 2>&1) && (kill -HUP ${CHMPX_8023_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8023_PID}		> /dev/null 2>&1) && (kill -HUP ${K2HDKC_8023_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8023_PID}	> /dev/null 2>&1) && (kill -HUP ${K2HDKC_LT_8023_PID}	>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8021_PID}		> /dev/null 2>&1) && (kill -HUP ${CHMPX_8021_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8021_PID}		> /dev/null 2>&1) && (kill -HUP ${K2HDKC_8021_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8021_PID}	> /dev/null 2>&1) && (kill -HUP ${K2HDKC_LT_8021_PID}	>> ${LOGFILE} 2>&1)
	sleep 10
	(ps -p ${CHMPX_8031_PID}		> /dev/null 2>&1) && (kill -9 ${CHMPX_8031_PID}			>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8027_PID}		> /dev/null 2>&1) && (kill -9 ${CHMPX_8027_PID}			>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8027_PID}		> /dev/null 2>&1) && (kill -9 ${K2HDKC_8027_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8027_PID}	> /dev/null 2>&1) && (kill -9 ${K2HDKC_LT_8027_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8025_PID}		> /dev/null 2>&1) && (kill -9 ${CHMPX_8025_PID}			>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8025_PID}		> /dev/null 2>&1) && (kill -9 ${K2HDKC_8025_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8025_PID}	> /dev/null 2>&1) && (kill -9 ${K2HDKC_LT_8025_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8023_PID}		> /dev/null 2>&1) && (kill -9 ${CHMPX_8023_PID}			>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8023_PID}		> /dev/null 2>&1) && (kill -9 ${K2HDKC_8023_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8023_PID}	> /dev/null 2>&1) && (kill -9 ${K2HDKC_LT_8023_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8021_PID}		> /dev/null 2>&1) && (kill -9 ${CHMPX_8021_PID}			>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8021_PID}		> /dev/null 2>&1) && (kill -9 ${K2HDKC_8021_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8021_PID}	> /dev/null 2>&1) && (kill -9 ${K2HDKC_LT_8021_PID}		>> ${LOGFILE} 2>&1)

	########################################
	#
	# CHECK RESULT
	#
	echo -n "------- [JSON CONF] K2HDKCLINETOOL LOG DIFF RESULT : " >> ${LOGFILE}
	diff ${LINETOOLLOG} ${MASTER_LINETOOLLOG} >/dev/null 2>&1
	if [ $? -ne 0 ]; then
		echo "JSON CONF TEST RESULT --> FAILED" >> ${LOGFILE}
		echo "" >> ${LOGFILE}

		echo "======= JSON CONF FILE TEST            : FAILED"	>> ${LOGFILE}
		echo ""													>> ${LOGFILE}
		echo "================= `date` ================"		>> ${LOGFILE}

		echo ""
		echo "*********** K2HDKCTOOL LOG DIFF ******************"
		diff ${LINETOOLLOG} ${MASTER_LINETOOLLOG} 2> /dev/null

		echo ""
		echo "*********** DETAIL LOG : LOGFILE *****************"
		cat ${LOGFILE}

		echo ""
		echo "*********** DETAIL LOG : K2HDKCLINETOOL **********"
		cat ${LINETOOLLOG}

		echo ""
		echo "*********** DETAIL LOG : CHMPX(8021) *************"
		cat ${CHMPXLOG}_8021.log

		echo ""
		echo "*********** DETAIL LOG : CHMPX(8023) *************"
		cat ${CHMPXLOG}_8023.log

		echo ""
		echo "*********** DETAIL LOG : CHMPX(8025) *************"
		cat ${CHMPXLOG}_8025.log

		echo ""
		echo "*********** DETAIL LOG : CHMPX(8027) *************"
		cat ${CHMPXLOG}_8027.log

		echo ""
		echo "*********** DETAIL LOG : K2HDKC(8021) *************"
		cat ${K2HDKCLOG}_8021.log

		echo ""
		echo "*********** DETAIL LOG : K2HDKC(8023) *************"
		cat ${K2HDKCLOG}_8023.log

		echo ""
		echo "*********** DETAIL LOG : K2HDKC(8025) *************"
		cat ${K2HDKCLOG}_8025.log

		echo ""
		echo "*********** DETAIL LOG : K2HDKC(8027) *************"
		cat ${K2HDKCLOG}_8027.log

		echo ""
		echo "*********** DETAIL LOG : K2HDKCLINETOOL(C-PERM) ***"
		cat ${LINETOOLERRLOG}_C_P.log

		echo ""
		echo "*********** DETAIL LOG : K2HDKCLINETOOL(CPP-PERM) *"
		cat ${LINETOOLERRLOG}_CPP_P.log

		echo ""
		echo "*********** DETAIL LOG : K2HDKCLINETOOL(C) ********"
		cat ${LINETOOLERRLOG}_C.log

		echo ""
		echo "*********** DETAIL LOG : K2HDKCLINETOOL(CPP) ******"
		cat ${LINETOOLERRLOG}_CPP.log

		TEST_RESULT=1
	else
		echo "JSON CONF TEST RESULT --> SUCCEED" >> ${LOGFILE}
		echo "" >> ${LOGFILE}
	fi

	########################################
	#
	# CLEANUP
	#
	if [ ${TEST_RESULT} -eq 0 ]; then
		rm -f ${LOGFILE}
		rm -f ${LINETOOLLOG}
		rm -f ${CHMPXLOG}_8021.log
		rm -f ${CHMPXLOG}_8023.log
		rm -f ${CHMPXLOG}_8025.log
		rm -f ${CHMPXLOG}_8027.log
		rm -f ${CHMPXLOG}_8031.log
		rm -f ${K2HDKCLOG}_8021.log
		rm -f ${K2HDKCLOG}_8023.log
		rm -f ${K2HDKCLOG}_8025.log
		rm -f ${K2HDKCLOG}_8027.log
		rm -f ${LINETOOLERRLOG}_C_P.log
		rm -f ${LINETOOLERRLOG}_CPP_P.log
		rm -f ${LINETOOLERRLOG}_C.log
		rm -f ${LINETOOLERRLOG}_CPP.log
	fi

elif [ ${TEST_RESULT} -eq 0 -a "X${DO_JSON_STRING}" = "Xyes" ]; then
	###########################################################
	## Start JSON string
	###########################################################
	echo ""
	echo "====== START TEST FOR JSON STRING ============================="
	echo ""

	########################################
	#
	# RUN chmpx processes and k2hdkc server processes
	#
	echo "======= [JSON STRING] RUN CHMPX(server) / K2HDKC" >> ${LOGFILE}

	echo -n "------- CHMPX(server) 8021             : " >> ${LOGFILE}
	chmpx -json "${TEST_SERVER_JSON}" -ctlport 8021 ${CHMPX_DBG_PARAM} > ${CHMPXLOG}_8021.log 2>&1 &
	CHMPX_8021_PID=$!
	echo "${CHMPX_8021_PID}" >> ${LOGFILE}
	sleep 2

	echo -n "------- K2HDKC 8021                    : " >> ${LOGFILE}
	${K2HDKCBIN} -json "${TEST_SERVER_JSON}" -ctlport 8021 ${DKC_DBG_PARAM} ${DKC_COMLOG_PARAM} > ${K2HDKCLOG}_8021.log 2>&1 &
	K2HDKC_8021_PID=$!
	sleep 1
	K2HDKC_LT_8021_PID=`ps ax | grep 'lt-k2hdkc' | grep 8021 | grep -v grep | awk '{print $1}'`
	if [ "X${K2HDKC_LT_8021_PID}" = "X" ]; then
		K2HDKC_LT_8021_PID=${K2HDKC_8021_PID}
	fi
	echo "${K2HDKC_8021_PID}, ${K2HDKC_LT_8021_PID}" >> ${LOGFILE}

	echo -n "------- CHMPX(server) 8023             : " >> ${LOGFILE}
	chmpx -json "${TEST_SERVER_JSON}" -ctlport 8023 ${CHMPX_DBG_PARAM} > ${CHMPXLOG}_8023.log 2>&1 &
	CHMPX_8023_PID=$!
	echo "${CHMPX_8023_PID}" >> ${LOGFILE}
	sleep 2

	echo -n "------- K2HDKC 8023                    : " >> ${LOGFILE}
	${K2HDKCBIN} -json "${TEST_SERVER_JSON}" -ctlport 8023 ${DKC_DBG_PARAM} ${DKC_COMLOG_PARAM} > ${K2HDKCLOG}_8023.log 2>&1 &
	K2HDKC_8023_PID=$!
	sleep 1
	K2HDKC_LT_8023_PID=`ps ax | grep 'lt-k2hdkc' | grep 8023 | grep -v grep | awk '{print $1}'`
	if [ "X${K2HDKC_LT_8023_PID}" = "X" ]; then
		K2HDKC_LT_8023_PID=${K2HDKC_8023_PID}
	fi
	echo "${K2HDKC_8023_PID}, ${K2HDKC_LT_8023_PID}" >> ${LOGFILE}

	echo -n "------- CHMPX(server) 8025             : " >> ${LOGFILE}
	chmpx -json "${TEST_SERVER_JSON}" -ctlport 8025 ${CHMPX_DBG_PARAM} > ${CHMPXLOG}_8025.log 2>&1 &
	CHMPX_8025_PID=$!
	echo "${CHMPX_8025_PID}" >> ${LOGFILE}
	sleep 2

	echo -n "------- K2HDKC 8025                    : " >> ${LOGFILE}
	${K2HDKCBIN} -json "${TEST_SERVER_JSON}" -ctlport 8025 ${DKC_DBG_PARAM} ${DKC_COMLOG_PARAM} > ${K2HDKCLOG}_8025.log 2>&1 &
	K2HDKC_8025_PID=$!
	sleep 1
	K2HDKC_LT_8025_PID=`ps ax | grep 'lt-k2hdkc' | grep 8025 | grep -v grep | awk '{print $1}'`
	if [ "X${K2HDKC_LT_8025_PID}" = "X" ]; then
		K2HDKC_LT_8025_PID=${K2HDKC_8025_PID}
	fi
	echo "${K2HDKC_8025_PID}, ${K2HDKC_LT_8025_PID}" >> ${LOGFILE}

	echo -n "------- CHMPX(server) 8027             : " >> ${LOGFILE}
	chmpx -json "${TEST_SERVER_JSON}" -ctlport 8027 ${CHMPX_DBG_PARAM} > ${CHMPXLOG}_8027.log 2>&1 &
	CHMPX_8027_PID=$!
	echo "${CHMPX_8027_PID}" >> ${LOGFILE}
	sleep 2

	echo -n "------- K2HDKC 8027                    : " >> ${LOGFILE}
	${K2HDKCBIN} -json "${TEST_SERVER_JSON}" -ctlport 8027 ${DKC_DBG_PARAM} ${DKC_COMLOG_PARAM} > ${K2HDKCLOG}_8027.log 2>&1 &
	K2HDKC_8027_PID=$!
	sleep 1
	K2HDKC_LT_8027_PID=`ps ax | grep 'lt-k2hdkc' | grep 8027 | grep -v grep | awk '{print $1}'`
	if [ "X${K2HDKC_LT_8027_PID}" = "X" ]; then
		K2HDKC_LT_8027_PID=${K2HDKC_8027_PID}
	fi
	echo "${K2HDKC_8027_PID}, ${K2HDKC_LT_8027_PID}" >> ${LOGFILE}

	########################################
	#
	# RUN k2hdkclinetool process
	#
	TEST_RESULT=0

	echo "" >> ${LOGFILE}
	echo "======= [JSON STRING] RUN CHMPX(slave) / K2HDKCLINETOOL" >> ${LOGFILE}

	echo -n "------- CHMPX(slave) 8031              : " >> ${LOGFILE}
	chmpx -json "${TEST_SLAVE_JSON}" -ctlport 8031 ${CHMPX_DBG_PARAM} > ${CHMPXLOG}_8031.log 2>&1 &
	CHMPX_8031_PID=$!
	echo "${CHMPX_8031_PID}" >> ${LOGFILE}
	sleep 2

	echo -n "------- [JSON STRING] K2HDKCLINETOOL CAPI/PERM       : " >> ${LOGFILE}
	echo "=========================================================================="	> ${LINETOOLLOG}
	echo "======= CAPI : PERM : REJOIN : NOGIVEUP : NOCLEANUP"							>> ${LINETOOLLOG}
	echo "=========================================================================="	>> ${LINETOOLLOG}
	${LINETOOLBIN} -json "${TEST_SLAVE_JSON}" -ctlport 8031 ${DKCTOOL_COMLOG_PARAM} ${DKCTOOL_DBG_PARAM} -perm -rejoin -nogiveup -nocleanup -run ${LINETOOLCMD} -capi >> ${LINETOOLLOG} 2>> ${LINETOOLERRLOG}_C_P.log
	if [ $? -ne 0 ]; then
		echo "FAILED" >> ${LOGFILE}
		echo "FAILED" >> ${LINETOOLLOG}
		TEST_RESULT=1
	else
		echo "SUCCEED" >> ${LOGFILE}
		echo "SUCCEED" >> ${LINETOOLLOG}
	fi
	sleep 1

	echo -n "------- [JSON STRING] K2HDKCLINETOOL CPPAPI/PERM     : " >> ${LOGFILE}
	echo "=========================================================================="	>> ${LINETOOLLOG}
	echo "======= CPPAPI : PERM : REJOIN : NOGIVEUP : NOCLEANUP"						>> ${LINETOOLLOG}
	echo "=========================================================================="	>> ${LINETOOLLOG}
	${LINETOOLBIN} -json "${TEST_SLAVE_JSON}" -ctlport 8031 ${DKCTOOL_COMLOG_PARAM} ${DKCTOOL_DBG_PARAM} -perm -rejoin -nogiveup -nocleanup -run ${LINETOOLCMD} >> ${LINETOOLLOG} 2>> ${LINETOOLERRLOG}_CPP_P.log
	if [ $? -ne 0 ]; then
		echo "FAILED" >> ${LOGFILE}
		echo "FAILED" >> ${LINETOOLLOG}
		TEST_RESULT=1
	else
		echo "SUCCEED" >> ${LOGFILE}
		echo "SUCCEED" >> ${LINETOOLLOG}
	fi
	sleep 1

	echo -n "------- [JSON STRING] K2HDKCLINETOOL CAPI/NOT PERM   : " >> ${LOGFILE}
	echo "=========================================================================="	>> ${LINETOOLLOG}
	echo "======= CAPI : NOT PERM : REJOIN : NOGIVEUP : NOCLEANUP"						>> ${LINETOOLLOG}
	echo "=========================================================================="	>> ${LINETOOLLOG}
	${LINETOOLBIN} -json "${TEST_SLAVE_JSON}" -ctlport 8031 ${DKCTOOL_COMLOG_PARAM} ${DKCTOOL_DBG_PARAM} -rejoin -nogiveup -nocleanup -run ${LINETOOLCMD} -capi >> ${LINETOOLLOG} 2>> ${LINETOOLERRLOG}_C.log
	if [ $? -ne 0 ]; then
		echo "FAILED" >> ${LOGFILE}
		echo "FAILED" >> ${LINETOOLLOG}
		TEST_RESULT=1
	else
		echo "SUCCEED" >> ${LOGFILE}
		echo "SUCCEED" >> ${LINETOOLLOG}
	fi
	sleep 1

	echo -n "------- [JSON STRING] K2HDKCLINETOOL CPPAPI/NOT PERM : " >> ${LOGFILE}
	echo "=========================================================================="	>> ${LINETOOLLOG}
	echo "======= CPPAPI : NOT PERM : REJOIN : NOGIVEUP : NOCLEANUP"					>> ${LINETOOLLOG}
	echo "=========================================================================="	>> ${LINETOOLLOG}
	${LINETOOLBIN} -json "${TEST_SLAVE_JSON}" -ctlport 8031 ${DKCTOOL_COMLOG_PARAM} ${DKCTOOL_DBG_PARAM} -rejoin -nogiveup -nocleanup -run ${LINETOOLCMD} >> ${LINETOOLLOG} 2>> ${LINETOOLERRLOG}_CPP.log
	if [ $? -ne 0 ]; then
		echo "FAILED" >> ${LOGFILE}
		echo "FAILED" >> ${LINETOOLLOG}
		TEST_RESULT=1
	else
		echo "SUCCEED" >> ${LOGFILE}
		echo "SUCCEED" >> ${LINETOOLLOG}
	fi
	sleep 1

	########################################
	#
	# STOP ALL
	#
	echo "" >> ${LOGFILE}
	echo "======= [JSON STRING] STOP ALL PROCESSES CHMPX(server/slave) / K2HDKC" >> ${LOGFILE}
	(ps -p ${CHMPX_8031_PID}		> /dev/null 2>&1) && (kill -HUP ${CHMPX_8031_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8027_PID}		> /dev/null 2>&1) && (kill -HUP ${CHMPX_8027_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8027_PID}		> /dev/null 2>&1) && (kill -HUP ${K2HDKC_8027_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8027_PID}	> /dev/null 2>&1) && (kill -HUP ${K2HDKC_LT_8027_PID}	>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8025_PID}		> /dev/null 2>&1) && (kill -HUP ${CHMPX_8025_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8025_PID}		> /dev/null 2>&1) && (kill -HUP ${K2HDKC_8025_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8025_PID}	> /dev/null 2>&1) && (kill -HUP ${K2HDKC_LT_8025_PID}	>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8023_PID}		> /dev/null 2>&1) && (kill -HUP ${CHMPX_8023_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8023_PID}		> /dev/null 2>&1) && (kill -HUP ${K2HDKC_8023_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8023_PID}	> /dev/null 2>&1) && (kill -HUP ${K2HDKC_LT_8023_PID}	>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8021_PID}		> /dev/null 2>&1) && (kill -HUP ${CHMPX_8021_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8021_PID}		> /dev/null 2>&1) && (kill -HUP ${K2HDKC_8021_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8021_PID}	> /dev/null 2>&1) && (kill -HUP ${K2HDKC_LT_8021_PID}	>> ${LOGFILE} 2>&1)
	sleep 10
	(ps -p ${CHMPX_8031_PID}		> /dev/null 2>&1) && (kill -9 ${CHMPX_8031_PID}			>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8027_PID}		> /dev/null 2>&1) && (kill -9 ${CHMPX_8027_PID}			>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8027_PID}		> /dev/null 2>&1) && (kill -9 ${K2HDKC_8027_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8027_PID}	> /dev/null 2>&1) && (kill -9 ${K2HDKC_LT_8027_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8025_PID}		> /dev/null 2>&1) && (kill -9 ${CHMPX_8025_PID}			>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8025_PID}		> /dev/null 2>&1) && (kill -9 ${K2HDKC_8025_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8025_PID}	> /dev/null 2>&1) && (kill -9 ${K2HDKC_LT_8025_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8023_PID}		> /dev/null 2>&1) && (kill -9 ${CHMPX_8023_PID}			>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8023_PID}		> /dev/null 2>&1) && (kill -9 ${K2HDKC_8023_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8023_PID}	> /dev/null 2>&1) && (kill -9 ${K2HDKC_LT_8023_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8021_PID}		> /dev/null 2>&1) && (kill -9 ${CHMPX_8021_PID}			>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8021_PID}		> /dev/null 2>&1) && (kill -9 ${K2HDKC_8021_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8021_PID}	> /dev/null 2>&1) && (kill -9 ${K2HDKC_LT_8021_PID}		>> ${LOGFILE} 2>&1)

	########################################
	#
	# CHECK RESULT
	#
	echo -n "------- [JSON STRING] K2HDKCLINETOOL LOG DIFF RESULT : " >> ${LOGFILE}
	diff ${LINETOOLLOG} ${MASTER_LINETOOLLOG} >/dev/null 2>&1
	if [ $? -ne 0 ]; then
		echo "JSON STRING TEST RESULT --> FAILED" >> ${LOGFILE}
		echo "" >> ${LOGFILE}

		echo "======= JSON STRING TEST               : FAILED"	>> ${LOGFILE}
		echo ""													>> ${LOGFILE}
		echo "================= `date` ================"		>> ${LOGFILE}

		echo ""
		echo "*********** K2HDKCTOOL LOG DIFF ******************"
		diff ${LINETOOLLOG} ${MASTER_LINETOOLLOG} 2> /dev/null

		echo ""
		echo "*********** DETAIL LOG : LOGFILE *****************"
		cat ${LOGFILE}

		echo ""
		echo "*********** DETAIL LOG : K2HDKCLINETOOL **********"
		cat ${LINETOOLLOG}

		echo ""
		echo "*********** DETAIL LOG : CHMPX(8021) *************"
		cat ${CHMPXLOG}_8021.log

		echo ""
		echo "*********** DETAIL LOG : CHMPX(8023) *************"
		cat ${CHMPXLOG}_8023.log

		echo ""
		echo "*********** DETAIL LOG : CHMPX(8025) *************"
		cat ${CHMPXLOG}_8025.log

		echo ""
		echo "*********** DETAIL LOG : CHMPX(8027) *************"
		cat ${CHMPXLOG}_8027.log

		echo ""
		echo "*********** DETAIL LOG : K2HDKC(8021) *************"
		cat ${K2HDKCLOG}_8021.log

		echo ""
		echo "*********** DETAIL LOG : K2HDKC(8023) *************"
		cat ${K2HDKCLOG}_8023.log

		echo ""
		echo "*********** DETAIL LOG : K2HDKC(8025) *************"
		cat ${K2HDKCLOG}_8025.log

		echo ""
		echo "*********** DETAIL LOG : K2HDKC(8027) *************"
		cat ${K2HDKCLOG}_8027.log

		echo ""
		echo "*********** DETAIL LOG : K2HDKCLINETOOL(C-PERM) ***"
		cat ${LINETOOLERRLOG}_C_P.log

		echo ""
		echo "*********** DETAIL LOG : K2HDKCLINETOOL(CPP-PERM) *"
		cat ${LINETOOLERRLOG}_CPP_P.log

		echo ""
		echo "*********** DETAIL LOG : K2HDKCLINETOOL(C) ********"
		cat ${LINETOOLERRLOG}_C.log

		echo ""
		echo "*********** DETAIL LOG : K2HDKCLINETOOL(CPP) ******"
		cat ${LINETOOLERRLOG}_CPP.log

		TEST_RESULT=1
	else
		echo "JSON STRING TEST RESULT --> SUCCEED" >> ${LOGFILE}
		echo "" >> ${LOGFILE}
	fi

	########################################
	#
	# CLEANUP
	#
	if [ ${TEST_RESULT} -eq 0 ]; then
		rm -f ${LOGFILE}
		rm -f ${LINETOOLLOG}
		rm -f ${CHMPXLOG}_8021.log
		rm -f ${CHMPXLOG}_8023.log
		rm -f ${CHMPXLOG}_8025.log
		rm -f ${CHMPXLOG}_8027.log
		rm -f ${CHMPXLOG}_8031.log
		rm -f ${K2HDKCLOG}_8021.log
		rm -f ${K2HDKCLOG}_8023.log
		rm -f ${K2HDKCLOG}_8025.log
		rm -f ${K2HDKCLOG}_8027.log
		rm -f ${LINETOOLERRLOG}_C_P.log
		rm -f ${LINETOOLERRLOG}_CPP_P.log
		rm -f ${LINETOOLERRLOG}_C.log
		rm -f ${LINETOOLERRLOG}_CPP.log
	fi

elif [ ${TEST_RESULT} -eq 0 -a "X${DO_JSON_ENV}" = "Xyes" ]; then
	###########################################################
	## Start JSON environment
	###########################################################
	echo ""
	echo "====== START TEST FOR JSON ENV ============================="
	echo ""

	########################################
	#
	# RUN chmpx processes and k2hdkc server processes
	#
	echo "======= [JSON ENV] RUN CHMPX(server) / K2HDKC" >> ${LOGFILE}

	echo -n "------- CHMPX(server) 8021             : " >> ${LOGFILE}
	CHMJSONCONF="${TEST_SERVER_JSON}" chmpx -ctlport 8021 ${CHMPX_DBG_PARAM} > ${CHMPXLOG}_8021.log 2>&1 &
	CHMPX_8021_PID=$!
	echo "${CHMPX_8021_PID}" >> ${LOGFILE}
	sleep 2

	echo -n "------- K2HDKC 8021                    : " >> ${LOGFILE}
	K2HDKCJSONCONF="${TEST_SERVER_JSON}" ${K2HDKCBIN} -ctlport 8021 ${DKC_DBG_PARAM} ${DKC_COMLOG_PARAM} > ${K2HDKCLOG}_8021.log 2>&1 &
	K2HDKC_8021_PID=$!
	sleep 1
	K2HDKC_LT_8021_PID=`ps ax | grep 'lt-k2hdkc' | grep 8021 | grep -v grep | awk '{print $1}'`
	if [ "X${K2HDKC_LT_8021_PID}" = "X" ]; then
		K2HDKC_LT_8021_PID=${K2HDKC_8021_PID}
	fi
	echo "${K2HDKC_8021_PID}, ${K2HDKC_LT_8021_PID}" >> ${LOGFILE}

	echo -n "------- CHMPX(server) 8023             : " >> ${LOGFILE}
	CHMJSONCONF="${TEST_SERVER_JSON}" chmpx -ctlport 8023 ${CHMPX_DBG_PARAM} > ${CHMPXLOG}_8023.log 2>&1 &
	CHMPX_8023_PID=$!
	echo "${CHMPX_8023_PID}" >> ${LOGFILE}
	sleep 2

	echo -n "------- K2HDKC 8023                    : " >> ${LOGFILE}
	K2HDKCJSONCONF="${TEST_SERVER_JSON}" ${K2HDKCBIN} -ctlport 8023 ${DKC_DBG_PARAM} ${DKC_COMLOG_PARAM} > ${K2HDKCLOG}_8023.log 2>&1 &
	K2HDKC_8023_PID=$!
	sleep 1
	K2HDKC_LT_8023_PID=`ps ax | grep 'lt-k2hdkc' | grep 8023 | grep -v grep | awk '{print $1}'`
	if [ "X${K2HDKC_LT_8023_PID}" = "X" ]; then
		K2HDKC_LT_8023_PID=${K2HDKC_8023_PID}
	fi
	echo "${K2HDKC_8023_PID}, ${K2HDKC_LT_8023_PID}" >> ${LOGFILE}

	echo -n "------- CHMPX(server) 8025             : " >> ${LOGFILE}
	CHMJSONCONF="${TEST_SERVER_JSON}" chmpx -ctlport 8025 ${CHMPX_DBG_PARAM} > ${CHMPXLOG}_8025.log 2>&1 &
	CHMPX_8025_PID=$!
	echo "${CHMPX_8025_PID}" >> ${LOGFILE}
	sleep 2

	echo -n "------- K2HDKC 8025                    : " >> ${LOGFILE}
	K2HDKCJSONCONF="${TEST_SERVER_JSON}" ${K2HDKCBIN} -ctlport 8025 ${DKC_DBG_PARAM} ${DKC_COMLOG_PARAM} > ${K2HDKCLOG}_8025.log 2>&1 &
	K2HDKC_8025_PID=$!
	sleep 1
	K2HDKC_LT_8025_PID=`ps ax | grep 'lt-k2hdkc' | grep 8025 | grep -v grep | awk '{print $1}'`
	if [ "X${K2HDKC_LT_8025_PID}" = "X" ]; then
		K2HDKC_LT_8025_PID=${K2HDKC_8025_PID}
	fi
	echo "${K2HDKC_8025_PID}, ${K2HDKC_LT_8025_PID}" >> ${LOGFILE}

	echo -n "------- CHMPX(server) 8027             : " >> ${LOGFILE}
	CHMJSONCONF="${TEST_SERVER_JSON}" chmpx -ctlport 8027 ${CHMPX_DBG_PARAM} > ${CHMPXLOG}_8027.log 2>&1 &
	CHMPX_8027_PID=$!
	echo "${CHMPX_8027_PID}" >> ${LOGFILE}
	sleep 2

	echo -n "------- K2HDKC 8027                    : " >> ${LOGFILE}
	K2HDKCJSONCONF="${TEST_SERVER_JSON}" ${K2HDKCBIN} -ctlport 8027 ${DKC_DBG_PARAM} ${DKC_COMLOG_PARAM} > ${K2HDKCLOG}_8027.log 2>&1 &
	K2HDKC_8027_PID=$!
	sleep 1
	K2HDKC_LT_8027_PID=`ps ax | grep 'lt-k2hdkc' | grep 8027 | grep -v grep | awk '{print $1}'`
	if [ "X${K2HDKC_LT_8027_PID}" = "X" ]; then
		K2HDKC_LT_8027_PID=${K2HDKC_8027_PID}
	fi
	echo "${K2HDKC_8027_PID}, ${K2HDKC_LT_8027_PID}" >> ${LOGFILE}

	########################################
	#
	# RUN k2hdkclinetool process
	#
	TEST_RESULT=0

	echo "" >> ${LOGFILE}
	echo "======= [JSON ENV] RUN CHMPX(slave) / K2HDKCLINETOOL" >> ${LOGFILE}

	echo -n "------- CHMPX(slave) 8031              : " >> ${LOGFILE}
	CHMJSONCONF="${TEST_SLAVE_JSON}" chmpx -ctlport 8031 ${CHMPX_DBG_PARAM} > ${CHMPXLOG}_8031.log 2>&1 &
	CHMPX_8031_PID=$!
	echo "${CHMPX_8031_PID}" >> ${LOGFILE}
	sleep 2

	echo -n "------- [JSON ENV] K2HDKCLINETOOL CAPI/PERM       : " >> ${LOGFILE}
	echo "=========================================================================="	> ${LINETOOLLOG}
	echo "======= CAPI : PERM : REJOIN : NOGIVEUP : NOCLEANUP"							>> ${LINETOOLLOG}
	echo "=========================================================================="	>> ${LINETOOLLOG}
	K2HDKCJSONCONF="${TEST_SLAVE_JSON}" ${LINETOOLBIN} -ctlport 8031 ${DKCTOOL_COMLOG_PARAM} ${DKCTOOL_DBG_PARAM} -perm -rejoin -nogiveup -nocleanup -run ${LINETOOLCMD} -capi >> ${LINETOOLLOG} 2>> ${LINETOOLERRLOG}_C_P.log
	if [ $? -ne 0 ]; then
		echo "FAILED" >> ${LOGFILE}
		echo "FAILED" >> ${LINETOOLLOG}
		TEST_RESULT=1
	else
		echo "SUCCEED" >> ${LOGFILE}
		echo "SUCCEED" >> ${LINETOOLLOG}
	fi
	sleep 1

	echo -n "------- [JSON ENV] K2HDKCLINETOOL CPPAPI/PERM     : " >> ${LOGFILE}
	echo "=========================================================================="	>> ${LINETOOLLOG}
	echo "======= CPPAPI : PERM : REJOIN : NOGIVEUP : NOCLEANUP"						>> ${LINETOOLLOG}
	echo "=========================================================================="	>> ${LINETOOLLOG}
	K2HDKCJSONCONF="${TEST_SLAVE_JSON}" ${LINETOOLBIN} -ctlport 8031 ${DKCTOOL_COMLOG_PARAM} ${DKCTOOL_DBG_PARAM} -perm -rejoin -nogiveup -nocleanup -run ${LINETOOLCMD} >> ${LINETOOLLOG} 2>> ${LINETOOLERRLOG}_CPP_P.log
	if [ $? -ne 0 ]; then
		echo "FAILED" >> ${LOGFILE}
		echo "FAILED" >> ${LINETOOLLOG}
		TEST_RESULT=1
	else
		echo "SUCCEED" >> ${LOGFILE}
		echo "SUCCEED" >> ${LINETOOLLOG}
	fi
	sleep 1

	echo -n "------- [JSON ENV] K2HDKCLINETOOL CAPI/NOT PERM   : " >> ${LOGFILE}
	echo "=========================================================================="	>> ${LINETOOLLOG}
	echo "======= CAPI : NOT PERM : REJOIN : NOGIVEUP : NOCLEANUP"						>> ${LINETOOLLOG}
	echo "=========================================================================="	>> ${LINETOOLLOG}
	K2HDKCJSONCONF="${TEST_SLAVE_JSON}" ${LINETOOLBIN} -ctlport 8031 ${DKCTOOL_COMLOG_PARAM} ${DKCTOOL_DBG_PARAM} -rejoin -nogiveup -nocleanup -run ${LINETOOLCMD} -capi >> ${LINETOOLLOG} 2>> ${LINETOOLERRLOG}_C.log
	if [ $? -ne 0 ]; then
		echo "FAILED" >> ${LOGFILE}
		echo "FAILED" >> ${LINETOOLLOG}
		TEST_RESULT=1
	else
		echo "SUCCEED" >> ${LOGFILE}
		echo "SUCCEED" >> ${LINETOOLLOG}
	fi
	sleep 1

	echo -n "------- [JSON ENV] K2HDKCLINETOOL CPPAPI/NOT PERM : " >> ${LOGFILE}
	echo "=========================================================================="	>> ${LINETOOLLOG}
	echo "======= CPPAPI : NOT PERM : REJOIN : NOGIVEUP : NOCLEANUP"					>> ${LINETOOLLOG}
	echo "=========================================================================="	>> ${LINETOOLLOG}
	K2HDKCJSONCONF="${TEST_SLAVE_JSON}" ${LINETOOLBIN} -ctlport 8031 ${DKCTOOL_COMLOG_PARAM} ${DKCTOOL_DBG_PARAM} -rejoin -nogiveup -nocleanup -run ${LINETOOLCMD} >> ${LINETOOLLOG} 2>> ${LINETOOLERRLOG}_CPP.log
	if [ $? -ne 0 ]; then
		echo "FAILED" >> ${LOGFILE}
		echo "FAILED" >> ${LINETOOLLOG}
		TEST_RESULT=1
	else
		echo "SUCCEED" >> ${LOGFILE}
		echo "SUCCEED" >> ${LINETOOLLOG}
	fi
	sleep 1

	########################################
	#
	# STOP ALL
	#
	echo "" >> ${LOGFILE}
	echo "======= [JSON ENV] STOP ALL PROCESSES CHMPX(server/slave) / K2HDKC" >> ${LOGFILE}
	(ps -p ${CHMPX_8031_PID}		> /dev/null 2>&1) && (kill -HUP ${CHMPX_8031_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8027_PID}		> /dev/null 2>&1) && (kill -HUP ${CHMPX_8027_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8027_PID}		> /dev/null 2>&1) && (kill -HUP ${K2HDKC_8027_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8027_PID}	> /dev/null 2>&1) && (kill -HUP ${K2HDKC_LT_8027_PID}	>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8025_PID}		> /dev/null 2>&1) && (kill -HUP ${CHMPX_8025_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8025_PID}		> /dev/null 2>&1) && (kill -HUP ${K2HDKC_8025_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8025_PID}	> /dev/null 2>&1) && (kill -HUP ${K2HDKC_LT_8025_PID}	>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8023_PID}		> /dev/null 2>&1) && (kill -HUP ${CHMPX_8023_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8023_PID}		> /dev/null 2>&1) && (kill -HUP ${K2HDKC_8023_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8023_PID}	> /dev/null 2>&1) && (kill -HUP ${K2HDKC_LT_8023_PID}	>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8021_PID}		> /dev/null 2>&1) && (kill -HUP ${CHMPX_8021_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8021_PID}		> /dev/null 2>&1) && (kill -HUP ${K2HDKC_8021_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8021_PID}	> /dev/null 2>&1) && (kill -HUP ${K2HDKC_LT_8021_PID}	>> ${LOGFILE} 2>&1)
	sleep 10
	(ps -p ${CHMPX_8031_PID}		> /dev/null 2>&1) && (kill -9 ${CHMPX_8031_PID}			>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8027_PID}		> /dev/null 2>&1) && (kill -9 ${CHMPX_8027_PID}			>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8027_PID}		> /dev/null 2>&1) && (kill -9 ${K2HDKC_8027_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8027_PID}	> /dev/null 2>&1) && (kill -9 ${K2HDKC_LT_8027_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8025_PID}		> /dev/null 2>&1) && (kill -9 ${CHMPX_8025_PID}			>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8025_PID}		> /dev/null 2>&1) && (kill -9 ${K2HDKC_8025_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8025_PID}	> /dev/null 2>&1) && (kill -9 ${K2HDKC_LT_8025_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8023_PID}		> /dev/null 2>&1) && (kill -9 ${CHMPX_8023_PID}			>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8023_PID}		> /dev/null 2>&1) && (kill -9 ${K2HDKC_8023_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8023_PID}	> /dev/null 2>&1) && (kill -9 ${K2HDKC_LT_8023_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${CHMPX_8021_PID}		> /dev/null 2>&1) && (kill -9 ${CHMPX_8021_PID}			>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_8021_PID}		> /dev/null 2>&1) && (kill -9 ${K2HDKC_8021_PID}		>> ${LOGFILE} 2>&1)
	(ps -p ${K2HDKC_LT_8021_PID}	> /dev/null 2>&1) && (kill -9 ${K2HDKC_LT_8021_PID}		>> ${LOGFILE} 2>&1)

	########################################
	#
	# CHECK RESULT
	#
	echo -n "------- [JSON ENV] K2HDKCLINETOOL LOG DIFF RESULT : " >> ${LOGFILE}
	diff ${LINETOOLLOG} ${MASTER_LINETOOLLOG} >/dev/null 2>&1
	if [ $? -ne 0 ]; then
		echo "JSON ENV TEST RESULT --> FAILED" >> ${LOGFILE}
		echo "" >> ${LOGFILE}

		echo "======= JSON ENV TEST                  : FAILED"	>> ${LOGFILE}
		echo ""													>> ${LOGFILE}
		echo "================= `date` ================"		>> ${LOGFILE}

		echo ""
		echo "*********** K2HDKCTOOL LOG DIFF ******************"
		diff ${LINETOOLLOG} ${MASTER_LINETOOLLOG} 2> /dev/null

		echo ""
		echo "*********** DETAIL LOG : LOGFILE *****************"
		cat ${LOGFILE}

		echo ""
		echo "*********** DETAIL LOG : K2HDKCLINETOOL **********"
		cat ${LINETOOLLOG}

		echo ""
		echo "*********** DETAIL LOG : CHMPX(8021) *************"
		cat ${CHMPXLOG}_8021.log

		echo ""
		echo "*********** DETAIL LOG : CHMPX(8023) *************"
		cat ${CHMPXLOG}_8023.log

		echo ""
		echo "*********** DETAIL LOG : CHMPX(8025) *************"
		cat ${CHMPXLOG}_8025.log

		echo ""
		echo "*********** DETAIL LOG : CHMPX(8027) *************"
		cat ${CHMPXLOG}_8027.log

		echo ""
		echo "*********** DETAIL LOG : K2HDKC(8021) *************"
		cat ${K2HDKCLOG}_8021.log

		echo ""
		echo "*********** DETAIL LOG : K2HDKC(8023) *************"
		cat ${K2HDKCLOG}_8023.log

		echo ""
		echo "*********** DETAIL LOG : K2HDKC(8025) *************"
		cat ${K2HDKCLOG}_8025.log

		echo ""
		echo "*********** DETAIL LOG : K2HDKC(8027) *************"
		cat ${K2HDKCLOG}_8027.log

		echo ""
		echo "*********** DETAIL LOG : K2HDKCLINETOOL(C-PERM) ***"
		cat ${LINETOOLERRLOG}_C_P.log

		echo ""
		echo "*********** DETAIL LOG : K2HDKCLINETOOL(CPP-PERM) *"
		cat ${LINETOOLERRLOG}_CPP_P.log

		echo ""
		echo "*********** DETAIL LOG : K2HDKCLINETOOL(C) ********"
		cat ${LINETOOLERRLOG}_C.log

		echo ""
		echo "*********** DETAIL LOG : K2HDKCLINETOOL(CPP) ******"
		cat ${LINETOOLERRLOG}_CPP.log

		TEST_RESULT=1
	else
		echo "JSON ENV TEST RESULT --> SUCCEED" >> ${LOGFILE}
		echo "" >> ${LOGFILE}
	fi

	########################################
	#
	# CLEANUP
	#
	if [ ${TEST_RESULT} -eq 0 ]; then
		rm -f ${LOGFILE}
		rm -f ${LINETOOLLOG}
		rm -f ${CHMPXLOG}_8021.log
		rm -f ${CHMPXLOG}_8023.log
		rm -f ${CHMPXLOG}_8025.log
		rm -f ${CHMPXLOG}_8027.log
		rm -f ${CHMPXLOG}_8031.log
		rm -f ${K2HDKCLOG}_8021.log
		rm -f ${K2HDKCLOG}_8023.log
		rm -f ${K2HDKCLOG}_8025.log
		rm -f ${K2HDKCLOG}_8027.log
		rm -f ${LINETOOLERRLOG}_C_P.log
		rm -f ${LINETOOLERRLOG}_CPP_P.log
		rm -f ${LINETOOLERRLOG}_C.log
		rm -f ${LINETOOLERRLOG}_CPP.log
	fi
fi

###########################################################
## All test finish
###########################################################
if [ ${TEST_RESULT} -ne 0 ]; then
	echo "======= ALL TEST                       : FAILED"	>> ${LOGFILE}
	echo ""													>> ${LOGFILE}
	echo "================= `date` ================"		>> ${LOGFILE}
	cat ${LOGFILE}
else
	echo "======= ALL TEST                       : SUCCESS"	>> ${LOGFILE}
	echo ""													>> ${LOGFILE}
	echo "================= `date` ================"		>> ${LOGFILE}
	cat ${LOGFILE}
fi

exit ${TEST_RESULT}

#
# Local variables:
# tab-width: 4
# c-basic-offset: 4
# End:
# vim600: noexpandtab sw=4 ts=4 fdm=marker
# vim<600: noexpandtab sw=4 ts=4
#
