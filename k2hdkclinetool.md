---
layout: contents
language: en-us
title: k2hdkclinetool
short_desc: K2Hash based Distributed Kvs Cluster
lang_opp_file: k2hdkclinetoolja.html
lang_opp_word: To Japanese
prev_url: 
prev_string: 
top_url: tools.html
top_string: Tools
next_url: 
next_string: 
---

# k2hdkclinetool
## k2hdkclinetool
The k2hdkclinetool program connects to the k2hdkc cluster as a k2hdkc client and can execute data manipulation of the k2hdkc cluster.

### Overview
The k2hdkclinetool program can execute data manipulation of the k2hdkc cluster as a client of CLI(Command Line Interface).
You can also use a text file describing the k2hdkclinetool command as a batch utility that can be loaded and executed.

By using this tool, you can check the status of the k2hdkc cluster after construction, verify operation, debug, change data and so on.
k2hdkclinetool is a tool for developers and operators covering almost all functions of the interface of k2hdkc cluster.

### Run k2hdkclinetool
The following explains the startup options of the k2hdkclinetool.
The k2hdkclinetool program can receive the following startup options(**-h** option displays startup options).
```
$ k2hdkclinetool -h
Usage: lt-k2hdkclinetool [-conf <file> | -json <string>] [-ctlport <port>] [-cuk <cuk string>] [options...]
       lt-k2hdkclinetool -help

Option -help(h)           help display
       -conf <filename>   k2hdkc configuration file path(.ini .yaml .json)
       -json <string>     k2hdkc configuration by json string
       -ctlport <port>    slave node chmpx control port
       -cuk <cuk string>  slave node chmpx cuk string
       -lap               print lap time after line command
       -capi              use C API for calling internal library
       -perm              use permanent chmpx handle
       -rejoin            chmpx auto rejoin if disconnect
       -nogiveup          no give up auto rejoin
       -nocleanup         not cleanup backup chmpx files
       -comlog [on | off] enable/disable communication command for debug
       -d <debug level>   print debugging message mode: SILENT(SLT)/ERROR(ERR)/WARNING(WAN)/INFO(MSG)/DUMP(DMP)
       -dfile <file path> output file for debugging message(default stderr)
       -his <count>       set history count(default 500)
       -libversion        display k2hdkc library version
       -run <file path>   run command(history) file.

(*) You can specify "K2HDKCCONFFILE" or "K2HDKCJSONCONF" environment instead of
    "-conf" or "-json" option for configuration.
(*) You can set debug level by another way which is setting environment as "DKCDBGMODE".
    "DKCDBGMODE" environment is took as "SILENT(SLT)", "ERROR(ERR)", "WARNING(WAN)",
    "INFO(MSG)" or "DUMP(DMP)" value.
    When this process gets SIGUSR1 signal, the debug level is bumpup.
    (The debug level is changed as "SLT"->"ERR"->"WAN"->"MSG"->"DMP"->...)
(*) You can set debugging message log file by the environment "DKCDBGFILE".
```

