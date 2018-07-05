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
 * CREATE:   Mon Jul 11 2016
 * REVISION:
 * 
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <k2hash/k2htrans.h>
#include <k2hash/k2htransfunc.h>
#include <chmpx/chmconfutil.h>

#include <fstream>

#include "k2hdkcconfparser.h"
#include "k2hdkcthread.h"
#include "k2hdkcdbg.h"

using namespace	std;

//---------------------------------------------------------
// Symbols
//---------------------------------------------------------
#define	K2HDKC_DEFAULT_TRANSPLUGIN		"libk2htpdtor.so"

#define	INI_COMMENT_CHAR				'#'
#define	INI_INCLUDE_STR					"INCLUDE"
#define	INI_KV_SEP_CHAR					'='
#define	INI_SEC_START_CHAR				'['
#define	INI_SEC_END_CHAR				']'

#define	MAIN_SEC_STR					"K2HDKC"
#define	INI_MAIN_SEC_STR				"[" MAIN_SEC_STR "]"

#define	INI_KEY_RCV_TIMEOUT_STR			"RCVTIMEOUT"
#define	INI_KEY_SVRNODEINI_STR			"SVRNODEINI"
#define	INI_KEY_REPLCLUSTERINI_STR		"REPLCLUSTERINI"
#define	INI_KEY_DTORTHREADCNT_STR		"DTORTHREADCNT"
#define	INI_KEY_DTORCTP_STR				"DTORCTP"
#define	INI_KEY_K2HTYPE_STR				"K2HTYPE"
#define	INI_KEY_K2HFILE_STR				"K2HFILE"
#define	INI_KEY_K2HFULLMAP_STR			"K2HFULLMAP"
#define	INI_KEY_K2HINIT_STR				"K2HINIT"
#define	INI_KEY_K2HMASKBIT_STR			"K2HMASKBIT"
#define	INI_KEY_K2HCMASKBIT_STR			"K2HCMASKBIT"
#define	INI_KEY_K2HMAXELE_STR			"K2HMAXELE"
#define	INI_KEY_K2HPAGESIZE_STR			"K2HPAGESIZE"
#define	INI_KEY_PASSPHRASES_STR			"PASSPHRASES"
#define	INI_KEY_PASSFILE_STR			"PASSFILE"
#define	INI_KEY_HISTORY_STR				"HISTORY"
#define	INI_KEY_EXPIRE_STR				"EXPIRE"
#define	INI_KEY_ATTRPLUGIN_STR			"ATTRPLUGIN"
#define	INI_KEY_MIN_THREAD_CNT_STR		"MINTHREAD"
#define	INI_KEY_MAX_THREAD_CNT_STR		"MAXTHREAD"
#define	INI_KEY_REDUCE_TIME_STR			"REDUCETIME"

#define	INI_VAL_YES1_STR				"Y"
#define	INI_VAL_YES2_STR				"YES"
#define	INI_VAL_NO1_STR					"N"
#define	INI_VAL_NO2_STR					"NO"
#define	INI_VAL_ON_STR					"ON"
#define	INI_VAL_OFF_STR					"OFF"

#define	INI_VAL_MEM1_STR				"M"
#define	INI_VAL_MEM2_STR				"MEM"
#define	INI_VAL_MEM3_STR				"MEMORY"
#define	INI_VAL_FILE1_STR				"F"
#define	INI_VAL_FILE2_STR				"FILE"
#define	INI_VAL_TEMP1_STR				"T"
#define	INI_VAL_TEMP2_STR				"TEMP"
#define	INI_VAL_TEMP3_STR				"TEMPORARY"

//---------------------------------------------------------
// Class Methods
//---------------------------------------------------------
bool K2hdkcConfig::ReadContents(const char* path, strarr_t& lines, strarr_t& files)
{
	if(DKCEMPTYSTR(path)){
		ERR_DKCPRN("path is empty.");
		return false;
	}

	// get absolute file path
	string	abspath;
	if(!path_to_abspath(path, abspath)){
		ERR_DKCPRN("Could not convert absolute file path from file path(%s)", path);
		return false;
	}
	// is already listed
	for(strarr_t::const_iterator iter = files.begin(); iter != files.end(); ++iter){
		if(abspath == (*iter)){
			ERR_DKCPRN("file(%s:%s) is already loaded.", path, abspath.c_str());
			return false;
		}
	}
	files.push_back(abspath);

	// load
	ifstream	cfgstream(abspath.c_str(), ios::in);
	if(!cfgstream.good()){
		ERR_DKCPRN("Could not open(read only) file(%s:%s)", path, abspath.c_str());
		return false;
	}

	string	line;
	int		lineno;
	for(lineno = 1; cfgstream.good() && getline(cfgstream, line); lineno++){
		line = trim(line);
		if(0 == line.length()){
			continue;
		}
		if(INI_COMMENT_CHAR == line[0]){
			continue;
		}

		// check only include
		string::size_type	pos;
		if(string::npos != (pos = line.find(INI_INCLUDE_STR))){
			string	value	= trim(line.substr(pos + 1));
			string	key		= trim(line.substr(0, pos));
			if(key == INI_INCLUDE_STR){
				// found include, do reentrant
				if(!K2hdkcConfig::ReadContents(value.c_str(), lines, files)){
					ERR_DKCPRN("Failed to load include file(%s)", value.c_str());
					cfgstream.close();
					return false;
				}
				continue;
			}
		}
		// add
		lines.push_back(line);
	}
	cfgstream.close();

	return true;
}

void K2hdkcConfig::ParseLine(const string& line, string& key, string& value, bool allow_comment_ch)
{
	string				tmpline = line;
	string::size_type	pos;
	if(!allow_comment_ch){
		if(string::npos != (pos = line.find(INI_COMMENT_CHAR))){
			tmpline = trim(tmpline.substr(0, pos));
		}
	}
	if(string::npos != (pos = tmpline.find(INI_KV_SEP_CHAR))){
		key		= upper(trim(tmpline.substr(0, pos)));		// key convert to upper
		value	= trim(tmpline.substr(pos + 1));
	}else{
		key		= upper(trim(tmpline));						// key convert to upper
		value	= "";
	}
}

//---------------------------------------------------------
// Constructor / Destructor
//---------------------------------------------------------
K2hdkcConfig::K2hdkcConfig(void) :
	dkc_config(""), rcvtimeout_ms(-1), svrnode_config(""), repl_config(""), repl_thread_cnt(K2HTransManager::DEFAULT_THREAD_POOL), repl_ctp_file(K2HDKC_DEFAULT_TRANSPLUGIN),
	k2h_file(""), k2h_file_temp(false), k2h_fullmap(true), k2h_need_init(true), k2h_create_file(false),
	k2h_maskbit(K2HShm::DEFAULT_MASK_BITCOUNT), k2h_cmaskbit(K2HShm::DEFAULT_COLLISION_MASK_BITCOUNT), k2h_elementcnt(K2HShm::DEFAULT_MAX_ELEMENT_CNT), k2h_pagesize(K2HShm::MIN_PAGE_SIZE),
	enc_pass_file(""), is_history(false), expire_time(0), min_thread_cnt(K2hdkcThread::MIN_THREAD_COUNT), max_thread_cnt(K2hdkcThread::MAX_THREAD_COUNT), thread_reduce_time(K2hdkcThread::DEFAULT_REDUCE_TIMEOUT)
{
}

