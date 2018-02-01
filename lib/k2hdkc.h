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
 * CREATE:   Thu Mar 31 2016
 * REVISION:
 * 
 */

#ifndef	K2HDKC_H
#define	K2HDKC_H

#include <k2hash/k2hash.h>
#include <chmpx/chmpx.h>

#include "k2hdkccommon.h"
#include "k2hdkctypes.h"
#include "k2hdkccomstructure.h"

DECL_EXTERN_C_START

//---------------------------------------------------------
// Symbols
//---------------------------------------------------------
#define	K2HDKC_INVALID_HANDLE		0UL

//---------------------------------------------------------
// Structure & Utilities
//---------------------------------------------------------
// Utility structure for binary message which has key and value.
// 
// Some function gets K2HKEYPCK pointer for sending message.
// These structure and functions help you make binary message pack
// and make hash code.
//
// This structure types are based and defined in k2hash.
//
typedef K2HKEYPCK					K2HDKCKEYPCK;
typedef PK2HKEYPCK					PK2HDKCKEYPCK;

typedef K2HATTRPCK					K2HDKCATTRPCK;
typedef PK2HATTRPCK					PK2HDKCATTRPCK;

#define	DKC_FREE_KEYPACK			k2h_free_keypack
#define	DKC_FREE_KEYARRAY			k2h_free_keyarray

#define	DKC_FREE_ATTRPACK			k2h_free_attrpack

//---------------------------------------------------------
// Functions
//---------------------------------------------------------
// [debug]
//
// k2hdkc_bump_debug_level				bumpup debugging level(silent -> error -> warning -> messages ->...)
// k2hdkc_set_debug_level_silent		set silent for debugging level
// k2hdkc_set_debug_level_error			set error for debugging level
// k2hdkc_set_debug_level_warning		set warning for debugging level
// k2hdkc_set_debug_level_message		set message for debugging level
// k2hdkc_set_debug_level_dump			set dump for debugging level
// k2hdkc_set_debug_file				set file path for debugging message
// k2hdkc_unset_debug_file				unset file path for debugging message to stderr(default)
// k2hdkc_load_debug_env				set debugging level and file path by loading environment.
//
//k2hdkc_is_enable_comlog				check enabled comlog mode
//k2hdkc_enable_comlog					enable comlog
//k2hdkc_disable_comlog					disable comlog
//k2hdkc_toggle_comlog					toggle comlog mode
//
extern void k2hdkc_bump_debug_level(void);
extern void k2hdkc_set_debug_level_silent(void);
extern void k2hdkc_set_debug_level_error(void);
extern void k2hdkc_set_debug_level_warning(void);
extern void k2hdkc_set_debug_level_message(void);
extern void k2hdkc_set_debug_level_dump(void);
extern bool k2hdkc_set_debug_file(const char* filepath);
extern bool k2hdkc_unset_debug_file(void);
extern bool k2hdkc_load_debug_env(void);

extern bool k2hdkc_is_enable_comlog(void);
extern void k2hdkc_enable_comlog(void);
extern void k2hdkc_disable_comlog(void);
extern void k2hdkc_toggle_comlog(void);

//---------------------------------------------------------
// Functions - Get Last response code
//---------------------------------------------------------
// [get last response code]
// 
// k2hdkc_get_lastres_code				get lastest response code(unsafe for multithread)
// k2hdkc_get_lastres_subcode			get lastest response subcode(unsafe for multithread)
// k2hdkc_is_lastres_success			check success by lastest response code(unsafe for multithread)
// k2hdkc_get_res_code					get lastest response code(safe for multithread)
// k2hdkc_get_res_subcode				get lastest response subcode(safe for multithread)
// k2hdkc_is_res_success				check success by lastest response code(safe for multithread)
// 
extern dkcres_type_t k2hdkc_get_lastres_code(void);
extern dkcres_type_t k2hdkc_get_lastres_subcode(void);
extern bool k2hdkc_is_lastres_success(void);
extern dkcres_type_t k2hdkc_get_res_code(k2hdkc_chmpx_h handle);
extern dkcres_type_t k2hdkc_get_res_subcode(k2hdkc_chmpx_h handle);
extern bool k2hdkc_is_res_success(k2hdkc_chmpx_h handle);

//---------------------------------------------------------
// Functions - Chmpx slave object
//---------------------------------------------------------
// [open permanent slave chmpx]
// 
// k2hdkc_open_chmpx					initialize(join) chmpx slave node and open msgid
// k2hdkc_open_chmpx_ex					
// k2hdkc_close_chmpx					close msgid and uninitialize(leave) chmpx slave node
// k2hdkc_close_chmpx_ex				
// 
extern k2hdkc_chmpx_h k2hdkc_open_chmpx(const char* config);
extern k2hdkc_chmpx_h k2hdkc_open_chmpx_ex(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, bool is_clean_bup);
extern bool k2hdkc_close_chmpx(k2hdkc_chmpx_h handle);
extern bool k2hdkc_close_chmpx_ex(k2hdkc_chmpx_h handle, bool is_clean_bup);

//---------------------------------------------------------
// Functions - Get State
//---------------------------------------------------------
// [get state]
// 
// k2hdkc_get_state						get all server node states
// k2hdkc_get_direct_state				
// k2hdkc_ex_get_state					
// k2hdkc_ex_get_direct_state			
// k2hdkc_pm_get_state					
// k2hdkc_pm_get_direct_state			
// 
extern bool k2hdkc_get_state(const char* config, PDKC_NODESTATE* ppstates, size_t* pstatecount);
extern PDKC_NODESTATE k2hdkc_get_direct_state(const char* config, size_t* pstatecount);
extern bool k2hdkc_ex_get_state(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, PDKC_NODESTATE* ppstates, size_t* pstatecount);
extern PDKC_NODESTATE k2hdkc_ex_get_direct_state(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, size_t* pstatecount);
extern bool k2hdkc_pm_get_state(k2hdkc_chmpx_h handle, PDKC_NODESTATE* ppstates, size_t* pstatecount);
extern PDKC_NODESTATE k2hdkc_pm_get_direct_state(k2hdkc_chmpx_h handle, size_t* pstatecount);

//---------------------------------------------------------
// Functions - Get Value
//---------------------------------------------------------
// [get value]
// 
// k2hdkc_get_value						get allocated binary value by key(use one-time slave command object)
// k2hdkc_get_direct_value				
// k2hdkc_get_str_value					get allocated string value by string key(use one-time slave command object)
// k2hdkc_get_str_direct_value			
// 
// k2hdkc_get_value_wp					get allocated binary value by key with manually decrypting(use one-time slave command object)
// k2hdkc_get_direct_value_wp			
// k2hdkc_get_str_value_wp				get allocated string value by string key with manually decrypting(use one-time slave command object)
// k2hdkc_get_str_direct_value_wp		
// 
// k2hdkc_get_value_np					no attribute protect, get allocated binary value by key(use one-time slave command object)
// k2hdkc_get_direct_value_np			
// k2hdkc_get_str_value_np				no attribute protect, get allocated string value by string key(use one-time slave command object)
// k2hdkc_get_str_direct_value_np		
// 
// k2hdkc_ex_get_value					get allocated binary value by key(use one-time slave command object, can specify chmpx parameters)
// k2hdkc_ex_get_direct_value			
// k2hdkc_ex_get_str_value				get allocated string value by string key(use one-time slave command object, can specify chmpx parameters)
// k2hdkc_ex_get_str_direct_value		
// 
// k2hdkc_ex_get_value_wp				get allocated binary value by key with manually decrypting(use one-time slave command object, can specify chmpx parameters)
// k2hdkc_ex_get_direct_value_wp		
// k2hdkc_ex_get_str_value_wp			get allocated string value by string key with manually decrypting(use one-time slave command object, can specify chmpx parameters)
// k2hdkc_ex_get_str_direct_value_wp	
// 
// k2hdkc_ex_get_value_np				no attribute protect, get allocated binary value by key(use one-time slave command object, can specify chmpx parameters)
// k2hdkc_ex_get_direct_value_np		
// k2hdkc_ex_get_str_value_np			no attribute protect, get allocated string value by string key(use one-time slave command object, can specify chmpx parameters)
// k2hdkc_ex_get_str_direct_value_np	
// 
// k2hdkc_pm_get_value					get allocated binary value by key(use permanent slave command object)
// k2hdkc_pm_get_direct_value			
// k2hdkc_pm_get_str_value				get allocated string value by string key(use permanent slave command object)
// k2hdkc_pm_get_str_direct_value		
// 
// k2hdkc_pm_get_value_wp				get allocated binary value by key with manually decrypting(use permanent slave command object)
// k2hdkc_pm_get_direct_value_wp		
// k2hdkc_pm_get_str_value_wp			get allocated string value by string key with manually decrypting(use permanent slave command object)
// k2hdkc_pm_get_str_direct_value_wp	
// 
// k2hdkc_pm_get_value_np				no attribute protect, get allocated binary value by key(use permanent slave command object)
// k2hdkc_pm_get_direct_value_np		
// k2hdkc_pm_get_str_value_np			no attribute protect, get allocated string value by string key(use permanent slave command object)
// k2hdkc_pm_get_str_direct_value_np	
// 
extern bool k2hdkc_get_value(const char* config, const unsigned char* pkey, size_t keylength, unsigned char** ppval, size_t* pvallength);
extern unsigned char* k2hdkc_get_direct_value(const char* config, const unsigned char* pkey, size_t keylength, size_t* pvallength);
extern bool k2hdkc_get_str_value(const char* config, const char* pkey, char** ppval);
extern char* k2hdkc_get_str_direct_value(const char* config, const char* pkey);

extern bool k2hdkc_get_value_wp(const char* config, const unsigned char* pkey, size_t keylength, const char* encpass, unsigned char** ppval, size_t* pvallength);
extern unsigned char* k2hdkc_get_direct_value_wp(const char* config, const unsigned char* pkey, size_t keylength, const char* encpass, size_t* pvallength);
extern bool k2hdkc_get_str_value_wp(const char* config, const char* pkey, const char* encpass, char** ppval);
extern char* k2hdkc_get_str_direct_value_wp(const char* config, const char* pkey, const char* encpass);

extern bool k2hdkc_get_value_np(const char* config, const unsigned char* pkey, size_t keylength, unsigned char** ppval, size_t* pvallength);
extern unsigned char* k2hdkc_get_direct_value_np(const char* config, const unsigned char* pkey, size_t keylength, size_t* pvallength);
extern bool k2hdkc_get_str_value_np(const char* config, const char* pkey, char** ppval);
extern char* k2hdkc_get_str_direct_value_np(const char* config, const char* pkey);

extern bool k2hdkc_ex_get_value(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, unsigned char** ppval, size_t* pvallength);
extern unsigned char* k2hdkc_ex_get_direct_value(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, size_t* pvallength);
extern bool k2hdkc_ex_get_str_value(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, char** ppval);
extern char* k2hdkc_ex_get_str_direct_value(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey);

extern bool k2hdkc_ex_get_value_wp(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, const char* encpass, unsigned char** ppval, size_t* pvallength);
extern unsigned char* k2hdkc_ex_get_direct_value_wp(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, const char* encpass, size_t* pvallength);
extern bool k2hdkc_ex_get_str_value_wp(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, const char* encpass, char** ppval);
extern char* k2hdkc_ex_get_str_direct_value_wp(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, const char* encpass);

extern bool k2hdkc_ex_get_value_np(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, unsigned char** ppval, size_t* pvallength);
extern unsigned char* k2hdkc_ex_get_direct_value_np(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, size_t* pvallength);
extern bool k2hdkc_ex_get_str_value_np(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, char** ppval);
extern char* k2hdkc_ex_get_str_direct_value_np(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey);

extern bool k2hdkc_pm_get_value(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, unsigned char** ppval, size_t* pvallength);
extern unsigned char* k2hdkc_pm_get_direct_value(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, size_t* pvallength);
extern bool k2hdkc_pm_get_str_value(k2hdkc_chmpx_h handle, const char* pkey, char** ppval);
extern char* k2hdkc_pm_get_str_direct_value(k2hdkc_chmpx_h handle, const char* pkey);

