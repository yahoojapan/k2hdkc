---
layout: contents
language: en-us
title: Usage
short_desc: K2Hash based Distributed Kvs Cluster
lang_opp_file: usageja.html
lang_opp_word: To Japanese
prev_url: feature.html
prev_string: Feature
top_url: index.html
top_string: TOP
next_url: build.html
next_string: Build
---

# Usage
This is an explanation about **K2HDKC** installation and easy operation confirmation.

# 1. Creating a usage environment

## Install K2HDKC
There are two ways to install **K2HDKC** in your environment.  
One is to download and install the package of **K2HDKC** from [packagecloud.io](https://packagecloud.io/).  
The other way is to build and install **K2HDKC** from source code yourself.  
These methods are described below.  

### Installing packages
The **K2HDKC** publishes [packages](https://packagecloud.io/app/antpickax/stable/search?q=k2hdkc) in [packagecloud.io - AntPickax stable repository](https://packagecloud.io/antpickax/stable) so that anyone can use it.  
The package of the **K2HDKC** is released in the form of Debian package, RPM package.  
Since the installation method differs depending on your OS, please check the following procedure and install it.  

#### For recent Debian-based Linux distributions users, follow the steps below:
```
$ sudo apt-get update -y
$ sudo apt-get install curl -y
$ curl -s https://packagecloud.io/install/repositories/antpickax/stable/script.deb.sh | sudo bash
$ sudo apt-get install k2hdkc
```
To install the developer package, please install the following package.
```
$ sudo apt-get install k2hdkc-dev
```

#### For users who use supported Fedora other than latest version, follow the steps below:
```
$ sudo dnf makecache
$ sudo dnf install curl -y
$ curl -s https://packagecloud.io/install/repositories/antpickax/stable/script.rpm.sh | sudo bash
$ sudo dnf install k2hdkc
```
To install the developer package, please install the following package.
```
$ sudo dnf install k2hdkc-devel
```

#### For other recent RPM-based Linux distributions users, follow the steps below:
```
$ sudo yum makecache
$ sudo yum install curl -y
$ curl -s https://packagecloud.io/install/repositories/antpickax/stable/script.rpm.sh | sudo bash
$ sudo yum install k2hdkc
```
To install the developer package, please install the following package.
```
$ sudo yum install k2hdkc-devel
```

#### Other OS
If you are not using the above OS, packages are not prepared and can not be installed directly.  
In this case, build from the [source code](https://github.com/yahoojapan/k2hdkc) described below and install it.

# 2. Operation check
## k2hdkc Server nodes
The k2hdkc cluster consists of multiple k2hdkc server nodes.

At the k2hdkc server node, at least one k2hdkc process and one chmpx process(server node) are running.
(It is also possible to start multiple k2hdkc server nodes on one HOST.)

## k2hdkc Client
The k2hdkc client connects to the k2hdkc cluster using the k2hdkc library.
For details on how to use the k2hdkc library, please refer to [Developer](developer.html).

The k2hdkc client communicates with the k2hdkc cluster through the chmpx process(slave node).

## How to use each process
This section explains how to start up the k2hdkc process and chmpx process(server node) on k2hdkc server node, and chmpx process(slave node) on k2hdkc client side.  
And this also explains how to start [k2hdkclinetool](k2hdkclinetool.html) (test program) as one of the k2hdkc client.

### k2hdkc Process for the k2hdkc server node
The k2hdkc process is the process of configuring the k2hdkc server node, and it is one process connecting to one chmpx process(server node).
Multiple k2hdkc processes connecting to one chmpx process(server node) can be started.
By loading multiple k2hdkc processes, load balancing of k2hdkc server node processing is realized.

The k2hdkc process(server node) receives a command request from the k2hdkc client through the chmpx process(server node), and replies the processing result to the k2hdkc client via the chmpx process(server node).
The k2hdkc process(server node) reads/writes the k2hash data(memory or file) on the k2hdkc server node directly.

#### Options
The following explains the startup options of the k2hdkc process.
The k2hdkc process can receive the following startup options(**-h** option displays startup options).
```
$ k2hdkc -h
[Usage]
[Usage]
k2hdkc [-conf <file path> | -json <json string>] [-ctlport <port>] [-cuk <cuk>] [-comlog] [-no_giveup_rejoin] [-d [silent|err|wan|msg|dump]] [-dfile <file path>]
k2hdkc [ -h | -v ]
[option]
  -conf <path>         specify the configuration file(.ini .yaml .json) path
  -json <string>       specify the configuration json string
  -ctlport <port>      specify the self control port(*)
  -cuk <cuk string>    specify the self CUK(*)
  -no_giveup_rejoin    not give up rejoining chmpx
  -comlog              enable logging communication command
  -d <param>           specify the debugging output mode:
                        silent - no output
                        err    - output error level
                        wan    - output warning level
                        msg    - output debug(message) level
                        dump   - output communication debug level
  -dfile <path>        specify the file path which is put output
  -h(help)             display this usage.
  -v(version)          display version.

[environment]
  K2HDKCCONFFILE       specify the configuration file(.ini .yaml .json) path
  K2HDKCJSONCONF       specify the configuration json string

(*) you can use environment DKCDBGMODE and DKCDBGFILE instead of -d/-dfile options.
(*) if ctlport and cuk option is specified, chmpx searches same ctlport/cuk
    in configuration file and ignores "CTLPORT" or "CUK" directive in
    "GLOBAL" section. and chmpx will start in the mode indicated by the
    server entry that has been detected.
```

Each startup option is explained below.
##### -conf <filepath>
Specifies the path to the configuration file(formatted by INI, YAML, JSON) for k2hdkc process.  
We recommend that the configuration of the chmpx process(server node) should be described in this configuration file.  
This option is exclusive with the **-json** option.  
If environment variable (K2HDKCCONFFILE or K2HDKCJSONCONF) is specified, both **-conf** and **-json** options can be omitted.
##### -json <json string>
Specifies the configuration for the k2hdkc process as a string in JSON format.  
We recommend that the configuration of the chmpx process(server node) should be described in this configuration file.  
This option is exclusive with the **-conf** option.  
If environment variable (K2HDKCCONFFILE or K2HDKCJSONCONF) is specified, both **-conf** and **-json** options can be omitted.
##### -ctlport <port number>
Specifies the control port number of the chmpx process(server node) to which the k2hdkc process connects.  
This option can be omitted if only one chmpx process(server node) is running on HOST.  
If more than one chmpx process(server node) is running on the same HOST, specifying this option is mandatory.
##### -cuk <cuk string>  
Specify CUK(Custom Unique Key) to clarify the chmpx process(server node) to which the k2hdkc process connects.  
When starting multiple CHMPX programs with the same host name(IP address) or control port in the CHMPX configuration, there are cases where the CHMPX program is ambiguous about which setting value it will read.  
When specifying such a configuration, CUK may be specified to clarify the settings.
CUK is a setting that must be unique within a cluster.
##### -no_giveup_rejoin
After the k2hdkc process is started, if the k2hdkc process could not connect with the chmpx process(server node), the k2hdkc process retries to reconnect to the chmpx process(server node).  
Specify this option to eliminate the upper limit trying to reconnect(not give up) and try again until you can connect.  
This option is optional. If this option is not specified, the k2hdkc process terminates with an error due to several connection failures.
##### -comlog
If this option is specified, the k2hdkc process outputs the contents of the communication command to be sent and received as a message.  
This option is optional. If this option is not specified, no message about communication is output.
##### -d <debug level>
Specifies the level of the message output for debugging of the k2hdkc process.  
Specify the debug level(SILENT / ERR / WARN / MSG / DUMP) as the parameter of this option.  
This option is optional. If this option is not specified, the message output level is silent, and no message is output.
##### -dfile <filepath>
Output the message output of the k2hdkc process to the specified file.    
This option is optional. If this option is not specified, the message is output to stderr.
##### -h(help)
Print help about options for the k2hdkc program.
##### -v(version)
Print the version of the k2hdkc program and related library version.

The k2hdkc program loads the following environment variables.  
For details, see [Environments](environments.html).
##### DKCDBGMODE
It is the same as the start option **-d** .  
k2hdkc controls debug output of process.  
The Debug level(SILENT / ERR / WARN / MSG / DUMP) can be specified by the environment's value.  
If the environment variable and the start option are specified at the same time, the start option takes precedence.
##### DKCDBGFILE
It is the same as the start option **-dfile** .  
k2hdkc replaces the debug output of the process with the specified file from stderr.  
If the environment variable and the start option are specified at the same time, the start option takes precedence.
##### K2HDKCCONFFILE
You can specify the path to the configuration file(formatted by INI, YAML, JSON) for k2hdkc and chmpx server nodes without specifying startup options **-conf** and **-json** .
##### K2HDKCJSONCONF
You can specify the configuration string formatted by JSON for the k2hdkc and chmpx server nodes if you do not specify startup options **-conf** and **-json** .
If the K2HDKCCONFFILE environment variable is specified, K2HDKCCONFFILE takes precedence.

#### Run the k2hdkc process
Start the k2hdkc process as shown below.
```
$ k2hdkc -conf test_server.ini -ctlport 8021
```

##### Notes
In order for the k2hdkc process to start, the chmpx process(server node) must be running.  
If the chmpx process(server node) is not running at k2hdkc process startup, the k2hdkc process will attempt to start the chmpx process(server node).
Therefore, make sure to include the configuration of the chmpx process(server node) in the configuration passed to the k2hdkc process.
If the chmpx process(server node) is already running, only the k2hdkc process will be started.

#### Configuration
The configuration(file path or JSON string) specified when starting the k2hdkc process can be described in the same configuration as the chmpx process(server node).  
For the sample configuration of the k2hdkc process, refer to the following files.
##### Configuration file formatted by INI
[test_server.ini]({{ site.github.repository_url }}/blob/master/tests/test_server.ini)
##### Configuration file formatted by YAML
[test_server.yaml]({{ site.github.repository_url }}/blob/master/tests/test_server.yaml)
##### Configuration file formatted by JSON
[test_server.json]({{ site.github.repository_url }}/blob/master/tests/test_server.json)
##### Configuration by string of JSON
The character string after **TEST_SERVER_JSON_STR=** in the [test_json_string.data]({{ site.github.repository_url }}/blob/master/tests/test_json_string.data) file

### chmpx Process(Server node)
The chmpx process(server node) is the process of configuring the k2hdkc server node and starts one process for one k2hdkc server node.  
The chmpx process(server node) communicates with the chmpx process(slave node), passes the command request from the k2hdkc client to the k2hdkc process, and relays the processing result to the chmpx process(slave node).

#### Run the chmpx Process(Server node)
Start the chmpx process(Server node) as shown below.
```
$ chmpx -conf test_server.ini -ctlport 8021
```

Refer to [CHMPX Datails](https://chmpx.antpick.ax/details.html) for the startup options and configuration of the chmpx process.

#### Configuration
The configuration(file path or JSON string) specified when starting the chmpx process(server node) can be described in the same configuration as the k2hdkc process.
For the sample configuration of the chmpx process(Server node), refer to the following files.
##### Configuration file formatted by INI
[test_server.ini]({{ site.github.repository_url }}/blob/master/tests/test_server.ini)
##### Configuration file formatted by YAML
[test_server.yaml]({{ site.github.repository_url }}/blob/master/tests/test_server.yaml)
##### Configuration file formatted by JSON
[test_server.json]({{ site.github.repository_url }}/blob/master/tests/test_server.json)
##### Configuration by string of JSON
The character string after **TEST_SERVER_JSON_STR=** in the [test_json_string.data]({{ site.github.repository_url }}/blob/master/tests/test_json_string.data) file

##### Notes
Must be OFF(or NO) for MQACK in the configuration.  
If this value is ON, a communication sequence error occurs and the k2hdkc server node can not be started.

### chmpx Process(Slave node)
The chmpx process(slave node) is the process of configuring the k2hdkc client side to communicate with the k2hdkc cluster.  
The chmpx process(slave node) communicates with the chmpx process(server node), passes a command request from the k2hdkc client to the chmpx process(server node), and relays the processing result to the k2hdkc client.

#### Run the chmpx Process(Slave node)
Start the chmpx process(Slave node) as shown below.
```
$ chmpx -conf test_slave.ini -ctlport 8031
```

Refer to [CHMPX Details](https://chmpx.antpick.ax/details.html) for the startup options and configuration of the chmpx process.

#### Configuration
For the sample configuration of the chmpx process(Slave node), refer to the following files.
##### Configuration file formatted by INI
[test_slave.ini]({{ site.github.repository_url }}/blob/master/tests/test_slave.ini)
##### Configuration file formatted by YAML
[test_slave.yaml]({{ site.github.repository_url }}/blob/master/tests/test_slave.yaml)
##### Configuration file formatted by JSON
[test_slave.json]({{ site.github.repository_url }}/blob/master/tests/test_slave.json)
##### Configuration by string of JSON
The character string after **TEST_SLAVE_JSON_STR=** in the [test_json_string.data]({{ site.github.repository_url }}/blob/master/tests/test_json_string.data) file

##### Notes
Must be OFF(or NO) for MQACK in the configuration.  
If this value is ON, a communication sequence error occurs and the k2hdkc server node can not be started.

### k2hdkclinetool Process for the k2hdkc test client
[k2hdkclinetool](k2hdkclinetool.html) is a client test program that provides basic commands as CLI(Command Line Tool) for the k2hdkc cluster.
This explanation explains how to start k2hdkclinetool as a k2hdkc client program.

#### Run the k2hdkclinetool Process
Start the k2hdkclinetool process as shown below.
```
$ k2hdkclinetool -conf test_slave.ini -ctlport 8031 -comlog off -perm -rejoin -nogiveup -nocleanup -capi
```

k2hdkclinetool specifies the same file or JSON string as the configuration of the chmpx process(slave node) and it is started up.  
Before starting the k2hdkclinetool process, the chmpx process(slave node) must be running.

For details on how to use k2hdkclinetool, options, please see [k2hdkclinetool](k2hdkclinetool.html).