K2hdkcConfig::~K2hdkcConfig()
{
	Clear();
}

//---------------------------------------------------------
// Methods
//---------------------------------------------------------
bool K2hdkcConfig::Clear(void)
{
	dkc_config.erase();
	svrnode_config.erase();
	repl_config.erase();
	k2h_file.erase();
	enc_pass_file.erase();
	attr_plugin_files.clear();

	repl_ctp_file	= K2HDKC_DEFAULT_TRANSPLUGIN;
	repl_thread_cnt	= K2HTransManager::DEFAULT_THREAD_POOL;
	k2h_file_temp	= false;
	k2h_fullmap		= true;
	k2h_need_init	= true;
	k2h_create_file	= false;
	k2h_maskbit		= K2HShm::DEFAULT_MASK_BITCOUNT;
	k2h_cmaskbit	= K2HShm::DEFAULT_COLLISION_MASK_BITCOUNT;
	k2h_elementcnt	= K2HShm::DEFAULT_MAX_ELEMENT_CNT;
	k2h_pagesize	= K2HShm::MIN_PAGE_SIZE;
	is_history		= false;
	expire_time		= 0;

	// [NOTE]
	// enc_passphrases has pass phrases, then we MUST clear buffer by NULL charactor.
	//
	for(strarr_t::iterator iter = enc_passphrases.begin(); iter != enc_passphrases.end(); ++iter){
		size_t	strlength	= iter->length();
		char*	pstring		= const_cast<char*>(iter->c_str());
		memset(pstring, '\0', strlength);
	}
	enc_passphrases.clear();

	return true;
}

bool K2hdkcConfig::Load(const char* config)
{
	// initialize all valiables
	Clear();

	// check configuration
	string		normalize_config("");
	CHMCONFTYPE	conftype= check_chmconf_type_ex(config, K2HDKC_CONFFILE_ENV_NAME, K2HDKC_JSONCONF_ENV_NAME, &normalize_config);

	// set default(initial value)
	dkc_config			= normalize_config;
	svrnode_config		= normalize_config;		// default is same configration for chmpx server node.

	bool		result	= false;
	switch(conftype){
		case	CHMCONF_TYPE_INI_FILE:
			result = LoadIni(normalize_config.c_str());
			break;

		case	CHMCONF_TYPE_YAML_FILE:
		case	CHMCONF_TYPE_JSON_FILE:
			result = LoadYaml(normalize_config.c_str(), false);
			break;

		case	CHMCONF_TYPE_JSON_STRING:
			result = LoadYaml(normalize_config.c_str(), true);
			break;

		case	CHMCONF_TYPE_UNKNOWN:
		case	CHMCONF_TYPE_NULL:
		default:
			ERR_DKCPRN("configuration file or json string is wrong.");
			break;
	}

	// check values
	if(result){
		if(svrnode_config.empty()){
			ERR_DKCPRN("Could not find keyworad(%s) in section(%s) in configuration(%s).", INI_KEY_SVRNODEINI_STR, INI_MAIN_SEC_STR, normalize_config.c_str());
			result = false;
		}
		if(repl_config.empty()){
			MSG_DKCPRN("Could not find keyworad(%s) in section(%s) in configuration(%s).", INI_KEY_REPLCLUSTERINI_STR, INI_MAIN_SEC_STR, normalize_config.c_str());
		}
		if(k2h_file.empty()){
			// memory type
			if(k2h_file_temp || !k2h_fullmap || !k2h_need_init || k2h_create_file){
				ERR_DKCPRN("keyworad(%s) in section(%s) in configuration(%s) is %s, but %s or %s value are wrong.", INI_KEY_K2HTYPE_STR, INI_MAIN_SEC_STR, normalize_config.c_str(), INI_VAL_MEM3_STR, INI_KEY_K2HFULLMAP_STR, INI_KEY_K2HINIT_STR);
				result = false;
			}
		}else{
			if(k2h_file_temp){
				// temporary file type
				if(!k2h_need_init){
					ERR_DKCPRN("keyworad(%s) in section(%s) in configuration(%s) is %s, but %s value are wrong.", INI_KEY_K2HTYPE_STR, INI_MAIN_SEC_STR, normalize_config.c_str(), INI_VAL_TEMP3_STR, INI_KEY_K2HINIT_STR);
					result = false;
				}
			}else{
				// permanent file type -> all cases are allowed
			}
		}
	}

	// dump configuration or clear if error
	if(result){
		Dump();
	}else{
		Clear();
	}
	return result;
}