extern bool k2hdkc_pm_get_value_wp(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, const char* encpass, unsigned char** ppval, size_t* pvallength);
extern unsigned char* k2hdkc_pm_get_direct_value_wp(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, const char* encpass, size_t* pvallength);
extern bool k2hdkc_pm_get_str_value_wp(k2hdkc_chmpx_h handle, const char* pkey, const char* encpass, char** ppval);
extern char* k2hdkc_pm_get_str_direct_value_wp(k2hdkc_chmpx_h handle, const char* pkey, const char* encpass);

extern bool k2hdkc_pm_get_value_np(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, unsigned char** ppval, size_t* pvallength);
extern unsigned char* k2hdkc_pm_get_direct_value_np(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, size_t* pvallength);
extern bool k2hdkc_pm_get_str_value_np(k2hdkc_chmpx_h handle, const char* pkey, char** ppval);
extern char* k2hdkc_pm_get_str_direct_value_np(k2hdkc_chmpx_h handle, const char* pkey);

//---------------------------------------------------------
// Functions - Get Value(direct access)
//---------------------------------------------------------
// [get value(direct access)]
// 
// k2hdkc_da_get_value					get allocated binary value from offset by key(use one-time slave command object)
// k2hdkc_da_get_direct_value			
// k2hdkc_da_get_str_value				get allocated string value from offset  by string key(use one-time slave command object)
// k2hdkc_da_get_str_direct_value		
// 
// k2hdkc_ex_da_get_value				get allocated binary value from offset by key(use one-time slave command object, can specify chmpx parameters)
// k2hdkc_ex_da_get_direct_value		
// k2hdkc_ex_da_get_str_value			get allocated string value from offset by string key(use one-time slave command object, can specify chmpx parameters)
// k2hdkc_ex_da_get_str_direct_value	
// 
// k2hdkc_pm_da_get_value				get allocated binary value from offset by key(use permanent slave command object)
// k2hdkc_pm_da_get_direct_value		
// k2hdkc_pm_da_get_str_value			get allocated string value from offset by string key(use permanent slave command object)
// k2hdkc_pm_da_get_str_direct_value	
// 
extern bool k2hdkc_da_get_value(const char* config, const unsigned char* pkey, size_t keylength, off_t getpos, size_t val_length, unsigned char** ppval, size_t* pvallength);
extern unsigned char* k2hdkc_da_get_direct_value(const char* config, const unsigned char* pkey, size_t keylength, off_t getpos, size_t val_length, size_t* pvallength);
extern bool k2hdkc_da_get_str_value(const char* config, const char* pkey, off_t getpos, size_t val_length, char** ppval);
extern char* k2hdkc_da_get_str_direct_value(const char* config, const char* pkey, off_t getpos, size_t val_length);

extern bool k2hdkc_ex_da_get_value(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, off_t getpos, size_t val_length, unsigned char** ppval, size_t* pvallength);
extern unsigned char* k2hdkc_ex_da_get_direct_value(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, off_t getpos, size_t val_length, size_t* pvallength);
extern bool k2hdkc_ex_da_get_str_value(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, off_t getpos, size_t val_length, char** ppval);
extern char* k2hdkc_ex_da_get_str_direct_value(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, off_t getpos, size_t val_length);

extern bool k2hdkc_pm_da_get_value(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, off_t getpos, size_t val_length, unsigned char** ppval, size_t* pvallength);
extern unsigned char* k2hdkc_pm_da_get_direct_value(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, off_t getpos, size_t val_length, size_t* pvallength);
extern bool k2hdkc_pm_da_get_str_value(k2hdkc_chmpx_h handle, const char* pkey, off_t getpos, size_t val_length, char** ppval);
extern char* k2hdkc_pm_da_get_str_direct_value(k2hdkc_chmpx_h handle, const char* pkey, off_t getpos, size_t val_length);

//---------------------------------------------------------
// Functions - Get Subkeys
//---------------------------------------------------------
// [get subkeys]
// 
// k2hdkc_get_subkeys					get allocated binary subkeys list packed K2HDKCKEYPCK(use one-time slave command object)
// k2hdkc_get_direct_subkeys			
// k2hdkc_get_str_subkeys				get allocated string subkeys list which is null terminated string array(use one-time slave command object)
// k2hdkc_get_str_direct_subkeys		
// 
// k2hdkc_get_subkeys_np				no attribute protect, get allocated binary subkeys list packed K2HDKCKEYPCK(use one-time slave command object)
// k2hdkc_get_direct_subkeys_np			
// k2hdkc_get_str_subkeys_np			no attribute protect, get allocated string subkeys list which is null terminated string array(use one-time slave command object)
// k2hdkc_get_str_direct_subkeys_np		
// 
// k2hdkc_ex_get_subkeys				get allocated binary subkeys list packed K2HDKCKEYPCK(use one-time slave command object, can specify chmpx parameters)
// k2hdkc_ex_get_direct_subkeys			
// k2hdkc_ex_get_str_subkeys			get allocated string subkeys list which is null terminated string array(use one-time slave command object, can specify chmpx parameters)
// k2hdkc_ex_get_str_direct_subkeys		
// 
// k2hdkc_ex_get_subkeys_np				no attribute protect, get allocated binary subkeys list packed K2HDKCKEYPCK(use one-time slave command object, can specify chmpx parameters)
// k2hdkc_ex_get_direct_subkeys_np		
// k2hdkc_ex_get_str_subkeys_np			no attribute protect, get allocated string subkeys list which is null terminated string array(use one-time slave command object, can specify chmpx parameters)
// k2hdkc_ex_get_str_direct_subkeys_np	
// 
// k2hdkc_pm_get_subkeys				get allocated binary subkeys list packed K2HDKCKEYPCK(use permanent slave command object)
// k2hdkc_pm_get_direct_subkeys			
// k2hdkc_pm_get_str_subkeys			get allocated string subkeys list which is null terminated string array(use permanent slave command object)
// k2hdkc_pm_get_str_direct_subkeys		
// 
// k2hdkc_pm_get_subkeys_np				no attribute protect, get allocated binary subkeys list packed K2HDKCKEYPCK(use permanent slave command object)
// k2hdkc_pm_get_direct_subkeys_np		
// k2hdkc_pm_get_str_subkeys_np			no attribute protect, get allocated string subkeys list which is null terminated string array(use permanent slave command object)
// k2hdkc_pm_get_str_direct_subkeys_np	
// 
extern bool k2hdkc_get_subkeys(const char* config, const unsigned char* pkey, size_t keylength, PK2HDKCKEYPCK* ppskeypck, int* pskeypckcnt);
extern PK2HDKCKEYPCK k2hdkc_get_direct_subkeys(const char* config, const unsigned char* pkey, size_t keylength, int* pskeypckcnt);
extern int k2hdkc_get_str_subkeys(const char* config, const char* pkey, char*** ppskeyarray);
extern char** k2hdkc_get_str_direct_subkeys(const char* config, const char* pkey);

extern bool k2hdkc_get_subkeys_np(const char* config, const unsigned char* pkey, size_t keylength, PK2HDKCKEYPCK* ppskeypck, int* pskeypckcnt);
extern PK2HDKCKEYPCK k2hdkc_get_direct_subkeys_np(const char* config, const unsigned char* pkey, size_t keylength, int* pskeypckcnt);
extern int k2hdkc_get_str_subkeys_np(const char* config, const char* pkey, char*** ppskeyarray);
extern char** k2hdkc_get_str_direct_subkeys_np(const char* config, const char* pkey);

extern bool k2hdkc_ex_get_subkeys(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, PK2HDKCKEYPCK* ppskeypck, int* pskeypckcnt);
extern PK2HDKCKEYPCK k2hdkc_ex_get_direct_subkeys(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, int* pskeypckcnt);
extern int k2hdkc_ex_get_str_subkeys(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, char*** ppskeyarray);
extern char** k2hdkc_ex_get_str_direct_subkeys(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey);

extern bool k2hdkc_ex_get_subkeys_np(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, PK2HDKCKEYPCK* ppskeypck, int* pskeypckcnt);
extern PK2HDKCKEYPCK k2hdkc_ex_get_direct_subkeys_np(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, int* pskeypckcnt);
extern int k2hdkc_ex_get_str_subkeys_np(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, char*** ppskeyarray);
extern char** k2hdkc_ex_get_str_direct_subkeys_np(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey);

extern bool k2hdkc_pm_get_subkeys(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, PK2HDKCKEYPCK* ppskeypck, int* pskeypckcnt);
extern PK2HDKCKEYPCK k2hdkc_pm_get_direct_subkeys(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, int* pskeypckcnt);
extern int k2hdkc_pm_get_str_subkeys(k2hdkc_chmpx_h handle, const char* pkey, char*** ppskeyarray);
extern char** k2hdkc_pm_get_str_direct_subkeys(k2hdkc_chmpx_h handle, const char* pkey);

extern bool k2hdkc_pm_get_subkeys_np(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, PK2HDKCKEYPCK* ppskeypck, int* pskeypckcnt);
extern PK2HDKCKEYPCK k2hdkc_pm_get_direct_subkeys_np(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, int* pskeypckcnt);
extern int k2hdkc_pm_get_str_subkeys_np(k2hdkc_chmpx_h handle, const char* pkey, char*** ppskeyarray);
extern char** k2hdkc_pm_get_str_direct_subkeys_np(k2hdkc_chmpx_h handle, const char* pkey);

//---------------------------------------------------------
// Functions - Get Attributes
//---------------------------------------------------------
// [get attributes]
// 
// k2hdkc_get_attrs						get allocated binary attribute(key and value) list packed K2HDKCATTRPCK(use one-time slave command object)
// k2hdkc_get_direct_attrs				
// k2hdkc_get_str_direct_attrs			
// 
// k2hdkc_ex_get_attrs					get allocated binary attribute(key and value) list packed K2HDKCATTRPCK(use one-time slave command object, can specify chmpx parameters)
// k2hdkc_ex_get_direct_attrs			
// k2hdkc_ex_get_str_direct_attrs		
// 
// k2hdkc_pm_get_attrs					get allocated binary attribute(key and value) list packed K2HDKCATTRPCK(use permanent slave command object)
// k2hdkc_pm_get_direct_attrs			
// k2hdkc_pm_get_str_direct_attrs		
// 
extern bool k2hdkc_get_attrs(const char* config, const unsigned char* pkey, size_t keylength, PK2HDKCATTRPCK* ppattrspck, int* pattrspckcnt);
extern PK2HDKCATTRPCK k2hdkc_get_direct_attrs(const char* config, const unsigned char* pkey, size_t keylength, int* pattrspckcnt);
extern PK2HDKCATTRPCK k2hdkc_get_str_direct_attrs(const char* config, const char* pkey, int* pattrspckcnt);

extern bool k2hdkc_ex_get_attrs(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, PK2HDKCATTRPCK* ppattrspck, int* pattrspckcnt);
extern PK2HDKCATTRPCK k2hdkc_ex_get_direct_attrs(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, int* pattrspckcnt);
extern PK2HDKCATTRPCK k2hdkc_ex_get_str_direct_attrs(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, int* pattrspckcnt);

extern bool k2hdkc_pm_get_attrs(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, PK2HDKCATTRPCK* ppattrspck, int* pattrspckcnt);
extern PK2HDKCATTRPCK k2hdkc_pm_get_direct_attrs(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, int* pattrspckcnt);
extern PK2HDKCATTRPCK k2hdkc_pm_get_str_direct_attrs(k2hdkc_chmpx_h handle, const char* pkey, int* pattrspckcnt);

//---------------------------------------------------------
// Functions - Set Value
//---------------------------------------------------------
// [set value]
// 
// k2hdkc_set_value						set binary value(use one-time slave command object)
// k2hdkc_set_str_value					set string value(use one-time slave command object)
// 
// k2hdkc_ex_set_value					set binary value(use one-time slave command object, can specify chmpx parameters)
// k2hdkc_ex_set_str_value				set string value(use one-time slave command object, can specify chmpx parameters)
// 
// k2hdkc_pm_set_value					set binary value(use permanent slave command object)
// k2hdkc_pm_set_str_value				set string value(use permanent slave command object)
// 
// k2hdkc_set_value_wa					set binary value with attributes(use one-time slave command object)
// k2hdkc_set_str_value_wa				set string value with attributes(use one-time slave command object)
// 
// k2hdkc_ex_set_value_wa				set binary value with attributes(use one-time slave command object, can specify chmpx parameters)
// k2hdkc_ex_set_str_value_wa			set string value with attributes(use one-time slave command object, can specify chmpx parameters)
// 
// k2hdkc_pm_set_value_wa				set binary value with attributes(use permanent slave command object)
// k2hdkc_pm_set_str_value_wa			set string value with attributes(use permanent slave command object)
// 
extern bool k2hdkc_set_value(const char* config, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength);
extern bool k2hdkc_set_str_value(const char* config, const char* pkey, const char* pval);

