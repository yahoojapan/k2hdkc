---
layout: contents
language: ja
title: Usage
short_desc: K2Hash based Distributed Kvs Cluster
lang_opp_file: usage.html
lang_opp_word: To English
prev_url: featureja.html
prev_string: Feature
top_url: indexja.html
top_string: TOP
next_url: buildja.html
next_string: Build
---

# 使い方
## k2hdkcサーバーノード
k2hdkcクラスタは、複数のk2hdkcサーバーノードにより構成されます。

k2hdkcサーバーノードでは、1つ以上のk2hdkcプロセスと1つのchmpxプロセス（サーバーノード）が起動しています。
（1台のHOST上に複数のk2hdkcサーバーノードを起動することもできます。）

## k2hdkcクライアント
k2hdkcクラスタと接続するk2hdkcクライアントは、k2hdkcライブラリを使って接続します。
k2hdkcライブラリの利用方法については、[開発者](developerja.html)を参照してください。

k2hdkcクライアントは、chmpxプロセス（スレーブノード）を通して、k2hdkcクラスタと通信します。

## 各プロセスの使い方
k2hdkcサーバーノードで起動するk2hdkcプロセス、chmpxプロセス（サーバーノード）と、k2hdkcクライアント側で起動するchmpxプロセス（スレーブノード）の起動方法について説明します。  
また、k2hdkcクライアントとして [k2hdkclinetool](k2hdkclinetoolja.html) （テストプログラム）の起動方法も説明します。

### k2hdkcプロセス（サーバーノード）
k2hdkcプロセス（サーバーノード）は、k2hdkcサーバーノードを構成するプロセスであり、1つのchmpxプロセス（サーバーノード）に接続する1つのプロセスです。  
1つのchmpxプロセス（サーバーノード）に接続するk2hdkcプロセスは複数起動することができます。
これより、処理の負荷分散を実現します。

k2hdkcプロセス（サーバーノード）は、k2hdkcクライアントからのコマンドリクエストをchmpxプロセス（サーバーノード）から受け取り、処理結果をchmpxプロセス（サーバーノード）を経由してk2hdkcクライアントに返信します。
k2hdkcプロセス（サーバーノード）が、k2hashデータ（メモリ、もしくはファイル）の読み出し・書き込みを行います。

#### 起動オプション
k2hdkcプロセスの起動オプションを説明します。
k2hdkcプロセスは、以下のような起動オプションを受け取ります。（**-h** オプションにより起動オプションを表示します。）
```
$ k2hdkc -h
[Usage]
k2hdkc [-conf <file path> | -json <json string>] [-ctlport <port>] [-comlog] [-no_giveup_rejoin] [-d [slient|err|wan|msg|dump]] [-dfile <file path>]
k2hdkc [ -h | -v ]
[option]
  -conf <path>         specify the configration file(.ini .yaml .json) path
  -json <string>       specify the configration json string
  -ctlport <port>      specify the self contrl port(*)
  -no_giveup_rejoin    not gitve up rejoining chmpx
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
[environemnt]
  K2HDKCCONFFILE       specify the configration file(.ini .yaml .json) path
  K2HDKCJSONCONF       specify the configration json string
(*) you can use environment DKCDBGMODE and DKCDBGFILE instead of -d/-dfile options.
(*) if ctlport option is specified, chmpx searches same ctlport in configuration
    file and ignores "CTLPORT" directive in "GLOBAL" section. and chmpx will
    start in the mode indicated by the server entry that has beed detected.
```

