---
layout: contents
language: ja
title: k2hdkclinetool
short_desc: K2Hash based Distributed Kvs Cluster
lang_opp_file: k2hdkclinetool.html
lang_opp_word: To English
prev_url: 
prev_string: 
top_url: toolsja.html
top_string: Tools
next_url: 
next_string: 
---

# k2hdkclinetool
## k2hdkclinetool
k2hdkclinetoolプログラムは、k2hdkcクラスタにk2hdkcクライアントとして接続し、k2hdkcクラスタのデータ操作を実行できるツールです。

### 概要
k2hdkclinetoolプログラムは、k2hdkcクラスタのデータ操作をCLI（Command Line Interface）のクライアントとして実行できます。  
また、k2hdkclinetoolコマンドを記述したテキストファイルを、ロード、実行できるバッチユーティリティとしても利用できます。

このツールを利用して、k2hdkcクラスタの構築後の状態確認、動作確認、デバッグ、データ変更などできます。
k2hdkclinetoolは、k2hdkcクラスタのインターフェースのほぼ全機能を網羅している開発者・運用者向けのツールです。

### 起動オプション
k2hdkclinetoolプロセスの起動オプションを説明します。
k2hdkclinetoolプロセスは、以下のような起動オプションを受け取ります。（**-h** オプションにより起動オプションを表示します。）
```
$ k2hdkclinetool -h
Usage: lt-k2hdkclinetool [-conf <file> | -json <string>] [-ctlport <port>] [options...]
       lt-k2hdkclinetool -help
Option -help(h)           help display
       -conf <filename>   k2hdkc configuration file path(.ini .yaml .json)
       -json <string>     k2hdkc configuration by json string
       -ctlport <port>    slave node chmpx control port
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
    When this process gets SIGUSER1 signal, the debug level is bumpup.
    (The debug level is changed as "SLT"->"ERR"->"WAN"->"MSG"->"DMP"->...)
(*) You can set debugging message log file by the environment "DKCDBGFILE".
```

以下に、各々のオプションについて説明します。
#### -conf <filename>
k2hdkcクラスタに接続しているchmpxプロセス（スレーブノード）のコンフィグレーションファイル（INI形式、YAML形式、JSON形式）を指定します。  
このオプションは、**-json** オプションと排他です。  
環境変数（K2HDKCCONFFILEもしくはK2HDKCJSONCONF）を指定している場合には、**-conf**、**-json** オプション共に省略が可能です。
#### -json <json string>
k2hdkcクラスタに接続しているchmpxプロセス（スレーブノード）のコンフィグレーションをJSON形式の文字列として指定します。  
このオプションは、**-conf** オプションと排他です。  
環境変数（K2HDKCCONFFILEもしくはK2HDKCJSONCONF）を指定している場合には、**-conf**、**-json** オプション共に省略が可能です。
#### -ctlport <port number>
k2hdkcクラスタに接続しているchmpxプロセス（スレーブノード）の制御ポート番号を指定します。  
このオプションは、HOST上に1つのchmpxプロセス（スレーブノード）のみ起動している場合、省略することができます。  
同一HOST上に複数のchmpxプロセス（スレーブノード）が起動している場合には、このオプションの指定は必須です。
#### -perm
本ツールのコマンド（k2hdkcクラスタへの通信コマンド）を実行するときに、chmpxプロセス（スレーブノード）への接続を使いまわす場合に指定してください。  
このオプションを指定することで、永続的接続のテストが可能です。  
本オプションが指定されない場合、コマンド実行毎にchmpxプロセス（スレーブノード）へ接続・切断をします。
#### -rejoin
本ツールのコマンド（k2hdkcクラスタへの通信コマンド）を実行するとき、chmpxプロセス（スレーブノード）に接続できない場合、自動的に再接続するためのオプションです。
#### -nogiveup
本ツールのコマンド（k2hdkcクラスタへの通信コマンド）を実行するとき、chmpxプロセス（スレーブノード）に接続できない場合、再接続を接続できるまで実行するためのオプションです
#### -nocleanup
本ツールの終了時、chmpxプロセス（スレーブノード）も存在しない場合、chmpxプロセス（スレーブノード）が利用していたファイルなどを消去します。
#### -comlog [on | off]
本ツールが送受信する通信内容をメッセージ出力するかどうか（on / off）を指定します。  
このオプションが省略された場合は、メッセージ出力（off）しません。
#### -run <filepath>
本ツール起動後に自動的に実行するコマンドテキストファイルを指定します。
#### -lap
本ツールで実行するコマンド毎にラップタイムを表示します。
#### -capi
本ツール内部で利用しているk2hdkcライブラリをC言語用のプログラミングI/Fを利用します。  
このオプションが省略された場合は、C++言語用のプログラミングI/Fを利用します。
#### -d <debug level>
本プロセスのメッセージ出力（デバッグ用）のレベルを指定します。  
本オプションのパラメータにはデバッグレベル（silent / err / wan / msg / dump）を指定します。  
このオプションは、省略可能です。省略した場合、メッセージ出力（デバッグ用）レベルはsilentであり、一切のメッセージ出力をしません。  
このメッセージ出力レベルは、SIGUSR1でBumpupすることができます。  
本ツール起動後にプロセスIDを指定して、signal（SIGUSR1）を送ることで、このメッセージ出力レベルをBumpupすることができます。
#### -dlog <file path>
本プロセスのメッセージ出力（デバッグ用）を指定したファイルに出力します。  
このオプションは、省略可能です。省略した場合、メッセージはstderrに出力されます。
#### -his <count>
本プロセスの起動後のコマンド履歴の最大数を指定します。  
このオプションは、省略可能です。省略した場合、コマンド履歴の最大数は1000です。
#### -h
ヘルプを表示します。
#### -libversion
k2hdkcライブラリ（libk2hdkc.so）のバージョンおよびクレジットを表示します。