extern bool k2hdkc_ex_set_value(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength);
extern bool k2hdkc_ex_set_str_value(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, const char* pval);

extern bool k2hdkc_pm_set_value(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength);
extern bool k2hdkc_pm_set_str_value(k2hdkc_chmpx_h handle, const char* pkey, const char* pval);

extern bool k2hdkc_set_value_wa(const char* config, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, bool rmsubkeylist, const char* encpass, const time_t* expire);
extern bool k2hdkc_set_str_value_wa(const char* config, const char* pkey, const char* pval, bool rmsubkeylist, const char* encpass, const time_t* expire);

extern bool k2hdkc_ex_set_value_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, bool rmsubkeylist, const char* encpass, const time_t* expire);
extern bool k2hdkc_ex_set_str_value_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, const char* pval, bool rmsubkeylist, const char* encpass, const time_t* expire);

extern bool k2hdkc_pm_set_value_wa(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, bool rmsubkeylist, const char* encpass, const time_t* expire);
extern bool k2hdkc_pm_set_str_value_wa(k2hdkc_chmpx_h handle, const char* pkey, const char* pval, bool rmsubkeylist, const char* encpass, const time_t* expire);

//---------------------------------------------------------
// Functions - Set Value(direct access)
//---------------------------------------------------------
// [set value(direct access)]
// 
// k2hdkc_da_set_value					set binary value from offset(use one-time slave command object)
// k2hdkc_da_set_str_value				set string value from offset(use one-time slave command object)
// 
// k2hdkc_ex_da_set_value				set binary value from offset(use one-time slave command object, can specify chmpx parameters)
// k2hdkc_ex_da_set_str_value			set string value from offset(use one-time slave command object, can specify chmpx parameters)
// 
// k2hdkc_pm_da_set_value				set binary value from offset(use permanent slave command object)
// k2hdkc_pm_da_set_str_value			set string value from offset(use permanent slave command object)
// 
extern bool k2hdkc_da_set_value(const char* config, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, const off_t setpos);
extern bool k2hdkc_da_set_str_value(const char* config, const char* pkey, const char* pval, const off_t setpos);

extern bool k2hdkc_ex_da_set_value(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, const off_t setpos);
extern bool k2hdkc_ex_da_set_str_value(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, const char* pval, const off_t setpos);

extern bool k2hdkc_pm_da_set_value(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, const off_t setpos);
extern bool k2hdkc_pm_da_set_str_value(k2hdkc_chmpx_h handle, const char* pkey, const char* pval, const off_t setpos);

//---------------------------------------------------------
// Functions - Set/Clear Subkeys
//---------------------------------------------------------
// [set/clear subkeys]
// 
// k2hdkc_set_subkeys					set subkeys by K2HDKCKEYPCK(use one-time slave command object)
// k2hdkc_set_str_subkeys				set subkeys by string array(use one-time slave command object)
// 
// k2hdkc_ex_set_subkeys				set subkeys by K2HDKCKEYPCK(use one-time slave command object, can specify chmpx parameters)
// k2hdkc_ex_set_str_subkeys			set subkeys by string array(use one-time slave command object, can specify chmpx parameters)
// 
// k2hdkc_pm_set_subkeys				set subkeys by K2HDKCKEYPCK(use permanent slave command object)
// k2hdkc_pm_set_str_subkeys			set subkeys by string array(use permanent slave command object)
// 
// k2hdkc_clear_subkeys					clear subkeys(use one-time slave command object)
// k2hdkc_clear_str_subkeys				
// 
// k2hdkc_ex_clear_subkeys				clear subkeys(use one-time slave command object, can specify chmpx parameters)
// k2hdkc_ex_clear_str_subkeys			
// 
// k2hdkc_pm_clear_subkeys				clear subkeys(use permanent slave command object)
// k2hdkc_pm_clear_str_subkeys			
// 
extern bool k2hdkc_set_subkeys(const char* config, const unsigned char* pkey, size_t keylength, const PK2HDKCKEYPCK pskeypck, int skeypckcnt);
extern bool k2hdkc_set_str_subkeys(const char* config, const char* pkey, const char** pskeyarray);

extern bool k2hdkc_ex_set_subkeys(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, const PK2HDKCKEYPCK pskeypck, int skeypckcnt);
extern bool k2hdkc_ex_set_str_subkeys(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, const char** pskeyarray);

extern bool k2hdkc_pm_set_subkeys(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, const PK2HDKCKEYPCK pskeypck, int skeypckcnt);
extern bool k2hdkc_pm_set_str_subkeys(k2hdkc_chmpx_h handle, const char* pkey, const char** pskeyarray);

extern bool k2hdkc_clear_subkeys(const char* config, const unsigned char* pkey, size_t keylength);
extern bool k2hdkc_clear_str_subkeys(const char* config, const char* pkey);

extern bool k2hdkc_ex_clear_subkeys(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength);
extern bool k2hdkc_ex_clear_str_subkeys(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey);

extern bool k2hdkc_pm_clear_subkeys(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength);
extern bool k2hdkc_pm_clear_str_subkeys(k2hdkc_chmpx_h handle, const char* pkey);

//---------------------------------------------------------
// Functions - Add Subkey
//---------------------------------------------------------
// [add subkey]
// 
// k2hdkc_set_subkey					set subkey with value into key(use one-time slave command object)
// k2hdkc_set_str_subkey				
// 
// k2hdkc_ex_set_subkey					set subkey with value into key(use one-time slave command object, can specify chmpx parameters)
// k2hdkc_ex_set_str_subkey				
// 
// k2hdkc_pm_set_subkey					set subkey with value into key(use permanent slave command object)
// k2hdkc_pm_set_str_subkey				
// 
// k2hdkc_set_subkey_wa					set subkey with value into key with checking/adding attributes(use one-time slave command object)
// k2hdkc_set_str_subkey_wa				
// 
// k2hdkc_ex_set_subkey_wa				set subkey with value into key with checking/adding attributes(use one-time slave command object, can specify chmpx parameters)
// k2hdkc_ex_set_str_subkey_wa			
// 
// k2hdkc_pm_set_subkey_wa				set subkey with value into key with checking/adding attributes(use permanent slave command object)
// k2hdkc_pm_set_str_subkey_wa			
// 
extern bool k2hdkc_set_subkey(const char* config, const unsigned char* pkey, size_t keylength, const unsigned char* psubkey, size_t subkeylength, const unsigned char* pskeyval, size_t skeyvallength);
extern bool k2hdkc_set_str_subkey(const char* config, const char* pkey, const char* psubkey, const char* pskeyval);

extern bool k2hdkc_ex_set_subkey(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, const unsigned char* psubkey, size_t subkeylength, const unsigned char* pskeyval, size_t skeyvallength);
extern bool k2hdkc_ex_set_str_subkey(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, const char* psubkey, const char* pskeyval);

extern bool k2hdkc_pm_set_subkey(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, const unsigned char* psubkey, size_t subkeylength, const unsigned char* pskeyval, size_t skeyvallength);
extern bool k2hdkc_pm_set_str_subkey(k2hdkc_chmpx_h handle, const char* pkey, const char* psubkey, const char* pskeyval);

extern bool k2hdkc_set_subkey_wa(const char* config, const unsigned char* pkey, size_t keylength, const unsigned char* psubkey, size_t subkeylength, const unsigned char* pskeyval, size_t skeyvallength, bool checkattr, const char* encpass, const time_t* expire);
extern bool k2hdkc_set_str_subkey_wa(const char* config, const char* pkey, const char* psubkey, const char* pskeyval, bool checkattr, const char* encpass, const time_t* expire);

extern bool k2hdkc_ex_set_subkey_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, const unsigned char* psubkey, size_t subkeylength, const unsigned char* pskeyval, size_t skeyvallength, bool checkattr, const char* encpass, const time_t* expire);
extern bool k2hdkc_ex_set_str_subkey_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, const char* psubkey, const char* pskeyval, bool checkattr, const char* encpass, const time_t* expire);

extern bool k2hdkc_pm_set_subkey_wa(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, const unsigned char* psubkey, size_t subkeylength, const unsigned char* pskeyval, size_t skeyvallength, bool checkattr, const char* encpass, const time_t* expire);
extern bool k2hdkc_pm_set_str_subkey_wa(k2hdkc_chmpx_h handle, const char* pkey, const char* psubkey, const char* pskeyval, bool checkattr, const char* encpass, const time_t* expire);

//---------------------------------------------------------
// Functions - Set All
//---------------------------------------------------------
// [add subkey]
// 
// k2hdkc_set_all						set key value and subkey without attributes(use one-time slave command object)
// k2hdkc_set_str_all					
// 
// k2hdkc_ex_set_all					set key value and subkey without attributes(use one-time slave command object, can specify chmpx parameters)
// k2hdkc_ex_set_str_all				
// 
// k2hdkc_pm_set_all					set key value and subkey without attributes(use permanent slave command object)
// k2hdkc_pm_set_str_all				
// 
// k2hdkc_set_all_wa					set key value and subkey with attributes(use one-time slave command object)
// k2hdkc_set_str_all_wa				
// 
// k2hdkc_ex_set_all_wa					set key value and subkey with attributes(use one-time slave command object, can specify chmpx parameters)
// k2hdkc_ex_set_str_all_wa				
// 
// k2hdkc_pm_set_all_wa					set key value and subkey with attributes(use permanent slave command object)
// k2hdkc_pm_set_str_all_wa				
// 
extern bool k2hdkc_set_all(const char* config, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, const PK2HDKCKEYPCK pskeypck, int skeypckcnt);
extern bool k2hdkc_set_str_all(const char* config, const char* pkey, const char* pval, const char** pskeyarray);

extern bool k2hdkc_ex_set_all(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, const PK2HDKCKEYPCK pskeypck, int skeypckcnt);
extern bool k2hdkc_ex_set_str_all(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, const char* pval, const char** pskeyarray);

extern bool k2hdkc_pm_set_all(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, const PK2HDKCKEYPCK pskeypck, int skeypckcnt);
extern bool k2hdkc_pm_set_str_all(k2hdkc_chmpx_h handle, const char* pkey, const char* pval, const char** pskeyarray);

extern bool k2hdkc_set_all_wa(const char* config, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, const PK2HDKCKEYPCK pskeypck, int skeypckcnt, const char* encpass, const time_t* expire);
extern bool k2hdkc_set_str_all_wa(const char* config, const char* pkey, const char* pval, const char** pskeyarray, const char* encpass, const time_t* expire);

extern bool k2hdkc_ex_set_all_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, const PK2HDKCKEYPCK pskeypck, int skeypckcnt, const char* encpass, const time_t* expire);
extern bool k2hdkc_ex_set_str_all_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, const char* pval, const char** pskeyarray, const char* encpass, const time_t* expire);

extern bool k2hdkc_pm_set_all_wa(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, const PK2HDKCKEYPCK pskeypck, int skeypckcnt, const char* encpass, const time_t* expire);
extern bool k2hdkc_pm_set_str_all_wa(k2hdkc_chmpx_h handle, const char* pkey, const char* pval, const char** pskeyarray, const char* encpass, const time_t* expire);

//---------------------------------------------------------
// Functions - Remove All
//---------------------------------------------------------
// [remove key]
// 
// k2hdkc_remove_all					remove key with all subkeys(use one-time slave command object)
// k2hdkc_remove_str_all				
// 
// k2hdkc_ex_remove_all					remove key with all subkeys(use one-time slave command object, can specify chmpx parameters)
// k2hdkc_ex_remove_str_all				
// 
// k2hdkc_pm_remove_all					remove key with all subkeys(use permanent slave command object)
// k2hdkc_pm_remove_str_all				
// 
// k2hdkc_remove						remove key without subkeys(use one-time slave command object)
// k2hdkc_remove_str					
// 
// k2hdkc_ex_remove						remove key without subkeys(use one-time slave command object, can specify chmpx parameters)
// k2hdkc_ex_remove_str					
// 
// k2hdkc_pm_remove						remove key without subkeys(use permanent slave command object)
// k2hdkc_pm_remove_str					
// 
extern bool k2hdkc_remove_all(const char* config, const unsigned char* pkey, size_t keylength);
extern bool k2hdkc_remove_str_all(const char* config, const char* pkey);