以下に、各々のオプションについて説明します。
##### -conf <filepath>
k2hdkcプロセスのためのコンフィグレーションファイル（INI形式、YAML形式、JSON形式）へのパスを指定します。  
chmpxプロセス（サーバーノード）のコンフィグレーションも一緒に記述しておくことができます。（推奨）  
このオプションは、**-json** オプションと排他です。  
環境変数（K2HDKCCONFFILEもしくはK2HDKCJSONCONF）を指定している場合には、**-conf**、**-json** オプション共に省略が可能です。
##### -json <json string>
k2hdkcプロセスのためのコンフィグレーションをJSON形式の文字列として指定します。
chmpxプロセス（サーバーノード）のコンフィグレーションも一緒に記述しておくことができます。（推奨）  
このオプションは、**-conf** オプションと排他です。  
環境変数（K2HDKCCONFFILEもしくはK2HDKCJSONCONF）を指定している場合には、**-conf**、**-json** オプション共に省略が可能です。
##### -ctlport <port number>
k2hdkcプロセスが接続するchmpxプロセス（サーバーノード）の制御ポート番号を指定します。  
このオプションは、HOST上に1つのchmpxプロセス（サーバーノード）のみ起動している場合、省略することができます。
同一HOST上に複数のchmpxプロセス（サーバーノード）が起動している場合には、このオプションの指定は必須です。
##### -no_giveup_rejoin
k2hdkcプロセス起動後、chmpxプロセス（サーバーノード）と接続ができない場合、k2hdkcプロセスはchmpxプロセス（サーバーノード）へ再接続を試みます。
再接続を試行する上限（ギブアップ）を無くし、接続できるまで試行するために本オプションを指定します。
このオプションは、省略可能です。省略した場合、数回の接続失敗によりk2hdkcプロセスはエラー終了します。
##### -comlog
k2hdkcプロセスが送受信する通信内容をログ出力する場合に指定します。
このオプションは、省略可能です。省略した場合、ログは出力されません。
##### -d <debug level>
k2hdkcプロセスのメッセージ出力（デバッグ用）のレベルを指定します。
本オプションのパラメータにはデバッグレベル（silent / err / wan / msg / dump）を指定します。
このオプションは、省略可能です。省略した場合、メッセージ出力（デバッグ用）レベルはsilentであり、一切のメッセージ出力をしません。
##### -dfile <filepath>
k2hdkcプロセスのメッセージ出力（デバッグ用）を指定したファイルに出力します。
このオプションは、省略可能です。省略した場合、メッセージは、stderrに出力されます。
##### -h(help)
ヘルプを表示します。
##### -v(version)
k2hdkcプログラムおよび関連ライブラリのバージョンを表示します。

k2hdkcプログラムは、以下の環境変数の読み込みを行います。詳しくは、[環境変数](environmentsja.html)を参照してください。
##### DKCDBGMODE
起動オプションの **-d** と同じです。
k2hdkcプロセスのデバッグ出力を制御します。
パラメータにはデバッグレベル（silent / err / wan / msg / dump）を指定します。
環境変数と起動オプションが同時に指定された場合には起動オプションが優先されます。
##### DKCDBGFILE
起動オプションの **-dfile** と同じです。
k2hdkcプロセスのデバッグ出力をstderrから指定ファイルに変更します。
環境変数と起動オプションが同時に指定された場合には起動オプションが優先されます。
##### K2HDKCCONFFILE
起動オプション **-conf** および **-json** が未指定のとき、k2hdkcプロセスおよびchmpxプロセス（サーバーノード）のためのコンフィグレーションファイル（INI形式、YAML形式、JSON形式）を指定できます。
##### K2HDKCJSONCONF
起動オプション **-conf** および **-json** が未指定のとき、k2hdkcプロセスおよびchmpxプロセス（サーバーノード）のためのJSON文字列のコンフィグレーションを指定できます。
K2HDKCCONFFILE環境変数も指定されている場合は、K2HDKCCONFFILEが優先されます。

#### 起動方法
k2hdkcプロセスを起動するには、以下のように実行します。
```
$ k2hdkc -conf test_server.ini -ctlport 8021
```

##### 注意
k2hdkcプロセスが起動するためには、chmpxプロセス（サーバーノード）が起動している必要があります。  
よって、k2hdkcプロセスの起動時にchmpxプロセス（サーバーノード）が起動していない場合には、k2hdkcプロセスによりchmpxプロセス（サーバーノード）の起動が試行されます。  
このため、k2hdkcプロセスに渡すコンフィグレーションには、chmpxプロセス（サーバーノード）のコンフィグレーションを含むようにしてください。  
すでに、chmpxプロセス（サーバーノード）が起動している場合は、k2hdkcプロセスのみが起動します。

#### コンフィグレーション
k2hdkcプロセスを起動するときに指定するコンフィグレーション（ファイルパスもしくはJSON文字列）は、chmpxプロセス（サーバーノード）と同じコンフィグレーションに記述することができます。

k2hdkcプロセスのサンプルコンフィグレーションについては、以下のファイルを参照してください。
##### INI形式
[test_server.ini]({{ site.github.repository_url }}/blob/master/tests/test_server.ini)
##### YAML形式
[test_server.yaml]({{ site.github.repository_url }}/blob/master/tests/test_server.yaml)
##### JSON形式
[test_server.json]({{ site.github.repository_url }}/blob/master/tests/test_server.json)
##### JSON文字列
[test_json_string.data]({{ site.github.repository_url }}/blob/master/tests/test_json_string.data) _(TEST_SERVER_JSON_STR 以降のJSON文字列)_