bool K2hdkcConfig::LoadIni(const char* file)
{
	if(DKCEMPTYSTR(file)){
		ERR_DKCPRN("Parameter is wrong.");
		return false;
	}

	strarr_t	lines;
	strarr_t	files;
	if(!K2hdkcConfig::ReadContents(file, lines, files)){
		ERR_DKCPRN("Failed to load oncfigration ini file(%s)", file);
		return false;
	}

	// Load
	bool	inner_sec		= false;
	bool	is_set_k2htype	= false;
	bool	is_set_passfile	= false;
	bool	is_set_passstr	= false;
	bool	is_set_maxthcnt	= false;
	for(strarr_t::const_iterator iter = lines.begin(); iter != lines.end(); ++iter){
		if(!inner_sec){
			if((*iter) == INI_MAIN_SEC_STR){
				inner_sec = true;
			}
			continue;
		}
		if(INI_SEC_START_CHAR == (*iter)[0] && INI_SEC_END_CHAR == (*iter)[iter->length() - 1]){
			// another section start, so break loop
			break;
		}
		// the line is in section

		// parse key(to upper) & value
		string	key;
		string	value;
		K2hdkcConfig::ParseLine((*iter), key, value);

		// set keys
		if(INI_KEY_RCV_TIMEOUT_STR == key){
			rcvtimeout_ms = static_cast<long>(atoi(value.c_str()));
			if(rcvtimeout_ms <= 0){
				ERR_DKCPRN("keyworad(%s)'s value(%s) in section(%s) must be over 0.", key.c_str(), value.c_str(), INI_MAIN_SEC_STR);
				return false;
			}

		}else if(INI_KEY_SVRNODEINI_STR == key){
			if(value.empty()){
				// value is empty, so nothing to set.
			}else if(right_check_json_string(value.c_str())){
				// value is json string type configuration, thus we do not check it.
				svrnode_config = value;
			}else{
				// value is configuration file, thus we need to check it.
				string	abspath;
				if(!is_file_exist(value.c_str()) || !path_to_abspath(value.c_str(), abspath)){
					ERR_DKCPRN("keyworad(%s)'s value(%s) in section(%s) is not existed file.", key.c_str(), value.c_str(), INI_MAIN_SEC_STR);
					return false;
				}
				svrnode_config = abspath;
			}

		}else if(INI_KEY_REPLCLUSTERINI_STR == key){
			if(value.empty()){
				// value is empty, so nothing to set.
			}else if(right_check_json_string(value.c_str())){
				// value is json string type configuration, thus we do not check it.
				repl_config = value;
			}else{
				// value is configuration file, thus we need to check it.
				string	abspath;
				if(!is_file_exist(value.c_str()) || !path_to_abspath(value.c_str(), abspath)){
					ERR_DKCPRN("keyworad(%s)'s value(%s) in section(%s) is not existed file.", key.c_str(), value.c_str(), INI_MAIN_SEC_STR);
					return false;
				}
				repl_config = abspath;
			}

		}else if(INI_KEY_DTORTHREADCNT_STR == key){
			repl_thread_cnt = atoi(value.c_str());
			if(repl_thread_cnt < K2HTransManager::NO_THREAD_POOL || K2HTransManager::MAX_THREAD_POOL < repl_thread_cnt){
				ERR_DKCPRN("keyworad(%s)'s value(%s) in section(%s) must be from %d to %d range.", key.c_str(), value.c_str(), INI_MAIN_SEC_STR, K2HTransManager::NO_THREAD_POOL, K2HTransManager::MAX_THREAD_POOL);
				return false;
			}

		}else if(INI_KEY_DTORCTP_STR == key){
			string	abspath;
			if(!is_file_exist(value.c_str()) || !path_to_abspath(value.c_str(), abspath)){
				ERR_DKCPRN("keyworad(%s)'s value(%s) in section(%s) is not existed file.", key.c_str(), value.c_str(), INI_MAIN_SEC_STR);
				return false;
			}
			repl_ctp_file = abspath;

		}else if(INI_KEY_K2HTYPE_STR == key){
			if(is_set_k2htype){
				ERR_DKCPRN("keyworad(%s) in section(%s) is already set. this value is set only onece.", key.c_str(), INI_MAIN_SEC_STR);
				return false;
			}
			is_set_k2htype	= true;
			value			= upper(value);			// convert to upper
			if(INI_VAL_MEM1_STR == value || INI_VAL_MEM2_STR == value || INI_VAL_MEM3_STR == value){
				k2h_file.erase();
				k2h_file_temp	= false;
				k2h_fullmap		= true;
				k2h_need_init	= true;
				k2h_create_file	= false;

			}else if(INI_VAL_FILE1_STR == value || INI_VAL_FILE2_STR == value){
				k2h_file_temp	= false;

			}else if(INI_VAL_TEMP1_STR == value || INI_VAL_TEMP2_STR == value || INI_VAL_TEMP3_STR == value){
				k2h_file_temp	= true;

			}else{
				ERR_DKCPRN("keyworad(%s)'s value(%s) in section(%s) is unkown value.", key.c_str(), value.c_str(), INI_MAIN_SEC_STR);
				return false;
			}

		}else if(INI_KEY_K2HFILE_STR == key){
			if(!is_file_exist(value.c_str())){
				k2h_create_file	= true;
				k2h_file		= value;			// this is not absolute path
			}else{
				string	abspath;
				if(!path_to_abspath(value.c_str(), abspath)){
					ERR_DKCPRN("keyworad(%s)'s value(%s) in section(%s) is something wrong.", key.c_str(), value.c_str(), INI_MAIN_SEC_STR);
					return false;
				}
				k2h_create_file	= false;
				k2h_file		= abspath;
			}

		}else if(INI_KEY_K2HFULLMAP_STR == key){
			value = upper(value);					// convert to upper
			if(INI_VAL_YES1_STR == value || INI_VAL_YES2_STR == value || INI_VAL_ON_STR == value){
				k2h_fullmap = true;
			}else if(INI_VAL_NO1_STR == value || INI_VAL_NO2_STR == value || INI_VAL_OFF_STR == value){
				k2h_fullmap = false;
			}else{
				ERR_DKCPRN("keyworad(%s)'s value(%s) in section(%s) is unkown value.", key.c_str(), value.c_str(), INI_MAIN_SEC_STR);
				return false;
			}

		}else if(INI_KEY_K2HINIT_STR == key){
			value = upper(value);					// convert to upper
			if(INI_VAL_YES1_STR == value || INI_VAL_YES2_STR == value || INI_VAL_ON_STR == value){
				k2h_need_init = true;
			}else if(INI_VAL_NO1_STR == value || INI_VAL_NO2_STR == value || INI_VAL_OFF_STR == value){
				k2h_need_init = false;
			}else{
				ERR_DKCPRN("keyworad(%s)'s value(%s) in section(%s) is unkown value.", key.c_str(), value.c_str(), INI_MAIN_SEC_STR);
				return false;
			}

		}else if(INI_KEY_K2HMASKBIT_STR == key){
			k2h_maskbit = atoi(value.c_str());
			if(k2h_maskbit < K2HShm::MIN_MASK_BITCOUNT || K2HShm::MAX_MASK_BITCOUNT < k2h_maskbit){
				ERR_DKCPRN("keyworad(%s)'s value(%s) in section(%s) must be from %d to %d range.", key.c_str(), value.c_str(), INI_MAIN_SEC_STR, K2HShm::MIN_MASK_BITCOUNT, K2HShm::MAX_MASK_BITCOUNT);
				return false;
			}

		}else if(INI_KEY_K2HCMASKBIT_STR == key){
			k2h_cmaskbit = atoi(value.c_str());
			if(k2h_cmaskbit <= 0){
				ERR_DKCPRN("keyworad(%s)'s value(%s) in section(%s) must be over 0.", key.c_str(), value.c_str(), INI_MAIN_SEC_STR);
				return false;
			}

		}else if(INI_KEY_K2HMAXELE_STR == key){
			k2h_elementcnt = atoi(value.c_str());
			if(k2h_elementcnt <= 0){
				ERR_DKCPRN("keyworad(%s)'s value(%s) in section(%s) must be over 0.", key.c_str(), value.c_str(), INI_MAIN_SEC_STR);
				return false;
			}

		}else if(INI_KEY_K2HPAGESIZE_STR == key){
			k2h_pagesize = static_cast<size_t>(atoi(value.c_str()));

		}else if(INI_KEY_PASSPHRASES_STR == key){
			if(is_set_passfile){
				ERR_DKCPRN("keyworad(%s) in section(%s) is already set by %s key. you can set only one of this key and %s key.", key.c_str(), INI_MAIN_SEC_STR, INI_KEY_PASSFILE_STR, INI_KEY_PASSFILE_STR);
				return false;
			}
			is_set_passstr = true;

			if(value.empty()){
				ERR_DKCPRN("keyworad(%s)'s value in section(%s) is empty string.", key.c_str(), INI_MAIN_SEC_STR);
				return false;
			}
			enc_passphrases.push_back(value);

		}else if(INI_KEY_PASSFILE_STR == key){
			if(is_set_passfile || is_set_passstr){
				ERR_DKCPRN("keyworad(%s) in section(%s) is already set for encrypt pass phrases. you can set only one of this key and %s key.", key.c_str(), INI_MAIN_SEC_STR, INI_KEY_PASSPHRASES_STR);
				return false;
			}
			is_set_passfile = true;

			string	abspath;
			if(!is_file_exist(value.c_str()) || !path_to_abspath(value.c_str(), abspath)){
				ERR_DKCPRN("keyworad(%s)'s value(%s) in section(%s) is not existed file.", key.c_str(), value.c_str(), INI_MAIN_SEC_STR);
				return false;
			}
			enc_pass_file = abspath;

		}else if(INI_KEY_HISTORY_STR == key){
			value = upper(value);					// convert to upper
			if(INI_VAL_YES1_STR == value || INI_VAL_YES2_STR == value || INI_VAL_ON_STR == value){
				is_history = true;
			}else if(INI_VAL_NO1_STR == value || INI_VAL_NO2_STR == value || INI_VAL_OFF_STR == value){
				is_history = false;
			}else{
				ERR_DKCPRN("keyworad(%s)'s value(%s) in section(%s) is unkown value.", key.c_str(), value.c_str(), INI_MAIN_SEC_STR);
				return false;
			}

		}else if(INI_KEY_EXPIRE_STR == key){
			expire_time = static_cast<size_t>(atoi(value.c_str()));

		}else if(INI_KEY_ATTRPLUGIN_STR == key){
			string	abspath;
			if(!is_file_exist(value.c_str()) || !path_to_abspath(value.c_str(), abspath)){
				ERR_DKCPRN("keyworad(%s)'s value(%s) in section(%s) is not existed file.", key.c_str(), value.c_str(), INI_MAIN_SEC_STR);
				return false;
			}
			attr_plugin_files.push_back(abspath);

		}else if(INI_KEY_MIN_THREAD_CNT_STR == key){
			min_thread_cnt = static_cast<size_t>(atoi(value.c_str()));
			if(min_thread_cnt < K2hdkcThread::MIN_THREAD_COUNT || (is_set_maxthcnt && (max_thread_cnt < min_thread_cnt))){
				ERR_DKCPRN("keyworad(%s)'s value(%s) in section(%s) is wrong(maximum value is %zu).", key.c_str(), value.c_str(), INI_MAIN_SEC_STR, max_thread_cnt);
				return false;
			}

		}else if(INI_KEY_MAX_THREAD_CNT_STR == key){
			max_thread_cnt = static_cast<size_t>(atoi(value.c_str()));
			if(max_thread_cnt < min_thread_cnt){
				ERR_DKCPRN("keyworad(%s)'s value(%s) in section(%s) is wrong(minimum value is %zu).", key.c_str(), value.c_str(), INI_MAIN_SEC_STR, min_thread_cnt);
				return false;
			}
			is_set_maxthcnt = true;

		}else if(INI_KEY_REDUCE_TIME_STR == key){
			thread_reduce_time = static_cast<time_t>(atoi(value.c_str()));
			if(thread_reduce_time <= 0){
				ERR_DKCPRN("keyworad(%s)'s value(%s) in section(%s) must be over 0.", key.c_str(), value.c_str(), INI_MAIN_SEC_STR);
				return false;
			}

		}else{
			WAN_DKCPRN("Unknown keyworad(%s) in section(%s), so skip it and continue...", key.c_str(), INI_MAIN_SEC_STR);
		}
	}

	// Check
	if(!inner_sec){
		ERR_DKCPRN("Could not find section(%s) in file(%s).", INI_MAIN_SEC_STR, file);
		return false;
	}
	return true;
}