extern bool k2hdkc_ex_remove_all(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength);
extern bool k2hdkc_ex_remove_str_all(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey);

extern bool k2hdkc_pm_remove_all(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength);
extern bool k2hdkc_pm_remove_str_all(k2hdkc_chmpx_h handle, const char* pkey);

extern bool k2hdkc_remove(const char* config, const unsigned char* pkey, size_t keylength);
extern bool k2hdkc_remove_str(const char* config, const char* pkey);

extern bool k2hdkc_ex_remove(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength);
extern bool k2hdkc_ex_remove_str(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey);

extern bool k2hdkc_pm_remove(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength);
extern bool k2hdkc_pm_remove_str(k2hdkc_chmpx_h handle, const char* pkey);

//---------------------------------------------------------
// Functions - Remove Subkey
//---------------------------------------------------------
// [remove subkey]
// 
// k2hdkc_remove_subkey					remove subkey with checking attribute(use one-time slave command object)
// k2hdkc_remove_str_subkey				
// 
// k2hdkc_ex_remove_subkey				remove subkey with checking attribute(use one-time slave command object, can specify chmpx parameters)
// k2hdkc_ex_remove_str_subkey			
// 
// k2hdkc_pm_remove_subkey				remove subkey with checking attribute(use permanent slave command object)
// k2hdkc_pm_remove_str_subkey			
// 
// k2hdkc_remove_subkey_np				remove subkey without checking attribute(use one-time slave command object)
// k2hdkc_remove_str_subkey_np			
// 
// k2hdkc_ex_remove_subkey_np			remove subkey without checking attribute(use one-time slave command object, can specify chmpx parameters)
// k2hdkc_ex_remove_str_subkey_np		
// 
// k2hdkc_pm_remove_subkey_np			remove subkey without checking attribute(use permanent slave command object)
// k2hdkc_pm_remove_str_subkey_np		
// 
extern bool k2hdkc_remove_subkey(const char* config, const unsigned char* pkey, size_t keylength, const unsigned char* psubkey, size_t subkeylength, bool is_nest);
extern bool k2hdkc_remove_str_subkey(const char* config, const char* pkey, const char* psubkey, bool is_nest);

extern bool k2hdkc_ex_remove_subkey(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, const unsigned char* psubkey, size_t subkeylength, bool is_nest);
extern bool k2hdkc_ex_remove_str_subkey(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, const char* psubkey, bool is_nest);

extern bool k2hdkc_pm_remove_subkey(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, const unsigned char* psubkey, size_t subkeylength, bool is_nest);
extern bool k2hdkc_pm_remove_str_subkey(k2hdkc_chmpx_h handle, const char* pkey, const char* psubkey, size_t subkeylength, bool is_nest);

extern bool k2hdkc_remove_subkey_np(const char* config, const unsigned char* pkey, size_t keylength, const unsigned char* psubkey, size_t subkeylength, bool is_nest);
extern bool k2hdkc_remove_str_subkey_np(const char* config, const char* pkey, const char* psubkey, bool is_nest);

extern bool k2hdkc_ex_remove_subkey_np(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, const unsigned char* psubkey, size_t subkeylength, bool is_nest);
extern bool k2hdkc_ex_remove_str_subkey_np(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, const char* psubkey, bool is_nest);

extern bool k2hdkc_pm_remove_subkey_np(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, const unsigned char* psubkey, size_t subkeylength, bool is_nest);
extern bool k2hdkc_pm_remove_str_subkey_np(k2hdkc_chmpx_h handle, const char* pkey, const char* psubkey, size_t subkeylength, bool is_nest);

//---------------------------------------------------------
// Functions - Rename key
//---------------------------------------------------------
// [remove key]
// 
// k2hdkc_rename						remove key with checking attribute(use one-time slave command object)
// k2hdkc_rename_str					
// 
// k2hdkc_ex_rename						remove key with checking attribute(use one-time slave command object, can specify chmpx parameters)
// k2hdkc_ex_rename_str					
// 
// k2hdkc_pm_rename						remove key with checking attribute(use permanent slave command object)
// k2hdkc_pm_rename_str					
// 
// k2hdkc_rename_wa						remove key with checking attribute and with setting new attribute(use one-time slave command object)
// k2hdkc_rename_str_wa					
// 
// k2hdkc_ex_rename_wa					remove key with checking attribute and with setting new attribute(use one-time slave command object, can specify chmpx parameters)
// k2hdkc_ex_rename_str_wa				
// 
// k2hdkc_pm_rename_wa					remove key with checking attribute and with setting new attribute(use permanent slave command object)
// k2hdkc_pm_rename_str_wa				
// 
// k2hdkc_rename_with_parent			remove key with checking attribute, and specify parent key(use one-time slave command object)
// k2hdkc_rename_with_parent_str		
// 
// k2hdkc_ex_rename_with_parent			remove key with checking attribute, and specify parent key(use one-time slave command object, can specify chmpx parameters)
// k2hdkc_ex_rename_with_parent_str		
// 
// k2hdkc_pm_rename_with_parent			remove key with checking attribute, and specify parent key(use permanent slave command object)
// k2hdkc_pm_rename_with_parent_str		
// 
// k2hdkc_rename_with_parent_wa			remove key with checking attribute and with setting new attribute, and specify parent key(use one-time slave command object)
// k2hdkc_rename_with_parent_str_wa		
// 
// k2hdkc_ex_rename_with_parent_wa		remove key with checking attribute and with setting new attribute, and specify parent key(use one-time slave command object, can specify chmpx parameters)
// k2hdkc_ex_rename_with_parent_str_wa	
// 
// k2hdkc_pm_rename_with_parent_wa		remove key with checking attribute and with setting new attribute, and specify parent key(use permanent slave command object)
// k2hdkc_pm_rename_with_parent_str_wa	
// 
extern bool k2hdkc_rename(const char* config, const unsigned char* poldkey, size_t oldkeylength, const unsigned char* pnewkey, size_t newkeylength);
extern bool k2hdkc_rename_str(const char* config, const char* poldkey, const char* pnewkey);

extern bool k2hdkc_ex_rename(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* poldkey, size_t oldkeylength, const unsigned char* pnewkey, size_t newkeylength);
extern bool k2hdkc_ex_rename_str(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* poldkey, const char* pnewkey);

extern bool k2hdkc_pm_rename(k2hdkc_chmpx_h handle, const unsigned char* poldkey, size_t oldkeylength, const unsigned char* pnewkey, size_t newkeylength);
extern bool k2hdkc_pm_rename_str(k2hdkc_chmpx_h handle, const char* poldkey, const char* pnewkey);

extern bool k2hdkc_rename_wa(const char* config, const unsigned char* poldkey, size_t oldkeylength, const unsigned char* pnewkey, size_t newkeylength, bool checkattr, const char* encpass, const time_t* expire);
extern bool k2hdkc_rename_str_wa(const char* config, const char* poldkey, const char* pnewkey, bool checkattr, const char* encpass, const time_t* expire);

extern bool k2hdkc_ex_rename_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* poldkey, size_t oldkeylength, const unsigned char* pnewkey, size_t newkeylength, bool checkattr, const char* encpass, const time_t* expire);
extern bool k2hdkc_ex_rename_str_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* poldkey, const char* pnewkey, bool checkattr, const char* encpass, const time_t* expire);

extern bool k2hdkc_pm_rename_wa(k2hdkc_chmpx_h handle, const unsigned char* poldkey, size_t oldkeylength, const unsigned char* pnewkey, size_t newkeylength, bool checkattr, const char* encpass, const time_t* expire);
extern bool k2hdkc_pm_rename_str_wa(k2hdkc_chmpx_h handle, const char* poldkey, const char* pnewkey, bool checkattr, const char* encpass, const time_t* expire);

extern bool k2hdkc_rename_with_parent(const char* config, const unsigned char* poldkey, size_t oldkeylength, const unsigned char* pnewkey, size_t newkeylength, const unsigned char* pparentkey, size_t parentkeylength);
extern bool k2hdkc_rename_with_parent_str(const char* config, const char* poldkey, const char* pnewkey, const char* pparentkey);

extern bool k2hdkc_ex_rename_with_parent(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* poldkey, size_t oldkeylength, const unsigned char* pnewkey, size_t newkeylength, const unsigned char* pparentkey, size_t parentkeylength);
extern bool k2hdkc_ex_rename_with_parent_str(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* poldkey, const char* pnewkey, const char* pparentkey);

extern bool k2hdkc_pm_rename_with_parent(k2hdkc_chmpx_h handle, const unsigned char* poldkey, size_t oldkeylength, const unsigned char* pnewkey, size_t newkeylength, const unsigned char* pparentkey, size_t parentkeylength);
extern bool k2hdkc_pm_rename_with_parent_str(k2hdkc_chmpx_h handle, const char* poldkey, const char* pnewkey, const char* pparentkey);

extern bool k2hdkc_rename_with_parent_wa(const char* config, const unsigned char* poldkey, size_t oldkeylength, const unsigned char* pnewkey, size_t newkeylength, const unsigned char* pparentkey, size_t parentkeylength, bool checkattr, const char* encpass, const time_t* expire);
extern bool k2hdkc_rename_with_parent_str_wa(const char* config, const char* poldkey, const char* pnewkey, const char* pparentkey, bool checkattr, const char* encpass, const time_t* expire);

extern bool k2hdkc_ex_rename_with_parent_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* poldkey, size_t oldkeylength, const unsigned char* pnewkey, size_t newkeylength, const unsigned char* pparentkey, size_t parentkeylength, bool checkattr, const char* encpass, const time_t* expire);
extern bool k2hdkc_ex_rename_with_parent_str_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* poldkey, const char* pnewkey, const char* pparentkey, bool checkattr, const char* encpass, const time_t* expire);

extern bool k2hdkc_pm_rename_with_parent_wa(k2hdkc_chmpx_h handle, const unsigned char* poldkey, size_t oldkeylength, const unsigned char* pnewkey, size_t newkeylength, const unsigned char* pparentkey, size_t parentkeylength, bool checkattr, const char* encpass, const time_t* expire);
extern bool k2hdkc_pm_rename_with_parent_str_wa(k2hdkc_chmpx_h handle, const char* poldkey, const char* pnewkey, const char* pparentkey, bool checkattr, const char* encpass, const time_t* expire);