Each startup option is explained below.
#### -conf <filename>
Specify the configuration file(formatted by INI, YAML, JSON) of the chmpx process(slave node) connected to the k2hdkc cluster.  
This option is exclusive with the **-json** option.  
If environment variable (K2HDKCCONFFILE or K2HDKCJSONCONF) is specified, both **-conf** and **-json** options can be omitted.
#### -json <json string>
Specify the configuration file(formatted by INI, YAML, JSON) as a string in JSON format of the chmpx process(slave node) connected to the k2hdkc cluster.  
This option is exclusive with the **-conf** option.  
If environment variable (K2HDKCCONFFILE or K2HDKCJSONCONF) is specified, both **-conf** and **-json** options can be omitted.
#### -ctlport <port number>
Specifies the control port number of the chmpx process(slave node) to the k2hdkc cluster.  
This option can be omitted if only one chmpx process(slave node) is running on HOST.  
If more than one chmpx process(slave node) is running on the same HOST, specifying this option is mandatory.
#### -cuk <cuk string>  
Specify CUK(Custom Unique Key) to clarify the chmpx process(server node) to which the k2hdkc process connects.  
When starting multiple CHMPX programs with the same host name(IP address) or control port in the CHMPX configuration, there are cases where the CHMPX program is ambiguous about which setting value it will read.  
When specifying such a configuration, CUK may be specified to clarify the settings.
CUK is a setting that must be unique within a cluster.
#### -perm
Specifies this option, then k2hdkclinetool uses existing connection to the chmpx process(slave node) when executing k2hdkclinetool command(communication command to k2hdkc cluster).  
You can test the persistent connection by specifying this option.  
If this option is not specified, connect/disconnect to the chmpx process(slave node) every command execution.
#### -rejoin
When executing the command of k2hdkclinetool(communication command to k2hdkc cluster), it is an option to automatically reconnect if it can not connect to the chmpx process(slave node).
#### -nogiveup
When executing the command of k2hdkclinetool(communication command to k2hdkc cluster), if it can not connect to the chmpx process(slave node), it is an option to execute reconnect until it can be connected.
#### -nocleanup
When k2hdkclinetool terminates, if the chmpx process(slave node) also does not exist, it erases the file used by the chmpx process(slave node).
#### -comlog [on | off]
Specify whether to output message(on / off) contents transmitted/received by k2hdkclinetool.  
If this option is omitted, the message is not output(off).
#### -run <filepath>
Specify the command text file to be executed automatically after starting k2hdkclinetool.
#### -lap
Lap time is displayed for each command executed with k2hdkclinetool.
#### -capi
Use the programming interface for C language for the k2hdkc library used inside k2hdkclinetool.  
If this option is omitted, use the programming interface for C++ language.
#### -d <debug level>
Specifies the level of the message output for debugging of k2hdkclinetool.  
Specify the debug level(SILENT / ERR / WARN / MSG / DUMP) as the parameter of this option.  
This option is optional. If this option is not specified, the message output level is silent, and no message is output.  
This message output level can be bumped up with SIGUSR1.  
You can bump up this message output level by specifying the process ID and sending signal(SIGUSR1) after running k2hdkclinetool.
#### -dlog <file path>
Output the message output of k2hdkclinetool to the specified file.  
This option is optional. If this option is not specified, the message is output to stderr.
#### -his <count>
Specify the maximum number of command history after starting this program.  
This option is optional. If omitted, the maximum number of command history is 1000.
#### -h
Print help about options for k2hdkclinetool program.
#### -libversion
Print the version and credits of the k2hdkc library(libk2hdkc.so).

### Environments
The k2hdkclinetool program loads the same environment variables as the k2hdkc program.
For details, see [Environments](environments.html).