bool K2hdkcConfig::LoadYaml(const char* config, bool is_json_string)
{
	// initialize yaml parser
	yaml_parser_t	yparser;
	if(!yaml_parser_initialize(&yparser)){
		ERR_DKCPRN("Failed to initialize yaml parser");
		return false;
	}

	FILE*	fp = NULL;
	if(!is_json_string){
		// open configuration file
		if(NULL == (fp = fopen(config, "r"))){
			ERR_DKCPRN("Could not open configuration file(%s). errno = %d", config, errno);
			return false;
		}
		// set file to parser
		yaml_parser_set_input_file(&yparser, fp);

	}else{	// JSON_STR
		// set string to parser
		yaml_parser_set_input_string(&yparser, reinterpret_cast<const unsigned char*>(config), strlen(config));
	}

	// Do parsing
	bool	result	= LoadYamlTopLevel(yparser);

	yaml_parser_delete(&yparser);
	if(fp){
		fclose(fp);
	}
	return result;
}

bool K2hdkcConfig::LoadYamlTopLevel(yaml_parser_t& yparser)
{
	CHMYamlDataStack	other_stack;
	bool				is_set_main	= false;
	bool				result		= true;
	for(bool is_loop = true, in_stream = false, in_document = false, in_toplevel = false; is_loop && result; ){
		// get event
		yaml_event_t	yevent;
		if(!yaml_parser_parse(&yparser, &yevent)){
			ERR_DKCPRN("Could not parse event. errno = %d", errno);
			result = false;
			continue;
		}

		// check event
		switch(yevent.type){
			case YAML_NO_EVENT:
				MSG_DKCPRN("There is no yaml event in loop");
				break;

			case YAML_STREAM_START_EVENT:
				if(!other_stack.empty()){
					MSG_DKCPRN("Found start yaml stream event in skipping event loop");
					if(!other_stack.add(yevent.type)){
						result = false;
					}
				}else if(in_stream){
					MSG_DKCPRN("Already start yaml stream event in loop, Thus stacks this event.");
					if(!other_stack.add(yevent.type)){
						result = false;
					}
				}else{
					MSG_DKCPRN("Start yaml stream event in loop");
					in_stream = true;
				}
				break;

			case YAML_STREAM_END_EVENT:
				if(!other_stack.empty()){
					MSG_DKCPRN("Found stop yaml stream event in skipping event loop");
					if(!other_stack.add(yevent.type)){
						result = false;
					}
				}else if(!in_stream){
					MSG_DKCPRN("Already stop yaml stream event in loop, Thus stacks this event.");
					if(!other_stack.add(yevent.type)){
						result = false;
					}
				}else{
					MSG_DKCPRN("Stop yaml stream event in loop");
					in_stream = false;
				}
				break;

			case YAML_DOCUMENT_START_EVENT:
				if(!other_stack.empty()){
					MSG_DKCPRN("Found start yaml document event in skipping event loop");
					if(!other_stack.add(yevent.type)){
						result = false;
					}
				}else if(!in_stream){
					MSG_DKCPRN("Found start yaml document event before yaml stream event in loop, Thus stacks this event.");
					if(!other_stack.add(yevent.type)){
						result = false;
					}
				}else if(in_document){
					MSG_DKCPRN("Already start yaml document event in loop, Thus stacks this event.");
					if(!other_stack.add(yevent.type)){
						result = false;
					}
				}else{
					MSG_DKCPRN("Start yaml document event in loop");
					in_document = true;
				}
				break;

			case YAML_DOCUMENT_END_EVENT:
				if(!other_stack.empty()){
					MSG_DKCPRN("Found stop yaml document event in skipping event loop");
					if(!other_stack.add(yevent.type)){
						result = false;
					}
				}else if(!in_document){
					MSG_DKCPRN("Already stop yaml document event in loop, Thus stacks this event.");
					if(!other_stack.add(yevent.type)){
						result = false;
					}
				}else{
					MSG_DKCPRN("Stop yaml document event in loop");
					in_document = false;
				}
				break;

			case YAML_MAPPING_START_EVENT:
				if(!other_stack.empty()){
					MSG_DKCPRN("Found start yaml mapping event in skipping event loop");
					if(!other_stack.add(yevent.type)){
						result = false;
					}
				}else if(!in_stream){
					MSG_DKCPRN("Found start yaml mapping event before yaml stream event in loop, Thus stacks this event.");
					if(!other_stack.add(yevent.type)){
						result = false;
					}
				}else if(!in_document){
					MSG_DKCPRN("Found start yaml mapping event before yaml document event in loop, Thus stacks this event.");
					if(!other_stack.add(yevent.type)){
						result = false;
					}
				}else if(in_toplevel){
					MSG_DKCPRN("Already start yaml mapping event in loop, Thus stacks this event.");
					if(!other_stack.add(yevent.type)){
						result = false;
					}
				}else{
					MSG_DKCPRN("Start yaml mapping event in loop");
					in_toplevel = true;
				}
				break;

			case YAML_MAPPING_END_EVENT:
				if(!other_stack.empty()){
					MSG_DKCPRN("Found stop yaml mapping event in skipping event loop");
					if(!other_stack.add(yevent.type)){
						result = false;
					}
				}else if(!in_toplevel){
					MSG_DKCPRN("Already stop yaml mapping event in loop, Thus stacks this event.");
					if(!other_stack.add(yevent.type)){
						result = false;
					}
				}else{
					MSG_DKCPRN("Stop yaml mapping event in loop");
					in_toplevel = false;
				}
				break;

			case YAML_SEQUENCE_START_EVENT:
				// always stacking
				//
				if(!other_stack.empty()){
					MSG_DKCPRN("Found start yaml sequence event in skipping event loop");
					if(!other_stack.add(yevent.type)){
						result = false;
					}
				}else{
					MSG_DKCPRN("Found start yaml sequence event before top level event in loop, Thus stacks this event.");
					if(!other_stack.add(yevent.type)){
						result = false;
					}
				}
				break;

			case YAML_SEQUENCE_END_EVENT:
				// always stacking
				//
				if(!other_stack.empty()){
					MSG_DKCPRN("Found stop yaml sequence event in skipping event loop");
					if(!other_stack.add(yevent.type)){
						result = false;
					}
				}else{
					MSG_DKCPRN("Found stop yaml sequence event before top level event in loop, Thus stacks this event.");
					if(!other_stack.add(yevent.type)){
						result = false;
					}
				}
				break;

			case YAML_SCALAR_EVENT:
				if(!other_stack.empty()){
					MSG_DKCPRN("Got yaml scalar event in skipping event loop");
					if(!other_stack.add(yevent.type)){
						result = false;
					}
				}else if(!in_stream){
					MSG_DKCPRN("Got yaml scalar event before yaml stream event in loop, Thus stacks this event.");
					if(!other_stack.add(yevent.type)){
						result = false;
					}
				}else if(!in_document){
					MSG_DKCPRN("Got yaml scalar event before yaml document event in loop, Thus stacks this event.");
					if(!other_stack.add(yevent.type)){
						result = false;
					}
				}else if(!in_toplevel){
					MSG_DKCPRN("Got yaml scalar event before yaml mapping event in loop, Thus stacks this event.");
					if(!other_stack.add(yevent.type)){
						result = false;
					}
				}else{
					// Found Top Level Keywards, start to loading
					if(0 == strcasecmp(MAIN_SEC_STR, reinterpret_cast<const char*>(yevent.data.scalar.value))){
						if(is_set_main){
							MSG_DKCPRN("Got yaml scalar event in loop, but already loading %s. Thus stacks this event.", MAIN_SEC_STR);
							if(!other_stack.add(yevent.type)){
								result = false;
							}
						}else{
							// Load MAIN_SEC_STR section
							if(!LoadYamlMainSec(yparser)){
								ERR_DKCPRN("Something error occured in loading %s section.", MAIN_SEC_STR);
								result = false;
							}
						}
					}else{
						MSG_DKCPRN("Got yaml scalar event in loop, but unknown keyward(%s) for me. Thus stacks this event.", reinterpret_cast<const char*>(yevent.data.scalar.value));
						if(!other_stack.add(yevent.type)){
							result = false;
						}
					}
				}
				break;

			case YAML_ALIAS_EVENT:
				// [TODO]
				// Now we do not supports alias(anchor) event.
				//
				MSG_DKCPRN("Got yaml alias(anchor) event in loop, but we does not support this event. Thus skip this event.");
				break;
		}
		// delete event
		is_loop = yevent.type != YAML_STREAM_END_EVENT;
		yaml_event_delete(&yevent);
	}
	return result;
}