//---------------------------------------------------------
// Functions - Queue
//---------------------------------------------------------
// [queue push]
// 
// k2hdkc_q_push						push queue(use one-time slave command object)
// k2hdkc_q_str_push					
// k2hdkc_ex_q_push						push queue(use one-time slave command object, can specify chmpx parameters)
// k2hdkc_ex_q_str_push					
// k2hdkc_pm_q_push						push queue(use permanent slave command object)
// k2hdkc_pm_q_str_push					
// k2hdkc_q_push_wa						push queue with attributes(use one-time slave command object)
// k2hdkc_q_str_push_wa					
// k2hdkc_ex_q_push_wa					push queue with attributes(use one-time slave command object, can specify chmpx parameters)
// k2hdkc_ex_q_str_push_wa				
// k2hdkc_pm_q_push_wa					push queue with attributes(use permanent slave command object)
// k2hdkc_pm_q_str_push_wa				
// k2hdkc_keyq_push						push key queue(use one-time slave command object)
// k2hdkc_keyq_str_push					
// k2hdkc_ex_keyq_push					push key queue(use one-time slave command object, can specify chmpx parameters)
// k2hdkc_ex_keyq_str_push				
// k2hdkc_pm_keyq_push					push key queue(use permanent slave command object)
// k2hdkc_pm_keyq_str_push				
// k2hdkc_keyq_push_wa					push key queue with attributes(use one-time slave command object)
// k2hdkc_keyq_str_push_wa				
// k2hdkc_ex_keyq_push_wa				push key queue with attributes(use one-time slave command object, can specify chmpx parameters)
// k2hdkc_ex_keyq_str_push_wa			
// k2hdkc_pm_keyq_push_wa				push key queue with attributes(use permanent slave command object)
// k2hdkc_pm_keyq_str_push_wa			
// 
extern bool k2hdkc_q_push(const char* config, const unsigned char* pprefix, size_t prefixlength, const unsigned char* pval, size_t vallength, bool is_fifo);
extern bool k2hdkc_q_str_push(const char* config, const char* pprefix, const char* pval, bool is_fifo);
extern bool k2hdkc_ex_q_push(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pprefix, size_t prefixlength, const unsigned char* pval, size_t vallength, bool is_fifo);
extern bool k2hdkc_ex_q_str_push(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pprefix, const char* pval, bool is_fifo);
extern bool k2hdkc_pm_q_push(k2hdkc_chmpx_h handle, const unsigned char* pprefix, size_t prefixlength, const unsigned char* pval, size_t vallength, bool is_fifo);
extern bool k2hdkc_pm_q_str_push(k2hdkc_chmpx_h handle, const char* pprefix, const char* pval, bool is_fifo);
extern bool k2hdkc_q_push_wa(const char* config, const unsigned char* pprefix, size_t prefixlength, const unsigned char* pval, size_t vallength, bool is_fifo, bool checkattr, const char* encpass, const time_t* expire);
extern bool k2hdkc_q_str_push_wa(const char* config, const char* pprefix, const char* pval, bool is_fifo, bool checkattr, const char* encpass, const time_t* expire);
extern bool k2hdkc_ex_q_push_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pprefix, size_t prefixlength, const unsigned char* pval, size_t vallength, bool is_fifo, bool checkattr, const char* encpass, const time_t* expire);
extern bool k2hdkc_ex_q_str_push_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pprefix, const char* pval, bool is_fifo, bool checkattr, const char* encpass, const time_t* expire);
extern bool k2hdkc_pm_q_push_wa(k2hdkc_chmpx_h handle, const unsigned char* pprefix, size_t prefixlength, const unsigned char* pval, size_t vallength, bool is_fifo, bool checkattr, const char* encpass, const time_t* expire);
extern bool k2hdkc_pm_q_str_push_wa(k2hdkc_chmpx_h handle, const char* pprefix, const char* pval, bool is_fifo, bool checkattr, const char* encpass, const time_t* expire);
extern bool k2hdkc_keyq_push(const char* config, const unsigned char* pprefix, size_t prefixlength, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, bool is_fifo);
extern bool k2hdkc_keyq_str_push(const char* config, const char* pprefix, const char* pkey, const char* pval, bool is_fifo);
extern bool k2hdkc_ex_keyq_push(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pprefix, size_t prefixlength, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, bool is_fifo);
extern bool k2hdkc_ex_keyq_str_push(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pprefix, const char* pkey, const char* pval, bool is_fifo);
extern bool k2hdkc_pm_keyq_push(k2hdkc_chmpx_h handle, const unsigned char* pprefix, size_t prefixlength, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, bool is_fifo);
extern bool k2hdkc_pm_keyq_str_push(k2hdkc_chmpx_h handle, const char* pprefix, const char* pkey, const char* pval, bool is_fifo);
extern bool k2hdkc_keyq_push_wa(const char* config, const unsigned char* pprefix, size_t prefixlength, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, bool is_fifo, bool checkattr, const char* encpass, const time_t* expire);
extern bool k2hdkc_keyq_str_push_wa(const char* config, const char* pprefix, const char* pkey, const char* pval, bool is_fifo, bool checkattr, const char* encpass, const time_t* expire);
extern bool k2hdkc_ex_keyq_push_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pprefix, size_t prefixlength, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, bool is_fifo, bool checkattr, const char* encpass, const time_t* expire);
extern bool k2hdkc_ex_keyq_str_push_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pprefix, const char* pkey, const char* pval, bool is_fifo, bool checkattr, const char* encpass, const time_t* expire);
extern bool k2hdkc_pm_keyq_push_wa(k2hdkc_chmpx_h handle, const unsigned char* pprefix, size_t prefixlength, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, bool is_fifo, bool checkattr, const char* encpass, const time_t* expire);
extern bool k2hdkc_pm_keyq_str_push_wa(k2hdkc_chmpx_h handle, const char* pprefix, const char* pkey, const char* pval, bool is_fifo, bool checkattr, const char* encpass, const time_t* expire);

// 
// [queue pop]
// 
// k2hdkc_q_pop							pop queue(use one-time slave command object)
// k2hdkc_q_str_pop						
// k2hdkc_ex_q_pop						pop queue(use one-time slave command object, can specify chmpx parameters)
// k2hdkc_ex_q_str_pop					
// k2hdkc_pm_q_pop						pop queue(use permanent slave command object)
// k2hdkc_pm_q_str_pop					
// k2hdkc_q_pop_wp						pop queue with passphrase(use one-time slave command object)
// k2hdkc_q_str_pop_wp					
// k2hdkc_ex_q_pop_wp					pop queue with passphrase(use one-time slave command object, can specify chmpx parameters)
// k2hdkc_ex_q_str_pop_wp				
// k2hdkc_pm_q_pop_wp					pop queue with passphrase(use permanent slave command object)
// k2hdkc_pm_q_str_pop_wp				
// k2hdkc_keyq_pop						pop key queue(use one-time slave command object)
// k2hdkc_keyq_str_pop					
// k2hdkc_ex_keyq_pop					pop key queue(use one-time slave command object, can specify chmpx parameters)
// k2hdkc_ex_keyq_str_pop				
// k2hdkc_pm_keyq_pop					pop key queue(use permanent slave command object)
// k2hdkc_pm_keyq_str_pop				
// k2hdkc_keyq_pop_wp					pop key queue with passphrase(use one-time slave command object)
// k2hdkc_keyq_str_pop_wp				
// k2hdkc_ex_keyq_pop_wp				pop key queue with passphrase(use one-time slave command object, can specify chmpx parameters)
// k2hdkc_ex_keyq_str_pop_wp			
// k2hdkc_pm_keyq_pop_wp				pop key queue with passphrase(use permanent slave command object)
// k2hdkc_pm_keyq_str_pop_wp			
// 
extern bool k2hdkc_q_pop(const char* config, const unsigned char* pprefix, size_t prefixlength, bool is_fifo, unsigned char** ppval, size_t* pvallength);
extern bool k2hdkc_q_str_pop(const char* config, const char* pprefix, bool is_fifo, const char** ppval);
extern bool k2hdkc_ex_q_pop(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pprefix, size_t prefixlength, bool is_fifo, unsigned char** ppval, size_t* pvallength);
extern bool k2hdkc_ex_q_str_pop(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pprefix, bool is_fifo, const char** ppval);
extern bool k2hdkc_pm_q_pop(k2hdkc_chmpx_h handle, const unsigned char* pprefix, size_t prefixlength, bool is_fifo, unsigned char** ppval, size_t* pvallength);
extern bool k2hdkc_pm_q_str_pop(k2hdkc_chmpx_h handle, const char* pprefix, bool is_fifo, const char** ppval);
extern bool k2hdkc_q_pop_wp(const char* config, const unsigned char* pprefix, size_t prefixlength, bool is_fifo, const char* encpass, unsigned char** ppval, size_t* pvallength);
extern bool k2hdkc_q_str_pop_wp(const char* config, const char* pprefix, bool is_fifo, const char* encpass, const char** ppval);
extern bool k2hdkc_ex_q_pop_wp(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pprefix, size_t prefixlength, bool is_fifo, const char* encpass, unsigned char** ppval, size_t* pvallength);
extern bool k2hdkc_ex_q_str_pop_wp(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pprefix, bool is_fifo, const char* encpass, const char** ppval);
extern bool k2hdkc_pm_q_pop_wp(k2hdkc_chmpx_h handle, const unsigned char* pprefix, size_t prefixlength, bool is_fifo, const char* encpass, unsigned char** ppval, size_t* pvallength);
extern bool k2hdkc_pm_q_str_pop_wp(k2hdkc_chmpx_h handle, const char* pprefix, bool is_fifo, const char* encpass, const char** ppval);
extern bool k2hdkc_keyq_pop(const char* config, const unsigned char* pprefix, size_t prefixlength, bool is_fifo, unsigned char** ppkey, size_t* pkeylength, unsigned char** ppval, size_t* pvallength);
extern bool k2hdkc_keyq_str_pop(const char* config, const char* pprefix, bool is_fifo, const char** ppkey, const char** ppval);
extern bool k2hdkc_ex_keyq_pop(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pprefix, size_t prefixlength, bool is_fifo, unsigned char** ppkey, size_t* pkeylength, unsigned char** ppval, size_t* pvallength);
extern bool k2hdkc_ex_keyq_str_pop(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pprefix, bool is_fifo, const char** ppkey, const char** ppval);
extern bool k2hdkc_pm_keyq_pop(k2hdkc_chmpx_h handle, const unsigned char* pprefix, size_t prefixlength, bool is_fifo, unsigned char** ppkey, size_t* pkeylength, unsigned char** ppval, size_t* pvallength);
extern bool k2hdkc_pm_keyq_str_pop(k2hdkc_chmpx_h handle, const char* pprefix, bool is_fifo, const char** ppkey, const char** ppval);
extern bool k2hdkc_keyq_pop_wp(const char* config, const unsigned char* pprefix, size_t prefixlength, bool is_fifo, const char* encpass, unsigned char** ppkey, size_t* pkeylength, unsigned char** ppval, size_t* pvallength);
extern bool k2hdkc_keyq_str_pop_wp(const char* config, const char* pprefix, bool is_fifo, const char* encpass, const char** ppkey, const char** ppval);
extern bool k2hdkc_ex_keyq_pop_wp(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pprefix, size_t prefixlength, bool is_fifo, const char* encpass, unsigned char** ppkey, size_t* pkeylength, unsigned char** ppval, size_t* pvallength);
extern bool k2hdkc_ex_keyq_str_pop_wp(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pprefix, bool is_fifo, const char* encpass, const char** ppkey, const char** ppval);
extern bool k2hdkc_pm_keyq_pop_wp(k2hdkc_chmpx_h handle, const unsigned char* pprefix, size_t prefixlength, bool is_fifo, const char* encpass, unsigned char** ppkey, size_t* pkeylength, unsigned char** ppval, size_t* pvallength);
extern bool k2hdkc_pm_keyq_str_pop_wp(k2hdkc_chmpx_h handle, const char* pprefix, bool is_fifo, const char* encpass, const char** ppkey, const char** ppval);