### k2hdkclinetool Commands
The k2hdkclinetool program is an interactive CLI(Command Line Interface) tool.
After startup, a prompt to enter a command is displayed, and you can enter and execute a command from the prompt.
You can enter help(help) command and display command help.
```
K2HDKC> h
Command: [command] [parameters...]
help(h)                                                                                       print help
quit(q)/exit                                                                                  quit
print(p) <key> [all] [noattrcheck] [pass=....] [dump]                                         print value by key(noattrcheck and pass parameter are mutually exclusive).
directprint(dp) <key> <length> <offset> [dump]                                                print value from offset and length directly.
printattr(pa) <key> [name=<attr name>] [dump]                                                 print attributes.
copyfile(cf) <key> <length> <offset> <file>                                                   output directly key-value to file.
set(s) <key> <value> [rmsublist | rmsub] [noattrcheck] [pass=....] [expire=sec]               set value(if value is "null", it means no value).
directset(dset) <key> <value> <offset>                                                        set value from offset directly.
setfile(sf) <key> <offset> <file>                                                             set directly key-value from file.
fill(f) <prefix> <value> <count> [rmsublist | rmsub] [noattrcheck] [pass=....] [expire=sec]   set key-value by prefix repeating by count.
fillsub(fs) <parent key> <prefix> <value> <count> [noattrcheck] [pass=....] [expire=sec]      set key-value under parent key by prefix repeating by count
rm(del) <key> [all]                                                                           delete(remove) key, if all parameter is specified, remove all sub key under key
setsub(ss) <parent key> <key> [noattrcheck]                                                   set only subkey name into parent key.
setsub(ss) <parent key> <key> <value> [noattrcheck] [pass=....] [expire=sec]                  set subkey-value under parent key. if need to set null value, must specify "null".
rmsub(delsub) <parent key> [<key> | all] [rmsub] [noattrcheck]                                remove key under parent key.
rename(ren) <key> <new key> [parent=<key>] [noattrcheck] [pass=....] [expire=sec]             rename key to new key name.
queue(que) <name> push <fifo | lifo> <value> [noattrcheck] [pass=....] [expire=sec]           push the value to queue(fifo/lifo)
queue(que) <name> pop <fifo | lifo> [noattrcheck] [pass=...] [dump]                           pop the value from queue
queue(que) <name> remove <fifo | lifo> <count> [pass=...]                                     remove count of values in queue
keyqueue(kque) <name> push <fifo | lifo> <key> <value> [noattrcheck] [pass=....] [expire=sec] push the key to queue(fifo/lifo)
keyqueue(kque) <name> pop <fifo | lifo> [noattrcheck] [pass=...] [dump]                       pop the key from queue
keyqueue(kque) <name> remove <fifo | lifo> <count> [pass=...]                                 remove count of keys in queue
cas[8 | 16 | 32 | 64] init <key> <value> [pass=....] [expire=sec]                             initialize compare and store key(default value size is 8, and value is 0)
cas[8 | 16 | 32 | 64] get <key> [pass=....]                                                   get compare and store key(default value size is 8)
cas[8 | 16 | 32 | 64] set <key> <old value> <new value> [pass=....] [expire=sec]              set compare and store key(default value size is 8)
cas [increment(inc) | decrement(dec)] <key> [pass=....] [expire=sec]                          increment/decrement compare and store key
status chmpx [self] [full]                                                                    print chmpx status
status node [full]                                                                            print k2hash states on all server nodes
comlog [on | off]                                                                             toggle or enable/disable communication command log
dbglevel [slt | err | wan | msg | dmp]                                                        bumpup debugging level or specify level
history(his)                                                                                  display all history, you can use a command line in history by "!<number>".
save <file path>                                                                              save history to file.
load <file path>                                                                              load and run command file.
shell                                                                                         exit shell(same as "!" command).
echo <string>...                                                                              echo string
sleep <second>                                                                                sleep seconds
```

The command options are shown below.
#### quit(q)/exit
Exit k2hdkclinetool program.
#### print(p) <key> [all] [noattrcheck] [pass=....] [dump]
Displays all information set for the specified key.
#### directprint(dp) <key> <length> <offset> [dump]
Displays the value of the specified key with the specified length from the specified offset.
#### printattr(pa) <key> [name=<attr name>] [dump]
Displays the attribute information set for the specified key.  
You can also specify attribute names to display only the values of specific attribute information.
#### copyfile(cf) <key> <length> <offset> <file>
Writes data from the specified file to the specified offset value and specified length in the value of the specified key.
#### set(s) <key> <value> [rmsublist | rmsub] [noattrcheck] [pass=....] [expire=sec]
Set a value for the specified key.  
If rmsublist is specified, only the subkey list set for the key is deleted.  
If rmsub is specified, the subkey list set in the key and subkey are also deleted.
#### directset(dset) <key> <value> <offset>
Writes data from the specified offset to the value of the specified key.
#### setfile(sf) <key> <offset> <file>
Reads the value of the specified key from the specified offset and writes it to the specified file.
#### fill(f) <prefix> <value> <count> [rmsublist | rmsub] [noattrcheck] [pass=....] [expire=sec]
Writes the specified value to the specified number of keys using the specified prefix.  
The key name is **prefix-number**.
#### fillsub(fs) <parent key> <prefix> <value> <count> [noattrcheck] [pass=....] [expire=sec]
Using the specified prefix, write the specified value of the sub key in the specified parent key to the specified number of keys.  
The subkey name is written as **prefix-number**.
#### rm(del) <key> [all]
  Deletes the specified key.