bool K2hdkcConfig::LoadYamlMainSec(yaml_parser_t& yparser)
{
	// Must start yaml mapping event.
	yaml_event_t	yevent;
	if(!yaml_parser_parse(&yparser, &yevent)){
		ERR_DKCPRN("Could not parse event. errno = %d", errno);
		return false;
	}
	if(YAML_MAPPING_START_EVENT != yevent.type){
		ERR_DKCPRN("Parsed event type is not start mapping(%d)", yevent.type);
		yaml_event_delete(&yevent);
		return false;
	}
	yaml_event_delete(&yevent);

	// Loading
	string	key("");
	bool	is_set_passfile	= false;
	bool	is_set_passstr	= false;
	bool	is_set_maxthcnt	= false;
	bool	result			= true;
	for(bool is_loop = true; is_loop && result; ){
		// get event
		if(!yaml_parser_parse(&yparser, &yevent)){
			ERR_DKCPRN("Could not parse event. errno = %d", errno);
			result = false;
			continue;
		}

		// check event
		if(YAML_MAPPING_END_EVENT == yevent.type){
			// End of mapping event
			is_loop = false;

		}else if(YAML_SCALAR_EVENT == yevent.type){
			// Load key & value
			if(key.empty()){
				key = reinterpret_cast<const char*>(yevent.data.scalar.value);
			}else{
				//
				// Compare key and set value
				//
				if(0 == strcasecmp(INI_KEY_RCV_TIMEOUT_STR, key.c_str())){
					rcvtimeout_ms = static_cast<long>(atoi(reinterpret_cast<const char*>(yevent.data.scalar.value)));
					if(rcvtimeout_ms <= 0){
						ERR_DKCPRN("keyworad(%s)'s value(%s) in section(%s) must be over 0.", key.c_str(), reinterpret_cast<const char*>(yevent.data.scalar.value), MAIN_SEC_STR);
						result = false;
					}

				}else if(0 == strcasecmp(INI_KEY_SVRNODEINI_STR, key.c_str())){
					if(DKCEMPTYSTR(reinterpret_cast<const char*>(yevent.data.scalar.value))){
						// value is empty, so nothing to set.
					}else if(right_check_json_string(reinterpret_cast<const char*>(yevent.data.scalar.value))){
						// value is json string type configuration, thus we do not check it.
						svrnode_config = reinterpret_cast<const char*>(yevent.data.scalar.value);
					}else{
						// value is configuration file, thus we need to check it.
						string	abspath;
						if(!is_file_exist(reinterpret_cast<const char*>(yevent.data.scalar.value)) || !path_to_abspath(reinterpret_cast<const char*>(yevent.data.scalar.value), abspath)){
							ERR_DKCPRN("keyworad(%s)'s value(%s) in section(%s) is not existed file.", key.c_str(), reinterpret_cast<const char*>(yevent.data.scalar.value), MAIN_SEC_STR);
							result = false;
						}else{
							svrnode_config = abspath;
						}
					}

				}else if(0 == strcasecmp(INI_KEY_REPLCLUSTERINI_STR, key.c_str())){
					if(DKCEMPTYSTR(reinterpret_cast<const char*>(yevent.data.scalar.value))){
						// value is empty, so nothing to set.
					}else if(right_check_json_string(reinterpret_cast<const char*>(yevent.data.scalar.value))){
						// value is json string type configuration, thus we do not check it.
						repl_config = reinterpret_cast<const char*>(yevent.data.scalar.value);
					}else{
						// value is configuration file, thus we need to check it.
						string	abspath;
						if(!is_file_exist(reinterpret_cast<const char*>(yevent.data.scalar.value)) || !path_to_abspath(reinterpret_cast<const char*>(yevent.data.scalar.value), abspath)){
							ERR_DKCPRN("keyworad(%s)'s value(%s) in section(%s) is not existed file.", key.c_str(), reinterpret_cast<const char*>(yevent.data.scalar.value), MAIN_SEC_STR);
							result = false;
						}else{
							repl_config = abspath;
						}
					}

				}else if(0 == strcasecmp(INI_KEY_DTORTHREADCNT_STR, key.c_str())){
					repl_thread_cnt = atoi(reinterpret_cast<const char*>(yevent.data.scalar.value));
					if(repl_thread_cnt < K2HTransManager::NO_THREAD_POOL || K2HTransManager::MAX_THREAD_POOL < repl_thread_cnt){
						ERR_DKCPRN("keyworad(%s)'s value(%s) in section(%s) must be from %d to %d range.", key.c_str(), reinterpret_cast<const char*>(yevent.data.scalar.value), MAIN_SEC_STR, K2HTransManager::NO_THREAD_POOL, K2HTransManager::MAX_THREAD_POOL);
						result = false;
					}

				}else if(0 == strcasecmp(INI_KEY_DTORCTP_STR, key.c_str())){
					string	abspath;
					if(!is_file_exist(reinterpret_cast<const char*>(yevent.data.scalar.value)) || !path_to_abspath(reinterpret_cast<const char*>(yevent.data.scalar.value), abspath)){
						ERR_DKCPRN("keyworad(%s)'s value(%s) in section(%s) is not existed file.", key.c_str(), reinterpret_cast<const char*>(yevent.data.scalar.value), MAIN_SEC_STR);
						result = false;
					}else{
						repl_ctp_file = abspath;
					}

				}else if(0 == strcasecmp(INI_KEY_K2HTYPE_STR, key.c_str())){
					string	strvalue	= upper(reinterpret_cast<const char*>(yevent.data.scalar.value));		// convert to upper
					if(INI_VAL_MEM1_STR == strvalue || INI_VAL_MEM2_STR == strvalue || INI_VAL_MEM3_STR == strvalue){
						k2h_file.erase();
						k2h_file_temp	= false;
						k2h_fullmap		= true;
						k2h_need_init	= true;
						k2h_create_file	= false;

					}else if(INI_VAL_FILE1_STR == strvalue || INI_VAL_FILE2_STR == strvalue){
						k2h_file_temp	= false;

					}else if(INI_VAL_TEMP1_STR == strvalue || INI_VAL_TEMP2_STR == strvalue || INI_VAL_TEMP3_STR == strvalue){
						k2h_file_temp	= true;

					}else{
						ERR_DKCPRN("keyworad(%s)'s value(%s) in section(%s) is unkown value.", key.c_str(), reinterpret_cast<const char*>(yevent.data.scalar.value), MAIN_SEC_STR);
						result = false;
					}

				}else if(0 == strcasecmp(INI_KEY_K2HFILE_STR, key.c_str())){
					if(!is_file_exist(reinterpret_cast<const char*>(yevent.data.scalar.value))){
						k2h_create_file	= true;
						k2h_file		= reinterpret_cast<const char*>(yevent.data.scalar.value);				// this is not absolute path
					}else{
						string	abspath;
						if(!path_to_abspath(reinterpret_cast<const char*>(yevent.data.scalar.value), abspath)){
							ERR_DKCPRN("keyworad(%s)'s value(%s) in section(%s) is something wrong.", key.c_str(), reinterpret_cast<const char*>(yevent.data.scalar.value), MAIN_SEC_STR);
							result = false;
						}else{
							k2h_create_file	= false;
							k2h_file		= abspath;
						}
					}

				}else if(0 == strcasecmp(INI_KEY_K2HFULLMAP_STR, key.c_str())){
					string	strvalue = upper(reinterpret_cast<const char*>(yevent.data.scalar.value));			// convert to upper
					if(INI_VAL_YES1_STR == strvalue || INI_VAL_YES2_STR == strvalue || INI_VAL_ON_STR == strvalue){
						k2h_fullmap = true;
					}else if(INI_VAL_NO1_STR == strvalue || INI_VAL_NO2_STR == strvalue || INI_VAL_OFF_STR == strvalue){
						k2h_fullmap = false;
					}else{
						ERR_DKCPRN("keyworad(%s)'s value(%s) in section(%s) is unkown value.", key.c_str(), reinterpret_cast<const char*>(yevent.data.scalar.value), MAIN_SEC_STR);
						result = false;
					}

				}else if(0 == strcasecmp(INI_KEY_K2HINIT_STR, key.c_str())){
					string	strvalue = upper(reinterpret_cast<const char*>(yevent.data.scalar.value));			// convert to upper
					if(INI_VAL_YES1_STR == strvalue || INI_VAL_YES2_STR == strvalue || INI_VAL_ON_STR == strvalue){
						k2h_need_init = true;
					}else if(INI_VAL_NO1_STR == strvalue || INI_VAL_NO2_STR == strvalue || INI_VAL_OFF_STR == strvalue){
						k2h_need_init = false;
					}else{
						ERR_DKCPRN("keyworad(%s)'s value(%s) in section(%s) is unkown value.", key.c_str(), reinterpret_cast<const char*>(yevent.data.scalar.value), MAIN_SEC_STR);
						result = false;
					}

				}else if(0 == strcasecmp(INI_KEY_K2HMASKBIT_STR, key.c_str())){
					k2h_maskbit = atoi(reinterpret_cast<const char*>(yevent.data.scalar.value));
					if(k2h_maskbit < K2HShm::MIN_MASK_BITCOUNT || K2HShm::MAX_MASK_BITCOUNT < k2h_maskbit){
						ERR_DKCPRN("keyworad(%s)'s value(%s) in section(%s) must be from %d to %d range.", key.c_str(), reinterpret_cast<const char*>(yevent.data.scalar.value), MAIN_SEC_STR, K2HShm::MIN_MASK_BITCOUNT, K2HShm::MAX_MASK_BITCOUNT);
						result = false;
					}

				}else if(0 == strcasecmp(INI_KEY_K2HCMASKBIT_STR, key.c_str())){
					k2h_cmaskbit = atoi(reinterpret_cast<const char*>(yevent.data.scalar.value));
					if(k2h_cmaskbit <= 0){
						ERR_DKCPRN("keyworad(%s)'s value(%s) in section(%s) must be over 0.", key.c_str(), reinterpret_cast<const char*>(yevent.data.scalar.value), MAIN_SEC_STR);
						result = false;
					}

				}else if(0 == strcasecmp(INI_KEY_K2HMAXELE_STR, key.c_str())){
					k2h_elementcnt = atoi(reinterpret_cast<const char*>(yevent.data.scalar.value));
					if(k2h_elementcnt <= 0){
						ERR_DKCPRN("keyworad(%s)'s value(%s) in section(%s) must be over 0.", key.c_str(), reinterpret_cast<const char*>(yevent.data.scalar.value), MAIN_SEC_STR);
						result = false;
					}

				}else if(0 == strcasecmp(INI_KEY_K2HPAGESIZE_STR, key.c_str())){
					k2h_pagesize = static_cast<size_t>(atoi(reinterpret_cast<const char*>(yevent.data.scalar.value)));

				}else if(0 == strcasecmp(INI_KEY_PASSPHRASES_STR, key.c_str())){
					if(is_set_passfile){
						ERR_DKCPRN("keyworad(%s) in section(%s) is already set by %s key. you can set only one of this key and %s key.", key.c_str(), MAIN_SEC_STR, INI_KEY_PASSFILE_STR, INI_KEY_PASSFILE_STR);
						result = false;
					}else{
						is_set_passstr = true;

						if(DKCEMPTYSTR(reinterpret_cast<const char*>(yevent.data.scalar.value))){
							ERR_DKCPRN("keyworad(%s)'s value in section(%s) is empty string.", key.c_str(), MAIN_SEC_STR);
							result = false;
						}else{
							enc_passphrases.push_back(reinterpret_cast<const char*>(yevent.data.scalar.value));
						}
					}

				}else if(0 == strcasecmp(INI_KEY_PASSFILE_STR, key.c_str())){
					if(is_set_passfile || is_set_passstr){
						ERR_DKCPRN("keyworad(%s) in section(%s) is already set for encrypt pass phrases. you can set only one of this key and %s key.", key.c_str(), MAIN_SEC_STR, INI_KEY_PASSPHRASES_STR);
						result = false;
					}else{
						is_set_passfile = true;

						string	abspath;
						if(!is_file_exist(reinterpret_cast<const char*>(yevent.data.scalar.value)) || !path_to_abspath(reinterpret_cast<const char*>(yevent.data.scalar.value), abspath)){
							ERR_DKCPRN("keyworad(%s)'s value(%s) in section(%s) is not existed file.", key.c_str(), reinterpret_cast<const char*>(yevent.data.scalar.value), MAIN_SEC_STR);
							result = false;
						}else{
							enc_pass_file = abspath;
						}
					}

				}else if(0 == strcasecmp(INI_KEY_HISTORY_STR, key.c_str())){
					string	strvalue = upper(reinterpret_cast<const char*>(yevent.data.scalar.value));					// convert to upper
					if(INI_VAL_YES1_STR == strvalue || INI_VAL_YES2_STR == strvalue || INI_VAL_ON_STR == strvalue){
						is_history = true;
					}else if(INI_VAL_NO1_STR == strvalue || INI_VAL_NO2_STR == strvalue || INI_VAL_OFF_STR == strvalue){
						is_history = false;
					}else{
						ERR_DKCPRN("keyworad(%s)'s value(%s) in section(%s) is unkown value.", key.c_str(), reinterpret_cast<const char*>(yevent.data.scalar.value), MAIN_SEC_STR);
						result = false;
					}

				}else if(0 == strcasecmp(INI_KEY_EXPIRE_STR, key.c_str())){
					expire_time = static_cast<size_t>(atoi(reinterpret_cast<const char*>(yevent.data.scalar.value)));

				}else if(0 == strcasecmp(INI_KEY_ATTRPLUGIN_STR, key.c_str())){
					string	abspath;
					if(!is_file_exist(reinterpret_cast<const char*>(yevent.data.scalar.value)) || !path_to_abspath(reinterpret_cast<const char*>(yevent.data.scalar.value), abspath)){
						ERR_DKCPRN("keyworad(%s)'s value(%s) in section(%s) is not existed file.", key.c_str(), reinterpret_cast<const char*>(yevent.data.scalar.value), MAIN_SEC_STR);
						result = false;
					}else{
						attr_plugin_files.push_back(abspath);
					}

				}else if(0 == strcasecmp(INI_KEY_MIN_THREAD_CNT_STR, key.c_str())){
					min_thread_cnt = static_cast<size_t>(atoi(reinterpret_cast<const char*>(yevent.data.scalar.value)));
					if(min_thread_cnt < K2hdkcThread::MIN_THREAD_COUNT || (is_set_maxthcnt && (max_thread_cnt < min_thread_cnt))){
						ERR_DKCPRN("keyworad(%s)'s value(%s) in section(%s) is wrong(maximum value is %zu).", key.c_str(), reinterpret_cast<const char*>(yevent.data.scalar.value), MAIN_SEC_STR, max_thread_cnt);
						result = false;
					}

				}else if(0 == strcasecmp(INI_KEY_MAX_THREAD_CNT_STR, key.c_str())){
					max_thread_cnt = static_cast<size_t>(atoi(reinterpret_cast<const char*>(yevent.data.scalar.value)));
					if(max_thread_cnt < min_thread_cnt){
						ERR_DKCPRN("keyworad(%s)'s value(%s) in section(%s) is wrong(minimum value is %zu).", key.c_str(), reinterpret_cast<const char*>(yevent.data.scalar.value), MAIN_SEC_STR, min_thread_cnt);
						result = false;
					}else{
						is_set_maxthcnt = true;
					}

				}else if(0 == strcasecmp(INI_KEY_REDUCE_TIME_STR, key.c_str())){
					thread_reduce_time = static_cast<time_t>(atoi(reinterpret_cast<const char*>(yevent.data.scalar.value)));
					if(thread_reduce_time <= 0){
						ERR_DKCPRN("keyworad(%s)'s value(%s) in section(%s) must be over 0.", key.c_str(), reinterpret_cast<const char*>(yevent.data.scalar.value), MAIN_SEC_STR);
						result = false;
					}

				}else{
					WAN_DKCPRN("Found unexpected key(%s) in %s section, thus skip this key and value.", key.c_str(), MAIN_SEC_STR);
				}
				key.clear();
			}
		}else{
			// [TODO] Now not support alias(anchor) event
			//
			ERR_DKCPRN("Found unexpected yaml event(%d) in %s section.", yevent.type, MAIN_SEC_STR);
			result = false;
		}

		// delete event
		if(is_loop){
			is_loop = yevent.type != YAML_STREAM_END_EVENT;
		}
		yaml_event_delete(&yevent);
	}
	return result;
}