// 
// [queue remove]
// 
// k2hdkc_q_remove						remove queue(use one-time slave command object)
// k2hdkc_q_str_remove					
// k2hdkc_ex_q_remove					remove queue(use one-time slave command object, can specify chmpx parameters)
// k2hdkc_ex_q_str_remove				
// k2hdkc_pm_q_remove					remove queue(use permanent slave command object)
// k2hdkc_pm_q_str_remove				
// k2hdkc_q_remove_wp					remove queue with passphrase(use one-time slave command object)
// k2hdkc_q_str_remove_wp				
// k2hdkc_ex_q_remove_wp				remove queue with passphrase(use one-time slave command object, can specify chmpx parameters)
// k2hdkc_ex_q_str_remove_wp			
// k2hdkc_pm_q_remove_wp				remove queue with passphrase(use permanent slave command object)
// k2hdkc_pm_q_str_remove_wp			
// k2hdkc_keyq_remove					remove key queue(use one-time slave command object)
// k2hdkc_keyq_str_remove				
// k2hdkc_ex_keyq_remove				remove key queue(use one-time slave command object, can specify chmpx parameters)
// k2hdkc_ex_keyq_str_remove			
// k2hdkc_pm_keyq_remove				remove key queue(use permanent slave command object)
// k2hdkc_pm_keyq_str_remove			
// k2hdkc_keyq_remove_wp				remove key queue with passphrase(use one-time slave command object)
// k2hdkc_keyq_str_remove_wp			
// k2hdkc_ex_keyq_remove_wp				remove key queue with passphrase(use one-time slave command object, can specify chmpx parameters)
// k2hdkc_ex_keyq_str_remove_wp			
// k2hdkc_pm_keyq_remove_wp				remove key queue with passphrase(use permanent slave command object)
// k2hdkc_pm_keyq_str_remove_wp			
// 
extern bool k2hdkc_q_remove(const char* config, const unsigned char* pprefix, size_t prefixlength, int count, bool is_fifo);
extern bool k2hdkc_q_str_remove(const char* config, const char* pprefix, int count, bool is_fifo);
extern bool k2hdkc_ex_q_remove(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pprefix, size_t prefixlength, int count, bool is_fifo);
extern bool k2hdkc_ex_q_str_remove(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pprefix, int count, bool is_fifo);
extern bool k2hdkc_pm_q_remove(k2hdkc_chmpx_h handle, const unsigned char* pprefix, size_t prefixlength, int count, bool is_fifo);
extern bool k2hdkc_pm_q_str_remove(k2hdkc_chmpx_h handle, const char* pprefix, int count, bool is_fifo);
extern bool k2hdkc_q_remove_wp(const char* config, const unsigned char* pprefix, size_t prefixlength, int count, bool is_fifo, const char* encpass);
extern bool k2hdkc_q_str_remove_wp(const char* config, const char* pprefix, int count, bool is_fifo, const char* encpass);
extern bool k2hdkc_ex_q_remove_wp(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pprefix, size_t prefixlength, int count, bool is_fifo, const char* encpass);
extern bool k2hdkc_ex_q_str_remove_wp(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pprefix, int count, bool is_fifo, const char* encpass);
extern bool k2hdkc_pm_q_remove_wp(k2hdkc_chmpx_h handle, const unsigned char* pprefix, size_t prefixlength, int count, bool is_fifo, const char* encpass);
extern bool k2hdkc_pm_q_str_remove_wp(k2hdkc_chmpx_h handle, const char* pprefix, int count, bool is_fifo, const char* encpass);
extern bool k2hdkc_keyq_remove(const char* config, const unsigned char* pprefix, size_t prefixlength, int count, bool is_fifo);
extern bool k2hdkc_keyq_str_remove(const char* config, const char* pprefix, int count, bool is_fifo);
extern bool k2hdkc_ex_keyq_remove(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pprefix, size_t prefixlength, int count, bool is_fifo);
extern bool k2hdkc_ex_keyq_str_remove(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pprefix, int count, bool is_fifo);
extern bool k2hdkc_pm_keyq_remove(k2hdkc_chmpx_h handle, const unsigned char* pprefix, size_t prefixlength, int count, bool is_fifo);
extern bool k2hdkc_pm_keyq_str_remove(k2hdkc_chmpx_h handle, const char* pprefix, int count, bool is_fifo);
extern bool k2hdkc_keyq_remove_wp(const char* config, const unsigned char* pprefix, size_t prefixlength, int count, bool is_fifo, const char* encpass);
extern bool k2hdkc_keyq_str_remove_wp(const char* config, const char* pprefix, int count, bool is_fifo, const char* encpass);
extern bool k2hdkc_ex_keyq_remove_wp(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pprefix, size_t prefixlength, int count, bool is_fifo, const char* encpass);
extern bool k2hdkc_ex_keyq_str_remove_wp(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pprefix, int count, bool is_fifo, const char* encpass);
extern bool k2hdkc_pm_keyq_remove_wp(k2hdkc_chmpx_h handle, const unsigned char* pprefix, size_t prefixlength, int count, bool is_fifo, const char* encpass);
extern bool k2hdkc_pm_keyq_str_remove_wp(k2hdkc_chmpx_h handle, const char* pprefix, int count, bool is_fifo, const char* encpass);

//---------------------------------------------------------
// Functions - CAS(Compare and Swap)
//---------------------------------------------------------
// 
// [CAS init]
// 
// k2hdkc_cas64_init					cas 64bit value initialize(use one-time slave command object)
// k2hdkc_cas64_str_init				
// k2hdkc_ex_cas64_init					cas 64bit value initialize(use one-time slave command object, can specify chmpx parameters)
// k2hdkc_ex_cas64_str_init				
// k2hdkc_pm_cas64_init					cas 64bit value initialize(use permanent slave command object)
// k2hdkc_pm_cas64_str_init				
// k2hdkc_cas64_init_wa					cas 64bit value initialize with attributes(use one-time slave command object)
// k2hdkc_cas64_str_init_wa				
// k2hdkc_ex_cas64_init_wa				cas 64bit value initialize with attributes(use one-time slave command object, can specify chmpx parameters)
// k2hdkc_ex_cas64_str_init_wa			
// k2hdkc_pm_cas64_init_wa				cas 64bit value initialize with attributes(use permanent slave command object)
// k2hdkc_pm_cas64_str_init_wa			
// 
// k2hdkc_cas32_init					cas 32bit value initialize(use one-time slave command object)
// k2hdkc_cas32_str_init				
// k2hdkc_ex_cas32_init					cas 32bit value initialize(use one-time slave command object, can specify chmpx parameters)
// k2hdkc_ex_cas32_str_init				
// k2hdkc_pm_cas32_init					cas 32bit value initialize(use permanent slave command object)
// k2hdkc_pm_cas32_str_init				
// k2hdkc_cas32_init_wa					cas 32bit value initialize with attributes(use one-time slave command object)
// k2hdkc_cas32_str_init_wa				
// k2hdkc_ex_cas32_init_wa				cas 32bit value initialize with attributes(use one-time slave command object, can specify chmpx parameters)
// k2hdkc_ex_cas32_str_init_wa			
// k2hdkc_pm_cas32_init_wa				cas 32bit value initialize with attributes(use permanent slave command object)
// k2hdkc_pm_cas32_str_init_wa			
// 
// k2hdkc_cas16_init					cas 16bit value initialize(use one-time slave command object)
// k2hdkc_cas16_str_init				
// k2hdkc_ex_cas16_init					cas 16bit value initialize(use one-time slave command object, can specify chmpx parameters)
// k2hdkc_ex_cas16_str_init				
// k2hdkc_pm_cas16_init					cas 16bit value initialize(use permanent slave command object)
// k2hdkc_pm_cas16_str_init				
// k2hdkc_cas16_init_wa					cas 16bit value initialize with attributes(use one-time slave command object)
// k2hdkc_cas16_str_init_wa				
// k2hdkc_ex_cas16_init_wa				cas 16bit value initialize with attributes(use one-time slave command object, can specify chmpx parameters)
// k2hdkc_ex_cas16_str_init_wa			
// k2hdkc_pm_cas16_init_wa				cas 16bit value initialize with attributes(use permanent slave command object)
// k2hdkc_pm_cas16_str_init_wa			
// 
// k2hdkc_cas8_init						cas 8bit value initialize(use one-time slave command object)
// k2hdkc_cas8_str_init					
// k2hdkc_ex_cas8_init					cas 8bit value initialize(use one-time slave command object, can specify chmpx parameters)
// k2hdkc_ex_cas8_str_init				
// k2hdkc_pm_cas8_init					cas 8bit value initialize(use permanent slave command object)
// k2hdkc_pm_cas8_str_init				
// k2hdkc_cas8_init_wa					cas 8bit value initialize with attributes(use one-time slave command object)
// k2hdkc_cas8_str_init_wa				
// k2hdkc_ex_cas8_init_wa				cas 8bit value initialize with attributes(use one-time slave command object, can specify chmpx parameters)
// k2hdkc_ex_cas8_str_init_wa			
// k2hdkc_pm_cas8_init_wa				cas 8bit value initialize with attributes(use permanent slave command object)
// k2hdkc_pm_cas8_str_init_wa			
// 
extern bool k2hdkc_cas64_init(const char* config, const unsigned char* pkey, size_t keylength, uint64_t val);
extern bool k2hdkc_cas64_str_init(const char* config, const char* pkey, uint64_t val);
extern bool k2hdkc_ex_cas64_init(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint64_t val);
extern bool k2hdkc_ex_cas64_str_init(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint64_t val);
extern bool k2hdkc_pm_cas64_init(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint64_t val);
extern bool k2hdkc_pm_cas64_str_init(k2hdkc_chmpx_h handle, const char* pkey, uint64_t val);
extern bool k2hdkc_cas64_init_wa(const char* config, const unsigned char* pkey, size_t keylength, uint64_t val, const char* encpass, const time_t* expire);
extern bool k2hdkc_cas64_str_init_wa(const char* config, const char* pkey, uint64_t val, const char* encpass, const time_t* expire);
extern bool k2hdkc_ex_cas64_init_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint64_t val, const char* encpass, const time_t* expire);
extern bool k2hdkc_ex_cas64_str_init_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint64_t val, const char* encpass, const time_t* expire);
extern bool k2hdkc_pm_cas64_init_wa(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint64_t val, const char* encpass, const time_t* expire);
extern bool k2hdkc_pm_cas64_str_init_wa(k2hdkc_chmpx_h handle, const char* pkey, uint64_t val, const char* encpass, const time_t* expire);

extern bool k2hdkc_cas32_init(const char* config, const unsigned char* pkey, size_t keylength, uint32_t val);
extern bool k2hdkc_cas32_str_init(const char* config, const char* pkey, uint32_t val);
extern bool k2hdkc_ex_cas32_init(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint32_t val);
extern bool k2hdkc_ex_cas32_str_init(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint32_t val);
extern bool k2hdkc_pm_cas32_init(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint32_t val);
extern bool k2hdkc_pm_cas32_str_init(k2hdkc_chmpx_h handle, const char* pkey, uint32_t val);
extern bool k2hdkc_cas32_init_wa(const char* config, const unsigned char* pkey, size_t keylength, uint32_t val, const char* encpass, const time_t* expire);
extern bool k2hdkc_cas32_str_init_wa(const char* config, const char* pkey, uint32_t val, const char* encpass, const time_t* expire);
extern bool k2hdkc_ex_cas32_init_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint32_t val, const char* encpass, const time_t* expire);
extern bool k2hdkc_ex_cas32_str_init_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint32_t val, const char* encpass, const time_t* expire);
extern bool k2hdkc_pm_cas32_init_wa(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint32_t val, const char* encpass, const time_t* expire);
extern bool k2hdkc_pm_cas32_str_init_wa(k2hdkc_chmpx_h handle, const char* pkey, uint32_t val, const char* encpass, const time_t* expire);

extern bool k2hdkc_cas16_init(const char* config, const unsigned char* pkey, size_t keylength, uint16_t val);
extern bool k2hdkc_cas16_str_init(const char* config, const char* pkey, uint16_t val);
extern bool k2hdkc_ex_cas16_init(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint16_t val);
extern bool k2hdkc_ex_cas16_str_init(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint16_t val);
extern bool k2hdkc_pm_cas16_init(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint16_t val);
extern bool k2hdkc_pm_cas16_str_init(k2hdkc_chmpx_h handle, const char* pkey, uint16_t val);
extern bool k2hdkc_cas16_init_wa(const char* config, const unsigned char* pkey, size_t keylength, uint16_t val, const char* encpass, const time_t* expire);
extern bool k2hdkc_cas16_str_init_wa(const char* config, const char* pkey, uint16_t val, const char* encpass, const time_t* expire);
extern bool k2hdkc_ex_cas16_init_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint16_t val, const char* encpass, const time_t* expire);
extern bool k2hdkc_ex_cas16_str_init_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint16_t val, const char* encpass, const time_t* expire);
extern bool k2hdkc_pm_cas16_init_wa(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint16_t val, const char* encpass, const time_t* expire);
extern bool k2hdkc_pm_cas16_str_init_wa(k2hdkc_chmpx_h handle, const char* pkey, uint16_t val, const char* encpass, const time_t* expire);

extern bool k2hdkc_cas8_init(const char* config, const unsigned char* pkey, size_t keylength, uint8_t val);
extern bool k2hdkc_cas8_str_init(const char* config, const char* pkey, uint8_t val);
extern bool k2hdkc_ex_cas8_init(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint8_t val);
extern bool k2hdkc_ex_cas8_str_init(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint8_t val);
extern bool k2hdkc_pm_cas8_init(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint8_t val);
extern bool k2hdkc_pm_cas8_str_init(k2hdkc_chmpx_h handle, const char* pkey, uint8_t val);
extern bool k2hdkc_cas8_init_wa(const char* config, const unsigned char* pkey, size_t keylength, uint8_t val, const char* encpass, const time_t* expire);
extern bool k2hdkc_cas8_str_init_wa(const char* config, const char* pkey, uint8_t val, const char* encpass, const time_t* expire);
extern bool k2hdkc_ex_cas8_init_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint8_t val, const char* encpass, const time_t* expire);
extern bool k2hdkc_ex_cas8_str_init_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint8_t val, const char* encpass, const time_t* expire);
extern bool k2hdkc_pm_cas8_init_wa(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint8_t val, const char* encpass, const time_t* expire);
extern bool k2hdkc_pm_cas8_str_init_wa(k2hdkc_chmpx_h handle, const char* pkey, uint8_t val, const char* encpass, const time_t* expire);

