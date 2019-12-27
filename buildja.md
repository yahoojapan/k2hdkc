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

# ビルド

この章は3つの部分で構成されています。

* ローカルPC上で開発するためのK2HDKCの設定方法
* ソースコードからK2HDKCを構築する方法
* K2HDKCのインストール方法

## 1.ビルド環境の構築

**K2HDKC**は主に[FULLOCK](https://fullock.antpick.ax/indexja.html)、[K2HASH](https://k2hash.antpick.ax/indexja.html) 、[CHMPX](https://chmpx.antpick.ax/indexja.html)に依存します。 それぞれの依存ライブラリとヘッダファイルは**K2HDKC**を構築するために必要です。 それらをインストールする方法は2つあります。 好きなものを選ぶことができます。

* [GitHub](https://github.com/yahoojapan)を使う  
依存ライブラリのソースコードとヘッダファイルをインストールします。 あなたはそれらをビルドしてインストールします。
* [packagecloud.io](https://packagecloud.io/antpickax/stable)を使用する  
依存ライブラリのパッケージとヘッダファイルをインストールします。 あなたはそれらをインストールするだけです。 ライブラリはすでに構築されています。

### 1.1. GitHubから各依存ライブラリとヘッダファイルをインストールする

詳細については以下の文書を読んでください。

* [FULLOCK](https://fullock.antpick.ax/buildja.html)
* [K2HASH](https://k2hash.antpick.ax/buildja.html) 
* [CHMPX](https://chmpx.antpick.ax/buildja.html)

### 1.2. packagecloud.ioから各依存ライブラリとヘッダファイルをインストールします。

このセクションでは、[packagecloud.io](https://packagecloud.io/antpickax/stable)から各依存ライブラリとヘッダーファイルをインストールする方法を説明します。

注：前のセクションで[GitHub](https://github.com/yahoojapan)から各依存ライブラリとヘッダーファイルをインストールした場合は、このセクションを読み飛ばしてください。

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

[GitHub](https://github.com/yahoojapan/k2hdkc)から**K2HDKC**のソースコードをダウンロードしてください。

```
$ git clone https://github.com/yahoojapan/k2hdkc.git
```

## 3. ビルドしてインストールする

以下のステップに従って**K2HDKC**をビルドしてインストールしてください。 **K2HDKC**を構築するために[GNU Automake](https://www.gnu.org/software/automake/)を使います。

```
$ cd k2hdkc
$ sh autogen.sh
$ ./configure --prefix=/usr
$ make
$ sudo make install
```

**K2HDKC**を正常にインストールすると、k2hdkcのヘルプテキストが表示されます。
```bash
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
