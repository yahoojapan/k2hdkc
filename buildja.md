---
layout: contents
language: ja
title: Build
short_desc: K2Hash based Distributed Kvs Cluster
lang_opp_file: build.html
lang_opp_word: To English
prev_url: usageja.html
prev_string: Usage
top_url: indexja.html
top_string: TOP
next_url: developerja.html
next_string: Developer
---

# 造る

この章は3つの部分で構成されています。

* ローカルPC上で開発するためのK2HDKCの設定方法
* ソースコードからK2HDKCを構築する方法
* K2HDKCのインストール方法

## 1.前提条件をインストールする

K2HDKCは主にfullock、k2hashおよびchmpxに依存します。 それぞれの依存ライブラリとヘッダファイルはK2HDKCを構築するために必要です。 それらをインストールする方法は2つあります。 好きなものを選ぶことができます。

* GitHubを使う  
依存ライブラリのソースコードとヘッダファイルをインストールします。 あなたはそれらをビルドしてインストールします。
* packagecloud.ioを使用する  
依存ライブラリのパッケージとヘッダファイルをインストールします。 あなたはそれらをインストールするだけです。 ライブラリはすでに構築されています。

### 1.1. GitHubから各依存ライブラリとヘッダファイルをインストールする

詳細については以下の文書を読んでください。

* [fullock](https://fullock.antpick.ax/build.html)
* [k2hash](https://k2hash.antpick.ax/build.html) 
* [chmpx](https://chmpx.antpick.ax/build.html)

### 1.2. packagecloud.ioから各依存ライブラリとヘッダファイルをインストールします。

このセクションでは、packagecloud.ioから各依存ライブラリとヘッダーファイルをインストールする方法を説明します。

注：前のセクションで各依存ライブラリとGitHubからのヘッダーファイルをインストールした場合は、このセクションを読み飛ばしてください。

DebianStretchまたはUbuntu（Bionic Beaver）をお使いの場合は、以下の手順に従ってください。

```
$ sudo apt-get update -y
$ sudo apt-get install curl -y
$ curl -s https://packagecloud.io/install/repositories/antpickax/stable/script.deb.sh \
    | sudo bash
$ sudo apt-get install autoconf autotools-dev gcc g++ make gdb libtool pkg-config \
    libyaml-dev libfullock-dev k2hash-dev chmpx-dev -y
$ sudo apt-get install git -y
```

Fedora28またはCentOS7.x（6.x）ユーザーの場合は、以下の手順に従ってください。

```
$ sudo yum makecache
$ sudo yum install curl -y
$ curl -s https://packagecloud.io/install/repositories/antpickax/stable/script.rpm.sh \
    | sudo bash
$ sudo yum install autoconf automake gcc gcc-c++ gdb make libtool pkgconfig \
    libyaml-devel libfullock-devel k2hash-devel chmpx-devel -y
$ sudo yum install git -y
```

## 2. GitHubからソースコードを複製する

GitHubからK2HDKCのソースコードをダウンロードしてください。

```
$ git clone https://github.com/yahoojapan/k2hdkc.git
```

## 3. ビルドしてインストールする

以下のステップに従ってK2HDKCをビルドしてインストールしてください。 K2HDKCを構築するためにGNU Automakeを使います。

```
$ cd k2hdkc
$ sh autogen.sh
$ ./configure --prefix=/usr
$ make
$ sudo make install
```

K2HDKCを正常にインストールすると、k2hdkcのヘルプテキストが表示されます。
```bash
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