void K2hdkcConfig::Dump(void) const
{
	if(!IS_DKCDBG_DUMP()){
		return;
	}

	DMP_DKCPRN("k2hdkc configuration                = %s",		dkc_config.c_str());
	DMP_DKCPRN("{"												);
	RAW_DKCPRN("    server node chmpx configuration = %s",		svrnode_config.c_str());
	RAW_DKCPRN("    replication chmpx configuration = %s",		repl_config.c_str());
	RAW_DKCPRN("    replication thread count        = %d",		repl_thread_cnt);
	RAW_DKCPRN("    replication transaction plugin  = %s",		repl_ctp_file.c_str());

	if(k2h_file.empty()){
		RAW_DKCPRN("    k2hash type                     = memory");
	}else{
		RAW_DKCPRN("    k2hash type                     = %s file( %s )", !k2h_file_temp ? "permanent" : "temporary", k2h_file.c_str());
	}

	RAW_DKCPRN("    k2hash is mapped memory fully   = %s",		k2h_fullmap ? "yes" : "no");
	RAW_DKCPRN("    k2hash is need to initialize    = %s",		k2h_need_init ? "yes" : "no");
	RAW_DKCPRN("    k2hash is need to create        = %s",		k2h_create_file ? "yes" : "no");
	RAW_DKCPRN("    k2hash mask bit count           = %d",		k2h_maskbit);
	RAW_DKCPRN("    k2hash collision mask bit count = %d",		k2h_cmaskbit);
	RAW_DKCPRN("    k2hash element count            = %d",		k2h_elementcnt);
	RAW_DKCPRN("    k2hash page size                = %zu byte",k2h_pagesize);

	if(!enc_passphrases.empty()){
		RAW_DKCPRN("    k2hash encrypt mode             = yes( %zu pass phrase is registered )", enc_passphrases.size());
	}else if(!enc_pass_file.empty()){
		RAW_DKCPRN("    k2hash encrypt mode             = yes( pass phrase file : %s )", enc_pass_file.c_str());
	}else{
		RAW_DKCPRN("    k2hash encrypt mode             = no");
	}

	RAW_DKCPRN("    k2hash history mode             = %s",		is_history ? "yes" : "no");

	if(0 == expire_time){
		RAW_DKCPRN("    k2hash expire time              = not set expired time");
	}else{
		RAW_DKCPRN("    k2hash expire time              = %zd",	expire_time);
	}
	if(attr_plugin_files.empty()){
		RAW_DKCPRN("    k2hash attribute plugin         = not set");
	}else{
		string	strtmp;
		for(strarr_t::const_iterator iter = attr_plugin_files.begin(); iter != attr_plugin_files.end(); ++iter){
			if(!strtmp.empty()){
				strtmp += ", ";
			}
			strtmp += *iter;
		}
		RAW_DKCPRN("    k2hash attribute plugin         = %s", strtmp.c_str());
	}

	RAW_DKCPRN("    minimum processinf thread count = %zu",		min_thread_cnt);
	RAW_DKCPRN("    maximum processinf thread count = %zu",		max_thread_cnt);
	RAW_DKCPRN("    reduce time for idle thread     = %zd s",	thread_reduce_time);

	RAW_DKCPRN("}");
}

