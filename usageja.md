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
ここでは、**K2HDKC** のインストールと簡単な動作確認について説明します。

# 1. 利用環境構築

## K2HDKCインストール
**K2HDKC** をご利用の環境にインストールするには、2つの方法があります。  
ひとつは、[packagecloud.io](https://packagecloud.io/)から **K2HDKC** のパッケージをダウンロードし、インストールする方法です。  
もうひとつは、ご自身で **K2HDKC** をソースコードからビルドし、インストールする方法です。  
これらの方法について、以下に説明します。

### パッケージを使ったインストール
**K2HDKC** は、誰でも利用できるように[packagecloud.io - AntPickax stable repository](https://packagecloud.io/antpickax/stable/)で[パッケージ](https://packagecloud.io/app/antpickax/stable/search?q=k2hdkc)を公開しています。  
**K2HDKC** のパッケージは、Debianパッケージ、RPMパッケージの形式で公開しています。  
お使いのOSによりインストール方法が異なりますので、以下の手順を確認してインストールしてください。  

#### 最近のDebianベースLinuxの利用者は、以下の手順に従ってください。
```
$ sudo apt-get update -y
$ sudo apt-get install curl -y
$ curl -s https://packagecloud.io/install/repositories/antpickax/stable/script.deb.sh | sudo bash
$ sudo apt-get install k2hdkc
```
開発者向けパッケージをインストールする場合は、以下のパッケージをインストールしてください。
```
$ sudo apt-get install k2hdkc-dev
```

#### Fedoraの利用者は、以下の手順に従ってください。
```
$ sudo dnf makecache
$ sudo dnf install curl -y
$ curl -s https://packagecloud.io/install/repositories/antpickax/stable/script.rpm.sh | sudo bash
$ sudo dnf install k2hdkc
```
開発者向けパッケージをインストールする場合は、以下のパッケージをインストールしてください。
```
$ sudo dnf install k2hdkc-devel
```

#### その他最近のRPMベースのLinuxの場合は、以下の手順に従ってください。
```
$ sudo yum makecache
$ sudo yum install curl -y
$ curl -s https://packagecloud.io/install/repositories/antpickax/stable/script.rpm.sh | sudo bash
$ sudo yum install k2hdkc
```
開発者向けパッケージをインストールする場合は、以下のパッケージをインストールしてください。
```
$ sudo yum install k2hdkc-devel
```

#### 上記以外のOS
上述したOS以外をお使いの場合は、パッケージが準備されていないため、直接インストールすることはできません。  
この場合には、後述の[ソースコード](https://github.com/yahoojapan/k2hdkc)からビルドし、インストールするようにしてください。

### ソースコードからビルド・インストール
**K2HDKC** を[ソースコード](https://github.com/yahoojapan/k2hdkc)からビルドし、インストールする方法は、[ビルド](https://k2hdkc.antpick.ax/buildja.html)を参照してください。

# 2. 動作確認
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
##### -cuk <cuk string>  
k2hdkcプロセスが接続するchmpxプロセス（サーバーノード）を明確にするためにCUK（Custom Unique Key）を指定します。  
CHMPXのコンフィグレーションに同一のホスト名（IPアドレス）や制御ポートで複数のCHMPXプログラムを起動する場合、CHMPXプログラムは自信がどの設定値を読み込むのか曖昧なケースがあります。  
このようなコンフィグレーションを指定する場合に、設定値を明確にするために、CUKを指定している場合があります。  
CUKは、クラスタ内で一意であるべき設定値です。
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
