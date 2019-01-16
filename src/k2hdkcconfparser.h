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

#ifndef	K2HDKCINIPARSER_H
#define K2HDKCINIPARSER_H

#include <yaml.h>
#include <k2hash/k2hshm.h>
#include <k2hash/k2hutil.h>
#include <string>

#include "k2hdkccommon.h"
#include "k2hdkcutil.h"

//---------------------------------------------------------
// K2hdkcConfig class
//---------------------------------------------------------
class K2hdkcConfig
{
	protected:
		std::string		dkc_config;
		long			rcvtimeout_ms;
		std::string		svrnode_config;
		std::string		repl_config;				// if empty, do not work replication
		int				repl_thread_cnt;
		std::string		repl_ctp_file;
		std::string		k2h_file;					// if empty, k2hash type is memory
		bool			k2h_file_temp;				// valid only when k2h_file is not empty
		bool			k2h_fullmap;				// always true if k2hash type is memory
		bool			k2h_need_init;				// always true if k2hash type is memory
		bool			k2h_create_file;			// always false if k2hash type is memory
		int				k2h_maskbit;
		int				k2h_cmaskbit;
		int				k2h_elementcnt;
		size_t			k2h_pagesize;
		strarr_t		enc_passphrases;			// first pass is default pass, and this value MUST is clear with NULL.
		std::string		enc_pass_file;				// first pass is default pass(both enc_passphrases and enc_pass_file are empty, not encrypt data)
		bool			is_history;
		time_t			expire_time;				// 0 means the data is not expired
		strarr_t		attr_plugin_files;
		size_t			min_thread_cnt;				// minimum processing thread count
		size_t			max_thread_cnt;				// maximum processing thread count
		time_t			thread_reduce_time;			// timeout for reducing processing thread

	protected:
		static bool ReadContents(const char* path, strarr_t& lines, strarr_t& files);
		static void ParseLine(const std::string& line, std::string& key, std::string& value, bool allow_comment_ch = false);

		bool Clear(void);
		void Dump(void) const;

		bool LoadIni(const char* file);
		bool LoadYaml(const char* config, bool is_json_string);
		bool LoadYamlTopLevel(yaml_parser_t& yparser);
		bool LoadYamlMainSec(yaml_parser_t& yparser);

	public:
		K2hdkcConfig(void);
		virtual ~K2hdkcConfig(void);

		bool Load(const char* config);
		bool IsLoaded(void) const { return !dkc_config.empty(); }

		const char* GetServerNodeConfiguration(void) const { return (IsLoaded() && !svrnode_config.empty()) ? svrnode_config.c_str() : NULL; }
		bool GetServerNodeConfiguration(std::string& config) const { if(!IsLoaded() || svrnode_config.empty()){ return false; } config = svrnode_config; return true; }
		long GetReceiveTimeoutMs(void) const { return rcvtimeout_ms; }
		size_t GetMinThreadCount(void) const { return min_thread_cnt; }
		size_t GetMaxThreadCount(void) const { return max_thread_cnt; }
		time_t GetReduceThreadTime(void) const { return thread_reduce_time; }

		bool InitializeK2hash(K2HShm& k2hash) const;
};

#endif	// K2HDKCINIPARSER_H

/*
 * VIM modelines
 *
 * vim:set ts=4 fenc=utf-8:
 */