// 
// [CAS get]
// 
// k2hdkc_cas64_get						get cas 64bit value(use one-time slave command object)
// k2hdkc_cas64_str_get					
// k2hdkc_ex_cas64_get					get cas 64bit value(use one-time slave command object, can specify chmpx parameters)
// k2hdkc_ex_cas64_str_get				
// k2hdkc_pm_cas64_get					get cas 64bit value(use permanent slave command object)
// k2hdkc_pm_cas64_str_get				
// k2hdkc_cas64_get_wa					get cas 64bit value with attributes(use one-time slave command object)
// k2hdkc_cas64_str_get_wa				
// k2hdkc_ex_cas64_get_wa				get cas 64bit value with attributes(use one-time slave command object, can specify chmpx parameters)
// k2hdkc_ex_cas64_str_get_wa			
// k2hdkc_pm_cas64_get_wa				get cas 64bit value with attributes(use permanent slave command object)
// k2hdkc_pm_cas64_str_get_wa			
// 
// k2hdkc_cas32_get						get cas 32bit value(use one-time slave command object)
// k2hdkc_cas32_str_get					
// k2hdkc_ex_cas32_get					get cas 32bit value(use one-time slave command object, can specify chmpx parameters)
// k2hdkc_ex_cas32_str_get				
// k2hdkc_pm_cas32_get					get cas 32bit value(use permanent slave command object)
// k2hdkc_pm_cas32_str_get				
// k2hdkc_cas32_get_wa					get cas 32bit value with attributes(use one-time slave command object)
// k2hdkc_cas32_str_get_wa				
// k2hdkc_ex_cas32_get_wa				get cas 32bit value with attributes(use one-time slave command object, can specify chmpx parameters)
// k2hdkc_ex_cas32_str_get_wa			
// k2hdkc_pm_cas32_get_wa				get cas 32bit value with attributes(use permanent slave command object)
// k2hdkc_pm_cas32_str_get_wa			
// 
// k2hdkc_cas16_get						get cas 16bit value(use one-time slave command object)
// k2hdkc_cas16_str_get					
// k2hdkc_ex_cas16_get					get cas 16bit value(use one-time slave command object, can specify chmpx parameters)
// k2hdkc_ex_cas16_str_get				
// k2hdkc_pm_cas16_get					get cas 16bit value(use permanent slave command object)
// k2hdkc_pm_cas16_str_get				
// k2hdkc_cas16_get_wa					get cas 16bit value with attributes(use one-time slave command object)
// k2hdkc_cas16_str_get_wa				
// k2hdkc_ex_cas16_get_wa				get cas 16bit value with attributes(use one-time slave command object, can specify chmpx parameters)
// k2hdkc_ex_cas16_str_get_wa			
// k2hdkc_pm_cas16_get_wa				get cas 16bit value with attributes(use permanent slave command object)
// k2hdkc_pm_cas16_str_get_wa			
// 
// k2hdkc_cas8_get						get cas 8bit value(use one-time slave command object)
// k2hdkc_cas8_str_get					
// k2hdkc_ex_cas8_get					get cas 8bit value(use one-time slave command object, can specify chmpx parameters)
// k2hdkc_ex_cas8_str_get				
// k2hdkc_pm_cas8_get					get cas 8bit value(use permanent slave command object)
// k2hdkc_pm_cas8_str_get				
// k2hdkc_cas8_get_wa					get cas 8bit value with attributes(use one-time slave command object)
// k2hdkc_cas8_str_get_wa				
// k2hdkc_ex_cas8_get_wa				get cas 8bit value with attributes(use one-time slave command object, can specify chmpx parameters)
// k2hdkc_ex_cas8_str_get_wa			
// k2hdkc_pm_cas8_get_wa				get cas 8bit value with attributes(use permanent slave command object)
// k2hdkc_pm_cas8_str_get_wa			
// 
extern bool k2hdkc_cas64_get(const char* config, const unsigned char* pkey, size_t keylength, uint64_t* pval);
extern bool k2hdkc_cas64_str_get(const char* config, const char* pkey, uint64_t* pval);
extern bool k2hdkc_ex_cas64_get(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint64_t* pval);
extern bool k2hdkc_ex_cas64_str_get(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint64_t* pval);
extern bool k2hdkc_pm_cas64_get(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint64_t* pval);
extern bool k2hdkc_pm_cas64_str_get(k2hdkc_chmpx_h handle, const char* pkey, uint64_t* pval);
extern bool k2hdkc_cas64_get_wa(const char* config, const unsigned char* pkey, size_t keylength, const char* encpass, uint64_t* pval);
extern bool k2hdkc_cas64_str_get_wa(const char* config, const char* pkey, const char* encpass, uint64_t* pval);
extern bool k2hdkc_ex_cas64_get_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, const char* encpass, uint64_t* pval);
extern bool k2hdkc_ex_cas64_str_get_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, const char* encpass, uint64_t* pval);
extern bool k2hdkc_pm_cas64_get_wa(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, const char* encpass, uint64_t* pval);
extern bool k2hdkc_pm_cas64_str_get_wa(k2hdkc_chmpx_h handle, const char* pkey, const char* encpass, uint64_t* pval);

extern bool k2hdkc_cas32_get(const char* config, const unsigned char* pkey, size_t keylength, uint32_t* pval);
extern bool k2hdkc_cas32_str_get(const char* config, const char* pkey, uint32_t* pval);
extern bool k2hdkc_ex_cas32_get(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint32_t* pval);
extern bool k2hdkc_ex_cas32_str_get(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint32_t* pval);
extern bool k2hdkc_pm_cas32_get(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint32_t* pval);
extern bool k2hdkc_pm_cas32_str_get(k2hdkc_chmpx_h handle, const char* pkey, uint32_t* pval);
extern bool k2hdkc_cas32_get_wa(const char* config, const unsigned char* pkey, size_t keylength, const char* encpass, uint32_t* pval);
extern bool k2hdkc_cas32_str_get_wa(const char* config, const char* pkey, const char* encpass, uint32_t* pval);
extern bool k2hdkc_ex_cas32_get_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, const char* encpass, uint32_t* pval);
extern bool k2hdkc_ex_cas32_str_get_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, const char* encpass, uint32_t* pval);
extern bool k2hdkc_pm_cas32_get_wa(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, const char* encpass, uint32_t* pval);
extern bool k2hdkc_pm_cas32_str_get_wa(k2hdkc_chmpx_h handle, const char* pkey, const char* encpass, uint32_t* pval);

extern bool k2hdkc_cas16_get(const char* config, const unsigned char* pkey, size_t keylength, uint16_t* pval);
extern bool k2hdkc_cas16_str_get(const char* config, const char* pkey, uint16_t* pval);
extern bool k2hdkc_ex_cas16_get(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint16_t* pval);
extern bool k2hdkc_ex_cas16_str_get(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint16_t* pval);
extern bool k2hdkc_pm_cas16_get(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint16_t* pval);
extern bool k2hdkc_pm_cas16_str_get(k2hdkc_chmpx_h handle, const char* pkey, uint16_t* pval);
extern bool k2hdkc_cas16_get_wa(const char* config, const unsigned char* pkey, size_t keylength, const char* encpass, uint16_t* pval);
extern bool k2hdkc_cas16_str_get_wa(const char* config, const char* pkey, const char* encpass, uint16_t* pval);
extern bool k2hdkc_ex_cas16_get_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, const char* encpass, uint16_t* pval);
extern bool k2hdkc_ex_cas16_str_get_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, const char* encpass, uint16_t* pval);
extern bool k2hdkc_pm_cas16_get_wa(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, const char* encpass, uint16_t* pval);
extern bool k2hdkc_pm_cas16_str_get_wa(k2hdkc_chmpx_h handle, const char* pkey, const char* encpass, uint16_t* pval);

extern bool k2hdkc_cas8_get(const char* config, const unsigned char* pkey, size_t keylength, uint8_t* pval);
extern bool k2hdkc_cas8_str_get(const char* config, const char* pkey, uint8_t* pval);
extern bool k2hdkc_ex_cas8_get(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint8_t* pval);
extern bool k2hdkc_ex_cas8_str_get(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint8_t* pval);
extern bool k2hdkc_pm_cas8_get(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint8_t* pval);
extern bool k2hdkc_pm_cas8_str_get(k2hdkc_chmpx_h handle, const char* pkey, uint8_t* pval);
extern bool k2hdkc_cas8_get_wa(const char* config, const unsigned char* pkey, size_t keylength, const char* encpass, uint8_t* pval);
extern bool k2hdkc_cas8_str_get_wa(const char* config, const char* pkey, const char* encpass, uint8_t* pval);
extern bool k2hdkc_ex_cas8_get_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, const char* encpass, uint8_t* pval);
extern bool k2hdkc_ex_cas8_str_get_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, const char* encpass, uint8_t* pval);
extern bool k2hdkc_pm_cas8_get_wa(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, const char* encpass, uint8_t* pval);
extern bool k2hdkc_pm_cas8_str_get_wa(k2hdkc_chmpx_h handle, const char* pkey, const char* encpass, uint8_t* pval);

// 
// [CAS set]
// 
// k2hdkc_cas64_set						set cas 64bit value(use one-time slave command object)
// k2hdkc_cas64_str_set					
// k2hdkc_ex_cas64_set					set cas 64bit value(use one-time slave command object, can specify chmpx parameters)
// k2hdkc_ex_cas64_str_set				
// k2hdkc_pm_cas64_set					set cas 64bit value(use permanent slave command object)
// k2hdkc_pm_cas64_str_set				
// k2hdkc_cas64_set_wa					set cas 64bit value with attributes(use one-time slave command object)
// k2hdkc_cas64_str_set_wa				
// k2hdkc_ex_cas64_set_wa				set cas 64bit value with attributes(use one-time slave command object, can specify chmpx parameters)
// k2hdkc_ex_cas64_str_set_wa			
// k2hdkc_pm_cas64_set_wa				set cas 64bit value with attributes(use permanent slave command object)
// k2hdkc_pm_cas64_str_set_wa			
// 
// k2hdkc_cas32_set						set cas 32bit value(use one-time slave command object)
// k2hdkc_cas32_str_set					
// k2hdkc_ex_cas32_set					set cas 32bit value(use one-time slave command object, can specify chmpx parameters)
// k2hdkc_ex_cas32_str_set				
// k2hdkc_pm_cas32_set					set cas 32bit value(use permanent slave command object)
// k2hdkc_pm_cas32_str_set				
// k2hdkc_cas32_set_wa					set cas 32bit value with attributes(use one-time slave command object)
// k2hdkc_cas32_str_set_wa				
// k2hdkc_ex_cas32_set_wa				set cas 32bit value with attributes(use one-time slave command object, can specify chmpx parameters)
// k2hdkc_ex_cas32_str_set_wa			
// k2hdkc_pm_cas32_set_wa				set cas 32bit value with attributes(use permanent slave command object)
// k2hdkc_pm_cas32_str_set_wa			
// 
// k2hdkc_cas16_set						set cas 16bit value(use one-time slave command object)
// k2hdkc_cas16_str_set					
// k2hdkc_ex_cas16_set					set cas 16bit value(use one-time slave command object, can specify chmpx parameters)
// k2hdkc_ex_cas16_str_set				
// k2hdkc_pm_cas16_set					set cas 16bit value(use permanent slave command object)
// k2hdkc_pm_cas16_str_set				
// k2hdkc_cas16_set_wa					set cas 16bit value with attributes(use one-time slave command object)
// k2hdkc_cas16_str_set_wa				
// k2hdkc_ex_cas16_set_wa				set cas 16bit value with attributes(use one-time slave command object, can specify chmpx parameters)
// k2hdkc_ex_cas16_str_set_wa			
// k2hdkc_pm_cas16_set_wa				set cas 16bit value with attributes(use permanent slave command object)
// k2hdkc_pm_cas16_str_set_wa			
// 
// k2hdkc_cas8_set						set cas 8bit value(use one-time slave command object)
// k2hdkc_cas8_str_set					
// k2hdkc_ex_cas8_set					set cas 8bit value(use one-time slave command object, can specify chmpx parameters)
// k2hdkc_ex_cas8_str_set				
// k2hdkc_pm_cas8_set					set cas 8bit value(use permanent slave command object)
// k2hdkc_pm_cas8_str_set				
// k2hdkc_cas8_set_wa					set cas 8bit value with attributes(use one-time slave command object)
// k2hdkc_cas8_str_set_wa				
// k2hdkc_ex_cas8_set_wa				set cas 8bit value with attributes(use one-time slave command object, can specify chmpx parameters)
// k2hdkc_ex_cas8_str_set_wa			
// k2hdkc_pm_cas8_set_wa				set cas 8bit value with attributes(use permanent slave command object)
// k2hdkc_pm_cas8_str_set_wa			
// 
extern bool k2hdkc_cas64_set(const char* config, const unsigned char* pkey, size_t keylength, uint64_t oldval, uint64_t newval);
extern bool k2hdkc_cas64_str_set(const char* config, const char* pkey, uint64_t oldval, uint64_t newval);
extern bool k2hdkc_ex_cas64_set(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint64_t oldval, uint64_t newval);
extern bool k2hdkc_ex_cas64_str_set(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint64_t oldval, uint64_t newval);
extern bool k2hdkc_pm_cas64_set(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint64_t oldval, uint64_t newval);
extern bool k2hdkc_pm_cas64_str_set(k2hdkc_chmpx_h handle, const char* pkey, uint64_t oldval, uint64_t newval);
extern bool k2hdkc_cas64_set_wa(const char* config, const unsigned char* pkey, size_t keylength, uint64_t oldval, uint64_t newval, const char* encpass, const time_t* expire);
extern bool k2hdkc_cas64_str_set_wa(const char* config, const char* pkey, uint64_t oldval, uint64_t newval, const char* encpass, const time_t* expire);
extern bool k2hdkc_ex_cas64_set_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint64_t oldval, uint64_t newval, const char* encpass, const time_t* expire);
extern bool k2hdkc_ex_cas64_str_set_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint64_t oldval, uint64_t newval, const char* encpass, const time_t* expire);
extern bool k2hdkc_pm_cas64_set_wa(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint64_t oldval, uint64_t newval, const char* encpass, const time_t* expire);
extern bool k2hdkc_pm_cas64_str_set_wa(k2hdkc_chmpx_h handle, const char* pkey, uint64_t oldval, uint64_t newval, const char* encpass, const time_t* expire);

