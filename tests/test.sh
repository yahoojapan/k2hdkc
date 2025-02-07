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

#--------------------------------------------------------------
# Common Variables
#--------------------------------------------------------------
PRGNAME=$(basename "${0}")
SCRIPTDIR=$(dirname "${0}")
SCRIPTDIR=$(cd "${SCRIPTDIR}" || exit 1; pwd)
SRCTOP=$(cd "${SCRIPTDIR}/.." || exit 1; pwd)

#
# Directories / Files
#
SRCDIR="${SRCTOP}/src"
#LIBDIR="${SRCTOP}/lib"
TESTDIR="${SRCTOP}/tests"
LIBOBJDIR="${SRCTOP}/lib/.libs"
#TESTOBJDIR="${TESTDIR}/.libs"

#
# Process ID / Date
#
PROCID=$$
START_DATE=$(date)

#
# Test data/result files
#
CFG_JSON_STRING_FILE="${TESTDIR}/test_json_string.data"
CFG_SVR_FILE_PREFIX="${TESTDIR}/test_server"
CFG_SLV_FILE_PREFIX="${TESTDIR}/test_slave"

LINETOOLCMD="${TESTDIR}/k2hdkclinetool.cmd"
LINETOOLEXITCMD="${TESTDIR}/k2hdkclinetool_exit.cmd"
MASTER_LINETOOLLOG="${TESTDIR}/k2hdkclinetool.log"

LOGFILE="/tmp/test_sh_${PROCID}.log"
LINETOOLLOG="/tmp/test_linetool_${PROCID}.log"
LINETOOLEXITLOG="/tmp/test_linetool_exit_${PROCID}.log"

CHMPX_LOGFILE_PREFIX="/tmp/test_chmpx_${PROCID}"
CHMPX_8021_LOGFILE="${CHMPX_LOGFILE_PREFIX}_8021.log"
CHMPX_8023_LOGFILE="${CHMPX_LOGFILE_PREFIX}_8023.log"
CHMPX_8025_LOGFILE="${CHMPX_LOGFILE_PREFIX}_8025.log"
CHMPX_8027_LOGFILE="${CHMPX_LOGFILE_PREFIX}_8027.log"
CHMPX_8031_LOGFILE="${CHMPX_LOGFILE_PREFIX}_8031.log"

K2HDKC_LOGFILE_PREFIX="/tmp/test_k2hdkc_${PROCID}"
K2HDKC_8021_LOGFILE="${K2HDKC_LOGFILE_PREFIX}_8021.log"
K2HDKC_8023_LOGFILE="${K2HDKC_LOGFILE_PREFIX}_8023.log"
K2HDKC_8025_LOGFILE="${K2HDKC_LOGFILE_PREFIX}_8025.log"
K2HDKC_8027_LOGFILE="${K2HDKC_LOGFILE_PREFIX}_8027.log"

LINETOOL_LOGFILE_PREFIX="/tmp/test_linetool_${PROCID}"
LINETOOL_C_P_LOGFILE="${LINETOOL_LOGFILE_PREFIX}_C_P.log"
LINETOOL_CPP_P_LOGFILE="${LINETOOL_LOGFILE_PREFIX}_CPP_P.log"
LINETOOL_C_LOGFILE="${LINETOOL_LOGFILE_PREFIX}_C.log"
LINETOOL_CPP_LOGFILE="${LINETOOL_LOGFILE_PREFIX}_CPP.log"

SUB_SHELLGROUP_ERROR_FILE="/tmp/test_sh_status_error_${PROCID}"

#
# Other
#
WAIT_SEC_AFTER_RUN_CHMPX=2
WAIT_SEC_AFTER_RUN_K2HDKC=1
WAIT_SEC_AFTER_RUN_K2HDKCTOOL=1

#
# LD_LIBRARY_PATH
#
LD_LIBRARY_PATH="${LIBOBJDIR}"
export LD_LIBRARY_PATH

#--------------------------------------------------------------
# Usage
#--------------------------------------------------------------
func_usage()
{
	echo ""
	echo "Usage: $1 [-h] [ ini_conf & yaml_conf & json_conf & json_string & json_env ] [-chmpx_dbg_level <err|wan|msg|dump>] [-dkc_dbg_level <err|wan|msg|dump>] [-logfile <filepath>]"
	echo ""
}

#--------------------------------------------------------------
# Variables and Utility functions
#--------------------------------------------------------------
#
# Escape sequence
#
if [ -t 1 ]; then
	CBLD=$(printf '\033[1m')
	CREV=$(printf '\033[7m')
	CRED=$(printf '\033[31m')
	CYEL=$(printf '\033[33m')
	CGRN=$(printf '\033[32m')
	CDEF=$(printf '\033[0m')
else
	CBLD=""
	CREV=""
	CRED=""
	CYEL=""
	CGRN=""
	CDEF=""
fi

#--------------------------------------------------------------
# Message functions
#--------------------------------------------------------------
PRNTITLE()
{
	echo ""
	echo "${CGRN}---------------------------------------------------------------------${CDEF}"
	echo "${CGRN}${CREV}[TITLE]${CDEF} ${CGRN}$*${CDEF}"
	echo "${CGRN}---------------------------------------------------------------------${CDEF}"
}

PRNERR()
{
	echo "${CBLD}${CRED}[ERROR]${CDEF} ${CRED}$*${CDEF}"
}

PRNWARN()
{
	echo "${CYEL}${CREV}[WARNING]${CDEF} $*"
}

PRNMSG()
{
	echo "${CYEL}${CREV}[MSG]${CDEF} ${CYEL}$*${CDEF}"
}

PRNINFO()
{
	echo "${CREV}[INFO]${CDEF} $*"
}

PRNSUCCEED()
{
	echo "${CREV}[SUCCEED]${CDEF} $*"
}

