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

# ビルド方法
k2hdkc、および k2hdkclinetool をビルドする方法を説明します。

## 1. 事前環境
### Debian / Ubuntu
```
$ sudo aptitude update
$ sudo aptitude install git autoconf autotools-dev gcc g++ make gdb dh-make fakeroot dpkg-dev devscripts libtool pkg-config libssl-dev libyaml-dev
```

### Fedora / CentOS
```
$ sudo yum install git autoconf automake gcc libstdc++-devel gcc-c++ make libtool openssl-devel libyaml-devel
```

## 2. ビルド、インストール：fullock
```
$ git clone https://github.com/yahoojapan/fullock.git
$ cd fullock
$ ./autogen.sh
$ ./configure --prefix=/usr
$ make
$ sudo make install
```

## 3. ビルド、インストール：k2hash
```
$ git clone https://github.com/yahoojapan/k2hash.git
$ cd k2hash
$ ./autogen.sh
$ ./configure --prefix=/usr
$ make
$ sudo make install
```

## 4. ビルド、インストール：chmpx
```
$ git clone https://github.com/yahoojapan/chmpx.git
$ cd chmpx
$ ./autogen.sh
$ ./configure --prefix=/usr
$ make
$ sudo make install
```

## 5. clone
```
$ git clone git@github.com:yahoojapan/k2hdkc.git
```

## 6. ビルド、インストール：k2hdkc / k2hdkclinetool
```
$ ./autogen.sh
$ ./configure --prefix=/usr
$ make
$ sudo make install
```