//
// [NOTE]
// We initialize k2hash object in this class, because this class has encrypt pass phrase.
// So we should not copy those strings to any memory in other class/object/buffer.
// Then we initialize k2hash here.
//
bool K2hdkcConfig::InitializeK2hash(K2HShm& k2hash) const
{
	if(!IsLoaded()){
		ERR_DKCPRN("Configuration file is not loaded yet.");
		return false;
	}

	// set transaction plugin
	if(!repl_config.empty()){
		if(!K2HTransDynLib::get()->Load(repl_ctp_file.c_str())){
			ERR_DKCPRN("Failed to set k2hash transaction plugin(%s).", repl_ctp_file.c_str());
			return false;
		}
		MSG_DKCPRN("Succeed: set transaction plugin(%s)", repl_ctp_file.c_str());

		// set transaction thread count
		if(!K2HShm::SetTransThreadPool(repl_thread_cnt)){
			ERR_DKCPRN("Failed to set k2hash transaction thread count(%d).", repl_thread_cnt);
			return false;
		}
		MSG_DKCPRN("Succeed: set transaction thread count(%d)", repl_thread_cnt);
	}else{
		MSG_DKCPRN("Transaction plugin for replication is not specified, so not set plugin.");
	}

	// initialize k2hash
	bool	result;
	if(!k2h_file.empty() && k2h_create_file){
		result = k2hash.Create(k2h_file.c_str(), k2h_fullmap, k2h_maskbit, k2h_cmaskbit, k2h_elementcnt, k2h_pagesize);
	}else{
		result = k2hash.Attach(k2h_file.empty() ? NULL : k2h_file.c_str(), false, k2h_create_file, k2h_file_temp, k2h_fullmap, k2h_maskbit, k2h_cmaskbit, k2h_elementcnt, k2h_pagesize);
	}
	if(!result){
		ERR_DKCPRN("Failed to initialize k2hash.");
		return false;
	}
	MSG_DKCPRN("Succeed: initialize(attach/create) k2hash file/memory");

	// set attributes(must set after create/attach)
	bool			is_mtime	= true;		// must set true
	bool			is_defenc	= !enc_pass_file.empty();
	const char*		passfile	= enc_pass_file.empty() ? NULL : enc_pass_file.c_str();
	const bool*		phistory	= is_history ? &is_history : NULL;
	const time_t*	expire		= (0 == expire_time) ? NULL : &expire_time;
	const strarr_t*	pluginlibs	= attr_plugin_files.empty() ? NULL : &attr_plugin_files;

	if(!k2hash.SetCommonAttribute(&is_mtime, &is_defenc, passfile, phistory, expire, pluginlibs)){
		ERR_DKCPRN("Failed to set k2hash about attributes.");
		return false;
	}
	if(!enc_passphrases.empty()){
		bool	set_default = true;
		for(strarr_t::const_iterator iter = enc_passphrases.begin(); iter != enc_passphrases.end(); ++iter){
			if(!k2hash.AddAttrCryptPass(iter->c_str(), set_default)){
				ERR_DKCPRN("Failed to set k2hash about encrypt pass phrase.");
				return false;
			}
			if(set_default){
				set_default = false;
			}
		}
	}
	MSG_DKCPRN("Succeed: set attributes");

	// enable replication
	if(!repl_config.empty()){
		if(!k2hash.EnableTransaction(NULL, NULL, 0, reinterpret_cast<const unsigned char*>(repl_config.c_str()), repl_config.length() + 1, (0 == expire_time ? NULL : &expire_time))){
			ERR_DKCPRN("Failed to enable k2hash transaction(replication).");
			return false;
		}
		MSG_DKCPRN("Succeed: enable transaction");
	}

	return true;
}

/*
 * VIM modelines
 *
 * vim:set ts=4 fenc=utf-8:
 */