extern bool k2hdkc_cas32_set(const char* config, const unsigned char* pkey, size_t keylength, uint32_t oldval, uint32_t newval);
extern bool k2hdkc_cas32_str_set(const char* config, const char* pkey, uint32_t oldval, uint32_t newval);
extern bool k2hdkc_ex_cas32_set(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint32_t oldval, uint32_t newval);
extern bool k2hdkc_ex_cas32_str_set(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint32_t oldval, uint32_t newval);
extern bool k2hdkc_pm_cas32_set(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint32_t oldval, uint32_t newval);
extern bool k2hdkc_pm_cas32_str_set(k2hdkc_chmpx_h handle, const char* pkey, uint32_t oldval, uint32_t newval);
extern bool k2hdkc_cas32_set_wa(const char* config, const unsigned char* pkey, size_t keylength, uint32_t oldval, uint32_t newval, const char* encpass, const time_t* expire);
extern bool k2hdkc_cas32_str_set_wa(const char* config, const char* pkey, uint32_t oldval, uint32_t newval, const char* encpass, const time_t* expire);
extern bool k2hdkc_ex_cas32_set_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint32_t oldval, uint32_t newval, const char* encpass, const time_t* expire);
extern bool k2hdkc_ex_cas32_str_set_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint32_t oldval, uint32_t newval, const char* encpass, const time_t* expire);
extern bool k2hdkc_pm_cas32_set_wa(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint32_t oldval, uint32_t newval, const char* encpass, const time_t* expire);
extern bool k2hdkc_pm_cas32_str_set_wa(k2hdkc_chmpx_h handle, const char* pkey, uint32_t oldval, uint32_t newval, const char* encpass, const time_t* expire);

extern bool k2hdkc_cas16_set(const char* config, const unsigned char* pkey, size_t keylength, uint16_t oldval, uint16_t newval);
extern bool k2hdkc_cas16_str_set(const char* config, const char* pkey, uint16_t oldval, uint16_t newval);
extern bool k2hdkc_ex_cas16_set(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint16_t oldval, uint16_t newval);
extern bool k2hdkc_ex_cas16_str_set(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint16_t oldval, uint16_t newval);
extern bool k2hdkc_pm_cas16_set(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint16_t oldval, uint16_t newval);
extern bool k2hdkc_pm_cas16_str_set(k2hdkc_chmpx_h handle, const char* pkey, uint16_t oldval, uint16_t newval);
extern bool k2hdkc_cas16_set_wa(const char* config, const unsigned char* pkey, size_t keylength, uint16_t oldval, uint16_t newval, const char* encpass, const time_t* expire);
extern bool k2hdkc_cas16_str_set_wa(const char* config, const char* pkey, uint16_t oldval, uint16_t newval, const char* encpass, const time_t* expire);
extern bool k2hdkc_ex_cas16_set_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint16_t oldval, uint16_t newval, const char* encpass, const time_t* expire);
extern bool k2hdkc_ex_cas16_str_set_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint16_t oldval, uint16_t newval, const char* encpass, const time_t* expire);
extern bool k2hdkc_pm_cas16_set_wa(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint16_t oldval, uint16_t newval, const char* encpass, const time_t* expire);
extern bool k2hdkc_pm_cas16_str_set_wa(k2hdkc_chmpx_h handle, const char* pkey, uint16_t oldval, uint16_t newval, const char* encpass, const time_t* expire);

extern bool k2hdkc_cas8_set(const char* config, const unsigned char* pkey, size_t keylength, uint8_t oldval, uint8_t newval);
extern bool k2hdkc_cas8_str_set(const char* config, const char* pkey, uint8_t oldval, uint8_t newval);
extern bool k2hdkc_ex_cas8_set(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint8_t oldval, uint8_t newval);
extern bool k2hdkc_ex_cas8_str_set(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint8_t oldval, uint8_t newval);
extern bool k2hdkc_pm_cas8_set(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint8_t oldval, uint8_t newval);
extern bool k2hdkc_pm_cas8_str_set(k2hdkc_chmpx_h handle, const char* pkey, uint8_t oldval, uint8_t newval);
extern bool k2hdkc_cas8_set_wa(const char* config, const unsigned char* pkey, size_t keylength, uint8_t oldval, uint8_t newval, const char* encpass, const time_t* expire);
extern bool k2hdkc_cas8_str_set_wa(const char* config, const char* pkey, uint8_t oldval, uint8_t newval, const char* encpass, const time_t* expire);
extern bool k2hdkc_ex_cas8_set_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint8_t oldval, uint8_t newval, const char* encpass, const time_t* expire);
extern bool k2hdkc_ex_cas8_str_set_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint8_t oldval, uint8_t newval, const char* encpass, const time_t* expire);
extern bool k2hdkc_pm_cas8_set_wa(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint8_t oldval, uint8_t newval, const char* encpass, const time_t* expire);
extern bool k2hdkc_pm_cas8_str_set_wa(k2hdkc_chmpx_h handle, const char* pkey, uint8_t oldval, uint8_t newval, const char* encpass, const time_t* expire);

// 
// [CAS increment/decrement]
// 
// k2hdkc_cas_increment					increment cas value(use one-time slave command object)
// k2hdkc_cas_str_increment				
// k2hdkc_ex_cas_increment				increment cas value(use one-time slave command object, can specify chmpx parameters)
// k2hdkc_ex_cas_str_increment			
// k2hdkc_pm_cas_increment				increment cas value(use permanent slave command object)
// k2hdkc_pm_cas_str_increment			
// k2hdkc_cas_increment_wa				increment cas value with attributes(use one-time slave command object)
// k2hdkc_cas_str_increment_wa			
// k2hdkc_ex_cas_increment_wa			increment cas value with attributes(use one-time slave command object, can specify chmpx parameters)
// k2hdkc_ex_cas_str_increment_wa		
// k2hdkc_pm_cas_increment_wa			increment cas value with attributes(use permanent slave command object)
// k2hdkc_pm_cas_str_increment_wa		
// 
// k2hdkc_cas_decrement					decrement cas value(use one-time slave command object)
// k2hdkc_cas_str_decrement				
// k2hdkc_ex_cas_decrement				decrement cas value(use one-time slave command object, can specify chmpx parameters)
// k2hdkc_ex_cas_str_decrement			
// k2hdkc_pm_cas_decrement				decrement cas value(use permanent slave command object)
// k2hdkc_pm_cas_str_decrement			
// k2hdkc_cas_decrement_wa				decrement cas value with attributes(use one-time slave command object)
// k2hdkc_cas_str_decrement_wa			
// k2hdkc_ex_cas_decrement_wa			decrement cas value with attributes(use one-time slave command object, can specify chmpx parameters)
// k2hdkc_ex_cas_str_decrement_wa		
// k2hdkc_pm_cas_decrement_wa			decrement cas value with attributes(use permanent slave command object)
// k2hdkc_pm_cas_str_decrement_wa		
// 
extern bool k2hdkc_cas_increment(const char* config, const unsigned char* pkey, size_t keylength);
extern bool k2hdkc_cas_str_increment(const char* config, const char* pkey);
extern bool k2hdkc_ex_cas_increment(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength);
extern bool k2hdkc_ex_cas_str_increment(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey);
extern bool k2hdkc_pm_cas_increment(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength);
extern bool k2hdkc_pm_cas_str_increment(k2hdkc_chmpx_h handle, const char* pkey);
extern bool k2hdkc_cas_increment_wa(const char* config, const unsigned char* pkey, size_t keylength, const char* encpass, const time_t* expire);
extern bool k2hdkc_cas_str_increment_wa(const char* config, const char* pkey, const char* encpass, const time_t* expire);
extern bool k2hdkc_ex_cas_increment_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, const char* encpass, const time_t* expire);
extern bool k2hdkc_ex_cas_str_increment_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, const char* encpass, const time_t* expire);
extern bool k2hdkc_pm_cas_increment_wa(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, const char* encpass, const time_t* expire);
extern bool k2hdkc_pm_cas_str_increment_wa(k2hdkc_chmpx_h handle, const char* pkey, const char* encpass, const time_t* expire);

extern bool k2hdkc_cas_decrement(const char* config, const unsigned char* pkey, size_t keylength);
extern bool k2hdkc_cas_str_decrement(const char* config, const char* pkey);
extern bool k2hdkc_ex_cas_decrement(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength);
extern bool k2hdkc_ex_cas_str_decrement(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey);
extern bool k2hdkc_pm_cas_decrement(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength);
extern bool k2hdkc_pm_cas_str_decrement(k2hdkc_chmpx_h handle, const char* pkey);
extern bool k2hdkc_cas_decrement_wa(const char* config, const unsigned char* pkey, size_t keylength, const char* encpass, const time_t* expire);
extern bool k2hdkc_cas_str_decrement_wa(const char* config, const char* pkey, const char* encpass, const time_t* expire);
extern bool k2hdkc_ex_cas_decrement_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, const char* encpass, const time_t* expire);
extern bool k2hdkc_ex_cas_str_decrement_wa(const char* config, short ctlport, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, const char* encpass, const time_t* expire);
extern bool k2hdkc_pm_cas_decrement_wa(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, const char* encpass, const time_t* expire);
extern bool k2hdkc_pm_cas_str_decrement_wa(k2hdkc_chmpx_h handle, const char* pkey, const char* encpass, const time_t* expire);

//---------------------------------------------------------
// Version
//---------------------------------------------------------
extern void k2hdkc_print_version(FILE* stream);

DECL_EXTERN_C_END

#endif	// K2HDKC_H

/*
 * VIM modelines
 *
 * vim:set ts=4 fenc=utf-8:
 */