### 環境変数
k2hdkclinetoolプログラムは、k2hdkcプログラムと同じ環境変数の読み込みを行います。  
詳しくは、[環境変数](environmentsja.html)を参照してください。

### k2hdkclinetoolコマンド
k2hdkclinetoolプログラムは、対話形式のCLI（Command Line Interface）ツールです。  
起動後はコマンド入力を行うプロンプトが表示され、ユーザは、プロンプトからコマンドを入力し、実行することができます。  
ヘルプ（help）コマンドを入力し、コマンドのヘルプを表示することができます。

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

コマンドのオプションを以下に示します。

#### quit(q)/exit
プログラムを終了します。
#### print(p) <key> [all] [noattrcheck] [pass=....] [dump]
指定したキーに設定されている情報を表示します。
#### directprint(dp) <key> <length> <offset> [dump]
指定したキーの値を指定したオフセットから指定長で表示します。
#### printattr(pa) <key> [name=<attr name>] [dump]
指定したキーに設定されている属性情報を表示します。  
属性名を指定して、特定の属性情報の値のみを表示することもできます。
#### copyfile(cf) <key> <length> <offset> <file>
指定したキーの値に指定オフセットおよび指定長に、指定したファイルからデータを書き込みます。
#### set(s) <key> <value> [rmsublist | rmsub] [noattrcheck] [pass=....] [expire=sec]
指定したキーに値を設定します。  
rmsublistを指定した場合は、キーに設定されているsubkeyリストのみを削除します。  
rmsubを指定した場合は、キーに設定されているsubkeyリストと、subkeyも削除します。
#### directset(dset) <key> <value> <offset>
指定したキーの値に指定オフセットからデータを書き込みます。
#### setfile(sf) <key> <offset> <file>
指定したキーの値を指定オフセットから読み出し、指定したファイルに書き出します。
#### fill(f) <prefix> <value> <count> [rmsublist | rmsub] [noattrcheck] [pass=....] [expire=sec]
指定したプレフィックスを用いて、指定した値を指定数分のキーに書き込みします。  
キー名は、**プレフィックス-数値** になります。
#### fillsub(fs) <parent key> <prefix> <value> <count> [noattrcheck] [pass=....] [expire=sec]
指定したプレフィックスを用いて、指定した親キーにサブキーを指定した値を指定数分のキーに書き込みします。  
サブキー名は、**プレフィックス-数値** として書き込みされます。
#### rm(del) <key> [all]
指定したキーを削除します。
#### setsub(ss) <parent key> <key> [noattrcheck]
指定した親キーにサブキーを設定します。  
サブキーに値は書き込まれません。
#### setsub(ss) <parent key> <key> <value> [noattrcheck] [pass=....] [expire=sec]
サブキーを指定値で設定して、指定した親キーに設定します。
#### rmsub(delsub) <parent key> [<key> | all] [rmsub] [noattrcheck]
指定した親キーの全てのサブキー、もしくは指定したサブキーを削除します。
#### rename(ren) <key> <new key> [parent=<key>] [noattrcheck] [pass=....] [expire=sec]
指定したキーを別のキー名に変更します。  
親キーを指定した場合には、親キーの中にリストされているサブキーリストの中のキー名の変更も行います。
#### queue(que) <name> push <fifo | lifo> <value> [noattrcheck] [pass=....] [expire=sec]
指定したキューにFIFO/LIFOで値を蓄積します。
#### queue(que) <name> pop <fifo | lifo> [noattrcheck] [pass=...] [dump]
指定したキューからFIFO/LIFOで値を取り出します。
#### queue(que) <name> remove <fifo | lifo> <count> [pass=...]
指定したキューからFIFO/LIFOで指定した個数削除します。
#### keyqueue(kque) <name> push <fifo | lifo> <key> <value> [noattrcheck] [pass=....] [expire=sec]
指定したキューにFIFO/LIFOでキーと値を蓄積します。
#### keyqueue(kque) <name> pop <fifo | lifo> [noattrcheck] [pass=...] [dump]
指定したキューからFIFO/LIFOでキーと値を取り出します。
#### keyqueue(kque) <name> remove <fifo | lifo> <count> [pass=...]
指定したキューからFIFO/LIFOで指定した個数削除します。
#### cas[8 | 16 | 32 | 64] init <key> <value> [pass=....] [expire=sec]
指定したキーをCAS（Compare and swap）として初期化します。  
キーの値長に応じて、8bit/16bit/32bit/64bit用にコマンドがあります。
#### cas[8 | 16 | 32 | 64] get <key> [pass=....]
指定したキーをCAS（Compare and swap）として値を取得します。  
キーの値長に応じて、8bit/16bit/32bit/64bit用にコマンドがあります。
#### cas[8 | 16 | 32 | 64] set <key> <old value> <new value> [pass=....] [expire=sec]
指定したキーにCAS（Compare and swap）として値を設定します。  
キーの値長に応じて、8bit/16bit/32bit/64bit用にコマンドがあります。
#### cas [increment(inc) | decrement(dec)] <key> [pass=....] [expire=sec]
指定したキーをCAS（Compare and swap）としてインクリメント/デクリメントします。  
キーの値長は自動的に判定されます。
#### status chmpx [self] [full]
k2hdkcクラスタのchmpxプロセス（サーバーノード）の状態を表示します。
#### status node [full]
k2hdkcクラスタの全サーバーノードの状態を表示します。
#### comlog [on | off]
本ツールが送受信する通信内容をメッセージ出力するかどうか（on / off）を指定します。  
パラメータ（on / off）を省略した場合、メッセージ出力がトグルします。
#### dbglevel [slt | err | wan | msg | dmp]
本プロセスのメッセージ出力（デバッグ用）のレベルを指定します。  
本オプションのパラメータにはデバッグレベル（slt / err / wan / msg / dmp）を指定します。  
パラメータを省略した場合には、メッセージ出力レベルがBumpupします。
#### history(his)
コマンドの実行履歴を表示します。  
**!番号** を指定して、履歴のコマンドを再実行できます。
#### save <file path>
コマンドの実行履歴を指定ファイル名で保存します。
#### load <file path>
指定したファイルをコマンドテキストファイルとしてロードし、実行します。
#### shell
shellを実行します。
#### echo <string>...
echoコマンドと同じ動作をします。  
指定されたパラメータを表示します。
#### sleep <second>
sleepコマンドと同じ動作をします。  
指定された秒数のsleepを行い、コマンドを一時停止させます。