#--------------------------------------------------------------
# Utilitiy functions
#--------------------------------------------------------------
#
# $1:	type(ini, yaml, json, jsonstring, jsonenv)
#
run_all_processes()
{
	_JSON_ENV_MODE=0
	if [ $# -ne 1 ] || [ -z "$1" ]; then
		PRNERR "run_all_processes function parameter is wrong."
		return 1

	elif [ "$1" = "ini" ] || [ "$1" = "INI" ]; then
		CONF_OPT_STRING="-conf"
		CONF_FILE_EXT=".ini"
		CONF_OPT_SVR_PARAM="${CFG_SVR_FILE_PREFIX}${CONF_FILE_EXT}"
		CONF_OPT_SLV_PARAM="${CFG_SLV_FILE_PREFIX}${CONF_FILE_EXT}"

	elif [ "$1" = "yaml" ] || [ "$1" = "YAML" ]; then
		CONF_OPT_STRING="-conf"
		CONF_FILE_EXT=".yaml"
		CONF_OPT_SVR_PARAM="${CFG_SVR_FILE_PREFIX}${CONF_FILE_EXT}"
		CONF_OPT_SLV_PARAM="${CFG_SLV_FILE_PREFIX}${CONF_FILE_EXT}"

	elif [ "$1" = "json" ] || [ "$1" = "JSON" ]; then
		CONF_OPT_STRING="-conf"
		CONF_FILE_EXT=".json"
		CONF_OPT_SVR_PARAM="${CFG_SVR_FILE_PREFIX}${CONF_FILE_EXT}"
		CONF_OPT_SLV_PARAM="${CFG_SLV_FILE_PREFIX}${CONF_FILE_EXT}"

	elif [ "$1" = "jsonstring" ] || [ "$1" = "JSONSTRING" ] || [ "$1" = "sjson" ] || [ "$1" = "SJSON" ] || [ "$1" = "jsonstr" ] || [ "$1" = "JSONSTR" ]; then
		CONF_OPT_STRING="-json"
		CONF_OPT_SVR_PARAM="${TEST_SERVER_JSON}"
		CONF_OPT_SLV_PARAM="${TEST_SLAVE_JSON}"

	elif [ "$1" = "jsonenv" ] || [ "$1" = "JSONENV" ] || [ "$1" = "ejson" ] || [ "$1" = "EJSON" ]; then
		_JSON_ENV_MODE=1
	else
		PRNERR "run_all_processes function parameter is wrong."
		return 1
	fi

	#------------------------------------------------------
	# RUN chmpx processes and k2hdkc server processes
	#------------------------------------------------------
	PRNMSG "RUN CHMPX(server/slave) / K2HDKC"

	#
	# Run chmpx server 8021
	#
	if [ "${_JSON_ENV_MODE}" -ne 1 ]; then
		"${CHMPXBIN}" "${CONF_OPT_STRING}" "${CONF_OPT_SVR_PARAM}" -ctlport 8021 "${CHMPX_DBG_OPT}" "${CHMPX_DBG_PARAM}" > "${CHMPX_8021_LOGFILE}" 2>&1 &
	else
		CHMJSONCONF="${TEST_SERVER_JSON}" "${CHMPXBIN}" -ctlport 8021 "${CHMPX_DBG_OPT}" "${CHMPX_DBG_PARAM}" > "${CHMPX_8021_LOGFILE}" 2>&1 &
	fi
	CHMPX_8021_PID=$!
	sleep "${WAIT_SEC_AFTER_RUN_CHMPX}"
	PRNINFO "CHMPX(server) 8021             : ${CHMPX_8021_PID}"

	#
	# Run k2hdkc 8021
	#
	if [ "${_JSON_ENV_MODE}" -ne 1 ]; then
		"${K2HDKCBIN}" "${CONF_OPT_STRING}" "${CONF_OPT_SVR_PARAM}" -ctlport 8021 "${DKC_DBG_OPT}" "${DKC_DBG_PARAM}" > "${K2HDKC_8021_LOGFILE}" 2>&1 &
	else
		K2HDKCJSONCONF="${TEST_SERVER_JSON}" "${K2HDKCBIN}" -ctlport 8021 "${DKC_DBG_OPT}" "${DKC_DBG_PARAM}" > "${K2HDKC_8021_LOGFILE}" 2>&1 &
	fi
	K2HDKC_8021_PID=$!
	K2HDKC_LT_8021_PID=$(get_lt_k2hdkc_process_id 8021 "${K2HDKC_8021_PID}")
	sleep "${WAIT_SEC_AFTER_RUN_K2HDKC}"
	PRNINFO "K2HDKC 8021                    : ${K2HDKC_8021_PID}, ${K2HDKC_LT_8021_PID}"

	#
	# Run chmpx server 8023
	#
	if [ "${_JSON_ENV_MODE}" -ne 1 ]; then
		"${CHMPXBIN}" "${CONF_OPT_STRING}" "${CONF_OPT_SVR_PARAM}" -ctlport 8023 "${CHMPX_DBG_OPT}" "${CHMPX_DBG_PARAM}" > "${CHMPX_8023_LOGFILE}" 2>&1 &
	else
		CHMJSONCONF="${TEST_SERVER_JSON}" "${CHMPXBIN}" -ctlport 8023 "${CHMPX_DBG_OPT}" "${CHMPX_DBG_PARAM}" > "${CHMPX_8023_LOGFILE}" 2>&1 &
	fi
	CHMPX_8023_PID=$!
	sleep "${WAIT_SEC_AFTER_RUN_CHMPX}"
	PRNINFO "CHMPX(server) 8023             : ${CHMPX_8023_PID}"

	#
	# Run k2hdkc 8023
	#
	if [ "${_JSON_ENV_MODE}" -ne 1 ]; then
		"${K2HDKCBIN}" "${CONF_OPT_STRING}" "${CONF_OPT_SVR_PARAM}" -ctlport 8023 "${DKC_DBG_OPT}" "${DKC_DBG_PARAM}" > "${K2HDKC_8023_LOGFILE}" 2>&1 &
	else
		K2HDKCJSONCONF="${TEST_SERVER_JSON}" "${K2HDKCBIN}" -ctlport 8023 "${DKC_DBG_OPT}" "${DKC_DBG_PARAM}" > "${K2HDKC_8023_LOGFILE}" 2>&1 &
	fi
	K2HDKC_8023_PID=$!
	K2HDKC_LT_8023_PID=$(get_lt_k2hdkc_process_id 8023 "${K2HDKC_8023_PID}")
	sleep "${WAIT_SEC_AFTER_RUN_K2HDKC}"
	PRNINFO "K2HDKC 8023                    : ${K2HDKC_8023_PID}, ${K2HDKC_LT_8023_PID}"

	#
	# Run chmpx server 8025
	#
	if [ "${_JSON_ENV_MODE}" -ne 1 ]; then
		"${CHMPXBIN}" "${CONF_OPT_STRING}" "${CONF_OPT_SVR_PARAM}" -ctlport 8025 "${CHMPX_DBG_OPT}" "${CHMPX_DBG_PARAM}" > "${CHMPX_8025_LOGFILE}" 2>&1 &
	else
		CHMJSONCONF="${TEST_SERVER_JSON}" "${CHMPXBIN}" -ctlport 8025 "${CHMPX_DBG_OPT}" "${CHMPX_DBG_PARAM}" > "${CHMPX_8025_LOGFILE}" 2>&1 &
	fi
	CHMPX_8025_PID=$!
	sleep "${WAIT_SEC_AFTER_RUN_CHMPX}"
	PRNINFO "CHMPX(server) 8025             : ${CHMPX_8025_PID}"

	#
	# Run k2hdkc 8025
	#
	if [ "${_JSON_ENV_MODE}" -ne 1 ]; then
		"${K2HDKCBIN}" "${CONF_OPT_STRING}" "${CONF_OPT_SVR_PARAM}" -ctlport 8025 "${DKC_DBG_OPT}" "${DKC_DBG_PARAM}" > "${K2HDKC_8025_LOGFILE}" 2>&1 &
	else
		K2HDKCJSONCONF="${TEST_SERVER_JSON}" "${K2HDKCBIN}" -ctlport 8025 "${DKC_DBG_OPT}" "${DKC_DBG_PARAM}" > "${K2HDKC_8025_LOGFILE}" 2>&1 &
	fi
	K2HDKC_8025_PID=$!
	K2HDKC_LT_8025_PID=$(get_lt_k2hdkc_process_id 8025 "${K2HDKC_8025_PID}")
	sleep "${WAIT_SEC_AFTER_RUN_K2HDKC}"
	PRNINFO "K2HDKC 8025                    : ${K2HDKC_8025_PID}, ${K2HDKC_LT_8025_PID}"

	#
	# Run chmpx server 8027
	#
	if [ "${_JSON_ENV_MODE}" -ne 1 ]; then
		"${CHMPXBIN}" "${CONF_OPT_STRING}" "${CONF_OPT_SVR_PARAM}" -ctlport 8027 "${CHMPX_DBG_OPT}" "${CHMPX_DBG_PARAM}" > "${CHMPX_8027_LOGFILE}" 2>&1 &
	else
		CHMJSONCONF="${TEST_SERVER_JSON}" "${CHMPXBIN}" -ctlport 8027 "${CHMPX_DBG_OPT}" "${CHMPX_DBG_PARAM}" > "${CHMPX_8027_LOGFILE}" 2>&1 &
	fi
	CHMPX_8027_PID=$!
	sleep "${WAIT_SEC_AFTER_RUN_CHMPX}"
	PRNINFO "CHMPX(server) 8027             : ${CHMPX_8027_PID}"

	#
	# Run k2hdkc 8027
	#
	if [ "${_JSON_ENV_MODE}" -ne 1 ]; then
		"${K2HDKCBIN}" "${CONF_OPT_STRING}" "${CONF_OPT_SVR_PARAM}" -ctlport 8027 "${DKC_DBG_OPT}" "${DKC_DBG_PARAM}" > "${K2HDKC_8027_LOGFILE}" 2>&1 &
	else
		K2HDKCJSONCONF="${TEST_SERVER_JSON}" "${K2HDKCBIN}" -ctlport 8027 "${DKC_DBG_OPT}" "${DKC_DBG_PARAM}" > "${K2HDKC_8027_LOGFILE}" 2>&1 &
	fi
	K2HDKC_8027_PID=$!
	K2HDKC_LT_8027_PID=$(get_lt_k2hdkc_process_id 8027 "${K2HDKC_8027_PID}")
	sleep "${WAIT_SEC_AFTER_RUN_K2HDKC}"
	PRNINFO "K2HDKC 8027                    : ${K2HDKC_8027_PID}, ${K2HDKC_LT_8027_PID}"

	#
	# Run chmpx slave 8031
	#
	if [ "${_JSON_ENV_MODE}" -ne 1 ]; then
		"${CHMPXBIN}" "${CONF_OPT_STRING}" "${CONF_OPT_SLV_PARAM}" -ctlport 8031 "${CHMPX_DBG_OPT}" "${CHMPX_DBG_PARAM}" > "${CHMPX_8031_LOGFILE}" 2>&1 &
	else
		CHMJSONCONF="${TEST_SLAVE_JSON}" "${CHMPXBIN}" -ctlport 8031 "${CHMPX_DBG_OPT}" "${CHMPX_DBG_PARAM}" > "${CHMPX_8031_LOGFILE}" 2>&1 &
	fi
	CHMPX_8031_PID=$!
	sleep "${WAIT_SEC_AFTER_RUN_CHMPX}"
	PRNINFO "CHMPX(slave) 8031              : ${CHMPX_8031_PID}"

	return 0
}

stop_process()
{
	if [ $# -eq 0 ]; then
		return 1
	fi
	_STOP_PIDS="$*"

	_STOP_RESULT=0
	for _ONE_PID in ${_STOP_PIDS}; do
		_MAX_TRYCOUNT=10

		while [ "${_MAX_TRYCOUNT}" -gt 0 ]; do
			#
			# Check running status
			#
			# shellcheck disable=SC2009
			if ! ( ps -o pid,stat ax 2>/dev/null | grep -v 'PID' | awk '$2~/^[^Z]/ { print $1 }' | grep -q "^${_ONE_PID}$" || exit 1 && exit 0 ); then
				break
			fi

			#
			# Try HUP
			#
			kill -HUP "${_ONE_PID}" > /dev/null 2>&1
			sleep 1

			# shellcheck disable=SC2009
			if ! ( ps -o pid,stat ax 2>/dev/null | grep -v 'PID' | awk '$2~/^[^Z]/ { print $1 }' | grep -q "^${_ONE_PID}$" || exit 1 && exit 0 ); then
				break
			fi

			#
			# Try KILL
			#
			kill -KILL "${_ONE_PID}" > /dev/null 2>&1
			sleep 1

			# shellcheck disable=SC2009
			if ! ( ps -o pid,stat ax 2>/dev/null | grep -v 'PID' | awk '$2~/^[^Z]/ { print $1 }' | grep -q "^${_ONE_PID}$" || exit 1 && exit 0 ); then
				break
			fi
			_MAX_TRYCOUNT=$((_MAX_TRYCOUNT - 1))
		done

		if [ "${_MAX_TRYCOUNT}" -le 0 ]; then
			# shellcheck disable=SC2009
			if ( ps -o pid,stat ax 2>/dev/null | grep -v 'PID' | awk '$2~/^[^Z]/ { print $1 }' | grep -q "^${_ONE_PID}$" || exit 1 && exit 0 ); then
				PRNERR "Could not stop ${_ONE_PID} process"
				_STOP_RESULT=1
			else
				PRNWARN "Could not stop ${_ONE_PID} process, because it has maybe defunct status. So assume we were able to stop it."
			fi
		fi
	done
	return "${_STOP_RESULT}"
}

#
# Cleanup files
#
cleanup_files()
{
	rm -f "${LINETOOLLOG}"
	rm -f "${LINETOOLEXITLOG}"
	rm -f "${CHMPX_8021_LOGFILE}"
	rm -f "${CHMPX_8023_LOGFILE}"
	rm -f "${CHMPX_8025_LOGFILE}"
	rm -f "${CHMPX_8027_LOGFILE}"
	rm -f "${CHMPX_8031_LOGFILE}"
	rm -f "${K2HDKC_8021_LOGFILE}"
	rm -f "${K2HDKC_8023_LOGFILE}"
	rm -f "${K2HDKC_8025_LOGFILE}"
	rm -f "${K2HDKC_8027_LOGFILE}"
	rm -f "${LINETOOL_C_P_LOGFILE}"
	rm -f "${LINETOOL_CPP_P_LOGFILE}"
	rm -f "${LINETOOL_C_LOGFILE}"
	rm -f "${LINETOOL_CPP_LOGFILE}"
	rm -f "${SUB_SHELLGROUP_ERROR_FILE}"

	#
	# Cleanup temporary files in /var/lib/antpickax
	#
	rm -rf /var/lib/antpickax/.fullock /var/lib/antpickax/.k2h* /var/lib/antpickax/*

	return 0
}

print_log_detail()
{
	if [ $# -gt 0 ] && [ -n "$1" ] && [ "$1" = "diff" ]; then
		_PRINT_DIFF=1
	elif [ $# -gt 0 ] && [ -n "$1" ] && [ "$1" = "nodiff" ]; then
		_PRINT_DIFF=0
	else
		_PRINT_DIFF=0
	fi

	if [ "${_PRINT_DIFF}" -eq 1 ]; then
		if [ -f "${LINETOOLLOG}" ]; then
			echo "    [DETAIL] : K2HDKCTOOL LOG DIFF"
			diff "${LINETOOLLOG}" "${MASTER_LINETOOLLOG}" 2>/dev/null | sed -e 's/^/      /g'
		fi
	fi

	if [ -f "${LINETOOLLOG}" ]; then
		echo "    [DETAIL] : LINETOOLLOG"
		sed -e 's/^/      /g' "${LINETOOLLOG}"
	fi
	if [ -f "${CHMPX_8021_LOGFILE}" ]; then
		echo "    [DETAIL] : CHMPX SERVER(8021)"
		sed -e 's/^/      /g' "${CHMPX_8021_LOGFILE}"
	fi
	if [ -f "${CHMPX_8023_LOGFILE}" ]; then
		echo "    [DETAIL] : CHMPX SERVER(8023)"
		sed -e 's/^/      /g' "${CHMPX_8023_LOGFILE}"
	fi
	if [ -f "${CHMPX_8025_LOGFILE}" ]; then
		echo "    [DETAIL] : CHMPX SERVER(8025)"
		sed -e 's/^/      /g' "${CHMPX_8025_LOGFILE}"
	fi
	if [ -f "${CHMPX_8027_LOGFILE}" ]; then
		echo "    [DETAIL] : CHMPX SERVER(8027)"
		sed -e 's/^/      /g' "${CHMPX_8027_LOGFILE}"
	fi
	if [ -f "${CHMPX_8031_LOGFILE}" ]; then
		echo "    [DETAIL] : CHMPX SLAVE(8031)"
		sed -e 's/^/      /g' "${CHMPX_8031_LOGFILE}"
	fi
	if [ -f "${K2HDKC_8021_LOGFILE}" ]; then
		echo "    [DETAIL] : K2HDKC(8021)"
		sed -e 's/^/      /g' "${K2HDKC_8021_LOGFILE}"
	fi
	if [ -f "${K2HDKC_8023_LOGFILE}" ]; then
		echo "    [DETAIL] : K2HDKC(8023)"
		sed -e 's/^/      /g' "${K2HDKC_8023_LOGFILE}"
	fi
	if [ -f "${K2HDKC_8025_LOGFILE}" ]; then
		echo "    [DETAIL] : K2HDKC(8025)"
		sed -e 's/^/      /g' "${K2HDKC_8025_LOGFILE}"
	fi
	if [ -f "${K2HDKC_8027_LOGFILE}" ]; then
		echo "    [DETAIL] : K2HDKC(8027)"
		sed -e 's/^/      /g' "${K2HDKC_8027_LOGFILE}"
	fi
	if [ -f "${LINETOOL_C_P_LOGFILE}" ]; then
		echo "    [DETAIL] : K2HDKCLINETOOL(C-PERM)"
		sed -e 's/^/      /g' "${LINETOOL_C_P_LOGFILE}"
	fi
	if [ -f "${LINETOOL_CPP_P_LOGFILE}" ]; then
		echo "    [DETAIL] : K2HDKCLINETOOL(CPP-PERM)"
		sed -e 's/^/      /g' "${LINETOOL_CPP_P_LOGFILE}"
	fi
	if [ -f "${LINETOOL_C_LOGFILE}" ]; then
		echo "    [DETAIL] : K2HDKCLINETOOL(C)"
		sed -e 's/^/      /g' "${LINETOOL_C_LOGFILE}"
	fi
	if [ -f "${LINETOOL_CPP_LOGFILE}" ]; then
		echo "    [DETAIL] : K2HDKCLINETOOL(CPP)"
		sed -e 's/^/      /g' "${LINETOOL_CPP_LOGFILE}"
	fi

	return 0
}

# [NOTE]
# Libtools PIDs may be present because they use built binaries instead
# of package-installed binaries.
# This function returns the PID of the Libtools wrapper for the k2hdkc
# process. If it does not exist, the PID value passed in the second
# argument is returned as is.
#
get_lt_k2hdkc_process_id()
{
	if [ $# -ne 2 ]; then
		echo ""
		return 1
	fi
	# shellcheck disable=SC2009
	if ! _LT_PID="$(ps ax | grep -v grep | grep 'lt-k2hdkc' | grep "$1" | awk '{print $1}')"; then
		echo "$2"
	elif [ -z "${_LT_PID}" ]; then
		echo "$2"
	else
		echo "${_LT_PID}"
	fi
	return 0
}

#==============================================================
# Set and Check variables for test
#==============================================================
#
# Check binary path
#
if ! command -v chmpx >/dev/null 2>&1; then
	PRNERR "Not found chmpx binary"
	exit 1
fi
CHMPXBIN=$(command -v chmpx | tr -d '\n')

if [ -f "${SRCDIR}/k2hdkc" ]; then
	K2HDKCBIN="${SRCDIR}/k2hdkc"
elif [ -f "${SRCDIR}/k2hdkcmain" ]; then
	K2HDKCBIN="${SRCDIR}/k2hdkcmain"
else
	PRNERR "Not found k2hdkc binary"
	exit 1
fi
if [ -f "${TESTDIR}/k2hdkclinetool" ]; then
	LINETOOLBIN="${TESTDIR}/k2hdkclinetool"
else
	PRNERR "Not found k2hdkclinetool binary"
	exit 1
fi

#--------------------------------------------------------------
# Parse options
#--------------------------------------------------------------
#
# Variables
#
IS_FOUND_TEST_TYPE_OPT=0
DO_INI_CONF=0
DO_YAML_CONF=0
DO_JSON_CONF=0
DO_JSON_STRING=0
DO_JSON_ENV=0

OPT_CHMPX_DBG_PARAM=""
OPT_DKC_DBG_PARAM=""
OPT_LOGFILE=""

while [ $# -ne 0 ]; do
	if [ -z "$1" ]; then
		break

	elif [ "$1" = "-h" ] || [ "$1" = "-H" ] || [ "$1" = "--help" ] || [ "$1" = "--HELP" ]; then
		func_usage "${PRGNAME}"
		exit 0

	elif [ "$1" = "ini_conf" ] || [ "$1" = "INI_CONF" ]; then
		if [ "${DO_INI_CONF}" -eq 1 ]; then
			PRNERR "Already specified \"ini_conf\" mode."
			exit 1
		fi
		DO_INI_CONF=1
		IS_FOUND_TEST_TYPE_OPT=1

	elif [ "$1" = "yaml_conf" ] || [ "$1" = "YAML_CONF" ]; then
		if [ "${DO_YAML_CONF}" -eq 1 ]; then
			PRNERR "Already specified \"yaml_conf\" mode."
			exit 1
		fi
		DO_YAML_CONF=1
		IS_FOUND_TEST_TYPE_OPT=1

	elif [ "$1" = "json_conf" ] || [ "$1" = "JSON_CONF" ]; then
		if [ "${DO_JSON_CONF}" -eq 1 ]; then
			PRNERR "Already specified \"json_conf\" mode."
			exit 1
		fi
		DO_JSON_CONF=1
		IS_FOUND_TEST_TYPE_OPT=1

	elif [ "$1" = "json_string" ] || [ "$1" = "JSON_STRING" ]; then
		if [ "${DO_JSON_STRING}" -eq 1 ]; then
			PRNERR "Already specified \"json_string\" mode."
			exit 1
		fi
		DO_JSON_STRING=1
		IS_FOUND_TEST_TYPE_OPT=1

	elif [ "$1" = "json_env" ] || [ "$1" = "JSON_ENV" ]; then
		if [ "${DO_JSON_ENV}" -eq 1 ]; then
			PRNERR "Already specified \"json_env\" mode."
			exit 1
		fi
		DO_JSON_ENV=1
		IS_FOUND_TEST_TYPE_OPT=1

	elif [ "$1" = "-chmpx_dbg_level" ] || [ "$1" = "-CHMPX_DBG_LEVEL" ]; then
		if [ -n "${OPT_CHMPX_DBG_PARAM}" ]; then
			PRNERR "Already specified -chmpx_dbg_level option."
			exit 1
		fi
		shift
		if [ -z "$1" ]; then
			PRNERR "Option -chmpx_dbg_level needs parameter."
			exit 1
		fi
		OPT_CHMPX_DBG_PARAM="$1"

	elif [ "$1" = "-dkc_dbg_level" ] || [ "$1" = "-DKC_DBG_LEVEL" ]; then
		if [ -n "${OPT_DKC_DBG_PARAM}" ]; then
			PRNERR "Already specified -dkc_dbg_level option."
			exit 1
		fi
		shift
		if [ -z "$1" ]; then
			PRNERR "Option -dkc_dbg_level needs parameter."
			exit 1
		fi
		OPT_DKC_DBG_PARAM="$1"

	elif [ "$1" = "-logfile" ] || [ "$1" = "-LOGFILE" ]; then
		if [ -n "${OPT_LOGFILE}" ]; then
			PRNERR "Already specified -logfile option."
			exit 1
		fi
		shift
		if [ -z "$1" ]; then
			PRNERR "Option -logfile needs parameter."
			exit 1
		fi
		OPT_LOGFILE="$1"

	else
		PRNERR "Unknown option $1"
		exit 1
	fi

	shift
done

#
# Check and Set values
#
if [ "${IS_FOUND_TEST_TYPE_OPT}" -eq 0 ]; then
	DO_INI_CONF=1
	DO_YAML_CONF=1
	DO_JSON_CONF=1
	DO_JSON_STRING=1
	DO_JSON_ENV=1
fi

CHMPX_DBG_OPT="-d"
if [ -n "${OPT_CHMPX_DBG_PARAM}" ]; then
	CHMPX_DBG_PARAM="${OPT_CHMPX_DBG_PARAM}"
else
	CHMPX_DBG_PARAM="err"
fi

DKC_DBG_OPT="-d"
if [ -n "${OPT_DKC_DBG_PARAM}" ]; then
	DKC_DBG_PARAM="${OPT_DKC_DBG_PARAM}"
else
	DKC_DBG_PARAM="err"
fi

#
# Always use this parameter for k2hdkclinetool.
#
DKCTOOL_DBG_OPT="-d"
DKCTOOL_DBG_PARAM="err"
DKCTOOL_COMLOG_OPT="-comlog"
DKCTOOL_COMLOG_PARAM="off"

#--------------------------------------------------------------
# Setup data
#--------------------------------------------------------------
#
# JSON String data
#
TEST_SERVER_JSON=$(grep 'TEST_SERVER_JSON_STR=' "${CFG_JSON_STRING_FILE}" 2>/dev/null | sed -e 's/TEST_SERVER_JSON_STR=//g')
TEST_SLAVE_JSON=$(grep 'TEST_SLAVE_JSON_STR=' "${CFG_JSON_STRING_FILE}" 2>/dev/null | sed -e 's/TEST_SLAVE_JSON_STR=//g')
#TEST_TRANS_SERVER_JSON=$(grep 'TEST_TRANS_SERVER_JSON_STR=' "${CFG_JSON_STRING_FILE}" 2>/dev/null | sed -e 's/TEST_TRANS_SERVER_JSON_STR=//g')
#TEST_TRANS_SLAVE_JSON=$(grep 'TEST_TRANS_SLAVE_JSON_STR=' "${CFG_JSON_STRING_FILE}" 2>/dev/null | sed -e 's/TEST_TRANS_SLAVE_JSON_STR=//g')

#==============================================================
# Initialize before test
#==============================================================
PRNTITLE "Initialize before test"

#
# Cleanup logs
#
cleanup_files
rm -f "${LOGFILE}"

#
# Initialize Result
#
TEST_RESULT=0

PRNSUCCEED "Initialize before test"

#==============================================================
# Start INI conf file(execution)
#==============================================================
if [ "${TEST_RESULT}" -eq 0 ] && [ "${DO_INI_CONF}" -eq 1 ]; then
	#
	# Run shell process group for logging
	#
	{
		PRNTITLE "TEST FOR INI CONF FILE (EXECUTION)"

		#------------------------------------------------------
		# RUN all test processes
		#------------------------------------------------------
		# [NOTE]
		# The process runs in the background, so it doesn't do any error checking.
		# If it fails to start, an error will occur in subsequent tests.
		#
		run_all_processes "ini"

		#------------------------------------------------------
		# RUN k2hdkclinetool process
		#------------------------------------------------------
		PRNMSG "RUN K2HDKCLINETOOL"

		if ! "${LINETOOLBIN}" -conf "${CFG_SLV_FILE_PREFIX}.ini" -ctlport 8031 "${DKCTOOL_COMLOG_OPT}" "${DKCTOOL_COMLOG_PARAM}" "${DKCTOOL_DBG_OPT}" "${DKCTOOL_DBG_PARAM}" -rejoin -nogiveup -nocleanup -run "${LINETOOLEXITCMD}" > "${LINETOOLEXITLOG}" 2>&1; then
			PRNERR "FAILED : RUN K2HDKCLINETOOL"
			TEST_RESULT=1
		else
			PRNINFO "SUCCEED : RUN K2HDKCLINETOOL"
		fi

		#------------------------------------------------------
		# Stop all processes
		#------------------------------------------------------
		PRNMSG "STOP ALL PROCESSES CHMPX(server/slave) / K2HDKC"

		# [NOTE]
		# The order of the list is the stop order and is important.
		#
		ALL_PIDS="	${CHMPX_8031_PID}
					${K2HDKC_8027_PID}
					${K2HDKC_LT_8027_PID}
					${CHMPX_8027_PID}
					${K2HDKC_8025_PID}
					${K2HDKC_LT_8025_PID}
					${CHMPX_8025_PID}
					${K2HDKC_8023_PID}
					${K2HDKC_LT_8023_PID}
					${CHMPX_8023_PID}
					${K2HDKC_8021_PID}
					${K2HDKC_LT_8021_PID}
					${CHMPX_8021_PID}"

		if ! stop_process "${ALL_PIDS}"; then
			PRNERR "FAILED : STOP ALL PROCESSES CHMPX(server/slave) / K2HDKC"
			TEST_RESULT=1
		else
			PRNINFO "SUCCEED : STOP ALL PROCESSES CHMPX(server/slave) / K2HDKC"
		fi

		#------------------------------------------------------
		# Print result
		#------------------------------------------------------
		if [ "${TEST_RESULT}" -ne 0 ]; then
			PRNERR "FAILED : TEST FOR INI CONF FILE (EXECUTION)"
			PRNINFO "(The test has failed. Print each logs)"

			print_log_detail "nodiff"
		else
			PRNSUCCEED "TEST FOR INI CONF FILE (EXECUTION)"
		fi

		if [ "${TEST_RESULT}" -ne 0 ]; then
			touch "${SUB_SHELLGROUP_ERROR_FILE}"
		fi
	} | tee -a "${LOGFILE}"

	#
	# Set error flag
	#
	if [ -f "${SUB_SHELLGROUP_ERROR_FILE}" ]; then
		TEST_RESULT=1
	fi

	#----------------------------------------------------------
	# Cleanup
	#----------------------------------------------------------
	cleanup_files
fi

#==============================================================
# Start YAML conf file(execution)
#==============================================================
if [ "${TEST_RESULT}" -eq 0 ] && [ "${DO_YAML_CONF}" -eq 1 ]; then
	#
	# Run shell process group for logging
	#
	{
		PRNTITLE "TEST FOR YAML CONF FILE (EXECUTION)"

		#------------------------------------------------------
		# RUN all test processes
		#------------------------------------------------------
		# [NOTE]
		# The process runs in the background, so it doesn't do any error checking.
		# If it fails to start, an error will occur in subsequent tests.
		#
		run_all_processes "yaml"

		#------------------------------------------------------
		# RUN k2hdkclinetool process
		#------------------------------------------------------
		PRNMSG "RUN K2HDKCLINETOOL"

		if ! "${LINETOOLBIN}" -conf "${CFG_SLV_FILE_PREFIX}.yaml" -ctlport 8031 "${DKCTOOL_COMLOG_OPT}" "${DKCTOOL_COMLOG_PARAM}" "${DKCTOOL_DBG_OPT}" "${DKCTOOL_DBG_PARAM}" -rejoin -nogiveup -nocleanup -run "${LINETOOLEXITCMD}" > "${LINETOOLEXITLOG}" 2>&1; then
			PRNERR "FAILED : RUN K2HDKCLINETOOL"
			TEST_RESULT=1
		else
			PRNINFO "SUCCEED : RUN K2HDKCLINETOOL"
		fi

		#------------------------------------------------------
		# Stop all processes
		#------------------------------------------------------
		PRNMSG "STOP ALL PROCESSES CHMPX(server/slave) / K2HDKC"

		# [NOTE]
		# The order of the list is the stop order and is important.
		#
		ALL_PIDS="	${CHMPX_8031_PID}
					${K2HDKC_8027_PID}
					${K2HDKC_LT_8027_PID}
					${CHMPX_8027_PID}
					${K2HDKC_8025_PID}
					${K2HDKC_LT_8025_PID}
					${CHMPX_8025_PID}
					${K2HDKC_8023_PID}
					${K2HDKC_LT_8023_PID}
					${CHMPX_8023_PID}
					${K2HDKC_8021_PID}
					${K2HDKC_LT_8021_PID}
					${CHMPX_8021_PID}"

		if ! stop_process "${ALL_PIDS}"; then
			PRNERR "FAILED : STOP ALL PROCESSES CHMPX(server/slave) / K2HDKC"
			TEST_RESULT=1
		else
			PRNINFO "SUCCEED : STOP ALL PROCESSES CHMPX(server/slave) / K2HDKC"
		fi

		#------------------------------------------------------
		# Print result
		#------------------------------------------------------
		if [ "${TEST_RESULT}" -ne 0 ]; then
			PRNERR "FAILED : TEST FOR YAML CONF FILE (EXECUTION)"
			PRNINFO "(The test has failed. Print each logs)"

			print_log_detail "nodiff"
		else
			PRNSUCCEED "TEST FOR YAML CONF FILE (EXECUTION)"
		fi

		if [ "${TEST_RESULT}" -ne 0 ]; then
			touch "${SUB_SHELLGROUP_ERROR_FILE}"
		fi
	} | tee -a "${LOGFILE}"

	#
	# Set error flag
	#
	if [ -f "${SUB_SHELLGROUP_ERROR_FILE}" ]; then
		TEST_RESULT=1
	fi

	#----------------------------------------------------------
	# Cleanup
	#----------------------------------------------------------
	cleanup_files
fi

#==============================================================
# Start JSON conf file(execution)
#==============================================================
if [ "${TEST_RESULT}" -eq 0 ] && [ "${DO_JSON_CONF}" -eq 1 ]; then
	#
	# Run shell process group for logging
	#
	{
		PRNTITLE "TEST FOR JSON CONF FILE (EXECUTION)"

		#------------------------------------------------------
		# RUN all test processes
		#------------------------------------------------------
		# [NOTE]
		# The process runs in the background, so it doesn't do any error checking.
		# If it fails to start, an error will occur in subsequent tests.
		#
		run_all_processes "json"

		#------------------------------------------------------
		# RUN k2hdkclinetool process
		#------------------------------------------------------
		PRNMSG "RUN K2HDKCLINETOOL"

		if ! "${LINETOOLBIN}" -conf "${CFG_SLV_FILE_PREFIX}.json" -ctlport 8031 "${DKCTOOL_COMLOG_OPT}" "${DKCTOOL_COMLOG_PARAM}" "${DKCTOOL_DBG_OPT}" "${DKCTOOL_DBG_PARAM}" -rejoin -nogiveup -nocleanup -run "${LINETOOLEXITCMD}" > "${LINETOOLEXITLOG}" 2>&1; then
			PRNERR "FAILED : RUN K2HDKCLINETOOL"
			TEST_RESULT=1
		else
			PRNINFO "SUCCEED : RUN K2HDKCLINETOOL"
		fi

		#------------------------------------------------------
		# Stop all processes
		#------------------------------------------------------
		PRNMSG "STOP ALL PROCESSES CHMPX(server/slave) / K2HDKC"

		# [NOTE]
		# The order of the list is the stop order and is important.
		#
		ALL_PIDS="	${CHMPX_8031_PID}
					${K2HDKC_8027_PID}
					${K2HDKC_LT_8027_PID}
					${CHMPX_8027_PID}
					${K2HDKC_8025_PID}
					${K2HDKC_LT_8025_PID}
					${CHMPX_8025_PID}
					${K2HDKC_8023_PID}
					${K2HDKC_LT_8023_PID}
					${CHMPX_8023_PID}
					${K2HDKC_8021_PID}
					${K2HDKC_LT_8021_PID}
					${CHMPX_8021_PID}"

		if ! stop_process "${ALL_PIDS}"; then
			PRNERR "FAILED : STOP ALL PROCESSES CHMPX(server/slave) / K2HDKC"
			TEST_RESULT=1
		else
			PRNINFO "SUCCEED : STOP ALL PROCESSES CHMPX(server/slave) / K2HDKC"
		fi

		#------------------------------------------------------
		# Print result
		#------------------------------------------------------
		if [ "${TEST_RESULT}" -ne 0 ]; then
			PRNERR "FAILED : TEST FOR JSON CONF FILE (EXECUTION)"
			PRNINFO "(The test has failed. Print each logs)"

			print_log_detail "nodiff"
		else
			PRNSUCCEED "TEST FOR JSON CONF FILE (EXECUTION)"
		fi

		if [ "${TEST_RESULT}" -ne 0 ]; then
			touch "${SUB_SHELLGROUP_ERROR_FILE}"
		fi
	} | tee -a "${LOGFILE}"

	#
	# Set error flag
	#
	if [ -f "${SUB_SHELLGROUP_ERROR_FILE}" ]; then
		TEST_RESULT=1
	fi

	#----------------------------------------------------------
	# Cleanup
	#----------------------------------------------------------
	cleanup_files
fi

#==============================================================
# Start JSON string
#==============================================================
if [ "${TEST_RESULT}" -eq 0 ] && [ "${DO_JSON_STRING}" -eq 1 ]; then
	#
	# Run shell process group for logging
	#
	{
		PRNTITLE "TEST FOR JSON STRING (EXECUTION)"

		#------------------------------------------------------
		# RUN all test processes
		#------------------------------------------------------
		# [NOTE]
		# The process runs in the background, so it doesn't do any error checking.
		# If it fails to start, an error will occur in subsequent tests.
		#
		run_all_processes "jsonstring"

		#------------------------------------------------------
		# RUN k2hdkclinetool process
		#------------------------------------------------------
		PRNMSG "RUN K2HDKCLINETOOL"

		if ! "${LINETOOLBIN}" -json "${TEST_SLAVE_JSON}" -ctlport 8031 "${DKCTOOL_COMLOG_OPT}" "${DKCTOOL_COMLOG_PARAM}" "${DKCTOOL_DBG_OPT}" "${DKCTOOL_DBG_PARAM}" -rejoin -nogiveup -nocleanup -run "${LINETOOLEXITCMD}" > "${LINETOOLEXITLOG}" 2>&1; then
			PRNERR "FAILED : RUN K2HDKCLINETOOL"
			TEST_RESULT=1
		else
			PRNINFO "SUCCEED : RUN K2HDKCLINETOOL"
		fi

		#------------------------------------------------------
		# Stop all processes
		#------------------------------------------------------
		PRNMSG "STOP ALL PROCESSES CHMPX(server/slave) / K2HDKC"

		# [NOTE]
		# The order of the list is the stop order and is important.
		#
		ALL_PIDS="	${CHMPX_8031_PID}
					${K2HDKC_8027_PID}
					${K2HDKC_LT_8027_PID}
					${CHMPX_8027_PID}
					${K2HDKC_8025_PID}
					${K2HDKC_LT_8025_PID}
					${CHMPX_8025_PID}
					${K2HDKC_8023_PID}
					${K2HDKC_LT_8023_PID}
					${CHMPX_8023_PID}
					${K2HDKC_8021_PID}
					${K2HDKC_LT_8021_PID}
					${CHMPX_8021_PID}"

		if ! stop_process "${ALL_PIDS}"; then
			PRNERR "FAILED : STOP ALL PROCESSES CHMPX(server/slave) / K2HDKC"
			TEST_RESULT=1
		else
			PRNINFO "SUCCEED : STOP ALL PROCESSES CHMPX(server/slave) / K2HDKC"
		fi

		#------------------------------------------------------
		# Print result
		#------------------------------------------------------
		if [ "${TEST_RESULT}" -ne 0 ]; then
			PRNERR "FAILED : TEST FOR JSON STRING (EXECUTION)"
			PRNINFO "(The test has failed. Print each logs)"

			print_log_detail "nodiff"
		else
			PRNSUCCEED "TEST FOR JSON STRING (EXECUTION)"
		fi

		if [ "${TEST_RESULT}" -ne 0 ]; then
			touch "${SUB_SHELLGROUP_ERROR_FILE}"
		fi
	} | tee -a "${LOGFILE}"

	#
	# Set error flag
	#
	if [ -f "${SUB_SHELLGROUP_ERROR_FILE}" ]; then
		TEST_RESULT=1
	fi

	#----------------------------------------------------------
	# Cleanup
	#----------------------------------------------------------
	cleanup_files
fi

#==============================================================
# Start JSON environment(execution)
#==============================================================
if [ "${TEST_RESULT}" -eq 0 ] && [ "${DO_JSON_ENV}" -eq 1 ]; then
	#
	# Run shell process group for logging
	#
	{
		PRNTITLE "TEST FOR JSON ENV (EXECUTION)"

		#------------------------------------------------------
		# RUN all test processes
		#------------------------------------------------------
		# [NOTE]
		# The process runs in the background, so it doesn't do any error checking.
		# If it fails to start, an error will occur in subsequent tests.
		#
		run_all_processes "jsonenv"


		#------------------------------------------------------
		# RUN k2hdkclinetool process
		#------------------------------------------------------
		PRNMSG "RUN K2HDKCLINETOOL"

		if ! K2HDKCJSONCONF="${TEST_SLAVE_JSON}" "${LINETOOLBIN}" -ctlport 8031 "${DKCTOOL_COMLOG_OPT}" "${DKCTOOL_COMLOG_PARAM}" "${DKCTOOL_DBG_OPT}" "${DKCTOOL_DBG_PARAM}" -rejoin -nogiveup -nocleanup -run "${LINETOOLEXITCMD}" > "${LINETOOLEXITLOG}" 2>&1; then
			PRNERR "FAILED : RUN K2HDKCLINETOOL"
			TEST_RESULT=1
		else
			PRNINFO "SUCCEED : RUN K2HDKCLINETOOL"
		fi

		#------------------------------------------------------
		# Stop all processes
		#------------------------------------------------------
		PRNMSG "STOP ALL PROCESSES CHMPX(server/slave) / K2HDKC"

		# [NOTE]
		# The order of the list is the stop order and is important.
		#
		ALL_PIDS="	${CHMPX_8031_PID}
					${K2HDKC_8027_PID}
					${K2HDKC_LT_8027_PID}
					${CHMPX_8027_PID}
					${K2HDKC_8025_PID}
					${K2HDKC_LT_8025_PID}
					${CHMPX_8025_PID}
					${K2HDKC_8023_PID}
					${K2HDKC_LT_8023_PID}
					${CHMPX_8023_PID}
					${K2HDKC_8021_PID}
					${K2HDKC_LT_8021_PID}
					${CHMPX_8021_PID}"

		if ! stop_process "${ALL_PIDS}"; then
			PRNERR "FAILED : STOP ALL PROCESSES CHMPX(server/slave) / K2HDKC"
			TEST_RESULT=1
		else
			PRNINFO "SUCCEED : STOP ALL PROCESSES CHMPX(server/slave) / K2HDKC"
		fi

		#------------------------------------------------------
		# Print result
		#------------------------------------------------------
		if [ "${TEST_RESULT}" -ne 0 ]; then
			PRNERR "FAILED : TEST FOR JSON ENV (EXECUTION)"
			PRNINFO "(The test has failed. Print each logs)"

			print_log_detail "nodiff"
		else
			PRNSUCCEED "TEST FOR JSON ENV (EXECUTION)"
		fi

		if [ "${TEST_RESULT}" -ne 0 ]; then
			touch "${SUB_SHELLGROUP_ERROR_FILE}"
		fi
	} | tee -a "${LOGFILE}"

	#
	# Set error flag
	#
	if [ -f "${SUB_SHELLGROUP_ERROR_FILE}" ]; then
		TEST_RESULT=1
	fi

	#----------------------------------------------------------
	# Cleanup
	#----------------------------------------------------------
	cleanup_files
fi

#==============================================================
# Test all command(one of configuration)
#==============================================================
if [ "${TEST_RESULT}" -eq 0 ] && [ "${DO_INI_CONF}" -eq 1 ]; then
	#==========================================================
	# Test all command(INI)
	#==========================================================
	#
	# Run shell process group for logging
	#
	{
		PRNTITLE "TEST FOR INI CONF FILE (COMMAND TEST)"

		#------------------------------------------------------
		# RUN all test processes
		#------------------------------------------------------
		# [NOTE]
		# The process runs in the background, so it doesn't do any error checking.
		# If it fails to start, an error will occur in subsequent tests.
		#
		run_all_processes "ini"

		#------------------------------------------------------
		# RUN k2hdkclinetool process
		#------------------------------------------------------
		#
		# CAPI/PERM
		#
		PRNMSG "RUN K2HDKCLINETOOL CAPI/PERM"
		{
			echo "=========================================================================="
			echo "======= CAPI : PERM : REJOIN : NOGIVEUP : NOCLEANUP"
			echo "=========================================================================="
		} > "${LINETOOLLOG}"
		if ! "${LINETOOLBIN}" -conf "${CFG_SLV_FILE_PREFIX}.ini" -ctlport 8031 "${DKCTOOL_COMLOG_OPT}" "${DKCTOOL_COMLOG_PARAM}" "${DKCTOOL_DBG_OPT}" "${DKCTOOL_DBG_PARAM}" -perm -rejoin -nogiveup -nocleanup -run "${LINETOOLCMD}" -capi >> "${LINETOOLLOG}" 2>> "${LINETOOL_C_P_LOGFILE}"; then
			PRNERR "FAILED : RUN K2HDKCLINETOOL CAPI/PERM"
			echo "FAILED" >> "${LINETOOLLOG}"
			TEST_RESULT=1
		else
			PRNINFO "SUCCEED : RUN K2HDKCLINETOOL CAPI/PERM"
			echo "SUCCEED" >> "${LINETOOLLOG}"
		fi
		sleep "${WAIT_SEC_AFTER_RUN_K2HDKCTOOL}"

		#
		# CPPAPI/PERM
		#
		PRNMSG "RUN K2HDKCLINETOOL CPPAPI/PERM"
		{
			echo "=========================================================================="
			echo "======= CPPAPI : PERM : REJOIN : NOGIVEUP : NOCLEANUP"
			echo "=========================================================================="
		} >> "${LINETOOLLOG}"
		if ! "${LINETOOLBIN}" -conf "${CFG_SLV_FILE_PREFIX}.ini" -ctlport 8031 "${DKCTOOL_COMLOG_OPT}" "${DKCTOOL_COMLOG_PARAM}" "${DKCTOOL_DBG_OPT}" "${DKCTOOL_DBG_PARAM}" -perm -rejoin -nogiveup -nocleanup -run "${LINETOOLCMD}" >> "${LINETOOLLOG}" 2>> "${LINETOOL_CPP_P_LOGFILE}"; then
			PRNERR "FAILED : RUN K2HDKCLINETOOL CPPAPI/PERM"
			echo "FAILED" >> "${LINETOOLLOG}"
			TEST_RESULT=1
		else
			PRNINFO "SUCCEED : RUN K2HDKCLINETOOL CPPAPI/PERM"
			echo "SUCCEED" >> "${LINETOOLLOG}"
		fi
		sleep "${WAIT_SEC_AFTER_RUN_K2HDKCTOOL}"

		#
		# CPPAPI/PERM
		#
		PRNMSG "RUN K2HDKCLINETOOL CAPI/NOT PERM"
		{
			echo "=========================================================================="
			echo "======= CAPI : NOT PERM : REJOIN : NOGIVEUP : NOCLEANUP"
			echo "=========================================================================="
		} >> "${LINETOOLLOG}"
		if ! "${LINETOOLBIN}" -conf "${CFG_SLV_FILE_PREFIX}.ini" -ctlport 8031 "${DKCTOOL_COMLOG_OPT}" "${DKCTOOL_COMLOG_PARAM}" "${DKCTOOL_DBG_OPT}" "${DKCTOOL_DBG_PARAM}" -rejoin -nogiveup -nocleanup -run "${LINETOOLCMD}" -capi >> "${LINETOOLLOG}" 2>> "${LINETOOL_C_LOGFILE}"; then
			PRNERR "FAILED : RUN K2HDKCLINETOOL CAPI/NOT PERM"
			echo "FAILED" >> "${LINETOOLLOG}"
			TEST_RESULT=1
		else
			PRNINFO "SUCCEED : RUN K2HDKCLINETOOL CAPI/NOT PERM"
			echo "SUCCEED" >> "${LINETOOLLOG}"
		fi
		sleep "${WAIT_SEC_AFTER_RUN_K2HDKCTOOL}"

		#
		# CPPAPI/PERM
		#
		PRNMSG "RUN K2HDKCLINETOOL CPPAPI/NOT PERM"
		{
			echo "=========================================================================="
			echo "======= CPPAPI : NOT PERM : REJOIN : NOGIVEUP : NOCLEANUP"
			echo "=========================================================================="
		} >> "${LINETOOLLOG}"
		if ! "${LINETOOLBIN}" -conf "${CFG_SLV_FILE_PREFIX}.ini" -ctlport 8031 "${DKCTOOL_COMLOG_OPT}" "${DKCTOOL_COMLOG_PARAM}" "${DKCTOOL_DBG_OPT}" "${DKCTOOL_DBG_PARAM}" -rejoin -nogiveup -nocleanup -run "${LINETOOLCMD}" >> "${LINETOOLLOG}" 2>> "${LINETOOL_CPP_LOGFILE}"; then
			PRNERR "FAILED : RUN K2HDKCLINETOOL CPPAPI/NOT PERM"
			echo "FAILED" >> "${LINETOOLLOG}"
			TEST_RESULT=1
		else
			PRNINFO "SUCCEED : RUN K2HDKCLINETOOL CPPAPI/NOT PERM"
			echo "SUCCEED" >> "${LINETOOLLOG}"
		fi
		sleep "${WAIT_SEC_AFTER_RUN_K2HDKCTOOL}"

		#------------------------------------------------------
		# Stop all processes
		#------------------------------------------------------
		PRNMSG "STOP ALL PROCESSES CHMPX(server/slave) / K2HDKC"

		# [NOTE]
		# The order of the list is the stop order and is important.
		#
		ALL_PIDS="	${CHMPX_8031_PID}
					${K2HDKC_8027_PID}
					${K2HDKC_LT_8027_PID}
					${CHMPX_8027_PID}
					${K2HDKC_8025_PID}
					${K2HDKC_LT_8025_PID}
					${CHMPX_8025_PID}
					${K2HDKC_8023_PID}
					${K2HDKC_LT_8023_PID}
					${CHMPX_8023_PID}
					${K2HDKC_8021_PID}
					${K2HDKC_LT_8021_PID}
					${CHMPX_8021_PID}"

		if ! stop_process "${ALL_PIDS}"; then
			PRNERR "FAILED : STOP ALL PROCESSES CHMPX(server/slave) / K2HDKC"
			TEST_RESULT=1
		else
			PRNINFO "SUCCEED : STOP ALL PROCESSES CHMPX(server/slave) / K2HDKC"
		fi

		#------------------------------------------------------
		# Print result
		#------------------------------------------------------
		if ! diff "${LINETOOLLOG}" "${MASTER_LINETOOLLOG}" >/dev/null 2>&1; then
			TEST_RESULT=1
		fi
		if [ "${TEST_RESULT}" -ne 0 ]; then
			PRNERR "FAILED : TEST FOR INI CONF FILE (COMMAND TEST)"
			PRNINFO "(The test has failed. Print each logs)"

			print_log_detail "diff"
		else
			PRNSUCCEED "TEST FOR INI CONF FILE (COMMAND TEST)"
		fi

		if [ "${TEST_RESULT}" -ne 0 ]; then
			touch "${SUB_SHELLGROUP_ERROR_FILE}"
		fi
	} | tee -a "${LOGFILE}"

	#
	# Set error flag
	#
	if [ -f "${SUB_SHELLGROUP_ERROR_FILE}" ]; then
		TEST_RESULT=1
	fi

	#----------------------------------------------------------
	# Cleanup
	#----------------------------------------------------------
	cleanup_files

elif [ "${TEST_RESULT}" -eq 0 ] && [ "${DO_YAML_CONF}" -eq 1 ]; then
	#==========================================================
	# Test all command(YAML)
	#==========================================================
	#
	# Run shell process group for logging
	#
	{
		PRNTITLE "TEST FOR YAML CONF FILE (COMMAND TEST)"

		#------------------------------------------------------
		# RUN all test processes
		#------------------------------------------------------
		# [NOTE]
		# The process runs in the background, so it doesn't do any error checking.
		# If it fails to start, an error will occur in subsequent tests.
		#
		run_all_processes "yaml"

		#------------------------------------------------------
		# RUN k2hdkclinetool process
		#------------------------------------------------------
		#
		# CAPI/PERM
		#
		PRNMSG "RUN K2HDKCLINETOOL CAPI/PERM"
		{
			echo "=========================================================================="
			echo "======= CAPI : PERM : REJOIN : NOGIVEUP : NOCLEANUP"
			echo "=========================================================================="
		} > "${LINETOOLLOG}"
		if ! "${LINETOOLBIN}" -conf "${CFG_SLV_FILE_PREFIX}.yaml" -ctlport 8031 "${DKCTOOL_COMLOG_OPT}" "${DKCTOOL_COMLOG_PARAM}" "${DKCTOOL_DBG_OPT}" "${DKCTOOL_DBG_PARAM}" -perm -rejoin -nogiveup -nocleanup -run "${LINETOOLCMD}" -capi >> "${LINETOOLLOG}" 2>> "${LINETOOL_C_P_LOGFILE}"; then
			PRNERR "FAILED : RUN K2HDKCLINETOOL CAPI/PERM"
			echo "FAILED" >> "${LINETOOLLOG}"
			TEST_RESULT=1
		else
			PRNINFO "SUCCEED : RUN K2HDKCLINETOOL CAPI/PERM"
			echo "SUCCEED" >> "${LINETOOLLOG}"
		fi
		sleep "${WAIT_SEC_AFTER_RUN_K2HDKCTOOL}"

		#
		# CPPAPI/PERM
		#
		PRNMSG "RUN K2HDKCLINETOOL CPPAPI/PERM"
		{
			echo "=========================================================================="
			echo "======= CPPAPI : PERM : REJOIN : NOGIVEUP : NOCLEANUP"
			echo "=========================================================================="
		} >> "${LINETOOLLOG}"
		if ! "${LINETOOLBIN}" -conf "${CFG_SLV_FILE_PREFIX}.yaml" -ctlport 8031 "${DKCTOOL_COMLOG_OPT}" "${DKCTOOL_COMLOG_PARAM}" "${DKCTOOL_DBG_OPT}" "${DKCTOOL_DBG_PARAM}" -perm -rejoin -nogiveup -nocleanup -run "${LINETOOLCMD}" >> "${LINETOOLLOG}" 2>> "${LINETOOL_CPP_P_LOGFILE}"; then
			PRNERR "FAILED : RUN K2HDKCLINETOOL CPPAPI/PERM"
			echo "FAILED" >> "${LINETOOLLOG}"
			TEST_RESULT=1
		else
			PRNINFO "SUCCEED : RUN K2HDKCLINETOOL CPPAPI/PERM"
			echo "SUCCEED" >> "${LINETOOLLOG}"
		fi
		sleep "${WAIT_SEC_AFTER_RUN_K2HDKCTOOL}"

		#
		# CPPAPI/PERM
		#
		PRNMSG "RUN K2HDKCLINETOOL CAPI/NOT PERM"
		{
			echo "=========================================================================="
			echo "======= CAPI : NOT PERM : REJOIN : NOGIVEUP : NOCLEANUP"
			echo "=========================================================================="
		} >> "${LINETOOLLOG}"
		if ! "${LINETOOLBIN}" -conf "${CFG_SLV_FILE_PREFIX}.yaml" -ctlport 8031 "${DKCTOOL_COMLOG_OPT}" "${DKCTOOL_COMLOG_PARAM}" "${DKCTOOL_DBG_OPT}" "${DKCTOOL_DBG_PARAM}" -rejoin -nogiveup -nocleanup -run "${LINETOOLCMD}" -capi >> "${LINETOOLLOG}" 2>> "${LINETOOL_C_LOGFILE}"; then
			PRNERR "FAILED : RUN K2HDKCLINETOOL CAPI/NOT PERM"
			echo "FAILED" >> "${LINETOOLLOG}"
			TEST_RESULT=1
		else
			PRNINFO "SUCCEED : RUN K2HDKCLINETOOL CAPI/NOT PERM"
			echo "SUCCEED" >> "${LINETOOLLOG}"
		fi
		sleep "${WAIT_SEC_AFTER_RUN_K2HDKCTOOL}"

		#
		# CPPAPI/PERM
		#
		PRNMSG "RUN K2HDKCLINETOOL CPPAPI/NOT PERM"
		{
			echo "=========================================================================="
			echo "======= CPPAPI : NOT PERM : REJOIN : NOGIVEUP : NOCLEANUP"
			echo "=========================================================================="
		} >> "${LINETOOLLOG}"
		if ! "${LINETOOLBIN}" -conf "${CFG_SLV_FILE_PREFIX}.yaml" -ctlport 8031 "${DKCTOOL_COMLOG_OPT}" "${DKCTOOL_COMLOG_PARAM}" "${DKCTOOL_DBG_OPT}" "${DKCTOOL_DBG_PARAM}" -rejoin -nogiveup -nocleanup -run "${LINETOOLCMD}" >> "${LINETOOLLOG}" 2>> "${LINETOOL_CPP_LOGFILE}"; then
			PRNERR "FAILED : RUN K2HDKCLINETOOL CPPAPI/NOT PERM"
			echo "FAILED" >> "${LINETOOLLOG}"
			TEST_RESULT=1
		else
			PRNINFO "SUCCEED : RUN K2HDKCLINETOOL CPPAPI/NOT PERM"
			echo "SUCCEED" >> "${LINETOOLLOG}"
		fi
		sleep "${WAIT_SEC_AFTER_RUN_K2HDKCTOOL}"

		#------------------------------------------------------
		# Stop all processes
		#------------------------------------------------------
		PRNMSG "STOP ALL PROCESSES CHMPX(server/slave) / K2HDKC"

		# [NOTE]
		# The order of the list is the stop order and is important.
		#
		ALL_PIDS="	${CHMPX_8031_PID}
					${K2HDKC_8027_PID}
					${K2HDKC_LT_8027_PID}
					${CHMPX_8027_PID}
					${K2HDKC_8025_PID}
					${K2HDKC_LT_8025_PID}
					${CHMPX_8025_PID}
					${K2HDKC_8023_PID}
					${K2HDKC_LT_8023_PID}
					${CHMPX_8023_PID}
					${K2HDKC_8021_PID}
					${K2HDKC_LT_8021_PID}
					${CHMPX_8021_PID}"

		if ! stop_process "${ALL_PIDS}"; then
			PRNERR "FAILED : STOP ALL PROCESSES CHMPX(server/slave) / K2HDKC"
			TEST_RESULT=1
		else
			PRNINFO "SUCCEED : STOP ALL PROCESSES CHMPX(server/slave) / K2HDKC"
		fi

		#------------------------------------------------------
		# Print result
		#------------------------------------------------------
		if ! diff "${LINETOOLLOG}" "${MASTER_LINETOOLLOG}" >/dev/null 2>&1; then
			TEST_RESULT=1
		fi
		if [ "${TEST_RESULT}" -ne 0 ]; then
			PRNERR "FAILED : TEST FOR YAML CONF FILE (COMMAND TEST)"
			PRNINFO "(The test has failed. Print each logs)"

			print_log_detail "diff"
		else
			PRNSUCCEED "TEST FOR YAML CONF FILE (COMMAND TEST)"
		fi

		if [ "${TEST_RESULT}" -ne 0 ]; then
			touch "${SUB_SHELLGROUP_ERROR_FILE}"
		fi
	} | tee -a "${LOGFILE}"

	#
	# Set error flag
	#
	if [ -f "${SUB_SHELLGROUP_ERROR_FILE}" ]; then
		TEST_RESULT=1
	fi

	#----------------------------------------------------------
	# Cleanup
	#----------------------------------------------------------
	cleanup_files

elif [ "${TEST_RESULT}" -eq 0 ] && [ "${DO_JSON_CONF}" -eq 1 ]; then
	#==========================================================
	# Test all command(JSON)
	#==========================================================
	#
	# Run shell process group for logging
	#
	{
		PRNTITLE "TEST FOR JSON CONF FILE (COMMAND TEST)"

		#------------------------------------------------------
		# RUN all test processes
		#------------------------------------------------------
		# [NOTE]
		# The process runs in the background, so it doesn't do any error checking.
		# If it fails to start, an error will occur in subsequent tests.
		#
		run_all_processes "json"

		#------------------------------------------------------
		# RUN k2hdkclinetool process
		#------------------------------------------------------
		#
		# CAPI/PERM
		#
		PRNMSG "RUN K2HDKCLINETOOL CAPI/PERM"
		{
			echo "=========================================================================="
			echo "======= CAPI : PERM : REJOIN : NOGIVEUP : NOCLEANUP"
			echo "=========================================================================="
		} > "${LINETOOLLOG}"
		if ! "${LINETOOLBIN}" -conf "${CFG_SLV_FILE_PREFIX}.json" -ctlport 8031 "${DKCTOOL_COMLOG_OPT}" "${DKCTOOL_COMLOG_PARAM}" "${DKCTOOL_DBG_OPT}" "${DKCTOOL_DBG_PARAM}" -perm -rejoin -nogiveup -nocleanup -run "${LINETOOLCMD}" -capi >> "${LINETOOLLOG}" 2>> "${LINETOOL_C_P_LOGFILE}"; then
			PRNERR "FAILED : RUN K2HDKCLINETOOL CAPI/PERM"
			echo "FAILED" >> "${LINETOOLLOG}"
			TEST_RESULT=1
		else
			PRNINFO "SUCCEED : RUN K2HDKCLINETOOL CAPI/PERM"
			echo "SUCCEED" >> "${LINETOOLLOG}"
		fi
		sleep "${WAIT_SEC_AFTER_RUN_K2HDKCTOOL}"

		#
		# CPPAPI/PERM
		#
		PRNMSG "RUN K2HDKCLINETOOL CPPAPI/PERM"
		{
			echo "=========================================================================="
			echo "======= CPPAPI : PERM : REJOIN : NOGIVEUP : NOCLEANUP"
			echo "=========================================================================="
		} >> "${LINETOOLLOG}"
		if ! "${LINETOOLBIN}" -conf "${CFG_SLV_FILE_PREFIX}.json" -ctlport 8031 "${DKCTOOL_COMLOG_OPT}" "${DKCTOOL_COMLOG_PARAM}" "${DKCTOOL_DBG_OPT}" "${DKCTOOL_DBG_PARAM}" -perm -rejoin -nogiveup -nocleanup -run "${LINETOOLCMD}" >> "${LINETOOLLOG}" 2>> "${LINETOOL_CPP_P_LOGFILE}"; then
			PRNERR "FAILED : RUN K2HDKCLINETOOL CPPAPI/PERM"
			echo "FAILED" >> "${LINETOOLLOG}"
			TEST_RESULT=1
		else
			PRNINFO "SUCCEED : RUN K2HDKCLINETOOL CPPAPI/PERM"
			echo "SUCCEED" >> "${LINETOOLLOG}"
		fi
		sleep "${WAIT_SEC_AFTER_RUN_K2HDKCTOOL}"

		#
		# CPPAPI/PERM
		#
		PRNMSG "RUN K2HDKCLINETOOL CAPI/NOT PERM"
		{
			echo "=========================================================================="
			echo "======= CAPI : NOT PERM : REJOIN : NOGIVEUP : NOCLEANUP"
			echo "=========================================================================="
		} >> "${LINETOOLLOG}"
		if ! "${LINETOOLBIN}" -conf "${CFG_SLV_FILE_PREFIX}.json" -ctlport 8031 "${DKCTOOL_COMLOG_OPT}" "${DKCTOOL_COMLOG_PARAM}" "${DKCTOOL_DBG_OPT}" "${DKCTOOL_DBG_PARAM}" -rejoin -nogiveup -nocleanup -run "${LINETOOLCMD}" -capi >> "${LINETOOLLOG}" 2>> "${LINETOOL_C_LOGFILE}"; then
			PRNERR "FAILED : RUN K2HDKCLINETOOL CAPI/NOT PERM"
			echo "FAILED" >> "${LINETOOLLOG}"
			TEST_RESULT=1
		else
			PRNINFO "SUCCEED : RUN K2HDKCLINETOOL CAPI/NOT PERM"
			echo "SUCCEED" >> "${LINETOOLLOG}"
		fi
		sleep "${WAIT_SEC_AFTER_RUN_K2HDKCTOOL}"

		#
		# CPPAPI/PERM
		#
		PRNMSG "RUN K2HDKCLINETOOL CPPAPI/NOT PERM"
		{
			echo "=========================================================================="
			echo "======= CPPAPI : NOT PERM : REJOIN : NOGIVEUP : NOCLEANUP"
			echo "=========================================================================="
		} >> "${LINETOOLLOG}"
		if ! "${LINETOOLBIN}" -conf "${CFG_SLV_FILE_PREFIX}.json" -ctlport 8031 "${DKCTOOL_COMLOG_OPT}" "${DKCTOOL_COMLOG_PARAM}" "${DKCTOOL_DBG_OPT}" "${DKCTOOL_DBG_PARAM}" -rejoin -nogiveup -nocleanup -run "${LINETOOLCMD}" >> "${LINETOOLLOG}" 2>> "${LINETOOL_CPP_LOGFILE}"; then
			PRNERR "FAILED : RUN K2HDKCLINETOOL CPPAPI/NOT PERM"
			echo "FAILED" >> "${LINETOOLLOG}"
			TEST_RESULT=1
		else
			PRNINFO "SUCCEED : RUN K2HDKCLINETOOL CPPAPI/NOT PERM"
			echo "SUCCEED" >> "${LINETOOLLOG}"
		fi
		sleep "${WAIT_SEC_AFTER_RUN_K2HDKCTOOL}"

		#------------------------------------------------------
		# Stop all processes
		#------------------------------------------------------
		PRNMSG "STOP ALL PROCESSES CHMPX(server/slave) / K2HDKC"

		# [NOTE]
		# The order of the list is the stop order and is important.
		#
		ALL_PIDS="	${CHMPX_8031_PID}
					${K2HDKC_8027_PID}
					${K2HDKC_LT_8027_PID}
					${CHMPX_8027_PID}
					${K2HDKC_8025_PID}
					${K2HDKC_LT_8025_PID}
					${CHMPX_8025_PID}
					${K2HDKC_8023_PID}
					${K2HDKC_LT_8023_PID}
					${CHMPX_8023_PID}
					${K2HDKC_8021_PID}
					${K2HDKC_LT_8021_PID}
					${CHMPX_8021_PID}"

		if ! stop_process "${ALL_PIDS}"; then
			PRNERR "FAILED : STOP ALL PROCESSES CHMPX(server/slave) / K2HDKC"
			TEST_RESULT=1
		else
			PRNINFO "SUCCEED : STOP ALL PROCESSES CHMPX(server/slave) / K2HDKC"
		fi

		#------------------------------------------------------
		# Print result
		#------------------------------------------------------
		if ! diff "${LINETOOLLOG}" "${MASTER_LINETOOLLOG}" >/dev/null 2>&1; then
			TEST_RESULT=1
		fi
		if [ "${TEST_RESULT}" -ne 0 ]; then
			PRNERR "FAILED : TEST FOR JSON CONF FILE (COMMAND TEST)"
			PRNINFO "(The test has failed. Print each logs)"

			print_log_detail "diff"
		else
			PRNSUCCEED "TEST FOR JSON CONF FILE (COMMAND TEST)"
		fi

		if [ "${TEST_RESULT}" -ne 0 ]; then
			touch "${SUB_SHELLGROUP_ERROR_FILE}"
		fi
	} | tee -a "${LOGFILE}"

	#
	# Set error flag
	#
	if [ -f "${SUB_SHELLGROUP_ERROR_FILE}" ]; then
		TEST_RESULT=1
	fi

	#----------------------------------------------------------
	# Cleanup
	#----------------------------------------------------------
	cleanup_files

elif [ "${TEST_RESULT}" -eq 0 ] && [ "${DO_JSON_STRING}" -eq 1 ]; then
	#==========================================================
	# Test all command(JSON String)
	#==========================================================
	#
	# Run shell process group for logging
	#
	{
		PRNTITLE "TEST FOR JSON STRING (COMMAND TEST)"

		#------------------------------------------------------
		# RUN all test processes
		#------------------------------------------------------
		# [NOTE]
		# The process runs in the background, so it doesn't do any error checking.
		# If it fails to start, an error will occur in subsequent tests.
		#
		run_all_processes "jsonstring"

		#------------------------------------------------------
		# RUN k2hdkclinetool process
		#------------------------------------------------------
		#
		# CAPI/PERM
		#
		PRNMSG "RUN K2HDKCLINETOOL CAPI/PERM"
		{
			echo "=========================================================================="
			echo "======= CAPI : PERM : REJOIN : NOGIVEUP : NOCLEANUP"
			echo "=========================================================================="
		} > "${LINETOOLLOG}"
		if ! "${LINETOOLBIN}" -json "${TEST_SLAVE_JSON}" -ctlport 8031 "${DKCTOOL_COMLOG_OPT}" "${DKCTOOL_COMLOG_PARAM}" "${DKCTOOL_DBG_OPT}" "${DKCTOOL_DBG_PARAM}" -perm -rejoin -nogiveup -nocleanup -run "${LINETOOLCMD}" -capi >> "${LINETOOLLOG}" 2>> "${LINETOOL_C_P_LOGFILE}"; then
			PRNERR "FAILED : RUN K2HDKCLINETOOL CAPI/PERM"
			echo "FAILED" >> "${LINETOOLLOG}"
			TEST_RESULT=1
		else
			PRNINFO "SUCCEED : RUN K2HDKCLINETOOL CAPI/PERM"
			echo "SUCCEED" >> "${LINETOOLLOG}"
		fi
		sleep "${WAIT_SEC_AFTER_RUN_K2HDKCTOOL}"

		#
		# CPPAPI/PERM
		#
		PRNMSG "RUN K2HDKCLINETOOL CPPAPI/PERM"
		{
			echo "=========================================================================="
			echo "======= CPPAPI : PERM : REJOIN : NOGIVEUP : NOCLEANUP"
			echo "=========================================================================="
		} >> "${LINETOOLLOG}"
		if ! "${LINETOOLBIN}" -json "${TEST_SLAVE_JSON}" -ctlport 8031 "${DKCTOOL_COMLOG_OPT}" "${DKCTOOL_COMLOG_PARAM}" "${DKCTOOL_DBG_OPT}" "${DKCTOOL_DBG_PARAM}" -perm -rejoin -nogiveup -nocleanup -run "${LINETOOLCMD}" >> "${LINETOOLLOG}" 2>> "${LINETOOL_CPP_P_LOGFILE}"; then
			PRNERR "FAILED : RUN K2HDKCLINETOOL CPPAPI/PERM"
			echo "FAILED" >> "${LINETOOLLOG}"
			TEST_RESULT=1
		else
			PRNINFO "SUCCEED : RUN K2HDKCLINETOOL CPPAPI/PERM"
			echo "SUCCEED" >> "${LINETOOLLOG}"
		fi
		sleep "${WAIT_SEC_AFTER_RUN_K2HDKCTOOL}"

		#
		# CPPAPI/PERM
		#
		PRNMSG "RUN K2HDKCLINETOOL CAPI/NOT PERM"
		{
			echo "=========================================================================="
			echo "======= CAPI : NOT PERM : REJOIN : NOGIVEUP : NOCLEANUP"
			echo "=========================================================================="
		} >> "${LINETOOLLOG}"
		if ! "${LINETOOLBIN}" -json "${TEST_SLAVE_JSON}" -ctlport 8031 "${DKCTOOL_COMLOG_OPT}" "${DKCTOOL_COMLOG_PARAM}" "${DKCTOOL_DBG_OPT}" "${DKCTOOL_DBG_PARAM}" -rejoin -nogiveup -nocleanup -run "${LINETOOLCMD}" -capi >> "${LINETOOLLOG}" 2>> "${LINETOOL_C_LOGFILE}"; then
			PRNERR "FAILED : RUN K2HDKCLINETOOL CAPI/NOT PERM"
			echo "FAILED" >> "${LINETOOLLOG}"
			TEST_RESULT=1
		else
			PRNINFO "SUCCEED : RUN K2HDKCLINETOOL CAPI/NOT PERM"
			echo "SUCCEED" >> "${LINETOOLLOG}"
		fi
		sleep "${WAIT_SEC_AFTER_RUN_K2HDKCTOOL}"

		#
		# CPPAPI/PERM
		#
		PRNMSG "RUN K2HDKCLINETOOL CPPAPI/NOT PERM"
		{
			echo "=========================================================================="
			echo "======= CPPAPI : NOT PERM : REJOIN : NOGIVEUP : NOCLEANUP"
			echo "=========================================================================="
		} >> "${LINETOOLLOG}"
		if ! "${LINETOOLBIN}" -json "${TEST_SLAVE_JSON}" -ctlport 8031 "${DKCTOOL_COMLOG_OPT}" "${DKCTOOL_COMLOG_PARAM}" "${DKCTOOL_DBG_OPT}" "${DKCTOOL_DBG_PARAM}" -rejoin -nogiveup -nocleanup -run "${LINETOOLCMD}" >> "${LINETOOLLOG}" 2>> "${LINETOOL_CPP_LOGFILE}"; then
			PRNERR "FAILED : RUN K2HDKCLINETOOL CPPAPI/NOT PERM"
			echo "FAILED" >> "${LINETOOLLOG}"
			TEST_RESULT=1
		else
			PRNINFO "SUCCEED : RUN K2HDKCLINETOOL CPPAPI/NOT PERM"
			echo "SUCCEED" >> "${LINETOOLLOG}"
		fi
		sleep "${WAIT_SEC_AFTER_RUN_K2HDKCTOOL}"

		#------------------------------------------------------
		# Stop all processes
		#------------------------------------------------------
		PRNMSG "STOP ALL PROCESSES CHMPX(server/slave) / K2HDKC"

		# [NOTE]
		# The order of the list is the stop order and is important.
		#
		ALL_PIDS="	${CHMPX_8031_PID}
					${K2HDKC_8027_PID}
					${K2HDKC_LT_8027_PID}
					${CHMPX_8027_PID}
					${K2HDKC_8025_PID}
					${K2HDKC_LT_8025_PID}
					${CHMPX_8025_PID}
					${K2HDKC_8023_PID}
					${K2HDKC_LT_8023_PID}
					${CHMPX_8023_PID}
					${K2HDKC_8021_PID}
					${K2HDKC_LT_8021_PID}
					${CHMPX_8021_PID}"

		if ! stop_process "${ALL_PIDS}"; then
			PRNERR "FAILED : STOP ALL PROCESSES CHMPX(server/slave) / K2HDKC"
			TEST_RESULT=1
		else
			PRNINFO "SUCCEED : STOP ALL PROCESSES CHMPX(server/slave) / K2HDKC"
		fi

		#------------------------------------------------------
		# Print result
		#------------------------------------------------------
		if ! diff "${LINETOOLLOG}" "${MASTER_LINETOOLLOG}" >/dev/null 2>&1; then
			TEST_RESULT=1
		fi
		if [ "${TEST_RESULT}" -ne 0 ]; then
			PRNERR "FAILED : TEST FOR JSON STRING (COMMAND TEST)"
			PRNINFO "(The test has failed. Print each logs)"

			print_log_detail "diff"
		else
			PRNSUCCEED "TEST FOR JSON STRING (COMMAND TEST)"
		fi

		if [ "${TEST_RESULT}" -ne 0 ]; then
			touch "${SUB_SHELLGROUP_ERROR_FILE}"
		fi
	} | tee -a "${LOGFILE}"

	#
	# Set error flag
	#
	if [ -f "${SUB_SHELLGROUP_ERROR_FILE}" ]; then
		TEST_RESULT=1
	fi

	#----------------------------------------------------------
	# Cleanup
	#----------------------------------------------------------
	cleanup_files

elif [ "${TEST_RESULT}" -eq 0 ] && [ "${DO_JSON_ENV}" -eq 1 ]; then
	#==========================================================
	# Test all command(JSON Environment)
	#==========================================================
	#
	# Run shell process group for logging
	#
	{
		PRNTITLE "TEST FOR JSON ENV (COMMAND TEST)"

		#------------------------------------------------------
		# RUN all test processes
		#------------------------------------------------------
		# [NOTE]
		# The process runs in the background, so it doesn't do any error checking.
		# If it fails to start, an error will occur in subsequent tests.
		#
		run_all_processes "jsonenv"

		#------------------------------------------------------
		# RUN k2hdkclinetool process
		#------------------------------------------------------
		#
		# CAPI/PERM
		#
		PRNMSG "RUN K2HDKCLINETOOL CAPI/PERM"
		{
			echo "=========================================================================="
			echo "======= CAPI : PERM : REJOIN : NOGIVEUP : NOCLEANUP"
			echo "=========================================================================="
		} > "${LINETOOLLOG}"
		if ! K2HDKCJSONCONF="${TEST_SLAVE_JSON}" "${LINETOOLBIN}" -ctlport 8031 "${DKCTOOL_COMLOG_OPT}" "${DKCTOOL_COMLOG_PARAM}" "${DKCTOOL_DBG_OPT}" "${DKCTOOL_DBG_PARAM}" -perm -rejoin -nogiveup -nocleanup -run "${LINETOOLCMD}" -capi >> "${LINETOOLLOG}" 2>> "${LINETOOL_C_P_LOGFILE}"; then
			PRNERR "FAILED : RUN K2HDKCLINETOOL CAPI/PERM"
			echo "FAILED" >> "${LINETOOLLOG}"
			TEST_RESULT=1
		else
			PRNINFO "SUCCEED : RUN K2HDKCLINETOOL CAPI/PERM"
			echo "SUCCEED" >> "${LINETOOLLOG}"
		fi
		sleep "${WAIT_SEC_AFTER_RUN_K2HDKCTOOL}"

		#
		# CPPAPI/PERM
		#
		PRNMSG "RUN K2HDKCLINETOOL CPPAPI/PERM"
		{
			echo "=========================================================================="
			echo "======= CPPAPI : PERM : REJOIN : NOGIVEUP : NOCLEANUP"
			echo "=========================================================================="
		} >> "${LINETOOLLOG}"
		if ! K2HDKCJSONCONF="${TEST_SLAVE_JSON}" "${LINETOOLBIN}" -ctlport 8031 "${DKCTOOL_COMLOG_OPT}" "${DKCTOOL_COMLOG_PARAM}" "${DKCTOOL_DBG_OPT}" "${DKCTOOL_DBG_PARAM}" -perm -rejoin -nogiveup -nocleanup -run "${LINETOOLCMD}" >> "${LINETOOLLOG}" 2>> "${LINETOOL_CPP_P_LOGFILE}"; then
			PRNERR "FAILED : RUN K2HDKCLINETOOL CPPAPI/PERM"
			echo "FAILED" >> "${LINETOOLLOG}"
			TEST_RESULT=1
		else
			PRNINFO "SUCCEED : RUN K2HDKCLINETOOL CPPAPI/PERM"
			echo "SUCCEED" >> "${LINETOOLLOG}"
		fi
		sleep "${WAIT_SEC_AFTER_RUN_K2HDKCTOOL}"

		#
		# CPPAPI/PERM
		#
		PRNMSG "RUN K2HDKCLINETOOL CAPI/NOT PERM"
		{
			echo "=========================================================================="
			echo "======= CAPI : NOT PERM : REJOIN : NOGIVEUP : NOCLEANUP"
			echo "=========================================================================="
		} >> "${LINETOOLLOG}"
		if ! K2HDKCJSONCONF="${TEST_SLAVE_JSON}" "${LINETOOLBIN}" -ctlport 8031 "${DKCTOOL_COMLOG_OPT}" "${DKCTOOL_COMLOG_PARAM}" "${DKCTOOL_DBG_OPT}" "${DKCTOOL_DBG_PARAM}" -rejoin -nogiveup -nocleanup -run "${LINETOOLCMD}" -capi >> "${LINETOOLLOG}" 2>> "${LINETOOL_C_LOGFILE}"; then
			PRNERR "FAILED : RUN K2HDKCLINETOOL CAPI/NOT PERM"
			echo "FAILED" >> "${LINETOOLLOG}"
			TEST_RESULT=1
		else
			PRNINFO "SUCCEED : RUN K2HDKCLINETOOL CAPI/NOT PERM"
			echo "SUCCEED" >> "${LINETOOLLOG}"
		fi
		sleep "${WAIT_SEC_AFTER_RUN_K2HDKCTOOL}"

		#
		# CPPAPI/PERM
		#
		PRNMSG "RUN K2HDKCLINETOOL CPPAPI/NOT PERM"
		{
			echo "=========================================================================="
			echo "======= CPPAPI : NOT PERM : REJOIN : NOGIVEUP : NOCLEANUP"
			echo "=========================================================================="
		} >> "${LINETOOLLOG}"
		if ! K2HDKCJSONCONF="${TEST_SLAVE_JSON}" "${LINETOOLBIN}" -ctlport 8031 "${DKCTOOL_COMLOG_OPT}" "${DKCTOOL_COMLOG_PARAM}" "${DKCTOOL_DBG_OPT}" "${DKCTOOL_DBG_PARAM}" -rejoin -nogiveup -nocleanup -run "${LINETOOLCMD}" >> "${LINETOOLLOG}" 2>> "${LINETOOL_CPP_LOGFILE}"; then
			PRNERR "FAILED : RUN K2HDKCLINETOOL CPPAPI/NOT PERM"
			echo "FAILED" >> "${LINETOOLLOG}"
			TEST_RESULT=1
		else
			PRNINFO "SUCCEED : RUN K2HDKCLINETOOL CPPAPI/NOT PERM"
			echo "SUCCEED" >> "${LINETOOLLOG}"
		fi
		sleep "${WAIT_SEC_AFTER_RUN_K2HDKCTOOL}"

		#------------------------------------------------------
		# Stop all processes
		#------------------------------------------------------
		PRNMSG "STOP ALL PROCESSES CHMPX(server/slave) / K2HDKC"

		# [NOTE]
		# The order of the list is the stop order and is important.
		#
		ALL_PIDS="	${CHMPX_8031_PID}
					${K2HDKC_8027_PID}
					${K2HDKC_LT_8027_PID}
					${CHMPX_8027_PID}
					${K2HDKC_8025_PID}
					${K2HDKC_LT_8025_PID}
					${CHMPX_8025_PID}
					${K2HDKC_8023_PID}
					${K2HDKC_LT_8023_PID}
					${CHMPX_8023_PID}
					${K2HDKC_8021_PID}
					${K2HDKC_LT_8021_PID}
					${CHMPX_8021_PID}"

		if ! stop_process "${ALL_PIDS}"; then
			PRNERR "FAILED : STOP ALL PROCESSES CHMPX(server/slave) / K2HDKC"
			TEST_RESULT=1
		else
			PRNINFO "SUCCEED : STOP ALL PROCESSES CHMPX(server/slave) / K2HDKC"
		fi

		#------------------------------------------------------
		# Print result
		#------------------------------------------------------
		if ! diff "${LINETOOLLOG}" "${MASTER_LINETOOLLOG}" >/dev/null 2>&1; then
			TEST_RESULT=1
		fi
		if [ "${TEST_RESULT}" -ne 0 ]; then
			PRNERR "FAILED : TEST FOR JSON ENV (COMMAND TEST)"
			PRNINFO "(The test has failed. Print each logs)"

			print_log_detail "diff"
		else
			PRNSUCCEED "TEST FOR JSON ENV (COMMAND TEST)"
		fi

		if [ "${TEST_RESULT}" -ne 0 ]; then
			touch "${SUB_SHELLGROUP_ERROR_FILE}"
		fi
	} | tee -a "${LOGFILE}"

	#
	# Set error flag
	#
	if [ -f "${SUB_SHELLGROUP_ERROR_FILE}" ]; then
		TEST_RESULT=1
	fi

	#----------------------------------------------------------
	# Cleanup
	#----------------------------------------------------------
	cleanup_files
fi

#==============================================================
# SUMMARY
#==============================================================
FINISH_DATE=$(date)

echo ""
echo "=============================================================="
echo "[SUMMAY LOG] ${START_DATE} -> ${FINISH_DATE}"
echo "--------------------------------------------------------------"
if [ -f "${LOGFILE}" ]; then
	sed -e 's/^/  /g' "${LOGFILE}"
	rm -f "${LOGFILE}"
else
	PRNERR "Not found ${LOGFILE}"
fi
echo "=============================================================="
echo ""

if [ "${TEST_RESULT}" -ne 0 ]; then
	PRNERR "FAILED : ALL TEST"
else
	PRNSUCCEED "ALL TEST"
fi

exit "${TEST_RESULT}"

#
# Local variables:
# tab-width: 4
# c-basic-offset: 4
# End:
# vim600: noexpandtab sw=4 ts=4 fdm=marker
# vim<600: noexpandtab sw=4 ts=4
#
