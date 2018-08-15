---
layout: contents
language: en-us
title: Environments
short_desc: K2Hash based Distributed Kvs Cluster
lang_opp_file: environmentsja.html
lang_opp_word: To Japanese
prev_url: developer.html
prev_string: Developer
top_url: index.html
top_string: TOP
next_url: tools.html
next_string: Tools
---

# Environments

## Environment variables provided by k2hdkc
The k2hdkc program and library read the following environment variables.

### DKCDBGMODE
It is the same as the start option **-d**. k2hdkc controls debug output of process.  
The Debug level(SILENT / ERR / WARN / MSG / DUMP) can be specified by the environment's value.  
If the environment variable and the start option are specified at the same time, the start option takes precedence.
### DKCDBGFILE
It is the same as the start option **-dfile**.  
k2hdkc replaces the debug output of the process with the specified file from stderr.  
If the environment variable and the start option are specified at the same time, the start option takes precedence.
### K2HDKCCONFFILE
You can specify the path to the configuration file(formatted by INI, YAML, JSON) for k2hdkc and chmpx server nodes without specifying startup options **-conf** and **-json**.
### K2HDKCJSONCONF
You can specify the configuration string formatted by JSON for the k2hdkc and chmpx server nodes if you do not specify startup options **-conf** and **-json**.  
If the K2HDKCCONFFILE environment variable is specified, K2HDKCCONFFILE takes precedence.

<br />
If the same items as these are specified by the C/C++ API, the contents specified by the API will take precedence.

## Environment variables provided by k2hash and chmpx
The k2hdkc programs and libraries are built with Communication middleware( [CHMPX](https://chmpx.antpick.ax/) ) and the Key Value Store(KVS) library( [K2HASH](https://k2hash.antpick.ax/) ).  
When debugging k2hdkc, it is assumed that messages from these middleware and libraries are required.  
Environment variables related to middleware and library debug information used together with k2hdkc are shown below.

### CHMDBGMODE/CHMDBGFILE
The environment variables provided by the chmpx process and library.  
chmpx program and library operation log can be output.  
By setting these values, you can check chmpx operation check and log message.  
t  You can change the level of CHMDBGMODE using signal USR1.
### K2HDBGMODE/K2HDBGFILE
The environment variables provided by the k2hash library.  
You can output the operation log of the k2hash library.
By setting these values, you can check the operation of the operation through the k2hash library and check the log message.