#### setsub(ss) <parent key> <key> [noattrcheck]
Sets the subkey for the specified parent key.  
Values are not written to the subkey.
#### setsub(ss) <parent key> <key> <value> [noattrcheck] [pass=....] [expire=sec]
Set the subkey as the specified value and set it as the specified parent key.
#### rmsub(delsub) <parent key> [<key> | all] [rmsub] [noattrcheck]
Delete all subkeys of the specified parent key or specified subkey.
#### rename(ren) <key> <new key> [parent=<key>] [noattrcheck] [pass=....] [expire=sec]
Change the specified key to a different key name.  
If you specify a parent key, you also change the key name in the subkey list listed in the parent key.
#### queue(que) <name> push <fifo | lifo> <value> [noattrcheck] [pass=....] [expire=sec]
The value is accumulated in FIFO / LIFO in the specified queue.
#### queue(que) <name> pop <fifo | lifo> [noattrcheck] [pass=...] [dump]
Fetch values from the specified queue with FIFO / LIFO.
#### queue(que) <name> remove <fifo | lifo> <count> [pass=...]
Deletes the number specified by FIFO / LIFO from the specified queue.
#### keyqueue(kque) <name> push <fifo | lifo> <key> <value> [noattrcheck] [pass=....] [expire=sec]
Accumulate keys and values in FIFO / LIFO in the specified queue.
#### keyqueue(kque) <name> pop <fifo | lifo> [noattrcheck] [pass=...] [dump]
Retrieve the key and value from FIFO / LIFO from the specified queue.
#### keyqueue(kque) <name> remove <fifo | lifo> <count> [pass=...]
Deletes the number specified by FIFO / LIFO from the specified queue.
#### cas[8 | 16 | 32 | 64] init <key> <value> [pass=....] [expire=sec]
Initialize the specified key as CAS(Compare and swap).  
There are commands for 8 / 16 / 32 / 64bits depending on key value length.
#### cas[8 | 16 | 32 | 64] get <key> [pass=....]
Gets the value with the specified key as CAS(Compare and swap).  
There are commands for 8 / 16 / 32 / 64bits depending on key value length.
#### cas[8 | 16 | 32 | 64] set <key> <old value> <new value> [pass=....] [expire=sec]
Set the value as CAS(Compare and swap) for the specified key.  
There are commands for 8 / 16 / 32 / 64bits depending on key value length.
#### cas [increment(inc) | decrement(dec)] <key> [pass=....] [expire=sec]
Increment/decrement the specified key as CAS(Compare and swap).  
The value length of the key is automatically determined.
#### status chmpx [self] [full]
k2hdkc Displays the status of the chmpx process(server node) of the cluster.
#### status node [full]
k2hdkc Displays the status of all server nodes in the cluster.
#### comlog [on | off]
Specify whether to output message(on / off) contents transmitted/received by k2hdkclinetool.  
If you omit the parameter(on / off), message output will toggle.
#### dbglevel [slt | err | wan | msg | dmp]
Specifies the level of the message output for debugging of k2hdkclinetool.  
Specify the debug level(SLT / ERR / WAN / MSG / DMP) as the parameter of this option.  
If you omit the parameter, the message output level will be Bumpup.
#### history(his)
Displays the command execution history.  
**!(number)** and re-execute the history command.
#### save <file path>
Save the execution history of the command with the specified file name.
#### load <file path>
Load the specified file as a command text file and execute it.
#### shell
Execute shell.
#### echo <string>...
It performs the same operation as the echo command.
Displays the specified parameter.
#### sleep <second>
Same as the sleep command.  
It sleeps for the specified number of seconds and pauses the command.
