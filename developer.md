---
layout: contents
language: en-us
title: Developer
short_desc: K2Hash based Distributed Kvs Cluster
lang_opp_file: developerja.html
lang_opp_word: To Japanese
prev_url: build.html
prev_string: Build
top_url: index.html
top_string: TOP
next_url: environments.html
next_string: Environments
---

<!-- -----------------------------------------------------------　-->
# Developer

### [Precondition](#PRECONDITION)
[About Specification](#SPECIFICATIONS)  
[Persistence/Non-persistence chmpx connections for C++ API](#PERSISTENT)  
[About sample codes](#ABOUTSAMPLES)

### [C API](#CAPI)
[Debug family(C I/F)](#DEBUG)  
[For chmpx family(C I/F)](#CHMPX)  
[Last Response Code family(C I/F)](#LASTRESCODE)  
[Get family(C I/F)](#GET)  
[Get Direct family(C I/F)](#GETDIRECT)  
[Get Subkey family(C I/F)](#GETSUB)  
[Get Attribute family(C I/F)](#GETATTR)  
[Set family(C I/F)](#SET)  
[Set Direct family(C I/F)](#SETDIRECT)  
[Set Subkey family(C I/F)](#SETSUB)  
[Clear Subkey family(C I/F)](#CLEARSUB)  
[Add Subkey family(C I/F)](#ADDSUB)  
[Set All family(C I/F)](#SETALL)  
[Remove Key family(C I/F)](#REMOVE)  
[Remove Subkey family(C I/F)](#REMOVESUB)  
[Rename Key family(C I/F)](#RENAME)  
[Queue(KeyQUeue) Push family(C I/F)](#QUEUEPUSH)  
[Queue(KeyQUeue) Pop family(C I/F)](#QUEUEPOP)  
[Queue(KeyQUeue) Remove family(C I/F)](#QUEUEREMOVE)  
[CAS(Compare And Swap) Initialize family(C I/F)](#CASINIT)  
[CAS(Compare And Swap) Get family(C I/F)](#CASGET)  
[CAS(Compare And Swap) Set family(C I/F)](#CASSET)  
[CAS(Compare And Swap) Increment/Decrement family(C I/F)](#CASINCDEC)

### [C++ API](#CPP)
[Debug family(C++ I/F)](#DEBUGCPP)  
[Command Class Factory(C++ I/F)](#COMFACTORY)  
[K2hdkcComGet class](#GETCPP)  
[K2hdkcComGetDirect class](#GETDIRECTCPP)  
[K2hdkcComGetSubkeys class](#GETSUBSCPP)  
[K2hdkcComGetAttrs class](#GETATTRSCPP)  
[K2hdkcComGetAttr class](#GETATTRCPP)  
[K2hdkcComSet class](#SETAPP)  
[K2hdkcComSetDirect class](#SETDIRECTCPP)  
[K2hdkcComSetSubkeys class](#SETSUBSCPP)  
[K2hdkcComSetAll class](#SETALLCPP)  
[K2hdkcComAddSubkeys class](#ADDSUBSCPP)  
[K2hdkcComAddSubkey class](#ADDSUBCPP)  
[K2hdkcComDel class](#DELCPP)  
[K2hdkcComDelSubkeys class](#DELSUBSCPP)  
[K2hdkcComDelSubkey class](#DELSUBCPP)  
[K2hdkcComRen class](#RENCPP)  
[K2hdkcComQPush class](#QUEUEPUSHCPP)  
[K2hdkcComQPop class](#QUEUEPOPCPP)  
[K2hdkcComQDel class](#QUEUEDELCPP)  
[K2hdkcComCasInit class](#CASINITCPP)  
[K2hdkcComCasGet class](#CASGETCPP)  
[K2hdkcComCasSet class](#CASSETCPP)  
[K2hdkcComCasIncDec class](#CASINCDECCPP)  
[K2hdkcComState class](#STATECPP)

<!-- -----------------------------------------------------------　-->
***

## <a name="PRECONDITION"> Precondition
This section explains the premise for using the k2hdkc library.

### <a name="SPECIFICATIONS"> About Specification
Before using the k2hdkc library, it is necessary to understand the specifications of the k2hdkc library.
The k2hdkc cluster is built with communication middleware [CHMPX](https://chmpx.antpick.ax/) and Key Value Store [K2HASH](https://k2hash.antpick.ax/) library.
You can create a program to access this k2hdkc cluster using the k2hdkc library.

k2hdkc API(C/C++ API) that accesses cluster data sends and receives communication commands internally via communication middleware(chmpx).
When each API performs internal communication, use one of the following methods.
- Connects to chmpx every communication command and disconnects after communication(Not persistence chmpx connection)
- Permanently connect to chmpx before calling API, use/reuse this connection at least once(Persistence chmpx connection)

Although the internal operation of each API is the same, there is a difference whether to connect/disconnect with chmpx every operation of k2hdkc cluster data.
Non-persistence chmpx connection is used when the client process can not maintain a persistence connection(ex. the program is implemented as a handler for HTTP process).
Persistence chmpx connection can be used in a client process for a daemon.

### <a name="PERSISTENT"> Persistence/Non-persistence chmpx connections for C++ API
The C++ API has the following procedure differ depending on the case of Persistence/Non-persistence in connection with chmpx.
However, although the procedure differs, the internal processing is the same except for the connection/disconnection processing to chmpx.
The only difference is switching the call of global method(Implemented with MACROs) that creates a dedicated communication command class object.

#### Non-persistence chmpx connection for C++
For non-persistence chmpx connection in the C ++ API, call the library by the following procedure.
1. Get global communication command class object using global GetOtSlaveK2hdkcCom...() method(MACRO).
1. Call the CommandSend() method of the communication command class object to process it.
1. The processing result is acquired by the GetResponseData() method of the communication command class object(It can also be acquired simultaneously by the CommandSend() method).
1. Finally, destroy the communication command class object.

#### Persistence chmpx connection for C++
For Persistence chmpx connection in the C ++ API, call the library by the following procedure.
1. Create a K2hdkcSlave class instance and initialize it with the Initialize() method.
1. Call the Open() method of the K2hdkcSlave class instance and connect to chmpx.
1. Call the global GetPmSlaveK2hdkcCom...() method(MACRO) with the K2hdkcSlave class instance and get the communication command class object.
1. Call the CommandSend() method of the communication command class object to process it.
1. The processing result is acquired by the GetResponseData() method of the communication command class object(It can also be acquired simultaneously by the CommandSend() method).
1. Destroy the dedicated communication command class object.
1. Repeat the above 3 to 6
1. Call the Close() method of the K2hdkcSlave class instance and disconnect from chmpx.

### <a name="ABOUTSAMPLES"> About sample codes
A simple sample code is attached to the explanation of the C, C ++ API of this k2hdkc library interface.
For detailed usage, it is recommended to refer to the source codes of k2hdkc test tool(k2hdkclinetool).
The test tool contains codes comparable to almost all sample codes.
You can use it as sample code by searching the desired function and method in the source codes.

<!-- -----------------------------------------------------------　-->
***

## <a name="CAPI"> C API
It is an API for C language.
Include the following header files when developing.
```
#include <k2hdkc/k2hdkc.h>
```

When linking please specify the following as an option.
```
-lk2hdkc
```

The functions for C language are explained below.
<!-- -----------------------------------------------------------　-->
***

### <a name="DEBUG"> Debug family(C I/F)
The k2hdkc library can output messages to check internal operation and API operation.
This function group is a group of functions for controlling message output.

#### Format
- void k2hdkc_bump_debug_level(void)
- void k2hdkc_set_debug_level_silent(void)
- void k2hdkc_set_debug_level_error(void)
- void k2hdkc_set_debug_level_warning(void)
- void k2hdkc_set_debug_level_message(void)
- void k2hdkc_set_debug_level_dump(void)
- bool k2hdkc_set_debug_file(const char* filepath)
- bool k2hdkc_unset_debug_file(void)
- bool k2hdkc_load_debug_env(void)
- bool k2hdkc_is_enable_comlog(void)
- void k2hdkc_enable_comlog(void)
- void k2hdkc_disable_comlog(void)
- void k2hdkc_toggle_comlog(void)

#### Description
- k2hdkc_bump_debug_level  
  Bump up message output level as non-output -> error -> warning -> information -> dump -> non-output.
- k2hdkc_set_debug_level_silent  
  Set the message output level to no output.
- k2hdkc_set_debug_level_error  
  Set the message output level to error.
- k2hdkc_set_debug_level_warning  
  Set the message output level above the warning.
- k2hdkc_set_debug_level_message  
  Set the message output level above the information.
- k2hdkc_set_debug_level_dump  
  Set the message output level above the dump.
- k2hdkc_set_debug_file  
  Specify the file to output message. If it is not set, it is output to stderr.
- k2hdkc_unset_debug_file  
  Unset to output messages to stderr.
- k2hdkc_load_debug_env  
  Load the environment variables DKCDBGMODE, DKCDBGFILE and set the message output and output destination according to the value.
- k2hdkc_is_enable_comlog  
  Check the enable/disable of the communication log output.
- k2hdkc_enable_comlog  
  Enable the communication log output.
- k2hdkc_disable_comlog  
  Disable the communication log output.
- k2hdkc_toggle_comlog  
  Toggle the communication log output.

#### Parameters
- filepath  
  Specify the file path of the debug message output destination.

#### Return Values
- k2hdkc_set_debug_file / k2hdkc_unset_debug_file / k2hdkc_load_debug_env / k2hdkc_is_enable_comlog  
  Returns true if it succeeds. If it fails, it returns false.

#### Note
For environment variables DKCDBGMODE, DKCDBGFILE, see [Environments](environments.html).

#### Examples
```
k2hdkc_bump_debug_level();
k2hdkc_set_debug_file("/var/log/k2hdkc/error.log");
k2hdkc_set_debug_level_message();
```

<!-- -----------------------------------------------------------　-->
***

### <a name="CHMPX"> For chmpx family(C I/F)
This function group makes a persistent [CHMPX](https://chmpx.antpick.ax/) connection to use the k2hdkc library.
The handle k2hdkc_chmpx_h returned in this function group is a handle that can be used with k2hdkc library C I/F.

#### Format
- k2hdkc_chmpx_h k2hdkc_open_chmpx(const char* config)
- k2hdkc_chmpx_h k2hdkc_open_chmpx_full(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, bool is_clean_bup)
- bool k2hdkc_close_chmpx(k2hdkc_chmpx_h handle)
- bool k2hdkc_close_chmpx_ex(k2hdkc_chmpx_h handle, bool is_clean_bup)

#### Description
- k2hdkc_open_chmpx  
  Connect to the chmpx slave node.
- k2hdkc_open_chmpx_ex  
  Connect to the chmpx slave node with detailed setting.
- k2hdkc_close_chmpx  
  Disconnect from the chmpx slave node.
- k2hdkc_close_chmpx_ex  
  Disconnect from the chmpx slave node with detailed setting.

#### Parameters
- config  
  Specify the configuration of the chmpx slave node(INI/YAML/JSON format file path, or JSON character string).
- ctlport  
  Specify the control port number of the chmpx slave node(It is necessary to specify it when multiple chmpx are running on the same server).
- cuk  
  Specify the CUK string of the chmpx slave node(It is necessary to specify it when multiple chmpx are running on the same server).
- is_auto_rejoin  
  Specify whether to reconnect automatically when the connection with the slave chmpx node is disconnected.
- no_giveup_rejoin  
  Case of specifying to reconnect automatically to the slave node of chmpx, specify the upper limit of retrying.
- is_clean_bup  
  When disconnecting from the chmpx slave node, specify whether to delete the unnecessary information file after that.
- handle  
  Specify the handle of the chmpx connection used by the k2hdkc library.

#### Return Values
- k2hdkc_open_chmpx / k2hdkc_open_chmpx_ex  
  On success, it returns the connected k2hdkc_chmpx_h handle.  
  On failure, K2HDKC_INVALID_HANDLE is returned.
- k2hdkc_close_chmpx / k2hdkc_close_chmpx_ex  
  Returns true on success, false on failure.

#### Examples
```
k2hdkc_chmpx_h    chmpxhandle;
if(K2HDKC_INVALID_HANDLE == (chmpxhandle = k2hdkc_open_chmpx_ex(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin, isCleanupBup))){
    exit(EXIT_FAILURE);
}
...
...
...

if(!k2hdkc_close_chmpx_ex(chmpxhandle, isCleanupBup)){
    fprintf(stderr, "Could not close(leave and close msgid) slave node chmpx, but continue for cleanup...\n");
}
chmpxhandle = K2HDKC_INVALID_HANDLE;        // force
```

<!-- -----------------------------------------------------------　-->
***

### <a name="LASTRESCODE"> Last Response Code family(C I/F)
After using the C API of the k2hdkc library, you can obtain the response code of the API.  
The response code is 64 bits, indicating the success/failure with the upper 32 bits, and the lower 32 bits(subcode) indicates the detailed error.  
Macro is provided for error code discrimination(See k2hdkccomstructure.h).  
The error code is handled in the same way as errno in C language.

#### Format
- dkcres_type_t k2hdkc_get_lastres_code(void)
- dkcres_type_t k2hdkc_get_lastres_subcode(void)
- bool k2hdkc_is_lastres_success(void)
- dkcres_type_t k2hdkc_get_res_code(k2hdkc_chmpx_h handle)
- dkcres_type_t k2hdkc_get_res_subcode(k2hdkc_chmpx_h handle)
- bool k2hdkc_is_res_success(k2hdkc_chmpx_h handle)

#### Description
- k2hdkc_get_lastres_code  
  When Non-persistent connection to the slave chmpx node, the last response code can be acquired.
- k2hdkc_get_lastres_subcode  
  When Non-persistent connection to the slave chmpx node, the last response subcode can be acquired.
- k2hdkc_is_lastres_success  
  When non-persistent connection to the slave chmpx node, the last result can be acquired.
- k2hdkc_get_res_code  
  When persistent connection to the slave chmpx node, the last response code of specified chmpx handle can be acquired.
- k2hdkc_get_res_subcode  
  When persistent connection to the slave chmpx node, the last response subcode of specified chmpx handle can be acquired.
- k2hdkc_is_res_success  
  When persistent connection to the slave chmpx node, the last result of specified chmpx handle can be acquired.

#### Parameters
- handle  
  Specify the handle of the chmpx connection used by the k2hdkc library.

#### Return Values
- k2hdkc_get_lastres_code / k2hdkc_get_res_code  
  Returns the response code.
- k2hdkc_get_lastres_subcode / k2hdkc_get_res_subcode  
  Returns the response subcode.
- k2hdkc_is_lastres_success / k2hdkc_is_res_success  
  Returns true on success, false on failure.

#### Examples
```
// get response code
dkcres_type_t    rescode = k2hdkc_get_lastres_code();

// check result
if(!result){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}
```

<!-- -----------------------------------------------------------　-->
***

### <a name="GET"> Get family(C I/F)
This is a function group that obtains the value of the specified key from the cluster of k2hdkc.

#### Format
- bool k2hdkc_get_value(const char* config, const unsigned char* pkey, size_t keylength, unsigned char** ppval, size_t* pvallength);
- unsigned char* k2hdkc_get_direct_value(const char* config, const unsigned char* pkey, size_t keylength, size_t* pvallength);
- bool k2hdkc_get_str_value(const char* config, const char* pkey, char** ppval);
- char* k2hdkc_get_str_direct_value(const char* config, const char* pkey);
 
- bool k2hdkc_get_value_wp(const char* config, const unsigned char* pkey, size_t keylength, const char* encpass, unsigned char** ppval, size_t* pvallength);
- unsigned char* k2hdkc_get_direct_value_wp(const char* config, const unsigned char* pkey, size_t keylength, const char* encpass, size_t* pvallength);
- bool k2hdkc_get_str_value_wp(const char* config, const char* pkey, const char* encpass, char** ppval);
- char* k2hdkc_get_str_direct_value_wp(const char* config, const char* pkey, const char* encpass);
 
- bool k2hdkc_get_value_np(const char* config, const unsigned char* pkey, size_t keylength, unsigned char** ppval, size_t* pvallength);
- unsigned char* k2hdkc_get_direct_value_np(const char* config, const unsigned char* pkey, size_t keylength, size_t* pvallength);
- bool k2hdkc_get_str_value_np(const char* config, const char* pkey, char** ppval);
- char* k2hdkc_get_str_direct_value_np(const char* config, const char* pkey);
 
- bool k2hdkc_full_get_value(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, unsigned char** ppval, size_t* pvallength);
- unsigned char* k2hdkc_full_get_direct_value(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, size_t* pvallength);
- bool k2hdkc_full_get_str_value(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, char** ppval);
- char* k2hdkc_full_get_str_direct_value(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey);
 
- bool k2hdkc_full_get_value_wp(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, const char* encpass, unsigned char** ppval, size_t* pvallength);
- unsigned char* k2hdkc_full_get_direct_value_wp(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, const char* encpass, size_t* pvallength);
- bool k2hdkc_full_get_str_value_wp(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, const char* encpass, char** ppval);
- char* k2hdkc_full_get_str_direct_value_wp(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, const char* encpass);
 
- bool k2hdkc_full_get_value_np(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, unsigned char** ppval, size_t* pvallength);
- unsigned char* k2hdkc_full_get_direct_value_np(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, size_t* pvallength);
- bool k2hdkc_full_get_str_value_np(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, char** ppval);
- char* k2hdkc_full_get_str_direct_value_np(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey);
 
- bool k2hdkc_pm_get_value(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, unsigned char** ppval, size_t* pvallength);
- unsigned char* k2hdkc_pm_get_direct_value(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, size_t* pvallength);
- bool k2hdkc_pm_get_str_value(k2hdkc_chmpx_h handle, const char* pkey, char** ppval);
- char* k2hdkc_pm_get_str_direct_value(k2hdkc_chmpx_h handle, const char* pkey);
 
- bool k2hdkc_pm_get_value_wp(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, const char* encpass, unsigned char** ppval, size_t* pvallength);
- unsigned char* k2hdkc_pm_get_direct_value_wp(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, const char* encpass, size_t* pvallength);
- bool k2hdkc_pm_get_str_value_wp(k2hdkc_chmpx_h handle, const char* pkey, const char* encpass, char** ppval);
- char* k2hdkc_pm_get_str_direct_value_wp(k2hdkc_chmpx_h handle, const char* pkey, const char* encpass);
 
- bool k2hdkc_pm_get_value_np(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, unsigned char** ppval, size_t* pvallength);
- unsigned char* k2hdkc_pm_get_direct_value_np(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, size_t* pvallength);
- bool k2hdkc_pm_get_str_value_np(k2hdkc_chmpx_h handle, const char* pkey, char** ppval);
- char* k2hdkc_pm_get_str_direct_value_np(k2hdkc_chmpx_h handle, const char* pkey);

#### Description
These function groups are classified according to the following rules and classified into 4 types.
- k2hdkc_get_value type  
  Store the processing result as a binary in the buffer specified by the argument.
- k2hdkc_get_direct_value type  
  Return the processing result as a binary.
- k2hdkc_get_str_value type  
  Store the processing result as a string in the buffer specified by the argument(Specify the key as a character string).
- k2hdkc_get_str_direct_value type  
  Return the processing result as a string(Specify the key as a character string).

Each of the above types has similar functions with differences such as arguments.
- k2hdkc_...  
  This function does not have any suffix. It requires basic arguments.
- k2hdkc_..._np  
  This function has **_np** suffix. This function does not perform attribute check(permission check).
- k2hdkc_..._wp  
  This function has **_wp** suffix. This function is a type that specifies a pass phrase.

In addition, depending on the connection type to chmpx slave node, it is divided into the following types.
- k2hdkc_...  
  This function specifies only the configuration of chmpx, connects to chmpx, and processes the command. This makes it easy to use Non-persistent chmpx connections.
- k2hdkc_full_...  
  This function specifies all the options of chmpx, connects to chmpx, and processes the command. This allows you to use Non-persistent chmpx connections with detailed settings.
- k2hdkc_pm_...  
  This function processes the command using the persistent chmpx connection handle.

#### Parameters
- config  
  Specify the configuration of the chmpx slave node(INI/YAML/JSON format file path, or JSON character string).
- ctlport  
  Specify the control port number of the chmpx slave node(It is necessary to specify it when multiple chmpx are running on the same server).
- cuk  
  Specify the CUK string of the chmpx slave node(It is necessary to specify it when multiple chmpx are running on the same server).
- is_auto_rejoin  
  Specify whether to reconnect automatically when the connection with the slave chmpx node is disconnected.
- no_giveup_rejoin  
  Case of specifying to reconnect automatically to the slave node of chmpx, specify the upper limit of retrying.
- pkey  
  For this argument, specify a pointer to a key which is a binary/string pointer.
- keylength  
  If the key is binary, specify the data length of the key.
- ppval  
  Specifies a pointer to the buffer that stores the value(as binary/string).
- pvallength  
  If the value is binary, specify the buffer length of the value.
- encpass  
  Specify the passphrase for decrypting the value.
- handle  
  Specify the handle of the chmpx connection used by the k2hdkc library.

#### Return Values
- k2hdkc_get_value / k2hdkc_get_str_value type  
  If there is no key or an error, false is returned. Returns true if it succeeds.  
  The reason for the failure can be obtained by Last Response Code.
- k2hdkc_get_direct_value type  
  If it succeeds, a pointer to the binary value is returned.  
  The reason for the failure can be obtained by Last Response Code.
- k2hdkc_get_str_direct_value type  
  If it succeeds, a pointer to the string value is returned.  
  The reason for the failure can be obtained by Last Response Code.

#### Examples
```
// do command
unsigned char* pval = NULL;
size_t         vallength = 0;
bool           result = k2hdkc_full_get_value(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin, pkey, keylength, &pval, &vallength);
dkcres_type_t  rescode = k2hdkc_get_lastres_code();

// check result
if(!result){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}
free(pval);
```

<!-- -----------------------------------------------------------　-->
***

### <a name="GETDIRECT"> Get Direct family(C I/F)
This is a function group that obtains the value of the specified key from the cluster of k2hdkc.
You can specify the position and size for the value to be acquired.

#### Format
- bool k2hdkc_da_get_value(const char* config, const unsigned char* pkey, size_t keylength, off_t getpos, size_t val_length, unsigned char** ppval, size_t* pvallength)
- unsigned char* k2hdkc_da_get_direct_value(const char* config, const unsigned char* pkey, size_t keylength, off_t getpos, size_t val_length, size_t* pvallength)
- bool k2hdkc_da_get_str_value(const char* config, const char* pkey, off_t getpos, size_t val_length, char** ppval)
- char* k2hdkc_da_get_str_direct_value(const char* config, const char* pkey, off_t getpos, size_t val_length)
 
- bool k2hdkc_full_da_get_value(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, off_t getpos, size_t val_length, unsigned char** ppval, size_t* pvallength)
- unsigned char* k2hdkc_full_da_get_direct_value(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, off_t getpos, size_t val_length, size_t* pvallength)
- bool k2hdkc_full_da_get_str_value(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, off_t getpos, size_t val_length, char** ppval)
- char* k2hdkc_full_da_get_str_direct_value(const char* conffile, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, off_t getpos, size_t val_length)
 
- bool k2hdkc_pm_da_get_value(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, off_t getpos, size_t val_length, unsigned char** ppval, size_t* pvallength)
- unsigned char* k2hdkc_pm_da_get_direct_value(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, off_t getpos, size_t val_length, size_t* pvallength)
- bool k2hdkc_pm_da_get_str_value(k2hdkc_chmpx_h handle, const char* pkey, off_t getpos, size_t val_length, char** ppval)
- char* k2hdkc_pm_da_get_str_direct_value(k2hdkc_chmpx_h handle, const char* pkey, off_t getpos, size_t val_length)

#### Description
These function groups are classified according to the following rules and classified into 4 types.
- k2hdkc_da_get_value type  
  Specify the key as binary, and specify the offset and get the value as binary. The value is stored in a pointer to the buffer passed as an argument.
- k2hdkc_da_get_direct_value type  
  Specify the key as binary, and specify the offset and get the value as binary. The value is returned as a pointer to the return value.
- k2hdkc_da_get_str_value type  
  Specify the key as string, and specify the offset and get the value as string. The value is stored in a pointer to the buffer passed as an argument.
- k2hdkc_da_get_str_direct_value type  
  Specify the key as string, and specify the offset and get the value as string. The value is returned as a pointer to the return value.

Each of the above types has similar functions with differences such as arguments.
- k2hdkc_...  
  This function specifies only the configuration of chmpx, connects to chmpx, and processes the command. This makes it easy to use Non-persistent chmpx connections.
- k2hdkc_full_...  
  This function specifies all the options of chmpx, connects to chmpx, and processes the command. This allows you to use Non-persistent chmpx connections with detailed settings.
- k2hdkc_pm_...  
  This function processes the command using the persistent chmpx connection handle.

#### Parameters
- config  
  Specify the configuration of the chmpx slave node(INI/YAML/JSON format file path, or JSON character string).
- ctlport  
  Specify the control port number of the chmpx slave node(It is necessary to specify it when multiple chmpx are running on the same server).
- cuk  
  Specify the CUK string of the chmpx slave node(It is necessary to specify it when multiple chmpx are running on the same server).
- is_auto_rejoin  
  Specify whether to reconnect automatically when the connection with the slave chmpx node is disconnected.
- no_giveup_rejoin  
  Case of specifying to reconnect automatically to the slave node of chmpx, specify the upper limit of retrying.
- pkey  
  For this argument, specify a pointer to a key which is a binary/string pointer.
- keylength  
  If the key is binary, specify the data length of the key.
- getpos  
  Specify the reading starting position of the value with an offset.
- ppval  
  Specifies a pointer to the buffer that stores the value(as binary/string).
- pvallength  
  If the value is binary, specify the buffer length of the value.
- handle  
  Specify the handle of the chmpx connection used by the k2hdkc library.

#### Return Values
- k2hdkc_da_get_value / k2hdkc_da_get_str_value type  
  If there is no key or an error, false is returned. Returns true if it succeeds.  
  The reason for the failure can be obtained by Last Response Code.
- k2hdkc_da_get_direct_value type  
  If it succeeds, a pointer to the binary value is returned.
  The reason for the failure can be obtained by Last Response Code.
- k2hdkc_da_get_str_direct_value type  
  If it succeeds, a pointer to the string value is returned.
  The reason for the failure can be obtained by Last Response Code.

#### Note
Get Direct family does not have a type that does not perform attribute check(permission check) and type that specifies pass phrase like Get family.

#### Examples
```
// get direct
unsigned char**    ppval    = NULL;
size_t        vallen    = 0;
bool        result    = k2hdkc_full_da_get_value(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin, pkey, keylen, offset, length, ppval, &vallen);
dkcres_type_t    rescode    = k2hdkc_get_lastres_code();

// check result
if(!result){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}
```

<!-- -----------------------------------------------------------　-->
***

### <a name="GETSUB"> Get Subkey family(C I/F)
This function specifies a key and obtains a list of Subkeys from a k2hdkc cluster.

#### Format
- bool k2hdkc_get_subkeys(const char* config, const unsigned char* pkey, size_t keylength, PK2HDKCKEYPCK* ppskeypck, int* pskeypckcnt)
- PK2HDKCKEYPCK k2hdkc_get_direct_subkeys(const char* config, const unsigned char* pkey, size_t keylength, int* pskeypckcnt)
- int k2hdkc_get_str_subkeys(const char* config, const char* pkey, char*** ppskeyarray)
- char** k2hdkc_get_str_direct_subkeys(const char* config, const char* pkey)
 
- bool k2hdkc_get_subkeys_np(const char* config, const unsigned char* pkey, size_t keylength, PK2HDKCKEYPCK* ppskeypck, int* pskeypckcnt)
- PK2HDKCKEYPCK k2hdkc_get_direct_subkeys_np(const char* config, const unsigned char* pkey, size_t keylength, int* pskeypckcnt)
- int k2hdkc_get_str_subkeys_np(const char* config, const char* pkey, char*** ppskeyarray)
- char** k2hdkc_get_str_direct_subkeys_np(const char* config, const char* pkey)
 
- bool k2hdkc_full_get_subkeys(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, PK2HDKCKEYPCK* ppskeypck, int* pskeypckcnt)
- PK2HDKCKEYPCK k2hdkc_full_get_direct_subkeys(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, int* pskeypckcnt)
- int k2hdkc_full_get_str_subkeys(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, char*** ppskeyarray)
- char** k2hdkc_full_get_str_direct_subkeys(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey)
 
- bool k2hdkc_full_get_subkeys_np(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, PK2HDKCKEYPCK* ppskeypck, int* pskeypckcnt)
- PK2HDKCKEYPCK k2hdkc_full_get_direct_subkeys_np(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, int* pskeypckcnt)
- int k2hdkc_full_get_str_subkeys_np(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, char*** ppskeyarray)
- char** k2hdkc_full_get_str_direct_subkeys_np(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey)
 
- bool k2hdkc_pm_get_subkeys(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, PK2HDKCKEYPCK* ppskeypck, int* pskeypckcnt)
- PK2HDKCKEYPCK k2hdkc_pm_get_direct_subkeys(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, int* pskeypckcnt)
- int k2hdkc_pm_get_str_subkeys(k2hdkc_chmpx_h handle, const char* pkey, char*** ppskeyarray)
- char** k2hdkc_pm_get_str_direct_subkeys(k2hdkc_chmpx_h handle, const char* pkey)
 
- bool k2hdkc_pm_get_subkeys_np(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, PK2HDKCKEYPCK* ppskeypck, int* pskeypckcnt)
- PK2HDKCKEYPCK k2hdkc_pm_get_direct_subkeys_np(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, int* pskeypckcnt)
- int k2hdkc_pm_get_str_subkeys_np(k2hdkc_chmpx_h handle, const char* pkey, char*** ppskeyarray)
- char** k2hdkc_pm_get_str_direct_subkeys_np(k2hdkc_chmpx_h handle, const char* pkey)

#### Description
These function groups are classified according to the following rules and classified into 4 types.
- k2hdkc_get_subkeys type  
  Specify the key as binary, and get PK2HDKCKEYPCK. The value is stored in a pointer to the buffer passed as an argument.
- k2hdkc_get_direct_subkeys type  
  Specify the key as binary, and get PK2HDKCKEYPCK. The value is returned as a pointer to the return value.
- k2hdkc_get_str_subkeys type  
  Specify the key as string, and get string array. The value is stored in a pointer to the buffer passed as an argument.
- k2hdkc_get_str_direct_subkeys type  
  Specify the key as binary, and get string array(last of array is NULL). The value is returned as a pointer to the return value.

Each of the above types has similar functions with differences such as arguments.
- k2hdkc_...  
  This function does not have any suffix. It requires basic arguments.
- k2hdkc_..._np  
  This function has **_np** suffix. This function does not perform attribute check(permission check).

In addition, depending on the connection type to chmpx slave node, it is divided into the following types.
- k2hdkc_...  
  This function specifies only the configuration of chmpx, connects to chmpx, and processes the command. This makes it easy to use Non-persistent chmpx connections.
- k2hdkc_full_...  
  This function specifies all the options of chmpx, connects to chmpx, and processes the command. This allows you to use Non-persistent chmpx connections with detailed settings.
- k2hdkc_pm_...  
  This function processes the command using the persistent chmpx connection handle.

#### Parameters
- config  
  Specify the configuration of the chmpx slave node(INI/YAML/JSON format file path, or JSON character string).
- ctlport  
  Specify the control port number of the chmpx slave node(It is necessary to specify it when multiple chmpx are running on the same server).
- cuk  
  Specify the CUK string of the chmpx slave node(It is necessary to specify it when multiple chmpx are running on the same server).
- is_auto_rejoin  
  Specify whether to reconnect automatically when the connection with the slave chmpx node is disconnected.
- no_giveup_rejoin  
  Case of specifying to reconnect automatically to the slave node of chmpx, specify the upper limit of retrying.
- pkey  
  For this argument, specify a pointer to a key which is a binary/string pointer.
- keylength  
  If the key is binary, specify the data length of the key.
- ppskeypck  
  Specify a pointer to obtain the list of acquired Subkey as a binary array.
- pskeypckcnt  
  It is a pointer to return the number of acquired Subkey lists.
- ppskeyarray  
  It is a pointer to obtain the list of acquired Subkey as a string array.
- handle  
  Specify the handle of the chmpx connection used by the k2hdkc library.

#### Return Values
- k2hdkc_get_subkeys / k2hdkc_get_str_subkeys type  
  If there is no key or an error, false is returned. Returns true if it succeeds.  
  The reason for the failure can be obtained by Last Response Code.
- k2hdkc_get_direct_subkeys type  
  If it succeeds, the PK2HDKCKEYPCK pointer of the Subkey list is returned.
  The reason for the failure can be obtained by Last Response Code.
- k2hdkc_get_str_direct_subkeys type  
  On success, a pointer to the string array of values is returned.
  The reason for the failure can be obtained by Last Response Code.

#### About structure
The base of the K2HDKCKEYPCK structure is defined in [K2HASH](https://k2hash.antpick.ax/).
That structure is typedef by the k2hdkc library.
You can use the following macro function(provided by k2hdkc) or k2hash API to release a pointer to this structured area and a pointer to an array of structures.
The structure is as follows.

```
// structure
typedef struct k2h_key_pack{
    unsigned char*    pkey;
    size_t            length;
}K2HKEYPCK, *PK2HKEYPCK;
 
// typedefs by k2hdkc
typedef K2HKEYPCK                   K2HDKCKEYPCK;
typedef PK2HKEYPCK                  PK2HDKCKEYPCK;
 
// Free function
extern bool k2h_free_keypack(PK2HKEYPCK pkeys, int keycnt);
extern bool k2h_free_keyarray(char** pkeys);
 
// Free macro presented by k2hdkc
#define    DKC_FREE_KEYPACK            k2h_free_keypack
#define    DKC_FREE_KEYARRAY           k2h_free_keyarray
```

#### Examples
```
PK2HDKCKEYPCK    pskeypck = NULL;
int        skeypckcnt = 0;
bool        result = k2hdkc_full_get_subkeys(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin, pkey, keylength, &pskeypck, &skeypckcnt);
dkcres_type_t    rescode = k2hdkc_get_lastres_code();
 
// check result
if(!result){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}
```

<!-- -----------------------------------------------------------　-->
***

### <a name="GETATTR"> Get Attribute family(C I/F)
Function group that specifies a key and acquires an attribute from a k2hdkc cluster.

#### Format
- bool k2hdkc_get_attrs(const char* config, const unsigned char* pkey, size_t keylength, PK2HDKCATTRPCK* ppattrspck, int* pattrspckcnt)
- PK2HDKCATTRPCK k2hdkc_get_direct_attrs(const char* config, const unsigned char* pkey, size_t keylength, int* pattrspckcnt)
- PK2HDKCATTRPCK k2hdkc_get_str_direct_attrs(const char* config, const char* pkey, int* pattrspckcnt)
 
- bool k2hdkc_full_get_attrs(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, PK2HDKCATTRPCK* ppattrspck, int* pattrspckcnt)
- PK2HDKCATTRPCK k2hdkc_full_get_direct_attrs(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, int* pattrspckcnt)
- PK2HDKCATTRPCK k2hdkc_full_get_str_direct_attrs(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, int* pattrspckcnt)
 
- bool k2hdkc_pm_get_attrs(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, PK2HDKCATTRPCK* ppattrspck, int* pattrspckcnt)
- PK2HDKCATTRPCK k2hdkc_pm_get_direct_attrs(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, int* pattrspckcnt)
- PK2HDKCATTRPCK k2hdkc_pm_get_str_direct_attrs(k2hdkc_chmpx_h handle, const char* pkey, int* pattrspckcnt)

#### Description
These function groups are classified according to the following rules and classified into 4 types.
- k2hdkc_get_attrs type  
  Specify the key as binary, and get PK2HDKCATTRPCK. The value is stored in a pointer to the buffer passed as an argument.
- k2hdkc_get_direct_attrs type  
  Specify the key as binary, and get PK2HDKCATTRPCK. The value is returned as a pointer to the return value.
- k2hdkc_get_str_direct_attrs type  
  Specify the key as string, and get PK2HDKCATTRPCK. The value is returned as a pointer to the return value.

Each of the above types has similar functions with differences such as arguments.
- k2hdkc_...  
  This function specifies only the configuration of chmpx, connects to chmpx, and processes the command. This makes it easy to use Non-persistent chmpx connections.
- k2hdkc_full_...  
  This function specifies all the options of chmpx, connects to chmpx, and processes the command. This allows you to use Non-persistent chmpx connections with detailed settings.
- k2hdkc_pm_...  
  This function processes the command using the persistent chmpx connection handle.

#### Parameters
- config  
  Specify the configuration of the chmpx slave node(INI/YAML/JSON format file path, or JSON character string).
- ctlport  
  Specify the control port number of the chmpx slave node(It is necessary to specify it when multiple chmpx are running on the same server).
- cuk  
  Specify the CUK string of the chmpx slave node(It is necessary to specify it when multiple chmpx are running on the same server).
- is_auto_rejoin  
  Specify whether to reconnect automatically when the connection with the slave chmpx node is disconnected.
- no_giveup_rejoin  
  Case of specifying to reconnect automatically to the slave node of chmpx, specify the upper limit of retrying.
- pkey  
  For this argument, specify a pointer to a key which is a binary/string pointer.
- keylength  
  If the key is binary, specify the data length of the key.
- ppattrspck  
  Specifies a pointer to get the attribute list as a structure array.
- pattrspckcnt  
  Specifies a pointer to return the number of attributes list.
- handle  
  Specify the handle of the chmpx connection used by the k2hdkc library.

#### Return Values
- k2hdkc_get_attrs type  
  If there is no key or an error, false is returned. Returns true if it succeeds.  
  The reason for the failure can be obtained by Last Response Code.
- k2hdkc_get_direct_attrs / k2hdkc_get_str_direct_attrs type  
  If it succeeds, the PK2HDKCATTRPCK pointer of the attribute list is returned.
  The reason for the failure can be obtained by Last Response Code.

#### About structure
The base of the K2HDKCATTRPCK structure is defined in [K2HASH](https://k2hash.antpick.ax/).
That structure is typedef by the k2hdkc library.
You can use the following macro function(provided by k2hdkc) or k2hash API to release a pointer to this structured area and a pointer to an array of structures.
The structure is as follows.
```
// for attributes list
typedef struct k2h_attr_pack{
    unsigned char*    pkey;
    size_t            keylength;
    unsigned char*    pval;
    size_t            vallength;
}K2HATTRPCK, *PK2HATTRPCK;
 
// typedefs by k2hdkc
typedef K2HATTRPCK                    K2HDKCATTRPCK;
typedef PK2HATTRPCK                    PK2HDKCATTRPCK;
 
// Free attributes array
extern bool k2h_free_attrpack(PK2HATTRPCK pattrs, int attrcnt);
 
// Free attributes macro presented by k2hdkc
#define    DKC_FREE_ATTRPACK    k2h_free_attrpack
```

#### Examples
```
PK2HDKCATTRPCK    pattrspck    = NULL;
int        attrspckcnt    = 0;
bool        result = k2hdkc_full_get_attrs(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, &pattrspck, &attrspckcnt);
dkcres_type_t    rescode= k2hdkc_get_lastres_code();
 
// check result
if(!result){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
    DKC_FREE_ATTRPACK(pattrspck, attrspckcnt);
}
```

<!-- -----------------------------------------------------------　-->
***

### <a name="SET"> Set family(C I/F)
Function group that specifies a key and set a value to a k2hdkc cluster.

#### Format
- bool k2hdkc_set_value(const char* config, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength)
- bool k2hdkc_set_str_value(const char* config, const char* pkey, const char* pval)
 
- bool k2hdkc_full_set_value(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength)
- bool k2hdkc_full_set_str_value(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, const char* pval)
 
- bool k2hdkc_pm_set_value(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength)
- bool k2hdkc_pm_set_str_value(k2hdkc_chmpx_h handle, const char* pkey, const char* pval)
 
- bool k2hdkc_set_value_wa(const char* config, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, bool rmsubkeylist, const char* encpass, const time_t* expire)
- bool k2hdkc_set_str_value_wa(const char* config, const char* pkey, const char* pval, bool rmsubkeylist, const char* encpass, const time_t* expire)
 
- bool k2hdkc_full_set_value_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, bool rmsubkeylist, const char* encpass, const time_t* expire)
- bool k2hdkc_full_set_str_value_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, const char* pval, bool rmsubkeylist, const char* encpass, const time_t* expire)
 
- bool k2hdkc_pm_set_value_wa(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, bool rmsubkeylist, const char* encpass, const time_t* expire)
- bool k2hdkc_pm_set_str_value_wa(k2hdkc_chmpx_h handle, const char* pkey, const char* pval, bool rmsubkeylist, const char* encpass, const time_t* expire)

#### Description
These function groups are classified according to the following rules and classified into 2 types.
- k2hdkc_set_value type  
  Specifies the binary key and sets the binary value.
- k2hdkc_set_str_value type  
  Specifies the string key and sets the string value.

Each of the above types has similar functions with differences such as arguments.
- k2hdkc_...  
  This function does not have any suffix. It requires basic arguments.
- k2hdkc_..._wa  
  This function has **_wa** suffix. This function is a type that specifies a pass phrase and expire.

In addition, depending on the connection type to chmpx slave node, it is divided into the following types.
- k2hdkc_...  
  This function specifies only the configuration of chmpx, connects to chmpx, and processes the command. This makes it easy to use Non-persistent chmpx connections.
- k2hdkc_full_...  
  This function specifies all the options of chmpx, connects to chmpx, and processes the command. This allows you to use Non-persistent chmpx connections with detailed settings.
- k2hdkc_pm_...  
  This function processes the command using the persistent chmpx connection handle.

#### Parameters
- config  
  Specify the configuration of the chmpx slave node(INI/YAML/JSON format file path, or JSON character string).
- ctlport  
  Specify the control port number of the chmpx slave node(It is necessary to specify it when multiple chmpx are running on the same server).
- cuk  
  Specify the CUK string of the chmpx slave node(It is necessary to specify it when multiple chmpx are running on the same server).
- is_auto_rejoin  
  Specify whether to reconnect automatically when the connection with the slave chmpx node is disconnected.
- no_giveup_rejoin  
  Case of specifying to reconnect automatically to the slave node of chmpx, specify the upper limit of retrying.
- pkey  
  For this argument, specify a pointer to a key which is a binary/string pointer.
- keylength  
  If the key is binary, specify the data length of the key.
- pval  
  Specifies a pointer for the value(binary/string).
- vallength  
  If the value is binary, specifies length of binary value.
- rmsubkeylist  
  If the key has a subkey, specify whether to delete the list of subkeys. Even when deleting, do not delete the subkey and its value.
- encpass  
  Specify the passphrase for encrypting the value.
- expire  
  To set an expiration date for a value, specify the expiration date.
- handle  
  Specify the handle of the chmpx connection used by the k2hdkc library.

#### Return Values
Returns true if it succeeds, false if it fails.

#### Examples
```
bool        result = k2hdkc_full_set_value(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin, pKey, KeyLen, pVal, ValLen);
dkcres_type_t    rescode = k2hdkc_get_lastres_code();
 
// check result
if(!result){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}
```

<!-- -----------------------------------------------------------　-->
***

### <a name="SETDIRECT"> Set Direct family(C I/F)
Function group that specifies a key, position/length of the value and set a value to a k2hdkc cluster.

#### Format
- bool k2hdkc_da_set_value(const char* config, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, const off_t setpos)
- bool k2hdkc_da_set_str_value(const char* config, const char* pkey, const char* pval, const off_t setpos)
 
- bool k2hdkc_full_da_set_value(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, const off_t setpos)
- bool k2hdkc_full_da_set_str_value(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, const char* pval, const off_t setpos)
 
- bool k2hdkc_pm_da_set_value(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, const off_t setpos)
- bool k2hdkc_pm_da_set_str_value(k2hdkc_chmpx_h handle, const char* pkey, const char* pval, const off_t setpos)

#### Description
These function groups are classified according to the following rules and classified into 2 types.
- k2hdkc_da_set_value type  
  This function specifies the binary key, the offset to set, and sets the binary value.
- k2hdkc_da_set_str_value type  
  This function specifies the string key, the offset to set, and sets the string value.

Each of the above types has similar functions with differences such as arguments.
- k2hdkc_...  
  This function specifies only the configuration of chmpx, connects to chmpx, and processes the command. This makes it easy to use Non-persistent chmpx connections.
- k2hdkc_full_...  
  This function specifies all the options of chmpx, connects to chmpx, and processes the command. This allows you to use Non-persistent chmpx connections with detailed settings.
- k2hdkc_pm_...  
  This function processes the command using the persistent chmpx connection handle.

#### Parameters
- config  
  Specify the configuration of the chmpx slave node(INI/YAML/JSON format file path, or JSON character string).
- ctlport  
  Specify the control port number of the chmpx slave node(It is necessary to specify it when multiple chmpx are running on the same server).
- cuk  
  Specify the CUK string of the chmpx slave node(It is necessary to specify it when multiple chmpx are running on the same server).
- is_auto_rejoin  
  Specify whether to reconnect automatically when the connection with the slave chmpx node is disconnected.
- no_giveup_rejoin  
  Case of specifying to reconnect automatically to the slave node of chmpx, specify the upper limit of retrying.
- pkey  
  For this argument, specify a pointer to a key which is a binary/string pointer.
- keylength  
  If the key is binary, specify the data length of the key.
- pval  
  Specifies a pointer for the value(binary/string).
- vallength  
  If the value is binary, specifies length of binary value.
- setpos  
  Specifies offset for start of value.
- handle  
  Specify the handle of the chmpx connection used by the k2hdkc library.

#### Return Values
Returns true if it succeeds, false if it fails.

#### Examples
```
bool        result    = k2hdkc_full_da_set_value(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin, pkey, keylen, pval, vallen, offset);
dkcres_type_t    rescode    = k2hdkc_get_lastres_code();
 
// check result
if(!result){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}
```

<!-- -----------------------------------------------------------　-->
***

### <a name="SETSUB"> Set Subkey family(C I/F)
Function group that specifies a key, and set a subkey list to a k2hdkc cluster.

#### Format
- bool k2hdkc_set_subkeys(const char* config, const unsigned char* pkey, size_t keylength, const PK2HDKCKEYPCK pskeypck, int skeypckcnt)
- bool k2hdkc_set_str_subkeys(const char* config, const char* pkey, const char** pskeyarray)
 
- bool k2hdkc_full_set_subkeys(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, const PK2HDKCKEYPCK pskeypck, int skeypckcnt)
- bool k2hdkc_full_set_str_subkeys(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, const char** pskeyarray)
 
- bool k2hdkc_pm_set_subkeys(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, const PK2HDKCKEYPCK pskeypck, int skeypckcnt)
- bool k2hdkc_pm_set_str_subkeys(k2hdkc_chmpx_h handle, const char* pkey, const char** pskeyarray)

#### Description
These function groups are classified according to the following rules and classified into 2 types.
- k2hdkc_set_subkeys type  
  Specify the key as binary and PK2HDKCKEYPCK pointer, and set Subkey list to it.
- k2hdkc_set_str_subkeys type  
  Specify the key as string and string array pointer of Subkeys, and set Subkey list to it.

Each of the above types has similar functions with differences such as arguments.
- k2hdkc_...  
  This function specifies only the configuration of chmpx, connects to chmpx, and processes the command. This makes it easy to use Non-persistent chmpx connections.
- k2hdkc_full_...  
  This function specifies all the options of chmpx, connects to chmpx, and processes the command. This allows you to use Non-persistent chmpx connections with detailed settings.
- k2hdkc_pm_...  
  This function processes the command using the persistent chmpx connection handle.

#### Parameters
- config  
  Specify the configuration of the chmpx slave node(INI/YAML/JSON format file path, or JSON character string).
- ctlport  
  Specify the control port number of the chmpx slave node(It is necessary to specify it when multiple chmpx are running on the same server).
- cuk  
  Specify the CUK string of the chmpx slave node(It is necessary to specify it when multiple chmpx are running on the same server).
- is_auto_rejoin  
  Specify whether to reconnect automatically when the connection with the slave chmpx node is disconnected.
- no_giveup_rejoin  
  Case of specifying to reconnect automatically to the slave node of chmpx, specify the upper limit of retrying.
- pkey  
  For this argument, specify a pointer to a key which is a binary/string pointer.
- keylength  
  If the key is binary, specify the data length of the key.
- pskeypck  
  Specify a pointer of PK2HDKCKEYPCK for Subkey list.
- skeypckcnt  
  Specifies a count of Subkey list in PK2HDKCKEYPCK pointer.
- pskeyarray  
  Specify a pointer of string Subkeys array pointer(the end of array is NULL).
- handle  
  Specify the handle of the chmpx connection used by the k2hdkc library.

#### Return Values
Returns true if it succeeds, false if it fails.

#### Examples
```
bool        result    = k2hdkc_full_set_subkeys(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin, pPKey, PKeyLen, pskeypck, skeypckcnt);
dkcres_type_t    rescode = k2hdkc_get_lastres_code();
 
// check result
if(!result){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}
```

<!-- -----------------------------------------------------------　-->
***

### <a name="CLEARSUB"> Clear Subkey family(C I/F)
Function group that specifies a key, and clear subkey list to a k2hdkc cluster.

#### Format
- bool k2hdkc_clear_subkeys(const char* config, const unsigned char* pkey, size_t keylength)
- bool k2hdkc_clear_str_subkeys(const char* config, const char* pkey)
 
- bool k2hdkc_full_clear_subkeys(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength)
- bool k2hdkc_full_clear_str_subkeys(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey)
 
- bool k2hdkc_pm_clear_subkeys(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength)
- bool k2hdkc_pm_clear_str_subkeys(k2hdkc_chmpx_h handle, const char* pkey)

#### Description
These function groups are classified according to the following rules and classified into 2 types.
- k2hdkc_clear_subkeys type  
  Specify the key as binary, and clear Subkey list to it.
- k2hdkc_clear_str_subkeys type  
  Specify the key as string, and clear Subkey list to it.

Each of the above types has similar functions with differences such as arguments.
- k2hdkc_...  
  This function specifies only the configuration of chmpx, connects to chmpx, and processes the command. This makes it easy to use Non-persistent chmpx connections.
- k2hdkc_full_...  
  This function specifies all the options of chmpx, connects to chmpx, and processes the command. This allows you to use Non-persistent chmpx connections with detailed settings.
- k2hdkc_pm_...  
  This function processes the command using the persistent chmpx connection handle.

#### Parameters
- config  
  Specify the configuration of the chmpx slave node(INI/YAML/JSON format file path, or JSON character string).
- ctlport  
  Specify the control port number of the chmpx slave node(It is necessary to specify it when multiple chmpx are running on the same server).
- cuk  
  Specify the CUK string of the chmpx slave node(It is necessary to specify it when multiple chmpx are running on the same server).
- is_auto_rejoin  
  Specify whether to reconnect automatically when the connection with the slave chmpx node is disconnected.
- no_giveup_rejoin  
  Case of specifying to reconnect automatically to the slave node of chmpx, specify the upper limit of retrying.
- pkey  
  For this argument, specify a pointer to a key which is a binary/string pointer.
- keylength  
  If the key is binary, specify the data length of the key.
- handle  
  Specify the handle of the chmpx connection used by the k2hdkc library.

#### Return Values
Returns true if it succeeds, false if it fails.

#### Examples
```
bool        result    = k2hdkc_full_clear_subkeys(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin, pPKey, PKeyLen);
dkcres_type_t    rescode = k2hdkc_get_lastres_code();
 
// check result
if(!result){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}
```

<!-- -----------------------------------------------------------　-->
***

### <a name="ADDSUB"> Add Subkey family(C I/F)
Function group that specifies the parent key and the Subkey which is a child of the parent key, and set/create Subkey and value, and set Subkey into parent key's subkey list.

#### Format
- bool k2hdkc_set_subkey(const char* config, const unsigned char* pkey, size_t keylength, const unsigned char* psubkey, size_t subkeylength, const unsigned char* pskeyval, size_t skeyvallength)
- bool k2hdkc_set_str_subkey(const char* config, const char* pkey, const char* psubkey, const char* pskeyval)
 
- bool k2hdkc_full_set_subkey(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, const unsigned char* psubkey, size_t subkeylength, const unsigned char* pskeyval, size_t skeyvallength)
- bool k2hdkc_full_set_str_subkey(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, const char* psubkey, const char* pskeyval)
 
- bool k2hdkc_pm_set_subkey(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, const unsigned char* psubkey, size_t subkeylength, const unsigned char* pskeyval, size_t skeyvallength)
- bool k2hdkc_pm_set_str_subkey(k2hdkc_chmpx_h handle, const char* pkey, const char* psubkey, const char* pskeyval)
 
- bool k2hdkc_set_subkey_wa(const char* config, const unsigned char* pkey, size_t keylength, const unsigned char* psubkey, size_t subkeylength, const unsigned char* pskeyval, size_t skeyvallength, bool checkattr, const char* encpass, const time_t* expire)
- bool k2hdkc_set_str_subkey_wa(const char* config, const char* pkey, const char* psubkey, const char* pskeyval, bool checkattr, const char* encpass, const time_t* expire)
 
- bool k2hdkc_full_set_subkey_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, const unsigned char* psubkey, size_t subkeylength, const unsigned char* pskeyval, size_t skeyvallength, bool checkattr, const char* encpass, const time_t* expire)
- bool k2hdkc_full_set_str_subkey_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, const char* psubkey, const char* pskeyval, bool checkattr, const char* encpass, const time_t* expire)
 
- bool k2hdkc_pm_set_subkey_wa(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, const unsigned char* psubkey, size_t subkeylength, const unsigned char* pskeyval, size_t skeyvallength, bool checkattr, const char* encpass, const time_t* expire)
- bool k2hdkc_pm_set_str_subkey_wa(k2hdkc_chmpx_h handle, const char* pkey, const char* psubkey, const char* pskeyval, bool checkattr, const char* encpass, const time_t* expire)

#### Description
These function groups are classified according to the following rules and classified into 2 types.
- k2hdkc_set_subkey type  
  Specify the parent key/key/value as binary, and set these.
- k2hdkc_set_str_subkey type  
  Specify the parent key/key/value as string, and set these.

Each of the above types has similar functions with differences such as arguments.
- k2hdkc_...  
  This function does not have any suffix. It requires basic arguments.
- k2hdkc_..._wa  
  This function has **_wa** suffix. This function is a type that specifies a pass phrase and expire.

In addition, depending on the connection type to chmpx slave node, it is divided into the following types.
- k2hdkc_...  
  This function specifies only the configuration of chmpx, connects to chmpx, and processes the command. This makes it easy to use Non-persistent chmpx connections.
- k2hdkc_full_...  
  This function specifies all the options of chmpx, connects to chmpx, and processes the command. This allows you to use Non-persistent chmpx connections with detailed settings.
- k2hdkc_pm_...  
  This function processes the command using the persistent chmpx connection handle.

#### Parameters
- config  
  Specify the configuration of the chmpx slave node(INI/YAML/JSON format file path, or JSON character string).
- ctlport  
  Specify the control port number of the chmpx slave node(It is necessary to specify it when multiple chmpx are running on the same server).
- cuk  
  Specify the CUK string of the chmpx slave node(It is necessary to specify it when multiple chmpx are running on the same server).
- is_auto_rejoin  
  Specify whether to reconnect automatically when the connection with the slave chmpx node is disconnected.
- no_giveup_rejoin  
  Case of specifying to reconnect automatically to the slave node of chmpx, specify the upper limit of retrying.
- pkey  
  For this argument, specify a pointer to a key which is a binary/string pointer.
- keylength  
  If the key is binary, specify the data length of the key.
- psubkey  
  Specifies a pointer for the Subkey(binary/string).
- subkeylength  
  Specifies the length of Subkey as binary.
- pskeyval  
  Specifies a pointer for the value(binary/string).
- skeyvallength  
  Specifies the length of value as binary.
- checkattr  
  When adding a subkey to a parent key, specify whether to check the attribute(passphrase, expiration date) of the parent key.
- encpass  
  Specify the passphrase for encrypting the value.
- expire  
  To set an expiration date for a value, specify the expiration date.
- handle  
  Specify the handle of the chmpx connection used by the k2hdkc library.

#### Return Values
Returns true if it succeeds, false if it fails.

#### Examples
```
bool        result    = k2hdkc_full_set_subkeys(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin, pPKey, PKeyLen, pskeypck, skeypckcnt);
dkcres_type_t    rescode = k2hdkc_get_lastres_code();
 
// check result
if(!result){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}
```

<!-- -----------------------------------------------------------　-->
***

### <a name="SETALL"> Set All family(C I/F)
Function group that specifies the key, and set/create all data(value/Subkey list/attribute).

#### Format
- bool k2hdkc_set_all(const char* config, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, const PK2HDKCKEYPCK pskeypck, int skeypckcnt)
- bool k2hdkc_set_str_all(const char* config, const char* pkey, const char* pval, const char** pskeyarray)
 
- bool k2hdkc_full_set_all(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, const PK2HDKCKEYPCK pskeypck, int skeypckcnt)
- bool k2hdkc_full_set_str_all(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, const char* pval, const char** pskeyarray)
 
- bool k2hdkc_pm_set_all(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, const PK2HDKCKEYPCK pskeypck, int skeypckcnt)
- bool k2hdkc_pm_set_str_all(k2hdkc_chmpx_h handle, const char* pkey, const char* pval, const char** pskeyarray)
 
- bool k2hdkc_set_all_wa(const char* config, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, const PK2HDKCKEYPCK pskeypck, int skeypckcnt, const char* encpass, const time_t* expire)
- bool k2hdkc_set_str_all_wa(const char* config, const char* pkey, const char* pval, const char** pskeyarray, const char* encpass, const time_t* expire)
 
- bool k2hdkc_full_set_all_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, const PK2HDKCKEYPCK pskeypck, int skeypckcnt, const char* encpass, const time_t* expire)
- bool k2hdkc_full_set_str_all_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, const char* pval, const char** pskeyarray, const char* encpass, const time_t* expire)
 
- bool k2hdkc_pm_set_all_wa(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, const PK2HDKCKEYPCK pskeypck, int skeypckcnt, const char* encpass, const time_t* expire)
- bool k2hdkc_pm_set_str_all_wa(k2hdkc_chmpx_h handle, const char* pkey, const char* pval, const char** pskeyarray, const char* encpass, const time_t* expire)

#### Description
These function groups are classified according to the following rules and classified into 2 types.
- k2hdkc_set_all type  
  Specify the key as binary and PK2HDKCKEYPCK pointer, and set Subkey list to it.
- k2hdkc_set_str_all type  
  Specify the key as string and string array pointer of Subkeys, and set Subkey list to it.

Each of the above types has similar functions with differences such as arguments.
- k2hdkc_...  
  This function does not have any suffix. It requires basic arguments.
- k2hdkc_..._wa  
  This function has **_wa** suffix. This function is a type that specifies a pass phrase and expire.

In addition, depending on the connection type to chmpx slave node, it is divided into the following types.
- k2hdkc_...  
  This function specifies only the configuration of chmpx, connects to chmpx, and processes the command. This makes it easy to use Non-persistent chmpx connections.
- k2hdkc_full_...  
  This function specifies all the options of chmpx, connects to chmpx, and processes the command. This allows you to use Non-persistent chmpx connections with detailed settings.
- k2hdkc_pm_...  
  This function processes the command using the persistent chmpx connection handle.

#### Parameters
- config  
  Specify the configuration of the chmpx slave node(INI/YAML/JSON format file path, or JSON character string).
- ctlport  
  Specify the control port number of the chmpx slave node(It is necessary to specify it when multiple chmpx are running on the same server).
- cuk  
  Specify the CUK string of the chmpx slave node(It is necessary to specify it when multiple chmpx are running on the same server).
- is_auto_rejoin  
  Specify whether to reconnect automatically when the connection with the slave chmpx node is disconnected.
- no_giveup_rejoin  
  Case of specifying to reconnect automatically to the slave node of chmpx, specify the upper limit of retrying.
- pkey  
  For this argument, specify a pointer to a key which is a binary/string pointer.
- keylength  
  If the key is binary, specify the data length of the key.
- pval  
  Specifies a pointer for the value(binary/string).
- vallength  
  If the value is binary, specifies length of binary value.
- pskeypck  
  Specify a pointer of PK2HDKCKEYPCK for Subkey list.
- skeypckcnt  
  Specifies a count of Subkey list in PK2HDKCKEYPCK pointer.
- pskeyarray  
  Specify a pointer of string Subkeys array pointer(the end of array is NULL).
- encpass  
  Specify the passphrase for encrypting the value.
- expire  
  To set an expiration date for a value, specify the expiration date.
- handle  
  Specify the handle of the chmpx connection used by the k2hdkc library.

#### Return Values
Returns true if it succeeds, false if it fails.

#### Examples
```
bool        result = k2hdkc_full_set_all(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin, pKey, KeyLen, pVal, ValLen, NULL, 0);
dkcres_type_t    rescode = k2hdkc_get_lastres_code();
 
// check result
if(!result){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}
```

<!-- -----------------------------------------------------------　-->
***

### <a name="REMOVE"> Remove Key family(C I/F)
Function group that specifies the key, and remove it with/without key's Subkeys.

#### Format
- bool k2hdkc_remove_all(const char* config, const unsigned char* pkey, size_t keylength)
- bool k2hdkc_remove_str_all(const char* config, const char* pkey)
 
- bool k2hdkc_full_remove_all(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength)
- bool k2hdkc_full_remove_str_all(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey)
 
- bool k2hdkc_pm_remove_all(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength)
- bool k2hdkc_pm_remove_str_all(k2hdkc_chmpx_h handle, const char* pkey)
 
- bool k2hdkc_remove(const char* config, const unsigned char* pkey, size_t keylength)
- bool k2hdkc_remove_str(const char* config, const char* pkey)
 
- bool k2hdkc_full_remove(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength)
- bool k2hdkc_full_remove_str(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey)
 
- bool k2hdkc_pm_remove(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength)
- bool k2hdkc_pm_remove_str(k2hdkc_chmpx_h handle, const char* pkey)

#### Description
These function groups are classified according to the following rules and classified into 4 kinds(2 types).
- k2hdkc_remove_all type  
  Specify the key as binary, and remove it and remove all key's Subkeys.
- k2hdkc_remove type  
  Specify the key as binary, and remove only it(not remove it's Subkeys)).
- k2hdkc_remove_str_all type  
  Specify the key as string, and remove it and remove all key's Subkeys.
- k2hdkc_remove_str type  
  Specify the key as string, and remove only it(not remove it's Subkeys)).

Each of the above types has similar functions with differences such as arguments.
- k2hdkc_...  
  This function specifies only the configuration of chmpx, connects to chmpx, and processes the command. This makes it easy to use Non-persistent chmpx connections.
- k2hdkc_full_...  
  This function specifies all the options of chmpx, connects to chmpx, and processes the command. This allows you to use Non-persistent chmpx connections with detailed settings.
- k2hdkc_pm_...  
  This function processes the command using the persistent chmpx connection handle.

#### Parameters
- config  
   Specify the configuration of the chmpx slave node(INI/YAML/JSON format file path, or JSON character string).
- ctlport  
  Specify the control port number of the chmpx slave node(It is necessary to specify it when multiple chmpx are running on the same server).
- cuk  
  Specify the CUK string of the chmpx slave node(It is necessary to specify it when multiple chmpx are running on the same server).
- is_auto_rejoin  
  Specify whether to reconnect automatically when the connection with the slave chmpx node is disconnected.
- no_giveup_rejoin  
  Case of specifying to reconnect automatically to the slave node of chmpx, specify the upper limit of retrying.
- pkey  
  For this argument, specify a pointer to a key which is a binary/string pointer.
- keylength  
  If the key is binary, specify the data length of the key.
- handle  
  Specify the handle of the chmpx connection used by the k2hdkc library.

#### Return Values
Returns true if it succeeds, false if it fails.

#### Examples
```
bool        result = k2hdkc_full_remove(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin, pkey, keylen);
dkcres_type_t    rescode = k2hdkc_get_lastres_code();
 
// check result
if(!result){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}
```


<!-- -----------------------------------------------------------　-->
***

### <a name="REMOVESUB"> Remove Subkey family(C I/F)
Function group that specifies the parent key and Subkey, and remove Subkey.

#### Format
- bool k2hdkc_remove_subkey(const char* config, const unsigned char* pkey, size_t keylength, const unsigned char* psubkey, size_t subkeylength, bool is_nest)
- bool k2hdkc_remove_str_subkey(const char* config, const char* pkey, const char* psubkey, bool is_nest)
 
- bool k2hdkc_full_remove_subkey(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, const unsigned char* psubkey, size_t subkeylength, bool is_nest)
- bool k2hdkc_full_remove_str_subkey(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, const char* psubkey, bool is_nest)
 
- bool k2hdkc_pm_remove_subkey(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, const unsigned char* psubkey, size_t subkeylength, bool is_nest)
- bool k2hdkc_pm_remove_str_subkey(k2hdkc_chmpx_h handle, const char* pkey, const char* psubkey, size_t subkeylength, bool is_nest)
 
- bool k2hdkc_remove_subkey_np(const char* config, const unsigned char* pkey, size_t keylength, const unsigned char* psubkey, size_t subkeylength, bool is_nest)
- bool k2hdkc_remove_str_subkey_np(const char* config, const char* pkey, const char* psubkey, bool is_nest)
 
- bool k2hdkc_full_remove_subkey_np(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, const unsigned char* psubkey, size_t subkeylength, bool is_nest)
- bool k2hdkc_full_remove_str_subkey_np(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, const char* psubkey, bool is_nest)
 
- bool k2hdkc_pm_remove_subkey_np(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, const unsigned char* psubkey, size_t subkeylength, bool is_nest)
- bool k2hdkc_pm_remove_str_subkey_np(k2hdkc_chmpx_h handle, const char* pkey, const char* psubkey, size_t subkeylength, bool is_nest)

#### Description
These function groups are classified according to the following rules and classified into 2 types.
- k2hdkc_remove_subkey type  
  Specify the parent key and Subkey as binary, and remove Subkey.
- k2hdkc_remove_str_subkey type  
  Specify the parent key and Subkey as string, and remove Subkey.

Each of the above types has similar functions with differences such as arguments.
- k2hdkc_...  
  This function does not have any suffix. It requires basic arguments.
- k2hdkc_..._np  
  This function has **_np** suffix. This function does not perform attribute check(permission check).

In addition, depending on the connection type to chmpx slave node, it is divided into the following types.
- k2hdkc_...  
  This function specifies only the configuration of chmpx, connects to chmpx, and processes the command. This makes it easy to use Non-persistent chmpx connections.
- k2hdkc_full_...  
  This function specifies all the options of chmpx, connects to chmpx, and processes the command. This allows you to use Non-persistent chmpx connections with detailed settings.
- k2hdkc_pm_...  
  This function processes the command using the persistent chmpx connection handle.

#### Parameters
- config  
  Specify the configuration of the chmpx slave node(INI/YAML/JSON format file path, or JSON character string).
- ctlport  
  Specify the control port number of the chmpx slave node(It is necessary to specify it when multiple chmpx are running on the same server).
- cuk  
  Specify the CUK string of the chmpx slave node(It is necessary to specify it when multiple chmpx are running on the same server).
- is_auto_rejoin  
  Specify whether to reconnect automatically when the connection with the slave chmpx node is disconnected.
- no_giveup_rejoin  
  Case of specifying to reconnect automatically to the slave node of chmpx, specify the upper limit of retrying.
- pkey  
  For this argument, specify a pointer to a key which is a binary/string pointer.
- keylength  
  If the key is binary, specify the data length of the key.
- psubkey  
  Specifies a pointer for the Subkey(binary/string).
- subkeylength  
  Specifies the length of Subkey as binary.
- is_nest  
  If the Subkey has Subkeys, specifies whether to delete the nested Subkeys.
- handle  
  Specify the handle of the chmpx connection used by the k2hdkc library.

#### Return Values
Returns true if it succeeds, false if it fails.

#### Examples
```
bool        result = k2hdkc_full_remove_subkey(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin, pkey, keylen, psubkey, subkeylen, true);
dkcres_type_t    rescode = k2hdkc_get_lastres_code();
 
// check result
if(!result){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}
```

<!-- -----------------------------------------------------------　-->
***

### <a name="RENAME"> Rename Key family(C I/F)
Function group that specifies the key, and rename it.

#### Format
- bool k2hdkc_rename(const char* config, const unsigned char* poldkey, size_t oldkeylength, const unsigned char* pnewkey, size_t newkeylength)
- bool k2hdkc_rename_str(const char* config, const char* poldkey, const char* pnewkey)
 
- bool k2hdkc_full_rename(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* poldkey, size_t oldkeylength, const unsigned char* pnewkey, size_t newkeylength)
- bool k2hdkc_full_rename_str(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* poldkey, const char* pnewkey)
 
- bool k2hdkc_pm_rename(k2hdkc_chmpx_h handle, const unsigned char* poldkey, size_t oldkeylength, const unsigned char* pnewkey, size_t newkeylength)
- bool k2hdkc_pm_rename_str(k2hdkc_chmpx_h handle, const char* poldkey, const char* pnewkey)
 
- bool k2hdkc_rename_wa(const char* config, const unsigned char* poldkey, size_t oldkeylength, const unsigned char* pnewkey, size_t newkeylength, bool checkattr, const char* encpass, const time_t* expire)
- bool k2hdkc_rename_str_wa(const char* config, const char* poldkey, const char* pnewkey, bool checkattr, const char* encpass, const time_t* expire)
 
- bool k2hdkc_full_rename_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* poldkey, size_t oldkeylength, const unsigned char* pnewkey, size_t newkeylength, bool checkattr, const char* encpass, const time_t* expire)
- bool k2hdkc_full_rename_str_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* poldkey, const char* pnewkey, bool checkattr, const char* encpass, const time_t* expire)
 
- bool k2hdkc_pm_rename_wa(k2hdkc_chmpx_h handle, const unsigned char* poldkey, size_t oldkeylength, const unsigned char* pnewkey, size_t newkeylength, bool checkattr, const char* encpass, const time_t* expire)
- bool k2hdkc_pm_rename_str_wa(k2hdkc_chmpx_h handle, const char* poldkey, const char* pnewkey, bool checkattr, const char* encpass, const time_t* expire)
 
- bool k2hdkc_rename_with_parent(const char* config, const unsigned char* poldkey, size_t oldkeylength, const unsigned char* pnewkey, size_t newkeylength, const unsigned char* pparentkey, size_t parentkeylength)
- bool k2hdkc_rename_with_parent_str(const char* config, const char* poldkey, const char* pnewkey, const char* pparentkey)
 
- bool k2hdkc_full_rename_with_parent(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* poldkey, size_t oldkeylength, const unsigned char* pnewkey, size_t newkeylength, const unsigned char* pparentkey, size_t parentkeylength)
- bool k2hdkc_full_rename_with_parent_str(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* poldkey, const char* pnewkey, const char* pparentkey)
 
- bool k2hdkc_pm_rename_with_parent(k2hdkc_chmpx_h handle, const unsigned char* poldkey, size_t oldkeylength, const unsigned char* pnewkey, size_t newkeylength, const unsigned char* pparentkey, size_t parentkeylength)
- bool k2hdkc_pm_rename_with_parent_str(k2hdkc_chmpx_h handle, const char* poldkey, const char* pnewkey, const char* pparentkey)
 
- bool k2hdkc_rename_with_parent_wa(const char* config, const unsigned char* poldkey, size_t oldkeylength, const unsigned char* pnewkey, size_t newkeylength, const unsigned char* pparentkey, size_t parentkeylength, bool checkattr, const char* encpass, const time_t* expire)
- bool k2hdkc_rename_with_parent_str_wa(const char* config, const char* poldkey, const char* pnewkey, const char* pparentkey, bool checkattr, const char* encpass, const time_t* expire)
 
- bool k2hdkc_full_rename_with_parent_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* poldkey, size_t oldkeylength, const unsigned char* pnewkey, size_t newkeylength, const unsigned char* pparentkey, size_t parentkeylength, bool checkattr, const char* encpass, const time_t* expire)
- bool k2hdkc_full_rename_with_parent_str_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* poldkey, const char* pnewkey, const char* pparentkey, bool checkattr, const char* encpass, const time_t* expire)
 
- bool k2hdkc_pm_rename_with_parent_wa(k2hdkc_chmpx_h handle, const unsigned char* poldkey, size_t oldkeylength, const unsigned char* pnewkey, size_t newkeylength, const unsigned char* pparentkey, size_t parentkeylength, bool checkattr, const char* encpass, const time_t* expire)
- bool k2hdkc_pm_rename_with_parent_str_wa(k2hdkc_chmpx_h handle, const char* poldkey, const char* pnewkey, const char* pparentkey, bool checkattr, const char* encpass, const time_t* expire)

#### Description
These function groups are classified according to the following rules and classified into 4 kinds(2 types).
- k2hdkc_rename type  
  Specify old key and new key as binary, and rename it.
- k2hdkc_rename_with_parent type  
  Specify the parent key and old key, new key as binary, and rename it and change Subkey list in the parent key.
- k2hdkc_rename_str type  
  Specify old key and new key as string, and rename it.
- k2hdkc_rename_with_parent_str type  
  Specify the parent key and old key, new key as string, and rename it and change Subkey list in the parent key.

Each of the above types has similar functions with differences such as arguments.
- k2hdkc_...  
  This function does not have any suffix. It requires basic arguments.
- k2hdkc_..._wa  
  This function has **_wa** suffix. This function is a type that specifies a pass phrase and expire.

In addition, depending on the connection type to chmpx slave node, it is divided into the following types.
- k2hdkc_...  
  This function specifies only the configuration of chmpx, connects to chmpx, and processes the command. This makes it easy to use Non-persistent chmpx connections.
- k2hdkc_full_...  
  This function specifies all the options of chmpx, connects to chmpx, and processes the command. This allows you to use Non-persistent chmpx connections with detailed settings.
- k2hdkc_pm_...  
  This function processes the command using the persistent chmpx connection handle.

#### Parameters
- config  
  Specify the configuration of the chmpx slave node(INI/YAML/JSON format file path, or JSON character string).
- ctlport  
  Specify the control port number of the chmpx slave node(It is necessary to specify it when multiple chmpx are running on the same server).
- cuk  
  Specify the CUK string of the chmpx slave node(It is necessary to specify it when multiple chmpx are running on the same server).
- is_auto_rejoin  
  Specify whether to reconnect automatically when the connection with the slave chmpx node is disconnected.
- no_giveup_rejoin  
  Case of specifying to reconnect automatically to the slave node of chmpx, specify the upper limit of retrying.
- poldkey  
  For this argument, specify a pointer to old key which is a binary/string pointer.
- oldkeylength  
  If old key is binary, specify the data length of old key.
- pnewkey  
  For this argument, specify a pointer to new key which is a binary/string pointer.
- newkeylength  
  If new key is binary, specify the data length of new key.
- pparentkey  
  For this argument, specify a pointer to parent key which is a binary/string pointer.
- parentkeylength  
  If parent key is binary, specify the data length of parent key.
- checkattr  
  When changing a Subkey list to a parent key, specify whether to check the attribute(passphrase, expiration date) of the parent key.
- encpass  
  Specify the passphrase for encrypting the value.
- expire  
  To set an expiration date for a value, specify the expiration date.
- handle  
  Specify the handle of the chmpx connection used by the k2hdkc library.

#### Return Values
Returns true if it succeeds, false if it fails.

#### Note

#### Examples
```
bool        result = k2hdkc_full_rename_with_parent(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin, pOldKey, OldKeyLen, pNewKey, NewkeyLen, pParentKey, pParentKeyLen);
dkcres_type_t    rescode = k2hdkc_get_lastres_code();
 
// check result
if(!result){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}
```

<!-- -----------------------------------------------------------　-->
***

### <a name="QUEUEPUSH"> Queue(KeyQUeue) Push family(C I/F)
Function group that specifies the queue, and push the data to it.

#### Format
- bool k2hdkc_q_push(const char* config, const unsigned char* pprefix, size_t prefixlength, const unsigned char* pval, size_t vallength, bool is_fifo)
- bool k2hdkc_q_str_push(const char* config, const char* pprefix, const char* pval, bool is_fifo)
 
- bool k2hdkc_full_q_push(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pprefix, size_t prefixlength, const unsigned char* pval, size_t vallength, bool is_fifo)
- bool k2hdkc_full_q_str_push(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pprefix, const char* pval, bool is_fifo)
 
- bool k2hdkc_pm_q_push(k2hdkc_chmpx_h handle, const unsigned char* pprefix, size_t prefixlength, const unsigned char* pval, size_t vallength, bool is_fifo)
- bool k2hdkc_pm_q_str_push(k2hdkc_chmpx_h handle, const char* pprefix, const char* pval, bool is_fifo)
 
- bool k2hdkc_q_push_wa(const char* config, const unsigned char* pprefix, size_t prefixlength, const unsigned char* pval, size_t vallength, bool is_fifo, bool checkattr, const char* encpass, const time_t* expire)
- bool k2hdkc_q_str_push_wa(const char* config, const char* pprefix, const char* pval, bool is_fifo, bool checkattr, const char* encpass, const time_t* expire)
 
- bool k2hdkc_full_q_push_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pprefix, size_t prefixlength, const unsigned char* pval, size_t vallength, bool is_fifo, bool checkattr, const char* encpass, const time_t* expire)
- bool k2hdkc_full_q_str_push_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pprefix, const char* pval, bool is_fifo, bool checkattr, const char* encpass, const time_t* expire)
 
- bool k2hdkc_pm_q_push_wa(k2hdkc_chmpx_h handle, const unsigned char* pprefix, size_t prefixlength, const unsigned char* pval, size_t vallength, bool is_fifo, bool checkattr, const char* encpass, const time_t* expire)
- bool k2hdkc_pm_q_str_push_wa(k2hdkc_chmpx_h handle, const char* pprefix, const char* pval, bool is_fifo, bool checkattr, const char* encpass, const time_t* expire)
 
- bool k2hdkc_keyq_push(const char* config, const unsigned char* pprefix, size_t prefixlength, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, bool is_fifo)
- bool k2hdkc_keyq_str_push(const char* config, const char* pprefix, const char* pkey, const char* pval, bool is_fifo)
 
- bool k2hdkc_full_keyq_push(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pprefix, size_t prefixlength, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, bool is_fifo)
- bool k2hdkc_full_keyq_str_push(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pprefix, const char* pkey, const char* pval, bool is_fifo)
 
- bool k2hdkc_pm_keyq_push(k2hdkc_chmpx_h handle, const unsigned char* pprefix, size_t prefixlength, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, bool is_fifo)
- bool k2hdkc_pm_keyq_str_push(k2hdkc_chmpx_h handle, const char* pprefix, const char* pkey, const char* pval, bool is_fifo)
 
- bool k2hdkc_keyq_push_wa(const char* config, const unsigned char* pprefix, size_t prefixlength, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, bool is_fifo, bool checkattr, const char* encpass, const time_t* expire)
- bool k2hdkc_keyq_str_push_wa(const char* config, const char* pprefix, const char* pkey, const char* pval, bool is_fifo, bool checkattr, const char* encpass, const time_t* expire)
 
- bool k2hdkc_full_keyq_push_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pprefix, size_t prefixlength, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, bool is_fifo, bool checkattr, const char* encpass, const time_t* expire)
- bool k2hdkc_full_keyq_str_push_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pprefix, const char* pkey, const char* pval, bool is_fifo, bool checkattr, const char* encpass, const time_t* expire)
 
- bool k2hdkc_pm_keyq_push_wa(k2hdkc_chmpx_h handle, const unsigned char* pprefix, size_t prefixlength, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, bool is_fifo, bool checkattr, const char* encpass, const time_t* expire)
- bool k2hdkc_pm_keyq_str_push_wa(k2hdkc_chmpx_h handle, const char* pprefix, const char* pkey, const char* pval, bool is_fifo, bool checkattr, const char* encpass, const time_t* expire)

#### Description
These function groups are classified according to the following rules and classified into 4 kinds(2 types).
- k2hdkc_q_push type  
  Specify the queue and the value as binary, and push the value to the queue(QUEUE type).
- k2hdkc_keyq_push type  
  Specify the queue and the key/value as binary, and push the key/value to the queue(KEYQUEUE type).
- k2hdkc_q_str_push type  
  Specify the queue and the value as string, and push the value to the queue(QUEUE type).
- k2hdkc_keyq_str_push type  
  Specify the queue and the key/value as string, and push the key/value to the queue(KEYQUEUE type).

Each of the above types has similar functions with differences such as arguments.
- k2hdkc_...  
  This function does not have any suffix. It requires basic arguments.
- k2hdkc_..._wa  
  This function has **_wa** suffix. This function is a type that specifies a pass phrase and expire.

In addition, depending on the connection type to chmpx slave node, it is divided into the following types.
- k2hdkc_...  
  This function specifies only the configuration of chmpx, connects to chmpx, and processes the command. This makes it easy to use Non-persistent chmpx connections.
- k2hdkc_full_...  
  This function specifies all the options of chmpx, connects to chmpx, and processes the command. This allows you to use Non-persistent chmpx connections with detailed settings.
- k2hdkc_pm_...  
  This function processes the command using the persistent chmpx connection handle.

#### Parameters
- config  
  Specify the configuration of the chmpx slave node(INI/YAML/JSON format file path, or JSON character string).
- ctlport  
  Specify the control port number of the chmpx slave node(It is necessary to specify it when multiple chmpx are running on the same server).
- cuk  
  Specify the CUK string of the chmpx slave node(It is necessary to specify it when multiple chmpx are running on the same server).
- is_auto_rejoin  
  Specify whether to reconnect automatically when the connection with the slave chmpx node is disconnected.
- no_giveup_rejoin  
  Case of specifying to reconnect automatically to the slave node of chmpx, specify the upper limit of retrying.
- pprefix  
  For this argument, specify a pointer to a queue name prefix which is a binary/string pointer.
- prefixlength  
  If the queue name prefix is binary, specify the data length of it.
- pkey  
  For this argument, specify a pointer to pushed key which is a binary/string pointer.
- keylength  
  If the pushed key is binary, specify the data length of it.
- pval  
  Specifies a pointer for pushed value or set value for pushed key(binary/string).
- vallength  
  If pushed value or set value for pushed key is binary, specify the data length of it.
- is_fifo  
  Specifies pushing type for FIFO or LIFO.
- checkattr  
  When pushing, specify whether to check the attribute(passphrase, expiration date) of the queue.
- encpass  
  Specify the passphrase for encrypting the value.
- expire  
  To set an expiration date for a value, specify the expiration date.
- handle  
  Specify the handle of the chmpx connection used by the k2hdkc library.

#### Return Values
Returns true if it succeeds, false if it fails.

#### Examples
```
bool        result = k2hdkc_full_keyq_push(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin, pName, NameLen, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, reinterpret_cast<const unsigned char*>(strValue.c_str()), strValue.length() + 1, is_Fifo);
dkcres_type_t    rescode = k2hdkc_get_lastres_code();
 
// check result
if(!result){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}
```

<!-- -----------------------------------------------------------　-->
***

### <a name="QUEUEPOP"> Queue(KeyQUeue) Pop family(C I/F)
Function group that specifies the queue, and pop the data from it.

#### Format
- bool k2hdkc_q_pop(const char* config, const unsigned char* pprefix, size_t prefixlength, bool is_fifo, unsigned char** ppval, size_t* pvallength)
- bool k2hdkc_q_pop_wp(const char* config, const unsigned char* pprefix, size_t prefixlength, bool is_fifo, const char* encpass, unsigned char** ppval, size_t* pvallength)
- bool k2hdkc_q_str_pop(const char* config, const char* pprefix, bool is_fifo, const char** ppval)
- bool k2hdkc_q_str_pop_wp(const char* config, const char* pprefix, bool is_fifo, const char* encpass, const char** ppval)
 
- bool k2hdkc_full_q_pop(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pprefix, size_t prefixlength, bool is_fifo, unsigned char** ppval, size_t* pvallength)
- bool k2hdkc_full_q_pop_wp(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pprefix, size_t prefixlength, bool is_fifo, const char* encpass, unsigned char** ppval, size_t* pvallength)
- bool k2hdkc_full_q_str_pop(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pprefix, bool is_fifo, const char** ppval)
- bool k2hdkc_full_q_str_pop_wp(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pprefix, bool is_fifo, const char* encpass, const char** ppval)
 
- bool k2hdkc_full_keyq_pop(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pprefix, size_t prefixlength, bool is_fifo, unsigned char** ppkey, size_t* pkeylength, unsigned char** ppval, size_t* pvallength)
- bool k2hdkc_full_keyq_pop_wp(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pprefix, size_t prefixlength, bool is_fifo, const char* encpass, unsigned char** ppkey, size_t* pkeylength, unsigned char** ppval, size_t* pval- bool k2hdkc_full_keyq_str_pop(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pprefix, bool is_fifo, const char** ppkey, const char** ppval)
length)
- bool k2hdkc_full_keyq_str_pop_wp(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pprefix, bool is_fifo, const char* encpass, const char** ppkey, const char** ppval)
 
- bool k2hdkc_pm_q_pop(k2hdkc_chmpx_h handle, const unsigned char* pprefix, size_t prefixlength, bool is_fifo, unsigned char** ppval, size_t* pvallength)
- bool k2hdkc_pm_q_pop_wp(k2hdkc_chmpx_h handle, const unsigned char* pprefix, size_t prefixlength, bool is_fifo, const char* encpass, unsigned char** ppval, size_t* pvallength)
- bool k2hdkc_pm_q_str_pop(k2hdkc_chmpx_h handle, const char* pprefix, bool is_fifo, const char** ppval)
- bool k2hdkc_pm_q_str_pop_wp(k2hdkc_chmpx_h handle, const char* pprefix, bool is_fifo, const char* encpass, const char** ppval)
 
- bool k2hdkc_pm_keyq_pop(k2hdkc_chmpx_h handle, const unsigned char* pprefix, size_t prefixlength, bool is_fifo, unsigned char** ppkey, size_t* pkeylength, unsigned char** ppval, size_t* pvallength)
- bool k2hdkc_pm_keyq_pop_wp(k2hdkc_chmpx_h handle, const unsigned char* pprefix, size_t prefixlength, bool is_fifo, const char* encpass, unsigned char** ppkey, size_t* pkeylength, unsigned char** ppval, size_t* pvallength)
- bool k2hdkc_pm_keyq_str_pop(k2hdkc_chmpx_h handle, const char* pprefix, bool is_fifo, const char** ppkey, const char** ppval)
- bool k2hdkc_pm_keyq_str_pop_wp(k2hdkc_chmpx_h handle, const char* pprefix, bool is_fifo, const char* encpass, const char** ppkey, const char** ppval)


#### Description
These function groups are classified according to the following rules and classified into 4 kinds(2 types).
- k2hdkc_q_pop type  
  Specify the queue as binary, and pop the value from the queue(QUEUE type).
- k2hdkc_keyq_pop type  
  Specify the queue as binary, and pop the key/value from the queue(KEYQUEUE type).
- k2hdkc_q_str_pop type  
  Specify the queue as string, and pop the value from the queue(QUEUE type).
- k2hdkc_keyq_str_pop type  
  Specify the queue as string, and pop the key/value from the queue(KEYQUEUE type).

Each of the above types has similar functions with differences such as arguments.
- k2hdkc_...  
  This function does not have any suffix. It requires basic arguments.
- k2hdkc_..._wp  
  This function has **_wp** suffix. This function is a type that specifies a pass phrase.

In addition, depending on the connection type to chmpx slave node, it is divided into the following types.
- k2hdkc_...  
  This function specifies only the configuration of chmpx, connects to chmpx, and processes the command. This makes it easy to use Non-persistent chmpx connections.
- k2hdkc_full_...  
  This function specifies all the options of chmpx, connects to chmpx, and processes the command. This allows you to use Non-persistent chmpx connections with detailed settings.
- k2hdkc_pm_...  
  This function processes the command using the persistent chmpx connection handle.

#### Parameters
- config  
  Specify the configuration of the chmpx slave node(INI/YAML/JSON format file path, or JSON character string).
- ctlport  
  Specify the control port number of the chmpx slave node(It is necessary to specify it when multiple chmpx are running on the same server).
- cuk  
  Specify the CUK string of the chmpx slave node(It is necessary to specify it when multiple chmpx are running on the same server).
- is_auto_rejoin  
  Specify whether to reconnect automatically when the connection with the slave chmpx node is disconnected.
- no_giveup_rejoin  
  Case of specifying to reconnect automatically to the slave node of chmpx, specify the upper limit of retrying.
- pprefix  
  For this argument, specify a pointer to a queue name prefix which is a binary/string pointer.
- prefixlength  
  If the queue name prefix is binary, specify the data length of it.
- ppkey  
  For this argument, specify a pointer to store the key as binary retrieved from the queue.
- pkeylength  
  For this argument, specify a pointer to store length of the key as binary retrieved from the queue.
- ppval  
  For this argument, specify a pointer to store the value as binary retrieved from the queue.
- pvallength  
  For this argument, specify a pointer to store length of the value as binary retrieved from the queue.
- is_fifo  
  Specifies popping type for FIFO or LIFO.
- encpass  
  Specify the passphrase for encrypting the value.
- handle  
  Specify the handle of the chmpx connection used by the k2hdkc library.

#### Return Values
Returns true if it succeeds, false if it fails.

#### Examples
```
unsigned char*    pkey        = NULL;
size_t        keylength    = 0;
unsigned char*    pval        = NULL;
size_t        vallength    = 0;
bool        result        = k2hdkc_full_keyq_pop(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin, pName, NameLen, is_Fifo, &pkey, &keylength, &pval, &vallength);
dkcres_type_t    rescode        = k2hdkc_get_lastres_code();
 
// check result
if(!result){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}
```

<!-- -----------------------------------------------------------　-->
***

### <a name="QUEUEREMOVE"> Queue(KeyQUeue) Remove family(C I/F)
Function group that specifies the queue and count, and remove the specified number of data from it.

#### Format
- bool k2hdkc_q_remove(const char* config, const unsigned char* pprefix, size_t prefixlength, int count, bool is_fifo)
- bool k2hdkc_q_str_remove(const char* config, const char* pprefix, int count, bool is_fifo)
 
- bool k2hdkc_full_q_remove(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pprefix, size_t prefixlength, int count, bool is_fifo)
- bool k2hdkc_full_q_str_remove(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pprefix, int count, bool is_fifo)
 
- bool k2hdkc_pm_q_remove(k2hdkc_chmpx_h handle, const unsigned char* pprefix, size_t prefixlength, int count, bool is_fifo)
- bool k2hdkc_pm_q_str_remove(k2hdkc_chmpx_h handle, const char* pprefix, int count, bool is_fifo)
 
- bool k2hdkc_q_remove_wp(const char* config, const unsigned char* pprefix, size_t prefixlength, int count, bool is_fifo, const char* encpass)
- bool k2hdkc_q_str_remove_wp(const char* config, const char* pprefix, int count, bool is_fifo, const char* encpass)
 
- bool k2hdkc_full_q_remove_wp(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pprefix, size_t prefixlength, int count, bool is_fifo, const char* encpass)
- bool k2hdkc_full_q_str_remove_wp(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pprefix, int count, bool is_fifo, const char* encpass)
 
- bool k2hdkc_pm_q_remove_wp(k2hdkc_chmpx_h handle, const unsigned char* pprefix, size_t prefixlength, int count, bool is_fifo, const char* encpass)
- bool k2hdkc_pm_q_str_remove_wp(k2hdkc_chmpx_h handle, const char* pprefix, int count, bool is_fifo, const char* encpass)
 
- bool k2hdkc_keyq_remove(const char* config, const unsigned char* pprefix, size_t prefixlength, int count, bool is_fifo)
- bool k2hdkc_keyq_str_remove(const char* config, const char* pprefix, int count, bool is_fifo)
 
- bool k2hdkc_full_keyq_remove(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pprefix, size_t prefixlength, int count, bool is_fifo)
- bool k2hdkc_full_keyq_str_remove(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pprefix, int count, bool is_fifo)
 
- bool k2hdkc_pm_keyq_remove(k2hdkc_chmpx_h handle, const unsigned char* pprefix, size_t prefixlength, int count, bool is_fifo)
- bool k2hdkc_pm_keyq_str_remove(k2hdkc_chmpx_h handle, const char* pprefix, int count, bool is_fifo)
 
- bool k2hdkc_keyq_remove_wp(const char* config, const unsigned char* pprefix, size_t prefixlength, int count, bool is_fifo, const char* encpass)
- bool k2hdkc_keyq_str_remove_wp(const char* config, const char* pprefix, int count, bool is_fifo, const char* encpass)
 
- bool k2hdkc_full_keyq_remove_wp(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pprefix, size_t prefixlength, int count, bool is_fifo, const char* encpass)
- bool k2hdkc_full_keyq_str_remove_wp(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pprefix, int count, bool is_fifo, const char* encpass)
 
- bool k2hdkc_pm_keyq_remove_wp(k2hdkc_chmpx_h handle, const unsigned char* pprefix, size_t prefixlength, int count, bool is_fifo, const char* encpass)
- bool k2hdkc_pm_keyq_str_remove_wp(k2hdkc_chmpx_h handle, const char* pprefix, int count, bool is_fifo, const char* encpass)

#### Description
These function groups are classified according to the following rules and classified into 4 kinds(2 types).
- k2hdkc_q_remove type  
  Specify the queue as binary, and pop remove the specified number of value from the queue(QUEUE type).
- k2hdkc_keyq_remove type  
  Specify the queue as binary, and remove the number of key/value from the queue(KEYQUEUE type).
- k2hdkc_q_str_remove type  
  Specify the queue as string, and pop remove the specified number of value from the queue(QUEUE type).
- k2hdkc_keyq_str_remove type  
  Specify the queue as string, and remove the number of key/value from the queue(KEYQUEUE type).

Each of the above types has similar functions with differences such as arguments.
- k2hdkc_...  
  This function does not have any suffix. It requires basic arguments.
- k2hdkc_..._wp  
  This function has **_wp** suffix. This function is a type that specifies a pass phrase.

In addition, depending on the connection type to chmpx slave node, it is divided into the following types.
- k2hdkc_...  
  This function specifies only the configuration of chmpx, connects to chmpx, and processes the command. This makes it easy to use Non-persistent chmpx connections.
- k2hdkc_full_...  
  This function specifies all the options of chmpx, connects to chmpx, and processes the command. This allows you to use Non-persistent chmpx connections with detailed settings.
- k2hdkc_pm_...  
  This function processes the command using the persistent chmpx connection handle.

#### Parameters
- config  
  Specify the configuration of the chmpx slave node(INI/YAML/JSON format file path, or JSON character string).
- ctlport  
  Specify the control port number of the chmpx slave node(It is necessary to specify it when multiple chmpx are running on the same server).
- cuk  
  Specify the CUK string of the chmpx slave node(It is necessary to specify it when multiple chmpx are running on the same server).
- is_auto_rejoin  
  Specify whether to reconnect automatically when the connection with the slave chmpx node is disconnected.
- no_giveup_rejoin  
  Case of specifying to reconnect automatically to the slave node of chmpx, specify the upper limit of retrying.
- pprefix  
  For this argument, specify a pointer to a queue name prefix which is a binary/string pointer.
- prefixlength  
  If the queue name prefix is binary, specify the data length of it.
- is_fifo  
  Specifies removing type for FIFO or LIFO.
  PUSHするときにFIFO/LIFOを指定します。
- encpass  
  Specify the passphrase for encrypting the value.
- handle  
  Specify the handle of the chmpx connection used by the k2hdkc library.

#### Return Values
Returns true if it succeeds, false if it fails.

#### Examples
```
bool        result = k2hdkc_full_keyq_remove(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin, pName, NameLen, RmCount, is_Fifo);
dkcres_type_t    rescode = k2hdkc_get_lastres_code();
 
// check result
if(!result){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}
```

<!-- -----------------------------------------------------------　-->
***

### <a name="CASINIT"> CAS(Compare And Swap) Initialize family(C I/F)
Function group that specifies the key, and initialize the key for operation CAS(Compare And Swap).

#### Format
- bool k2hdkc_cas64_init(const char* config, const unsigned char* pkey, size_t keylength, uint64_t val)
- bool k2hdkc_cas64_str_init(const char* config, const char* pkey, uint64_t val)
- bool k2hdkc_full_cas64_init(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint64_t val)
- bool k2hdkc_full_cas64_str_init(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint64_t val)
- bool k2hdkc_pm_cas64_init(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint64_t val)
- bool k2hdkc_pm_cas64_str_init(k2hdkc_chmpx_h handle, const char* pkey, uint64_t val)
- bool k2hdkc_cas64_init_wa(const char* config, const unsigned char* pkey, size_t keylength, uint64_t val, const char* encpass, const time_t* expire)
- bool k2hdkc_cas64_str_init_wa(const char* config, const char* pkey, uint64_t val, const char* encpass, const time_t* expire)
- bool k2hdkc_full_cas64_init_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint64_t val, const char* encpass, const time_t* expire)
- bool k2hdkc_full_cas64_str_init_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint64_t val, const char* encpass, const time_t* expire)
- bool k2hdkc_pm_cas64_init_wa(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint64_t val, const char* encpass, const time_t* expire)
- bool k2hdkc_pm_cas64_str_init_wa(k2hdkc_chmpx_h handle, const char* pkey, uint64_t val, const char* encpass, const time_t* expire)
 
- bool k2hdkc_cas32_init(const char* config, const unsigned char* pkey, size_t keylength, uint32_t val)
- bool k2hdkc_cas32_str_init(const char* config, const char* pkey, uint32_t val)
- bool k2hdkc_full_cas32_init(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint32_t val)
- bool k2hdkc_full_cas32_str_init(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint32_t val)
- bool k2hdkc_pm_cas32_init(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint32_t val)
- bool k2hdkc_pm_cas32_str_init(k2hdkc_chmpx_h handle, const char* pkey, uint32_t val)
- bool k2hdkc_cas32_init_wa(const char* config, const unsigned char* pkey, size_t keylength, uint32_t val, const char* encpass, const time_t* expire)
- bool k2hdkc_cas32_str_init_wa(const char* config, const char* pkey, uint32_t val, const char* encpass, const time_t* expire)
- bool k2hdkc_full_cas32_init_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint32_t val, const char* encpass, const time_t* expire)
- bool k2hdkc_full_cas32_str_init_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint32_t val, const char* encpass, const time_t* expire)
- bool k2hdkc_pm_cas32_init_wa(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint32_t val, const char* encpass, const time_t* expire)
- bool k2hdkc_pm_cas32_str_init_wa(k2hdkc_chmpx_h handle, const char* pkey, uint32_t val, const char* encpass, const time_t* expire)
 
- bool k2hdkc_cas16_init(const char* config, const unsigned char* pkey, size_t keylength, uint16_t val)
- bool k2hdkc_cas16_str_init(const char* config, const char* pkey, uint16_t val)
- bool k2hdkc_full_cas16_init(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint16_t val)
- bool k2hdkc_full_cas16_str_init(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint16_t val)
- bool k2hdkc_pm_cas16_init(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint16_t val)
- bool k2hdkc_pm_cas16_str_init(k2hdkc_chmpx_h handle, const char* pkey, uint16_t val)
- bool k2hdkc_cas16_init_wa(const char* config, const unsigned char* pkey, size_t keylength, uint16_t val, const char* encpass, const time_t* expire)
- bool k2hdkc_cas16_str_init_wa(const char* config, const char* pkey, uint16_t val, const char* encpass, const time_t* expire)
- bool k2hdkc_full_cas16_init_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint16_t val, const char* encpass, const time_t* expire)
- bool k2hdkc_full_cas16_str_init_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint16_t val, const char* encpass, const time_t* expire)
- bool k2hdkc_pm_cas16_init_wa(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint16_t val, const char* encpass, const time_t* expire)
- bool k2hdkc_pm_cas16_str_init_wa(k2hdkc_chmpx_h handle, const char* pkey, uint16_t val, const char* encpass, const time_t* expire)
 
- bool k2hdkc_cas8_init(const char* config, const unsigned char* pkey, size_t keylength, uint8_t val)
- bool k2hdkc_cas8_str_init(const char* config, const char* pkey, uint8_t val)
- bool k2hdkc_full_cas8_init(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint8_t val)
- bool k2hdkc_full_cas8_str_init(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint8_t val)
- bool k2hdkc_pm_cas8_init(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint8_t val)
- bool k2hdkc_pm_cas8_str_init(k2hdkc_chmpx_h handle, const char* pkey, uint8_t val)
- bool k2hdkc_cas8_init_wa(const char* config, const unsigned char* pkey, size_t keylength, uint8_t val, const char* encpass, const time_t* expire)
- bool k2hdkc_cas8_str_init_wa(const char* config, const char* pkey, uint8_t val, const char* encpass, const time_t* expire)
- bool k2hdkc_full_cas8_init_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint8_t val, const char* encpass, const time_t* expire)
- bool k2hdkc_full_cas8_str_init_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint8_t val, const char* encpass, const time_t* expire)
- bool k2hdkc_pm_cas8_init_wa(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint8_t val, const char* encpass, const time_t* expire)
- bool k2hdkc_pm_cas8_str_init_wa(k2hdkc_chmpx_h handle, const char* pkey, uint8_t val, const char* encpass, const time_t* expire)

#### Description
These function groups are classified according to the following rules and classified into 2 types.
- k2hdkc_cas..._init type  
  Specify the key as binary and length of value, and initialize it for CAS.
- k2hdkc_cas..._str_init type  
  Specify the key as string and length of value, and initialize it for CAS.

Depending on the data length of CAS value, the following types of these functions are provided.
- k2hdkc_cas8_...  
  The length of CAS value is 8 bits(1 byte).
- k2hdkc_cas16_...  
  The length of CAS value is 16 bits(2 bytes).
- k2hdkc_cas32_...  
  The length of CAS value is 32 bits(4 bytes).
- k2hdkc_cas64_...  
  The length of CAS value is 64 bits(8 bytes).

In addition, the following types are provided for these functions depending on the argument difference.
- k2hdkc_...  
  This function does not have any suffix. It requires basic arguments.
- k2hdkc_..._wa  
  This function has **_wa** suffix. This function is a type that specifies a pass phrase and expire.

In addition, depending on the connection type to chmpx slave node, it is divided into the following types.
- k2hdkc_...  
  This function specifies only the configuration of chmpx, connects to chmpx, and processes the command. This makes it easy to use Non-persistent chmpx connections.
- k2hdkc_full_...  
  This function specifies all the options of chmpx, connects to chmpx, and processes the command. This allows you to use Non-persistent chmpx connections with detailed settings.
- k2hdkc_pm_...  
  This function processes the command using the persistent chmpx connection handle.

#### Parameters
- config  
  Specify the configuration of the chmpx slave node(INI/YAML/JSON format file path, or JSON character string).
- ctlport  
  Specify the control port number of the chmpx slave node(It is necessary to specify it when multiple chmpx are running on the same server).
- cuk  
  Specify the CUK string of the chmpx slave node(It is necessary to specify it when multiple chmpx are running on the same server).
- is_auto_rejoin  
  Specify whether to reconnect automatically when the connection with the slave chmpx node is disconnected.
- no_giveup_rejoin  
  Case of specifying to reconnect automatically to the slave node of chmpx, specify the upper limit of retrying.
- pkey  
  For this argument, specify a pointer to a key which is a binary/string pointer.
- keylength  
  If the key is binary, specify the data length of it.
- val  
  For this argument, specify the value as initial value. It's length is 8/16/32/64 bits for each function type.
- encpass  
  Specify the passphrase for encrypting the value.
- expire  
  To set an expiration date for a value, specify the expiration date.
- handle  
  Specify the handle of the chmpx connection used by the k2hdkc library.

#### Return Values
Returns true if it succeeds, false if it fails.

#### Examples
```
bool        result    = k2hdkc_full_cas64_init(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, val);
dkcres_type_t    rescode = k2hdkc_get_lastres_code();
 
// check result
if(!result){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}
```

<!-- -----------------------------------------------------------　-->
***

### <a name="CASGET"> CAS(Compare And Swap) Get family(C I/F)
Function group that specifies the key, and get the value for operation CAS(Compare And Swap).

#### Format
- bool k2hdkc_cas64_get(const char* config, const unsigned char* pkey, size_t keylength, uint64_t* pval)
- bool k2hdkc_cas64_str_get(const char* config, const char* pkey, uint64_t* pval)
- bool k2hdkc_full_cas64_get(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint64_t* pval)
- bool k2hdkc_full_cas64_str_get(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint64_t* pval)
- bool k2hdkc_pm_cas64_get(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint64_t* pval)
- bool k2hdkc_pm_cas64_str_get(k2hdkc_chmpx_h handle, const char* pkey, uint64_t* pval)
- bool k2hdkc_cas64_get_wa(const char* config, const unsigned char* pkey, size_t keylength, const char* encpass, uint64_t* pval)
- bool k2hdkc_cas64_str_get_wa(const char* config, const char* pkey, const char* encpass, uint64_t* pval)
- bool k2hdkc_full_cas64_get_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, const char* encpass, uint64_t* pval)
- bool k2hdkc_full_cas64_str_get_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, const char* encpass, uint64_t* pval)
- bool k2hdkc_pm_cas64_get_wa(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, const char* encpass, uint64_t* pval)
- bool k2hdkc_pm_cas64_str_get_wa(k2hdkc_chmpx_h handle, const char* pkey, const char* encpass, uint64_t* pval)
 
- bool k2hdkc_cas32_get(const char* config, const unsigned char* pkey, size_t keylength, uint32_t* pval)
- bool k2hdkc_cas32_str_get(const char* config, const char* pkey, uint32_t* pval)
- bool k2hdkc_full_cas32_get(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint32_t* pval)
- bool k2hdkc_full_cas32_str_get(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint32_t* pval)
- bool k2hdkc_pm_cas32_get(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint32_t* pval)
- bool k2hdkc_pm_cas32_str_get(k2hdkc_chmpx_h handle, const char* pkey, uint32_t* pval)
- bool k2hdkc_cas32_get_wa(const char* config, const unsigned char* pkey, size_t keylength, const char* encpass, uint32_t* pval)
- bool k2hdkc_cas32_str_get_wa(const char* config, const char* pkey, const char* encpass, uint32_t* pval)
- bool k2hdkc_full_cas32_get_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, const char* encpass, uint32_t* pval)
- bool k2hdkc_full_cas32_str_get_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, const char* encpass, uint32_t* pval)
- bool k2hdkc_pm_cas32_get_wa(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, const char* encpass, uint32_t* pval)
- bool k2hdkc_pm_cas32_str_get_wa(k2hdkc_chmpx_h handle, const char* pkey, const char* encpass, uint32_t* pval)
 
- bool k2hdkc_cas16_get(const char* config, const unsigned char* pkey, size_t keylength, uint16_t* pval)
- bool k2hdkc_cas16_str_get(const char* config, const char* pkey, uint16_t* pval)
- bool k2hdkc_full_cas16_get(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint16_t* pval)
- bool k2hdkc_full_cas16_str_get(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint16_t* pval)
- bool k2hdkc_pm_cas16_get(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint16_t* pval)
- bool k2hdkc_pm_cas16_str_get(k2hdkc_chmpx_h handle, const char* pkey, uint16_t* pval)
- bool k2hdkc_cas16_get_wa(const char* config, const unsigned char* pkey, size_t keylength, const char* encpass, uint16_t* pval)
- bool k2hdkc_cas16_str_get_wa(const char* config, const char* pkey, const char* encpass, uint16_t* pval)
- bool k2hdkc_full_cas16_get_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, const char* encpass, uint16_t* pval)
- bool k2hdkc_full_cas16_str_get_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, const char* encpass, uint16_t* pval)
- bool k2hdkc_pm_cas16_get_wa(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, const char* encpass, uint16_t* pval)
- bool k2hdkc_pm_cas16_str_get_wa(k2hdkc_chmpx_h handle, const char* pkey, const char* encpass, uint16_t* pval)
 
- bool k2hdkc_cas8_get(const char* config, const unsigned char* pkey, size_t keylength, uint8_t* pval)
- bool k2hdkc_cas8_str_get(const char* config, const char* pkey, uint8_t* pval)
- bool k2hdkc_full_cas8_get(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint8_t* pval)
- bool k2hdkc_full_cas8_str_get(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint8_t* pval)
- bool k2hdkc_pm_cas8_get(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint8_t* pval)
- bool k2hdkc_pm_cas8_str_get(k2hdkc_chmpx_h handle, const char* pkey, uint8_t* pval)
- bool k2hdkc_cas8_get_wa(const char* config, const unsigned char* pkey, size_t keylength, const char* encpass, uint8_t* pval)
- bool k2hdkc_cas8_str_get_wa(const char* config, const char* pkey, const char* encpass, uint8_t* pval)
- bool k2hdkc_full_cas8_get_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, const char* encpass, uint8_t* pval)
- bool k2hdkc_full_cas8_str_get_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, const char* encpass, uint8_t* pval)
- bool k2hdkc_pm_cas8_get_wa(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, const char* encpass, uint8_t* pval)
- bool k2hdkc_pm_cas8_str_get_wa(k2hdkc_chmpx_h handle, const char* pkey, const char* encpass, uint8_t* pval)

#### Description
These function groups are classified according to the following rules and classified into 2 types.
- k2hdkc_cas..._get type  
  Specify the key as binary, and get the value.
- k2hdkc_cas..._str_get type  
  Specify the key as string, and get the value.

Depending on the data length of CAS value, the following types of these functions are provided.
- k2hdkc_cas8_...  
  The length of CAS value is 8 bits(1 byte).
- k2hdkc_cas16_...  
  The length of CAS value is 16 bits(2 bytes).
- k2hdkc_cas32_...  
  The length of CAS value is 32 bits(4 bytes).
- k2hdkc_cas64_...  
  The length of CAS value is 64 bits(8 bytes).

In addition, the following types are provided for these functions depending on the argument difference.
- k2hdkc_...  
  This function does not have any suffix. It requires basic arguments.
- k2hdkc_..._wa  
  This function has **_wa** suffix. This function is a type that specifies a pass phrase and expire.

In addition, depending on the connection type to chmpx slave node, it is divided into the following types.
- k2hdkc_...  
  This function specifies only the configuration of chmpx, connects to chmpx, and processes the command. This makes it easy to use Non-persistent chmpx connections.
- k2hdkc_full_...  
  This function specifies all the options of chmpx, connects to chmpx, and processes the command. This allows you to use Non-persistent chmpx connections with detailed settings.
- k2hdkc_pm_...  
  This function processes the command using the persistent chmpx connection handle.

#### Parameters
- config  
  Specify the configuration of the chmpx slave node(INI/YAML/JSON format file path, or JSON character string).
- ctlport  
  Specify the control port number of the chmpx slave node(It is necessary to specify it when multiple chmpx are running on the same server).
- cuk  
  Specify the CUK string of the chmpx slave node(It is necessary to specify it when multiple chmpx are running on the same server).
- is_auto_rejoin  
  Specify whether to reconnect automatically when the connection with the slave chmpx node is disconnected.
- no_giveup_rejoin  
  Case of specifying to reconnect automatically to the slave node of chmpx, specify the upper limit of retrying.
- pkey  
  For this argument, specify a pointer to a key which is a binary/string pointer.
- keylength  
  If the key is binary, specify the data length of it.
- pval  
  Specifies a pointer for the value(binary/string).
  For this argument, specify the pointer for storing the value. The buffer's length must be 8/16/32/64 bits for each function type.
- encpass  
  Specify the passphrase for encrypting the value.
- handle  
  Specify the handle of the chmpx connection used by the k2hdkc library.

#### Return Values
Returns true if it succeeds, false if it fails.

#### Examples
```
uint64_t    val64    = 0;
bool        result    = k2hdkc_full_cas64_get(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, &val64);
dkcres_type_t    rescode = k2hdkc_get_lastres_code();
 
// check result
if(!result){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}
```


<!-- -----------------------------------------------------------　-->
***

### <a name="CASSET"> CAS(Compare And Swap) Set family(C I/F)
Function group that specifies the key, and when the value is as same as specified value, set the value for operation CAS(Compare And Swap).

#### Format
- bool k2hdkc_cas64_set(const char* config, const unsigned char* pkey, size_t keylength, uint64_t oldval, uint64_t newval)
- bool k2hdkc_cas64_str_set(const char* config, const char* pkey, uint64_t oldval, uint64_t newval)
- bool k2hdkc_full_cas64_set(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint64_t oldval, uint64_t newval)
- bool k2hdkc_full_cas64_str_set(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint64_t oldval, uint64_t newval)
- bool k2hdkc_pm_cas64_set(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint64_t oldval, uint64_t newval)
- bool k2hdkc_pm_cas64_str_set(k2hdkc_chmpx_h handle, const char* pkey, uint64_t oldval, uint64_t newval)
- bool k2hdkc_cas64_set_wa(const char* config, const unsigned char* pkey, size_t keylength, uint64_t oldval, uint64_t newval, const char* encpass, const time_t* expire)
- bool k2hdkc_cas64_str_set_wa(const char* config, const char* pkey, uint64_t oldval, uint64_t newval, const char* encpass, const time_t* expire)
- bool k2hdkc_full_cas64_set_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint64_t oldval, uint64_t newval, const char* encpass, const time_t* expire)
- bool k2hdkc_full_cas64_str_set_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint64_t oldval, uint64_t newval, const char* encpass, const time_t* expire)
- bool k2hdkc_pm_cas64_set_wa(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint64_t oldval, uint64_t newval, const char* encpass, const time_t* expire)
- bool k2hdkc_pm_cas64_str_set_wa(k2hdkc_chmpx_h handle, const char* pkey, uint64_t oldval, uint64_t newval, const char* encpass, const time_t* expire)
 
- bool k2hdkc_cas32_set(const char* config, const unsigned char* pkey, size_t keylength, uint32_t oldval, uint32_t newval)
- bool k2hdkc_cas32_str_set(const char* config, const char* pkey, uint32_t oldval, uint32_t newval)
- bool k2hdkc_full_cas32_set(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint32_t oldval, uint32_t newval)
- bool k2hdkc_full_cas32_str_set(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint32_t oldval, uint32_t newval)
- bool k2hdkc_pm_cas32_set(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint32_t oldval, uint32_t newval)
- bool k2hdkc_pm_cas32_str_set(k2hdkc_chmpx_h handle, const char* pkey, uint32_t oldval, uint32_t newval)
- bool k2hdkc_cas32_set_wa(const char* config, const unsigned char* pkey, size_t keylength, uint32_t oldval, uint32_t newval, const char* encpass, const time_t* expire)
- bool k2hdkc_cas32_str_set_wa(const char* config, const char* pkey, uint32_t oldval, uint32_t newval, const char* encpass, const time_t* expire)
- bool k2hdkc_full_cas32_set_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint32_t oldval, uint32_t newval, const char* encpass, const time_t* expire)
- bool k2hdkc_full_cas32_str_set_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint32_t oldval, uint32_t newval, const char* encpass, const time_t* expire)
- bool k2hdkc_pm_cas32_set_wa(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint32_t oldval, uint32_t newval, const char* encpass, const time_t* expire)
- bool k2hdkc_pm_cas32_str_set_wa(k2hdkc_chmpx_h handle, const char* pkey, uint32_t oldval, uint32_t newval, const char* encpass, const time_t* expire)
 
- bool k2hdkc_cas16_set(const char* config, const unsigned char* pkey, size_t keylength, uint16_t oldval, uint16_t newval)
- bool k2hdkc_cas16_str_set(const char* config, const char* pkey, uint16_t oldval, uint16_t newval)
- bool k2hdkc_full_cas16_set(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint16_t oldval, uint16_t newval)
- bool k2hdkc_full_cas16_str_set(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint16_t oldval, uint16_t newval)
- bool k2hdkc_pm_cas16_set(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint16_t oldval, uint16_t newval)
- bool k2hdkc_pm_cas16_str_set(k2hdkc_chmpx_h handle, const char* pkey, uint16_t oldval, uint16_t newval)
- bool k2hdkc_cas16_set_wa(const char* config, const unsigned char* pkey, size_t keylength, uint16_t oldval, uint16_t newval, const char* encpass, const time_t* expire)
- bool k2hdkc_cas16_str_set_wa(const char* config, const char* pkey, uint16_t oldval, uint16_t newval, const char* encpass, const time_t* expire)
- bool k2hdkc_full_cas16_set_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint16_t oldval, uint16_t newval, const char* encpass, const time_t* expire)
- bool k2hdkc_full_cas16_str_set_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint16_t oldval, uint16_t newval, const char* encpass, const time_t* expire)
- bool k2hdkc_pm_cas16_set_wa(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint16_t oldval, uint16_t newval, const char* encpass, const time_t* expire)
- bool k2hdkc_pm_cas16_str_set_wa(k2hdkc_chmpx_h handle, const char* pkey, uint16_t oldval, uint16_t newval, const char* encpass, const time_t* expire)
 
- bool k2hdkc_cas8_set(const char* config, const unsigned char* pkey, size_t keylength, uint8_t oldval, uint8_t newval)
- bool k2hdkc_cas8_str_set(const char* config, const char* pkey, uint8_t oldval, uint8_t newval)
- bool k2hdkc_full_cas8_set(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint8_t oldval, uint8_t newval)
- bool k2hdkc_full_cas8_str_set(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint8_t oldval, uint8_t newval)
- bool k2hdkc_pm_cas8_set(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint8_t oldval, uint8_t newval)
- bool k2hdkc_pm_cas8_str_set(k2hdkc_chmpx_h handle, const char* pkey, uint8_t oldval, uint8_t newval)
- bool k2hdkc_cas8_set_wa(const char* config, const unsigned char* pkey, size_t keylength, uint8_t oldval, uint8_t newval, const char* encpass, const time_t* expire)
- bool k2hdkc_cas8_str_set_wa(const char* config, const char* pkey, uint8_t oldval, uint8_t newval, const char* encpass, const time_t* expire)
- bool k2hdkc_full_cas8_set_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint8_t oldval, uint8_t newval, const char* encpass, const time_t* expire)
- bool k2hdkc_full_cas8_str_set_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint8_t oldval, uint8_t newval, const char* encpass, const time_t* expire)
- bool k2hdkc_pm_cas8_set_wa(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint8_t oldval, uint8_t newval, const char* encpass, const time_t* expire)
- bool k2hdkc_pm_cas8_str_set_wa(k2hdkc_chmpx_h handle, const char* pkey, uint8_t oldval, uint8_t newval, const char* encpass, const time_t* expire)

#### Description
These function groups are classified according to the following rules and classified into 2 types.
- k2hdkc_cas..._set type  
  Specify the key as binary and the value. If the value is as same as specified value, set the value for operation CAS(Compare And Swap).
- k2hdkc_cas..._str_set type  
  Specify the key as string and the value. If the value is as same as specified value, set the value for operation CAS(Compare And Swap).

Depending on the data length of CAS value, the following types of these functions are provided.
- k2hdkc_cas8_...  
  The length of CAS value is 8 bits(1 byte).
- k2hdkc_cas16_...  
  The length of CAS value is 16 bits(2 bytes).
- k2hdkc_cas32_...  
  The length of CAS value is 32 bits(4 bytes).
- k2hdkc_cas64_...  
  The length of CAS value is 64 bits(8 bytes).

In addition, the following types are provided for these functions depending on the argument difference.
- k2hdkc_...  
  This function does not have any suffix. It requires basic arguments.
- k2hdkc_..._wa  
  This function has **_wa** suffix. This function is a type that specifies a pass phrase and expire.

In addition, depending on the connection type to chmpx slave node, it is divided into the following types.
- k2hdkc_...  
  This function specifies only the configuration of chmpx, connects to chmpx, and processes the command. This makes it easy to use Non-persistent chmpx connections.
- k2hdkc_full_...  
  This function specifies all the options of chmpx, connects to chmpx, and processes the command. This allows you to use Non-persistent chmpx connections with detailed settings.
- k2hdkc_pm_...  
  This function processes the command using the persistent chmpx connection handle.

#### Parameters
- config  
  Specify the configuration of the chmpx slave node(INI/YAML/JSON format file path, or JSON character string).
- ctlport  
  Specify the control port number of the chmpx slave node(It is necessary to specify it when multiple chmpx are running on the same server).
- cuk  
  Specify the CUK string of the chmpx slave node(It is necessary to specify it when multiple chmpx are running on the same server).
- is_auto_rejoin  
  Specify whether to reconnect automatically when the connection with the slave chmpx node is disconnected.
- no_giveup_rejoin  
  Case of specifying to reconnect automatically to the slave node of chmpx, specify the upper limit of retrying.
- pkey  
  For this argument, specify a pointer to a key which is a binary/string pointer.
- keylength  
  If the key is binary, specify the data length of it.
- oldval  
  For this argument, specify the value as expected value. It's length is 8/16/32/64 bits for each function type.
- newval  
  For this argument, specify new value. It's length is 8/16/32/64 bits for each function type.
- encpass  
  Specify the passphrase for encrypting the value.
- expire  
  To set an expiration date for a value, specify the expiration date.
- handle  
  Specify the handle of the chmpx connection used by the k2hdkc library.

#### Return Values
Returns true if it succeeds, false if it fails.

#### Examples
```
bool        result    = k2hdkc_full_cas64_set(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, oldval, newval);
dkcres_type_t    rescode = k2hdkc_get_lastres_code();
 
// check result
if(!result){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}
```

<!-- -----------------------------------------------------------　-->
***

### <a name="CASINCDEC"> CAS(Compare And Swap) Increment/Decrement family(C I/F)
Function group that specifies the key, and increment/decrement the value for operation CAS(Compare And Swap).

#### Format
- bool k2hdkc_cas_increment(const char* config, const unsigned char* pkey, size_t keylength)
- bool k2hdkc_cas_str_increment(const char* config, const char* pkey)
- bool k2hdkc_full_cas_increment(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength)
- bool k2hdkc_full_cas_str_increment(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey)
- bool k2hdkc_pm_cas_increment(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength)
- bool k2hdkc_pm_cas_str_increment(k2hdkc_chmpx_h handle, const char* pkey)
- bool k2hdkc_cas_increment_wa(const char* config, const unsigned char* pkey, size_t keylength, const char* encpass, const time_t* expire)
- bool k2hdkc_cas_str_increment_wa(const char* config, const char* pkey, const char* encpass, const time_t* expire)
- bool k2hdkc_full_cas_increment_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, const char* encpass, const time_t* expire)
- bool k2hdkc_full_cas_str_increment_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, const char* encpass, const time_t* expire)
- bool k2hdkc_pm_cas_increment_wa(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, const char* encpass, const time_t* expire)
- bool k2hdkc_pm_cas_str_increment_wa(k2hdkc_chmpx_h handle, const char* pkey, const char* encpass, const time_t* expire)
 
- bool k2hdkc_cas_decrement(const char* config, const unsigned char* pkey, size_t keylength)
- bool k2hdkc_cas_str_decrement(const char* config, const char* pkey)
- bool k2hdkc_full_cas_decrement(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength)
- bool k2hdkc_full_cas_str_decrement(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey)
- bool k2hdkc_pm_cas_decrement(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength)
- bool k2hdkc_pm_cas_str_decrement(k2hdkc_chmpx_h handle, const char* pkey)
- bool k2hdkc_cas_decrement_wa(const char* config, const unsigned char* pkey, size_t keylength, const char* encpass, const time_t* expire)
- bool k2hdkc_cas_str_decrement_wa(const char* config, const char* pkey, const char* encpass, const time_t* expire)
- bool k2hdkc_full_cas_decrement_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, const char* encpass, const time_t* expire)
- bool k2hdkc_full_cas_str_decrement_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, const char* encpass, const time_t* expire)
- bool k2hdkc_pm_cas_decrement_wa(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, const char* encpass, const time_t* expire)
- bool k2hdkc_pm_cas_str_decrement_wa(k2hdkc_chmpx_h handle, const char* pkey, const char* encpass, const time_t* expire)

#### Description
These function groups are classified according to the following rules and classified into 4 kinds(2 types).
- k2hdkc_cas_increment type  
  Specify the key as binary and increment/decrement the value. It is exclusively controlled during processing and guaranteed to be safely incremented. It is not necessary to specify the data length of the value.
- k2hdkc_cas_decrement type  
  Specify the key as binary and increment/decrement the value. It is exclusively controlled during processing and guaranteed to be safely decremented. It is not necessary to specify the data length of the value.
- k2hdkc_cas_str_increment type  
  Specify the key as string and increment/decrement the value. It is exclusively controlled during processing and guaranteed to be safely incremented. It is not necessary to specify the data length of the value.
- k2hdkc_cas_str_decrement type  
  Specify the key as string and increment/decrement the value. It is exclusively controlled during processing and guaranteed to be safely decremented. It is not necessary to specify the data length of the value.

In addition, the following types are provided for these functions depending on the argument difference.
- k2hdkc_...  
  This function does not have any suffix. It requires basic arguments.
- k2hdkc_..._wa  
  This function has **_wa** suffix. This function is a type that specifies a pass phrase and expire.

In addition, depending on the connection type to chmpx slave node, it is divided into the following types.
- k2hdkc_...  
  This function specifies only the configuration of chmpx, connects to chmpx, and processes the command. This makes it easy to use Non-persistent chmpx connections.
- k2hdkc_full_...  
  This function specifies all the options of chmpx, connects to chmpx, and processes the command. This allows you to use Non-persistent chmpx connections with detailed settings.
- k2hdkc_pm_...  
  This function processes the command using the persistent chmpx connection handle.

#### Parameters
- config  
  Specify the configuration of the chmpx slave node(INI/YAML/JSON format file path, or JSON character string).
- ctlport  
  Specify the control port number of the chmpx slave node(It is necessary to specify it when multiple chmpx are running on the same server).
- cuk  
  Specify the CUK string of the chmpx slave node(It is necessary to specify it when multiple chmpx are running on the same server).
- is_auto_rejoin  
  Specify whether to reconnect automatically when the connection with the slave chmpx node is disconnected.
- no_giveup_rejoin  
  Case of specifying to reconnect automatically to the slave node of chmpx, specify the upper limit of retrying.
- pkey  
  For this argument, specify a pointer to a key which is a binary/string pointer.
- keylength  
  If the key is binary, specify the data length of it.
- encpass  
  Specify the passphrase for encrypting the value.
- expire  
  To set an expiration date for a value, specify the expiration date.
- handle  
  Specify the handle of the chmpx connection used by the k2hdkc library.

#### Return Values
Returns true if it succeeds, false if it fails.

#### Examples
```
bool        result = k2hdkc_full_cas_increment_wa(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), (-1 == Expire ? NULL : &Expire));
dkcres_type_t    rescode = k2hdkc_get_lastres_code();
 
// check result
if(!result){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}
```

<!-- -----------------------------------------------------------　-->
***

## <a name="CPP"> C++ API
This is an API for C++ language API for using the k2hdkc library.
Include the following header files when developing.
```
#include <k2hdkc/k2hdkcslave.h>
```

When linking please specify the following as an option.
```
-lk2hdkc
```

The functions for C++ language are explained below.
<!-- -----------------------------------------------------------　-->
***

### <a name="DEBUGCPP"> Debug family(C++ I/F)
The k2hdkc library can output messages to check internal operation and API operation.
This function group is a group of functions for controlling message output on C++ language.

#### Format
- K2hdkcDbgMode SetK2hdkcDbgMode(K2hdkcDbgMode mode)
- K2hdkcDbgMode BumpupK2hdkcDbgMode(void)
- K2hdkcDbgMode GetK2hdkcDbgMode(void)
- bool LoadK2hdkcDbgEnv(void)
- bool SetK2hdkcDbgFile(const char* filepath)
- bool UnsetK2hdkcDbgFile(void)
- const char* GetK2hdkcDbgFile(void)
- void SetK2hdkcComLog(bool enable)
- void EnableK2hdkcComLog(void)
- void DsableK2hdkcComLog(void)
- bool IsK2hdkcComLog(void)

#### Description
- SetK2hdkcDbgMode  
  Set message output level.
- BumpupK2hdkcDbgMode  
  Bump up message output level as non-output -> error -> warning -> information -> dump -> non-output.
- GetK2hdkcDbgMode  
  Get message output level.
- LoadK2hdkcDbgEnv  
  Load the environment variables DKCDBGMODE, DKCDBGFILE and set the message output and output destination according to the value.
- SetK2hdkcDbgFile  
  Specify the file path to output message. If it is not set, it is output to stderr.
- UnsetK2hdkcDbgFile  
  Unset to output messages to stderr.
- GetK2hdkcDbgFile  
  Get the file path to output message if it is set.
- SetK2hdkcComLog  
  Set enable/disable of the communication log output.
- k2hdkc_load_debug_env  
  Load the environment variables DKCDBGMODE, DKCDBGFILE and set the message output and output destination according to the value.
- IsK2hdkcComLog  
  Check the enable/disable of the communication log output.
- EnableK2hdkcComLog  
  Enable the communication log output.
- DsableK2hdkcComLog  
  Disable the communication log output.

#### Parameters
- mode  
  Specifies the message output level as non-output / error / warning / information / dump.
- filepath  
  Specifies the file path to output message.
- enable  
  Specifies the enable/disable of the communication log output.

#### Return Values
- SetK2hdkcDbgMode / BumpupK2hdkcDbgMode  
  Returns the message output level before set new level.
- GetK2hdkcDbgMode  
  Returns the message output level(DKCDBG_SILENT / DKCDBG_ERR / DKCDBG_WARN / DKCDBG_MSG / DKCDBG_DUMP).
- LoadK2hdkcDbgEnv / SetK2hdkcDbgFile / UnsetK2hdkcDbgFile / IsK2hdkcComLog  
  Returns true if it succeeds, false if it fails.
- GetK2hdkcDbgFile  
  Returns the file path to output message if it is set.

#### Note
For environment variables DKCDBGMODE, DKCDBGFILE, see [Environments](environments.html).

<!-- -----------------------------------------------------------　-->
***

### <a name="COMFACTORY"> Command Class Factory(C++ I/F)
The class factory creates a communication command class object.

#### Description
There are two kinds of class factories for Non-persistence chmpx connection and Persistence chmpx connection.  
These factories are built by MACRO.
The types of class factory types and the communication command classes to be returned are summarized below.

#### Class factory for Non-persistence chmpx connection
These functions(MACROs) have a prefix of GetOt...().
- GetOtSlaveK2hdkcComGet  
  Get the K2hdkcComGet class object to get the value for the key.
- GetOtSlaveK2hdkcComGetDirect  
  Get the K2hdkcComGetDirect class object to get the value with offset for the key.
- GetOtSlaveK2hdkcComGetSubkeys  
  Get the K2hdkcComGetSubkeys class object to get the Subkey list for the key.
- GetOtSlaveK2hdkcComGetAttrs  
  Get the K2hdkcComGetAttrs class object to get attributes for the key.
- GetOtSlaveK2hdkcComGetAttr  
  Get the K2hdkcComGetAttr class object to get the attribute for the key.
- GetOtSlaveK2hdkcComSet  
  Get the K2hdkcComSet class object to set the value for the key.
- GetOtSlaveK2hdkcComSetDirect  
  Get the K2hdkcComSetDirect class object to set the value with offset for the key.
- GetOtSlaveK2hdkcComSetSubkeys  
  Get the K2hdkcComSetSubkeys class object to set the Subkey list for the key.
- GetOtSlaveK2hdkcComSetAll  
  Get the K2hdkcComSetAll class object to set the value/Subkey list for the key.
- GetOtSlaveK2hdkcComAddSubkeys  
  Get the K2hdkcComAddSubkeys class object to add the Subkey list for the key.
- GetOtSlaveK2hdkcComAddSubkey  
  Get the K2hdkcComAddSubkey class object to set the Subkey and Subkey's value list for the parent key.
- GetOtSlaveK2hdkcComDel  
  Get the K2hdkcComDel class object to remove the key.
- GetOtSlaveK2hdkcComDelSubkeys  
  Get the K2hdkcComDelSubkeys class object to remove the Subkey from the key's Subkey list.
- GetOtSlaveK2hdkcComDelSubkey  
  Get the K2hdkcComDelSubkey class object to remove the Subkey from the key's Subkey list and remove Subkey.
- GetOtSlaveK2hdkcComRen  
  Get the K2hdkcComRen class object to rename the key.
- GetOtSlaveK2hdkcComQPush  
  Get the K2hdkcComQPush class object to push the value(or key and value) to the queue.
- GetOtSlaveK2hdkcComQPop  
  Get the K2hdkcComQPop class object to pop the value(or key and value) from the queue.
- GetOtSlaveK2hdkcComQDel  
  Get the K2hdkcComQDel class object to remove the value(or key and value) from the queue.
- GetOtSlaveK2hdkcComCasInit  
  Get the K2hdkcComCasInit class object to initialize the value for CAS(Compare And Swap) operation.
- GetOtSlaveK2hdkcComCasGet  
  Get the K2hdkcComCasGet class object to get the value for CAS(Compare And Swap) operation.
- GetOtSlaveK2hdkcComCasSet  
  Get the K2hdkcComCasSet class object to set the value for CAS(Compare And Swap) operation.
- GetOtSlaveK2hdkcComCasIncDec  
  Get the K2hdkcComCasIncDec class object to increment/decrement the value for CAS(Compare And Swap) operation.
- GetOtSlaveK2hdkcComState  
  Get the K2hdkcComState class object to get the status of all of k2hdkc nodes.

#### Class factory for Persistence chmpx connection
These functions(MACROs) have a prefix of GetPm...().
- GetPmSlaveK2hdkcComGet
  Get the K2hdkcComGet class object to get the value for the key.
- GetPmSlaveK2hdkcComGetDirect
  Get the K2hdkcComGetDirect class object to get the value with offset for the key.
- GetPmSlaveK2hdkcComGetSubkeys
  Get the K2hdkcComGetSubkeys class object to get the Subkey list for the key.
- GetPmSlaveK2hdkcComGetAttrs
  Get the K2hdkcComGetAttrs class object to get attributes for the key.
- GetPmSlaveK2hdkcComGetAttr
  Get the K2hdkcComGetAttr class object to get the attribute for the key.
- GetPmSlaveK2hdkcComSet
  Get the K2hdkcComSet class object to set the value for the key.
- GetPmSlaveK2hdkcComSetDirect
  Get the K2hdkcComSetDirect class object to set the value with offset for the key.
- GetPmSlaveK2hdkcComSetSubkeys
  Get the K2hdkcComSetSubkeys class object to set the Subkey list for the key.
- GetPmSlaveK2hdkcComSetAll
  Get the K2hdkcComSetAll class object to set the value/Subkey list for the key.
- GetPmSlaveK2hdkcComAddSubkeys
  Get the K2hdkcComAddSubkeys class object to add the Subkey list for the key.
- GetPmSlaveK2hdkcComAddSubkey
  Get the K2hdkcComAddSubkey class object to set the Subkey and Subkey's value list for the parent key.
- GetPmSlaveK2hdkcComDel
  Get the K2hdkcComDel class object to remove the key.
- GetPmSlaveK2hdkcComDelSubkeys
  Get the K2hdkcComDelSubkeys class object to remove the Subkey from the key's Subkey list.
- GetPmSlaveK2hdkcComDelSubkey
  Get the K2hdkcComDelSubkey class object to remove the Subkey from the key's Subkey list and remove Subkey.
- GetPmSlaveK2hdkcComRen
  Get the K2hdkcComRen class object to rename the key.
- GetPmSlaveK2hdkcComQPush
  Get the K2hdkcComQPush class object to push the value(or key and value) to the queue.
- GetPmSlaveK2hdkcComQPop
  Get the K2hdkcComQPop class object to pop the value(or key and value) from the queue.
- GetPmSlaveK2hdkcComQDel
  Get the K2hdkcComQDel class object to remove the value(or key and value) from the queue.
- GetPmSlaveK2hdkcComCasInit
  Get the K2hdkcComCasInit class object to initialize the value for CAS(Compare And Swap) operation.
- GetPmSlaveK2hdkcComCasGet
  Get the K2hdkcComCasGet class object to get the value for CAS(Compare And Swap) operation.
- GetPmSlaveK2hdkcComCasSet
  Get the K2hdkcComCasSet class object to set the value for CAS(Compare And Swap) operation.
- GetPmSlaveK2hdkcComCasIncDec
  Get the K2hdkcComCasIncDec class object to increment/decrement the value for CAS(Compare And Swap) operation.
- GetPmSlaveK2hdkcComState
  Get the K2hdkcComState class object to get the status of all of k2hdkc nodes.

#### Parameters
- config  
  Specify the configuration of the chmpx slave node(INI/YAML/JSON format file path, or JSON character string). If NULL is set, loads configuration file path(string data) from environments(K2HDKCCONFFILE / K2HDKCJSONCONF).
- ctlport  
  Specify the control port number of the chmpx slave node(It is necessary to specify it when multiple chmpx are running on the same server).
- cuk  
  Specify the CUK string of the chmpx slave node(It is necessary to specify it when multiple chmpx are running on the same server).
- is_auto_rejoin  
  Specify whether to reconnect automatically when the connection with the slave chmpx node is disconnected.
- no_giveup_rejoin  
  Case of specifying to reconnect automatically to the slave node of chmpx, specify the upper limit of retrying.
- comnum  
  Specify the sequence number of communication for k2hdkc cluster. This value is specified only during debugging. Normally you do not need to specify it. 
- pslaveobj  
  Specifies the pointer of K2hdkcSlave class instance when Persistence chmpx connection.

#### Return Values
These functions return a pointer to the corresponding communication command class object.  
Please delete the returned class object when it becomes unnecessary.

#### Examples
```
//-------------------------------------
// One time connection type
//-------------------------------------
K2hdkcComGet*    pComObj = GetOtSlaveK2hdkcComGet(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin);
bool        result = pComObj->CommandSend(pkey, keylength, !is_noattr, passphrase, &pconstval, &vallength, &rescode);
delete pComObj;
 
//-------------------------------------
// Permanent connection type
//-------------------------------------
// Open chmpx
K2hdkcSlave* pSlave = new K2hdkcSlave();
if(!pSlave->Initialize(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin)){
    cerr << "Could not join slave node chmpx." << endl;
    delete pSlave;
    return false;
}
if(!pSlave->Open(isNoGiveupRejoin)){
    cerr << "Could not open msgid on slave node chmpx." << endl;
    delete pSlave;
    return false;
}
for(int cnt = 0; cnt < 10; ++cnt){    // loop for example
    K2hdkcComGet*    pComObj= GetPmSlaveK2hdkcComGet(pSlave);
    bool        result = pComObj->CommandSend(pkey, keylength, !is_noattr, passphrase, &pconstval, &vallength, &rescode);
    delete pComObj;
}
if(!pSlave->Clean(isCleanupBup)){
    cerr << "Could not leave slave node chmpx, but continue for cleanup..." << endl;
}
delete pSlave;
```

<!-- -----------------------------------------------------------　-->
***

### <a name="GETCPP"> K2hdkcComGet class
This class is operation for getting the value for the key.

#### Format
- bool K2hdkcComGet::CommandSend(const unsigned char* pkey, size_t keylength, bool checkattr = true, const char* encpass = NULL)
- bool K2hdkcComGet::CommandSend(const unsigned char* pkey, size_t keylength, bool checkattr, const char* encpass, const unsigned char** ppval, size_t* pvallength, dkcres_type_t* prescode)
- bool K2hdkcComGet::GetResponseData(const unsigned char** ppdata, size_t* plength, dkcres_type_t* prescode) const

#### Parameters
- pkey  
  For this argument, specify a pointer to a key as binary.
- keylength  
  Specifies the length of the key.
- checkattr  
  Specifies whether to check the attribute(passphrase, expiration date) for the key.
- encpass  
  Specify the passphrase for decrypting the value.
- ppval  
  Specifies a pointer to the buffer that stores the value as binary.
- pvallength  
  Specifies the buffer length of the value as binary.
- prescode  
  Specify a pointer to store the processing result code.

#### Return Values
Returns true if it succeeds, false if it fails.

#### Note
There are two types of CommandSend() methods, one of which has a method type that sends commands and acquires results at once.

#### Examples
```
K2hdkcComGet* pComObj = GetOtSlaveK2hdkcComGet(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin);
bool result = pComObj->CommandSend(pkey, keylength, !is_noattr, passphrase, &pconstval, &vallength, &rescode);
```
<!-- -----------------------------------------------------------　-->
***

### <a name="GETDIRECTCPP"> K2hdkcComGetDirect class
This class is operation for getting the value by specifying the key and offset of the value.

#### Format
- bool K2hdkcComGetDirect::CommandSend(const unsigned char* pkey, size_t keylength, off_t val_pos, size_t val_length)
- bool K2hdkcComGetDirect::CommandSend(const unsigned char* pkey, size_t keylength, off_t val_pos, size_t val_length, const unsigned char** ppval, size_t* pvallength, dkcres_type_t* prescode)
- bool K2hdkcComGetDirect::GetResponseData(const unsigned char** ppdata, size_t* plength, dkcres_type_t* prescode) const

#### Parameters
- pkey  
  For this argument, specify a pointer to a key as binary.
- keylength  
  Specifies the length of the key.
- val_pos  
  Specifies offset for start of value.
- val_length  
  Specifies the length of getting the value.
- ppval  
  Specifies a pointer to the buffer that stores the value as binary.
- pvallength  
  Specifies the buffer length of the value as binary.
- prescode  
  Specify a pointer to store the processing result code.

#### Return Values
Returns true if it succeeds, false if it fails.

#### Note
There are two types of CommandSend() methods, one of which has a method type that sends commands and acquires results at once.

#### Examples
```
K2hdkcComGetDirect*    pComObj = GetOtSlaveK2hdkcComGetDirect(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin);
bool pComObj->CommandSend(pkey, keylen, offset, length, &pconstval, &vallen, &rescode);
```

<!-- -----------------------------------------------------------　-->
***

### <a name="GETSUBSCPP"> K2hdkcComGetSubkeys class
This class is operation for getting the Subkey list for the key.

#### Format
- bool K2hdkcComGetSubkeys::CommandSend(const unsigned char* pkey, size_t keylength, bool checkattr = true)
- bool K2hdkcComGetSubkeys::CommandSend(const unsigned char* pkey, size_t keylength, bool checkattr, K2HSubKeys** ppSubKeys, dkcres_type_t* prescode)
- bool K2hdkcComGetSubkeys::GetResponseData(K2HSubKeys** ppSubKeys, dkcres_type_t* prescode) const

#### Parameters
- pkey  
  For this argument, specify a pointer to a key as binary.
- keylength  
  Specifies the length of the key.
- checkattr  
  Specifies whether to check the attribute(passphrase, expiration date) for the key.
- ppSubKeys  
  Specifies a pointer for getting K2HSubkeys class object.
- prescode  
  Specify a pointer to store the processing result code.

#### Return Values
Returns true if it succeeds, false if it fails.サブキーを持たない場合にもfalseを返します。

#### Note
There are two types of CommandSend() methods, one of which has a method type that sends commands and acquires results at once.  
For the operation method of the K2HSubKeys class for the subkey list, see the document of [K2HASH](https://k2hash.antpick.ax/).

#### Examples
```
K2HSubKeys*           pSubKeys= NULL;
K2hdkcComGetSubkeys*  pComObj = GetOtSlaveK2hdkcComGetSubkeys(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin);
bool                  result  = pComObj->CommandSend(pkey, keylength, !is_noattr, &pSubKeys, &rescode);
```

<!-- -----------------------------------------------------------　-->
***

### <a name="GETATTRSCPP"> K2hdkcComGetAttrs class
This class is operation for getting the attributes for the key.

#### Format
- bool K2hdkcComGetAttrs::CommandSend(const unsigned char* pkey, size_t keylength)
- bool K2hdkcComGetAttrs::CommandSend(const unsigned char* pkey, size_t keylength, K2HAttrs** ppAttrsObj, dkcres_type_t* prescode)
- bool K2hdkcComGetAttrs::GetResponseData(K2HAttrs** ppAttrsObj, dkcres_type_t* prescode) const

#### Parameters
- pkey  
  For this argument, specify a pointer to a key as binary.
- keylength  
  Specifies the length of the key.
- ppAttrsObj  
  Specifies a pointer for getting K2HAttrs class object.
- prescode  
  Specify a pointer to store the processing result code.

#### Return Values
Returns true if it succeeds, false if it fails. If the key does not have any attribute, returns false.
Since the data of the k2hdkc cluster always has **mtime** attribute which is one of attribute, there is no reply without attribute.

#### Note
There are two types of CommandSend() methods, one of which has a method type that sends commands and acquires results at once.  
For the operation method of the K2HAttrs class for the subkey list, see the document of [K2HASH](https://k2hash.antpick.ax/).

#### Examples
```
K2HAttrs*            pAttrsObj    = NULL;
K2hdkcComGetAttrs*    pComObj     = GetOtSlaveK2hdkcComGetAttrs(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin);
bool            result = pComObj->CommandSend(reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, &pAttrsObj, &rescode);
```

<!-- -----------------------------------------------------------　-->
***

### <a name="GETATTRCPP"> K2hdkcComGetAttr class
This class is operation for getting the attribute value by specifying the key and attribute name as binary.

#### Format
- bool K2hdkcComGetAttr::CommandSend(const unsigned char* pkey, size_t keylength, const unsigned char* pattr, size_t attrlength)
- bool K2hdkcComGetAttr::CommandSend(const unsigned char* pkey, size_t keylength, const unsigned char* pattr, size_t attrlength, const unsigned char** ppattrval, size_t* pattrvallength, dkcres_type_t* prescode)
- bool K2hdkcComGetAttr::GetResponseData(const unsigned char** ppattrval, size_t* pattrvallength, dkcres_type_t* prescode) const

#### Parameters
- pkey  
  For this argument, specify a pointer to a key as binary.
- keylength  
  Specifies the length of the key.
- pattr  
  Specifies a pointer to a attribute name as binary.
- attrlength  
  Specifies the length of the attribute name.
- ppattrval  
  Specifies a pointer for storing the attribute value as binary.
- pattrvallength  
  Specifies the length of buffer which is stored the attribute value as binary.
- prescode  
  Specify a pointer to store the processing result code.

#### Return Values
Returns true if it succeeds, false if it fails. If the key does not have any attribute, returns false.

#### Note
There are two types of CommandSend() methods, one of which has a method type that sends commands and acquires results at once.

#### Examples
```
K2hdkcComGetAttrs*    pComObj     = GetOtSlaveK2hdkcComGetAttr(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin);
bool            result = pComObj->CommandSend(reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, pattrname, attrnamelen, &pAttrval, attrvallen, &rescode);
```

<!-- -----------------------------------------------------------　-->
***

### <a name="SETAPP"> K2hdkcComSet class
This class is operation for setting/creating the key and value.

#### Format
- bool K2hdkcComSet::CommandSend(const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, bool rm_subkeylist = false, const char* encpass = NULL, const time_t* expire = NULL)
- bool K2hdkcComSet::CommandSend(const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, bool rm_subkeylist, const char* encpass, const time_t* expire, dkcres_type_t* prescode)
- bool K2hdkcComSet::GetResponseData(dkcres_type_t* prescode) const;

#### Parameters
- pkey  
  For this argument, specify a pointer to a key as binary.
- keylength  
  Specifies the length of the key.
- pval  
  Specifies a pointer for the value as binary.
- vallength  
  Specifies length of binary value.
- rm_subkeylist  
  If the key exists and has a Subkey list, it specifies whether or not to delete that subkey list(Do not delete the subkey and value).
- encpass  
  Specify the passphrase for encrypting the value.
- expire  
  To set an expiration date for a value, specify the expiration date.
- prescode  
  Specify a pointer to store the processing result code.

#### Return Values
Returns true if it succeeds, false if it fails.

#### Note
There are two types of CommandSend() methods, one of which has a method type that sends commands and acquires results at once.  
Except for the two CommandSend() methods, they are methods used internally and can not be used.

#### Examples
```
K2hdkcComSet*    pComObj = GetOtSlaveK2hdkcComSet(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin);
bool            result = pComObj->CommandSend(pKey, KeyLen, pVal, ValLen, is_rmsublist, pPassPhrase, pExpire, &rescode);
```

<!-- -----------------------------------------------------------　-->
***

### <a name="SETDIRECTCPP"> K2hdkcComSetDirect class
This class is operation for setting/creating the key and value with offset.

#### Format
- bool K2hdkcComSetDirect::CommandSend(const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, const off_t valpos)
- bool K2hdkcComSetDirect::CommandSend(const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, const off_t valpos, dkcres_type_t* prescode)
- bool K2hdkcComSetDirect::GetResponseData(dkcres_type_t* prescode) const

#### Parameters
- pkey  
  For this argument, specify a pointer to a key as binary.
- keylength  
  Specifies the length of the key.
- pval  
  Specifies a pointer for the value as binary.
- vallength  
  Specifies length of binary value.
- valpos  
  Specifies the offset for the value.
- prescode  
  Specify a pointer to store the processing result code.

#### Return Values
Returns true if it succeeds, false if it fails.

#### Note
There are two types of CommandSend() methods, one of which has a method type that sends commands and acquires results at once.

#### Examples
```
K2hdkcComSetDirect*    pComObj = GetOtSlaveK2hdkcComSetDirect(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin);
bool            result = pComObj->CommandSend(pkey, keylen, pval, vallen, offset, &rescode);
```

<!-- -----------------------------------------------------------　-->
***

### <a name="SETSUBSCPP"> K2hdkcComSetSubkeys class
This class is operation for adding the Subkey list to the key(and can remove all Subkey list).

#### Format
- bool K2hdkcComSetSubkeys::CommandSend(const unsigned char* pkey, size_t keylength, const unsigned char* psubkeys, size_t subkeyslength)
- bool K2hdkcComSetSubkeys::CommandSend(const unsigned char* pkey, size_t keylength, const unsigned char* psubkeys, size_t subkeyslength, dkcres_type_t* prescode)
- bool K2hdkcComSetSubkeys::ClearSubkeysCommandSend(const unsigned char* pkey, size_t keylength)
- bool K2hdkcComSetSubkeys::ClearSubkeysCommandSend(const unsigned char* pkey, size_t keylength, dkcres_type_t* prescode)
- bool K2hdkcComSetSubkeys::GetResponseData(dkcres_type_t* prescode) const;

#### Parameters
- pkey  
  For this argument, specify a pointer to a key as binary.
- keylength  
  Specifies the length of the key.
- psubkeys  
  Specifies the Subkey list as binary to set. This value is the binary data created by the Serialize() method of the K2HSubKeys class.
- subkeyslength  
  Specifies the length of the Subkey list as binary.
- prescode  
  Specify a pointer to store the processing result code.

#### Return Values
Returns true if it succeeds, false if it fails.

#### Note
There are two types of CommandSend() methods, one of which has a method type that sends commands and acquires results at once.
The ClearSubkeysCommandSend() methods is a method to clear the Subkey list.

#### Examples
```
unsigned char*  psubkeys = NULL;
size_t        subkeyslength = 0;
K2HSubKeys    Subkeys;
Subkeys.insert(psubkey, subkeylength);        // Add one subkey
if(!Subkeys.Serialize(&psubkeys, &subkeyslength)){
    return false;
}
 
K2hdkcComSetSubkeys*    pComObj = GetOtSlaveK2hdkcComSetSubkeys(conffile, ctlport, NULL/* cuk is NULL */, is_auto_rejoin, no_giveup_rejoin);
bool            result = pComObj->CommandSend(pPKey, PKeyLen, psubkeys, subkeyslength , &rescode);
 
DKC_FREE(psubkeys);
```

<!-- -----------------------------------------------------------　-->
***

### <a name="SETALLCPP"> K2hdkcComSetAll class
This class is operation for set the value and Subkey list and attributes to the key.

#### Format
- bool K2hdkcComSetAll::CommandSend(const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, const unsigned char* psubkeys, size_t subkeyslength, const unsigned char* pattrs, size_t attrslength)
- bool K2hdkcComSetAll::CommandSend(const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, const unsigned char* psubkeys, size_t subkeyslength, const unsigned char* pattrs, size_t attrslength, dkcres_type_t* prescode)
- bool K2hdkcComSetAll::GetResponseData(dkcres_type_t* prescode) const

#### Parameters
- pkey  
  For this argument, specify a pointer to a key as binary.
- keylength  
  Specifies the length of the key.
- pval  
  Specifies a pointer for the value as binary.
- vallength  
  Specifies length of binary value.
- psubkeys  
  Specifies the Subkey list as binary to set. This value is the binary data created by the Serialize() method of the K2HSubKeys class.
- subkeyslength  
  Specifies the length of the Subkey list as binary.
- pattrs  
  Specifies the attributes as binary to set. This value is the binary data created by the Serialize() method of the K2HAttrs class.
- attrslength  
  Specifies the length of the attributes as binary.
- prescode  
  Specify a pointer to store the processing result code.

#### Return Values
Returns true if it succeeds, false if it fails.

#### Note
There are two types of CommandSend() methods, one of which has a method type that sends commands and acquires results at once.

#### Examples
```
unsigned char*  psubkeys = NULL;
size_t        subkeyslength = 0;
K2HSubKeys    Subkeys;
Subkeys.insert(psubkey, subkeylength);        // Add one subkey
if(!Subkeys.Serialize(&psubkeys, &subkeyslength)){
    return false;
}
 
K2hdkcComSetAll*    pComObj = GetOtSlaveK2hdkcComSetAll(conffile, ctlport, NULL/* cuk is NULL */, is_auto_rejoin, no_giveup_rejoin);
bool            result = pComObj->CommandSend(pPKey, PKeyLen, psubkeys, subkeyslength, NULL, 0, &rescode);
 
DKC_FREE(psubkeys);
```

<!-- -----------------------------------------------------------　-->
***

### <a name="ADDSUBSCPP"> K2hdkcComAddSubkeys class
This class is operation for adding the Subkey to the key's Subkey list.

#### Format
- bool K2hdkcComAddSubkeys::CommandSend(const unsigned char* pkey, size_t keylength, const unsigned char* psubkey, size_t subkeylength, bool checkattr = true)
- bool K2hdkcComAddSubkeys::CommandSend(const unsigned char* pkey, size_t keylength, const unsigned char* psubkey, size_t subkeylength, bool checkattr, dkcres_type_t* prescode)
- bool K2hdkcComAddSubkeys::GetResponseData(dkcres_type_t* prescode) const

#### Parameters
- pkey  
  For this argument, specify a pointer to a key as binary.
- keylength  
  Specifies the length of the key.
- psubkey  
  Specifies the Subkey name as binary.
- subkeylength  
  Specifies the length of the Subkey name.
- checkattr  
  Specifies whether to check the attribute(passphrase, expiration date) for the key.
- prescode  
  Specify a pointer to store the processing result code.

#### Return Values
Returns true if it succeeds, false if it fails.

#### Note
There are two types of CommandSend() methods, one of which has a method type that sends commands and acquires results at once.

#### Examples
```
K2hdkcComAddSubkeys*    pComObj = GetOtSlaveK2hdkcComAddSubkeys(conffile, ctlport, NULL/* cuk is NULL */, is_auto_rejoin, no_giveup_rejoin);
bool            result = pComObj->CommandSend(pPKey, PKeyLen, psubkey, subkeylen, true, &rescode);
```

<!-- -----------------------------------------------------------　-->
***

### <a name="ADDSUBCPP"> K2hdkcComAddSubkey class
This class is operation for creating the Subkey and value and adding it to the key's Subkey list.

#### Format
- bool K2hdkcComAddSubkey::CommandSend(const unsigned char* pkey, size_t keylength, const unsigned char* psubkey, size_t subkeylength, const unsigned char* pskeyval, size_t skeyvallength, bool checkattr = true, const char* encpass = NULL, const time_t* expire = NULL)
- bool K2hdkcComAddSubkey::CommandSend(const unsigned char* pkey, size_t keylength, const unsigned char* psubkey, size_t subkeylength, const unsigned char* pskeyval, size_t skeyvallength, bool checkattr, const char* encpass, const time_t* expire, dkcres_type_t* prescode)
- bool K2hdkcComAddSubkey::GetResponseData(dkcres_type_t* prescode) const

#### Parameters
- pkey  
  For this argument, specify a pointer to a key as binary.
- keylength  
  Specifies the length of the key.
- psubkey  
  Specifies the Subkey name as binary.
- subkeylength  
  Specifies the length of the Subkey name.
- pskeyval  
  Specifies the Subkey's value as binary.
- skeyvallength  
  Specifies the length of the Subkey's value.
- checkattr  
  Specifies whether to check the attribute(passphrase, expiration date) for the key.
- encpass  
  Specify the passphrase for encrypting the value of Subkey.
- expire  
  To set an expiration date for a value of Subkey, specify the expiration date.
- prescode  
  Specify a pointer to store the processing result code.

#### Return Values
Returns true if it succeeds, false if it fails.

#### Note
There are two types of CommandSend() methods, one of which has a method type that sends commands and acquires results at once.

#### Examples
```
K2hdkcComAddSubkey*    pComObj = GetOtSlaveK2hdkcComAddSubkey(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin);
bool            result = pComObj->CommandSend(pParentKey, ParentKeyLen, pSubKey, SubKeyLen, pVal, ValLen, !is_noattr, pPassPhrase, pExpire, &rescode);
```

<!-- -----------------------------------------------------------　-->
***

### <a name="DELCPP"> K2hdkcComDel class
This class is operation for removing the key.

#### Format
- bool K2hdkcComDel::CommandSend(const unsigned char* pkey, size_t keylength, bool is_subkeys, bool checkattr = true)
- bool K2hdkcComDel::CommandSend(const unsigned char* pkey, size_t keylength, bool is_subkeys, bool checkattr, dkcres_type_t* prescode)
- bool K2hdkcComDel::GetResponseData(dkcres_type_t* prescode) const

#### Parameters
- pkey  
  For this argument, specify a pointer to a key as binary.
- keylength  
  Specifies the length of the key.
- checkattr  
  Specifies whether to check the attribute(passphrase, expiration date) for the key.
- prescode  
  Specify a pointer to store the processing result code.

#### Return Values
Returns true if it succeeds, false if it fails.

#### Note
There are two types of CommandSend() methods, one of which has a method type that sends commands and acquires results at once.

#### Examples
```
K2hdkcComDel*    pComObj = GetOtSlaveK2hdkcComDel(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin);
bool            result = pComObj->CommandSend(pkey, keylen, is_subkeys, true, &rescode);
```

<!-- -----------------------------------------------------------　-->
***

### <a name="DELSUBSCPP"> K2hdkcComDelSubkeys class
This class is operation for removing the Subkey from the key's Subkey list.

#### Format
- bool K2hdkcComDelSubkeys::CommandSend(const unsigned char* pkey, size_t keylength, const unsigned char* psubkey, size_t subkeylength, bool checkattr = true)
- bool K2hdkcComDelSubkeys::CommandSend(const unsigned char* pkey, size_t keylength, const unsigned char* psubkey, size_t subkeylength, bool checkattr, dkcres_type_t* prescode)
- bool K2hdkcComDelSubkeys::GetResponseData(dkcres_type_t* prescode) const

#### Parameters
- pkey  
  For this argument, specify a pointer to a key as binary.
- keylength  
  Specifies the length of the key.
- psubkey  
  Specifies a pointer to Subkey name as binary.
- subkeylength  
  Specifies the length of the Subkey name.
- checkattr  
  Specifies whether to check the attribute(passphrase, expiration date) for the key.
- prescode  
  Specify a pointer to store the processing result code.

#### Return Values
Returns true if it succeeds, false if it fails.

#### Note
There are two types of CommandSend() methods, one of which has a method type that sends commands and acquires results at once.

#### Examples
```
K2hdkcComDelSubkeys*    pComObj = GetOtSlaveK2hdkcComDelSubkeys(conffile, ctlport, NULL/* cuk is NULL */, is_auto_rejoin, no_giveup_rejoin);
bool            result = pComObj->CommandSend(pPKey, PKeyLen, psubkey, subkeylen, true, &rescode);
```

<!-- -----------------------------------------------------------　-->
***

### <a name="DELSUBCPP"> K2hdkcComDelSubkey class
This class is operation for removing the Subkey from the key's Subkey list and remove the Subkey.

#### Format
- bool K2hdkcComDelSubkey::CommandSend(const unsigned char* pkey, size_t keylength, const unsigned char* psubkey, size_t subkeylength, bool is_subkeys, bool checkattr = true)
- bool K2hdkcComDelSubkey::CommandSend(const unsigned char* pkey, size_t keylength, const unsigned char* psubkey, size_t subkeylength, bool is_subkeys, bool checkattr, dkcres_type_t* prescode)
- bool K2hdkcComDelSubkey::GetResponseData(dkcres_type_t* prescode) const

#### Parameters
- pkey  
  For this argument, specify a pointer to a key as binary.
- keylength  
  Specifies the length of the key.
- psubkey  
  Specifies a pointer to Subkey name as binary.
- subkeylength  
  Specifies the length of the Subkey name.
- checkattr  
  Specifies whether to check the attribute(passphrase, expiration date) for the key.
- prescode  
  Specify a pointer to store the processing result code.

#### Return Values
Returns true if it succeeds, false if it fails.

#### Note
There are two types of CommandSend() methods, one of which has a method type that sends commands and acquires results at once.

#### Examples
```
K2hdkcComDelSubkey*    pComObj = GetOtSlaveK2hdkcComDelSubkey(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin);
bool            result = pComObj->CommandSend(pParentKey, ParentKeyLen, pSubKey, SubKeyLen, pVal, ValLen, true, &rescode);
```

<!-- -----------------------------------------------------------　-->
***

### <a name="RENCPP"> K2hdkcComRen class
This class is operation for renaming the key.  
If the parent key is specified, the Subkey name is removed from the parent's Subkey list.

#### Format
- bool K2hdkcComRen::CommandSend(const unsigned char* poldkey, size_t oldkeylength, const unsigned char* pnewkey, size_t newkeylength, const unsigned char* pparentkey = NULL, size_t parentkeylength = 0, bool checkattr = true, const char* encpass = NULL, const time_t* expire = NULL)
- bool K2hdkcComRen::CommandSend(const unsigned char* poldkey, size_t oldkeylength, const unsigned char* pnewkey, size_t newkeylength, bool checkattr = true, const char* encpass = NULL, const time_t* expire = NULL)
- bool K2hdkcComRen::CommandSend(const unsigned char* poldkey, size_t oldkeylength, const unsigned char* pnewkey, size_t newkeylength, const unsigned char* pparentkey, size_t parentkeylength, bool checkattr, const char* encpass, const time_t* expire, dkcres_type_t* prescode)
- bool K2hdkcComRen::CommandSend(const unsigned char* poldkey, size_t oldkeylength, const unsigned char* pnewkey, size_t newkeylength, bool checkattr, const char* encpass, const time_t* expire, dkcres_type_t* prescode)
- bool K2hdkcComRen::GetResponseData(dkcres_type_t* prescode) const;

#### Parameters
- poldkey  
  Specifies a pointer to old key name as binary.
- oldkeylength  
  Specifies the length of old key.
- pnewkey  
  Specifies a pointer to new key name as binary.
- newkeylength  
  Specifies the length of new key.
- pparentkey  
  Specifies a pointer to old parent key as binary which has the key in it's Subkey list.
- parentkeylength  
  Specifies the length of parent key.
- checkattr  
  Specifies whether to check the attribute(passphrase, expiration date) for parent key.
- encpass  
  Specify the passphrase for encrypting the value.
- expire  
  To set an expiration date for new key's value, specify the expiration date.
- prescode  
  Specify a pointer to store the processing result code.

#### Return Values
Returns true if it succeeds, false if it fails.

#### Note
There are two types of CommandSend() methods, one of which has a method type that sends commands and acquires results at once.

#### Examples
```
K2hdkcComRen*    pComObj = GetOtSlaveK2hdkcComRen(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin);
bool            result = pComObj->CommandSend(pOldKey, OldKeyLen, pNewKey, NewkeyLen, pParentKey, pParentKeyLen, !is_noattr, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), (-1 == Expire ? NULL : &Expire), &rescode);
```

<!-- -----------------------------------------------------------　-->
***

### <a name="QUEUEPUSHCPP"> K2hdkcComQPush class
This class is operation for pushing the value(or key and value) to the queue.

#### Format
- bool K2hdkcComQPush::QueueCommandSend(const unsigned char* pprefix, size_t prefixlength, const unsigned char* pval, size_t vallength, bool is_fifo, const unsigned char* pattrs = NULL, size_t attrslength = 0L, bool checkattr = true, const char* encpass = NULL, const time_t* expire = NULL)
- bool K2hdkcComQPush::QueueCommandSend(const unsigned char* pprefix, size_t prefixlength, const unsigned char* pval, size_t vallength, bool is_fifo, const unsigned char* pattrs, size_t attrslength, bool checkattr, const char* encpass, const time_t* expire, dkcres_type_t* prescode)
- bool K2hdkcComQPush::KeyQueueCommandSend(const unsigned char* pprefix, size_t prefixlength, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, bool is_fifo, const unsigned char* pattrs = NULL, size_t attrslength = 0L, bool checkattr = true, const char* encpass = NULL, const time_t* expire = NULL)
- bool K2hdkcComQPush::KeyQueueCommandSend(const unsigned char* pprefix, size_t prefixlength, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, bool is_fifo, const unsigned char* pattrs, size_t attrslength, bool checkattr, const char* encpass, const time_t* expire, dkcres_type_t* prescode)
- bool K2hdkcComQPush::GetResponseData(dkcres_type_t* prescode) const;

#### Parameters
- pprefix  
  Specifies a pointer to the queue prefix name as binary.
- prefixlength  
  Specifies the length of the queue prefix name.
- pkey  
  For queue of types that set keys and values, specifies the key as binary to push.
- keylength  
  Specifies the length of pushed key.
- pval  
  Specifies a pointer to pushing value as binary.
- vallength  
  Specifies the length of pushing value as binary.
- is_fifo  
  Specifies pushing type for FIFO or LIFO.
- pattrs  
  Specifies the attributes as binary to push. This value is the binary data created by the Serialize() method of the K2HAttrs class.
- attrslength  
  Specifies the length of the attributes as binary.
- checkattr  
  Specifies whether to check the attribute(passphrase, expiration date) for the queue.
- encpass  
  Specify the passphrase for encrypting the pushing value.
- expire  
  To set an expiration date for pushing value, specify the expiration date.
- prescode  
  Specify a pointer to store the processing result code.

#### Return Values
Returns true if it succeeds, false if it fails.

#### Note
There are two types of CommandSend() methods, one of which has a method type that sends commands and acquires results at once.

#### Examples
```
K2hdkcComQPush*    pComObj = GetOtSlaveK2hdkcComQPush(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin);
bool            result = pComObj->QueueCommandSend(pName, NameLen, reinterpret_cast<const unsigned char*>(strValue.c_str()), strValue.length() + 1, is_Fifo, NULL, 0UL, !is_noattr, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), (-1 == Expire ? NULL : &Expire), &rescode);
```

<!-- -----------------------------------------------------------　-->
***

### <a name="QUEUEPOPCPP"> K2hdkcComQPop class
This class is operation for popping the value(or key and value) from the queue.

#### Format
- bool K2hdkcComQPop::QueueCommandSend(const unsigned char* pprefix, size_t prefixlength, bool is_fifo, bool checkattr = true, const char* encpass = NULL)
- bool K2hdkcComQPop::QueueCommandSend(const unsigned char* pprefix, size_t prefixlength, bool is_fifo, bool checkattr, const char* encpass, const unsigned char** ppval, size_t* pvallength, dkcres_type_t* prescode)
- bool K2hdkcComQPop::KeyQueueCommandSend(const unsigned char* pprefix, size_t prefixlength, bool is_fifo, bool checkattr = true, const char* encpass = NULL)
- bool K2hdkcComQPop::KeyQueueCommandSend(const unsigned char* pprefix, size_t prefixlength, bool is_fifo, bool checkattr, const char* encpass, const unsigned char** ppkey, size_t* pkeylength, const unsigned char** ppval, size_t* pvallength, dkcres_type_t* prescode)
- bool K2hdkcComQPop::GetQueueResponseData(const unsigned char** ppval, size_t* pvallength, dkcres_type_t* prescode) const
- bool K2hdkcComQPop::GetKeyQueueResponseData(const unsigned char** ppkey, size_t* pkeylength, const unsigned char** ppval, size_t* pvallength, dkcres_type_t* prescode) const

#### Parameters
- pprefix  
  Specifies a pointer to the queue prefix name as binary.
- prefixlength  
  Specifies the length of the queue prefix name.
- is_fifo  
  Specifies popping type for FIFO or LIFO.
- checkattr  
  Specifies whether to check the attribute(passphrase, expiration date) for the queue.
- encpass  
  Specify the passphrase for decrypting popping value(and key).
- ppkey  
  Specifies a pointer to the buffer that pops the key as binary.
- pkeylength  
  Specifies the buffer length of popping key as binary.
- ppval  
  Specifies a pointer to the buffer that pops the value as binary.
- pvallength  
  Specifies the buffer length of popping value as binary.
- prescode  
  Specify a pointer to store the processing result code.

#### Return Values
Returns true if it succeeds, false if it fails.

#### Note
There are two types of CommandSend() methods, one of which has a method type that sends commands and acquires results at once.

#### Examples
```
K2hdkcComQPop*    pComObj = GetOtSlaveK2hdkcComQPop(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin);
bool            result = pComObj->KeyQueueCommandSend(pName, NameLen, is_Fifo, !is_noattr, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), &pkeytmp, &keytmplen, &pvaltmp, &valtmplen, &rescode);
```

<!-- -----------------------------------------------------------　-->
***

### <a name="QUEUEDELCPP"> K2hdkcComQDel class
This class is operation for removing the value(or key and value) from the queue(can specify the number to delete).

#### Format
- bool K2hdkcComQDel::QueueCommandSend(const unsigned char* pprefix, size_t prefixlength, int count, bool is_fifo, bool checkattr, const char* encpass)
- bool K2hdkcComQDel::QueueCommandSend(const unsigned char* pprefix, size_t prefixlength, int count, bool is_fifo, bool checkattr, const char* encpass, dkcres_type_t* prescode)
- bool K2hdkcComQDel::KeyQueueCommandSend(const unsigned char* pprefix, size_t prefixlength, int count, bool is_fifo, bool checkattr = true, const char* encpass = NULL)
- bool K2hdkcComQDel::KeyQueueCommandSend(const unsigned char* pprefix, size_t prefixlength, int count, bool is_fifo, bool checkattr, const char* encpass, dkcres_type_t* prescode)
- bool K2hdkcComQDel::GetResponseData(dkcres_type_t* prescode) const

#### Parameters
- pprefix  
  Specifies a pointer to the queue prefix name as binary.
- prefixlength  
  Specifies the length of the queue prefix name.
- is_fifo  
  Specifies popping type for FIFO or LIFO.
- count  
  Specifies count of removing value(or key and value) from the queue.
- checkattr  
  Specifies whether to check the attribute(passphrase, expiration date) for the queue.
- encpass  
  Specify the passphrase for decrypting removing value(and key).
- prescode  
  Specify a pointer to store the processing result code.

#### Return Values
Returns true if it succeeds, false if it fails.

#### Note
There are two types of CommandSend() methods, one of which has a method type that sends commands and acquires results at once.

#### Examples
```
K2hdkcComQDel*    pComObj = GetOtSlaveK2hdkcComQDel(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin);
bool            result = pComObj->QueueCommandSend(pName, NameLen, RmCount, is_Fifo, true, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), &rescode);
```

<!-- -----------------------------------------------------------　-->
***

### <a name="CASINITCPP"> K2hdkcComCasInit class
This class is operation for initializing the key for operation CAS(Compare And Swap). There are methods for the size(8 / 16 / 32 / 64 bits) of the value.

#### Format
- bool K2hdkcComCasInit::CommandSend(const unsigned char* pkey, size_t keylength, uint8_t val, const char* encpass = NULL, const time_t* expire = NULL)
- bool K2hdkcComCasInit::CommandSend(const unsigned char* pkey, size_t keylength, uint16_t val, const char* encpass = NULL, const time_t* expire = NULL)
- bool K2hdkcComCasInit::CommandSend(const unsigned char* pkey, size_t keylength, uint32_t val, const char* encpass = NULL, const time_t* expire = NULL)
- bool K2hdkcComCasInit::CommandSend(const unsigned char* pkey, size_t keylength, uint64_t val, const char* encpass = NULL, const time_t* expire = NULL)
- bool K2hdkcComCasInit::CommandSend(const unsigned char* pkey, size_t keylength, uint8_t val, const char* encpass, const time_t* expire, dkcres_type_t* prescode)
- bool K2hdkcComCasInit::CommandSend(const unsigned char* pkey, size_t keylength, uint16_t val, const char* encpass, const time_t* expire, dkcres_type_t* prescode)
- bool K2hdkcComCasInit::CommandSend(const unsigned char* pkey, size_t keylength, uint32_t val, const char* encpass, const time_t* expire, dkcres_type_t* prescode)
- bool K2hdkcComCasInit::CommandSend(const unsigned char* pkey, size_t keylength, uint64_t val, const char* encpass, const time_t* expire, dkcres_type_t* prescode)
- bool K2hdkcComCasInit::GetResponseData(dkcres_type_t* prescode) const

#### Parameters
- pkey  
  For this argument, specify a pointer to a key as binary.
- keylength  
  Specifies the length of the key.
- val  
  Specify the value as initial value. It's length is 8/16/32/64 bits for each methods.
- encpass  
  Specify the passphrase for encrypting the value.
- expire  
  To set an expiration date for the key, specify the expiration date.
- prescode  
  Specify a pointer to store the processing result code.

#### Return Values
Returns true if it succeeds, false if it fails.

#### Note
There are two types of CommandSend() methods, one of which has a method type that sends commands and acquires results at once.

#### Examples
```
K2hdkcComCasInit*    pComObj = GetOtSlaveK2hdkcComCasInit(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin);
bool                 result  = pComObj->CommandSend(reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, val, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), (-1 == Expire ? NULL : &Expire), &rescode);
```

<!-- -----------------------------------------------------------　-->
***

### <a name="CASGETCPP"> K2hdkcComCasGet class
This class is operation for getting the value for operation CAS(Compare And Swap). There are methods for the size(8 / 16 / 32 / 64 bits) of the value.

#### Format
- bool K2hdkcComCasGet::CommandSend(const unsigned char* pkey, size_t keylength, bool checkattr, const char* encpass, const uint8_t** ppval, dkcres_type_t* prescode)
- bool K2hdkcComCasGet::CommandSend(const unsigned char* pkey, size_t keylength, bool checkattr, const char* encpass, const uint16_t** ppval, dkcres_type_t* prescode)
- bool K2hdkcComCasGet::CommandSend(const unsigned char* pkey, size_t keylength, bool checkattr, const char* encpass, const uint32_t** ppval, dkcres_type_t* prescode)
- bool K2hdkcComCasGet::CommandSend(const unsigned char* pkey, size_t keylength, bool checkattr, const char* encpass, const uint64_t** ppval, dkcres_type_t* prescode)
- bool K2hdkcComCasGet::GetResponseData(const uint8_t** ppval, dkcres_type_t* prescode) const
- bool K2hdkcComCasGet::GetResponseData(const uint16_t** ppval, dkcres_type_t* prescode) const
- bool K2hdkcComCasGet::GetResponseData(const uint32_t** ppval, dkcres_type_t* prescode) const
- bool K2hdkcComCasGet::GetResponseData(const uint64_t** ppval, dkcres_type_t* prescode) const

#### Parameters
- pkey  
  For this argument, specify a pointer to a key as binary.
- keylength  
  Specifies the length of the key.
- checkattr  
  Specifies whether to check the attribute(passphrase, expiration date) for the key.
- encpass  
  Specify the passphrase for encrypting the value.
- ppval  
  Specifies a pointer to the buffer that stores the value as binary.
- prescode  
  Specify a pointer to store the processing result code.

#### Return Values
Returns true if it succeeds, false if it fails.

#### Note
There are two types of CommandSend() methods, one of which has a method type that sends commands and acquires results at once.

#### Examples
```
K2hdkcComCasGet*    pComObj = GetOtSlaveK2hdkcComCasGet(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin);
bool                result = pComObj->CommandSend(reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, true, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), &pval8, &rescode);
```

<!-- -----------------------------------------------------------　-->
***

### <a name="CASSETCPP"> K2hdkcComCasSet class
This class is operation to setting the value for operation CAS(Compare And Swap). There are methods for the size(8 / 16 / 32 / 64 bits) of the value.

#### Format
- bool K2hdkcComCasSet::CommandSend(const unsigned char* pkey, size_t keylength, uint8_t oldval, uint8_t newval, bool checkattr = true, const char* encpass = NULL, const time_t* expire = NULL)
- bool K2hdkcComCasSet::CommandSend(const unsigned char* pkey, size_t keylength, uint16_t oldval, uint16_t newval, bool checkattr = true, const char* encpass = NULL, const time_t* expire = NULL)
- bool K2hdkcComCasSet::CommandSend(const unsigned char* pkey, size_t keylength, uint32_t oldval, uint32_t newval, bool checkattr = true, const char* encpass = NULL, const time_t* expire = NULL)
- bool K2hdkcComCasSet::CommandSend(const unsigned char* pkey, size_t keylength, uint64_t oldval, uint64_t newval, bool checkattr = true, const char* encpass = NULL, const time_t* expire = NULL)
- bool K2hdkcComCasSet::CommandSend(const unsigned char* pkey, size_t keylength, uint8_t oldval, uint8_t newval, bool checkattr, const char* encpass, const time_t* expire, dkcres_type_t* prescode)
- bool K2hdkcComCasSet::CommandSend(const unsigned char* pkey, size_t keylength, uint16_t oldval, uint16_t newval, bool checkattr, const char* encpass, const time_t* expire, dkcres_type_t* prescode)
- bool K2hdkcComCasSet::CommandSend(const unsigned char* pkey, size_t keylength, uint32_t oldval, uint32_t newval, bool checkattr, const char* encpass, const time_t* expire, dkcres_type_t* prescode)
- bool K2hdkcComCasSet::CommandSend(const unsigned char* pkey, size_t keylength, uint64_t oldval, uint64_t newval, bool checkattr, const char* encpass, const time_t* expire, dkcres_type_t* prescode)
- bool K2hdkcComCasSet::GetResponseData(dkcres_type_t* prescode) const

#### Parameters
- pkey  
  For this argument, specify a pointer to a key as binary.
- keylength  
  Specifies the length of the key.
- oldval  
  Specify old value as binary. It's length is 8/16/32/64 bits for each methods.
- newval  
  Specify new value as binary. It's length is 8/16/32/64 bits for each methods.
- checkattr  
  Specifies whether to check the attribute(passphrase, expiration date) for the key.
- encpass  
  Specify the passphrase for encrypting the value.
- expire  
  To set an expiration date for the key, specify the expiration date.
- prescode  
  Specify a pointer to store the processing result code.

#### Return Values
Returns true if it succeeds, false if it fails.

#### Note
There are two types of CommandSend() methods, one of which has a method type that sends commands and acquires results at once.

#### Examples
```
K2hdkcComCasSet*    pComObj = GetOtSlaveK2hdkcComCasSet(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin);
bool                result  = pComObj->CommandSend(reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, oldval, newval, true, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), (-1 == Expire ? NULL : &Expire), &rescode);
```

<!-- -----------------------------------------------------------　-->
***

### <a name="CASINCDECCPP"> K2hdkcComCasIncDec class
This class is operation to increment/decrement the value for operation CAS(Compare And Swap). There are methods for the size(8 / 16 / 32 / 64 bits) of the value.

#### Format
- bool K2hdkcComCasIncDec::IncrementCommandSend(const unsigned char* pkey, size_t keylength, bool checkattr = true, const char* encpass = NULL, const time_t* expire = NULL)
- bool K2hdkcComCasIncDec::DecrementCommandSend(const unsigned char* pkey, size_t keylength, bool checkattr = true, const char* encpass = NULL, const time_t* expire = NULL)
- bool K2hdkcComCasIncDec::IncrementCommandSend(const unsigned char* pkey, size_t keylength, bool checkattr, const char* encpass, const time_t* expire, dkcres_type_t* prescode)
- bool K2hdkcComCasIncDec::DecrementCommandSend(const unsigned char* pkey, size_t keylength, bool checkattr, const char* encpass, const time_t* expire, dkcres_type_t* prescode)
- bool K2hdkcComCasIncDec::GetResponseData(dkcres_type_t* prescode) const

#### Parameters
- pkey  
  For this argument, specify a pointer to a key as binary.
- keylength  
  Specifies the length of the key.
- checkattr  
  Specifies whether to check the attribute(passphrase, expiration date) for the key.
- encpass  
  Specify the passphrase for encrypting the value.
- expire  
  To set an expiration date for the key, specify the expiration date.
- prescode  
  Specify a pointer to store the processing result code.

#### Return Values
Returns true if it succeeds, false if it fails.

#### Note
There are two types of CommandSend() methods, one of which has a method type that sends commands and acquires results at once.

#### Examples
```
K2hdkcComCasIncDec* pComObj = GetOtSlaveK2hdkcComCasIncDec(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin);
bool                result  = pComObj->pComObj->IncrementCommandSend(reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, true, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), (-1 == Expire ? NULL : &Expire), &rescode);
```

<!-- -----------------------------------------------------------　-->
***

### <a name="STATECPP"> K2hdkcComState class
This class is operation to getting the status of all of k2hdkc nodes.
This class can get the number(chmpxid, hash value, etc) in the k2hdkc cluster of chmpx and the status of k2hash data managed by each server node.

#### Format
- bool K2hdkcComState::CommandSend(const DKC_NODESTATE** ppStates, size_t* pstatecount, dkcres_type_t* prescode)
- bool K2hdkcComState::GetResponseData(const DKC_NODESTATE** ppStates, size_t* pstatecount, dkcres_type_t* prescode) const

#### Parameters
- ppStates  
  Specifies a pointer to the buffer that gets the status.
- pstatecount  
  Specifies a pointer to the buffer which is set the count of status.
- prescode  
  Specify a pointer to store the processing result code.

#### Structure
The status structure to be acquired for this class is shown below.
```
typedef struct k2hdkc_nodestate{
    chmpxid_t       chmpxid;                // See: CHMPX in chmpx/chmstructure.h
    char            name[NI_MAXHOST];       //
    chmhash_t       base_hash;              //
    chmhash_t       pending_hash;           //
    K2HSTATE        k2hstate;               // See: k2hash/k2hash.h
}DKC_NODESTATE, *PDKC_NODESTATE;
 
typedef struct k2h_state{
    char            version[K2H_VERSION_LENGTH];            // Version string as K2hash file
    char            hash_version[K2H_HASH_FUNC_VER_LENGTH]; // Version string as Hash Function
    char            trans_version[K2H_HASH_FUNC_VER_LENGTH];// Version string as Transaction Function
    int             trans_pool_count;                       // Transaction plugin thread pool count
    k2h_hash_t      max_mask;                               // Maximum value for cur_mask
    k2h_hash_t      min_mask;                               // Minimum value for cur_mask
    k2h_hash_t      cur_mask;                               // Current mask value for hash(This value is changed automatically)
    k2h_hash_t      collision_mask;                         // Mask value for collision when masked hash value by cur_mask(This value is not changed)
    unsigned long   max_element_count;                      // Maximum count for elements in collision key index structure(Increasing cur_mask when this value is over)
    size_t          total_size;                             // Total size of k2hash
    size_t          page_size;                              // Paging size(system)
    size_t          file_size;                              // k2hash file(memory) size
    size_t          total_used_size;                        // Total using size in k2hash
    size_t          total_map_size;                         // Total mapping size for k2hash
    size_t          total_element_size;                     // Total element area size
    size_t          total_page_size;                        // Total page area size
    long            total_area_count;                       // Total mmap area count( = MAX_K2HAREA_COUNT)
    long            total_element_count;                    // Total element count in k2hash
    long            total_page_count;                       // Total page count in k2hash
    long            assigned_area_count;                    // Assigned area count
    long            assigned_key_count;                     // Assigned key index count
    long            assigned_ckey_count;                    // Assigned collision key index count
    long            assigned_element_count;                 // Assigned element count
    long            assigned_page_count;                    // Assigned page count
    long            unassigned_element_count;               // Unassigned element count(free element count)
    long            unassigned_page_count;                  // Unassigned page count(free page count)
    struct timeval  last_update;                            // Last update of data
    struct timeval  last_area_update;                       // Last update of expanding area
}K2HSTATE, *PK2HSTATE;
```

#### Return Values
Returns true if it succeeds, false if it fails.

#### Examples
```
K2hdkcComState*    pComObj = GetOtSlaveK2hdkcComState(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin);
bool               result = pComObj->CommandSend(&ptmpstates, &tmpstatecount, &rescode);
```