### chmpxプロセス（サーバーノード）
chmpxプロセス（サーバーノード）は、k2hdkcサーバーノードを構成するプロセスであり、1つのk2hdkcサーバーノードに対して1プロセスを起動します。
chmpxプロセス（サーバーノード）は、chmpxプロセス（スレーブノード）と通信し、k2hdkcクライアントからのコマンドリクエストをk2hdkcプロセスに渡し、その処理結果をchmpxプロセス（スレーブノード）に中継します。

#### 起動方法
chmpxプロセス（サーバーノード）を起動するには、以下のように実行します。
```
$ chmpx -conf test_server.ini -ctlport 8021
```

chmpxプロセスの起動オプション、コンフィグレーションについては、[CHMPX 詳細](https://chmpx.antpick.ax/detailsja.html) を参照してください。

#### コンフィグレーション
chmpxプロセス（サーバーノード）を起動するときに指定するコンフィグレーション（ファイルパスもしくはJSON文字列）は、k2hdkcプロセスと同じコンフィグレーションに記述することができます。

chmpxプロセス（サーバーノード）のサンプルコンフィグレーションについては、以下のファイルを参照してください。
##### INI形式
[test_server.ini]({{ site.github.repository_url }}/blob/master/tests/test_server.ini)
##### YAML形式
[test_server.yaml]({{ site.github.repository_url }}/blob/master/tests/test_server.yaml)
##### JSON形式
[test_server.json]({{ site.github.repository_url }}/blob/master/tests/test_server.json)
##### JSON文字列
[test_json_string.data]({{ site.github.repository_url }}/blob/master/tests/test_json_string.data) _(TEST_SERVER_JSON_STR 以降のJSON文字列)_

##### 注意
MQACKはOFF（NO）を指定してください。
ONになっている場合には通信シーケンスエラーとなり、k2hdkcサーバーノードが起動できません。

### chmpxプロセス（スレーブノード）
chmpxプロセス（スレーブノード）は、k2hdkcクラスタと通信するためのk2hdkcクライアント側を構成するプロセスです。
chmpxプロセス（スレーブノード）は、chmpxプロセス（サーバーノード）と通信し、k2hdkcクライアントからのコマンドリクエストをchmpxプロセス（サーバーノード）に渡し、その処理結果をk2hdkcクライアントに中継します。

#### 起動方法
chmpxプロセス（スレーブノード）を起動するには、以下のように実行します。
```
$ chmpx -conf test_slave.ini -ctlport 8031
```

chmpxプロセスの起動オプション、コンフィグレーションについては、[CHMPX 詳細](https://chmpx.antpick.ax/detailsja.html) を参照してください。

#### コンフィグレーション
chmpxプロセス（スレーブノード）のサンプルコンフィグレーションについては、以下のファイルを参照してください。
##### INI形式
[test_slave.ini]({{ site.github.repository_url }}/blob/master/tests/test_slave.ini)
##### YAML形式
[test_slave.yaml]({{ site.github.repository_url }}/blob/master/tests/test_slave.yaml)
##### JSON形式
[test_slave.json]({{ site.github.repository_url }}/blob/master/tests/test_slave.json)
##### JSON文字列
[test_json_string.data]({{ site.github.repository_url }}/blob/master/tests/test_json_string.data) **TEST_SLAVE_JSON_STR=** 以降のJSON文字列

##### 注意
MQACKはOFF（NO）を指定してください。
ONになっている場合には通信シーケンスエラーとなり、k2hdkcクラスタと通信できません。

### k2hdkclinetoolプロセス（テストプログラム）
[k2hdkclinetool](k2hdkclinetoolja.html) は、基本的なコマンドをCLI（Command Line Tool）として提供するクライアントテストプログラムです。
ここの説明では、k2hdkcクライアントプログラムとしてk2hdkclinetoolを起動する方法を説明します。

#### 起動方法
k2hdkclinetoolプロセスを起動するには、以下のように実行します。
```
$ k2hdkclinetool -conf test_slave.ini -ctlport 8031 -comlog off -perm -rejoin -nogiveup -nocleanup -capi
```

k2hdkclinetoolは、chmpxプロセス（スレーブノード）のコンフィグレーションと同じファイル・JSON文字列を指定し、起動します。
k2hdkclinetoolプロセスを起動する前に、chmpxプロセス（スレーブノード）が起動している必要があります。

k2hdkclinetoolの使い方、オプション等は、[k2hdkclinetool](k2hdkclinetoolja.html) を参照してください。
