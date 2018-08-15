---
layout: contents
language: en-us
title: Build
short_desc: K2Hash based Distributed Kvs Cluster
lang_opp_file: buildja.html
lang_opp_word: To Japanese
prev_url: usage.html
prev_string: Usage
top_url: index.html
top_string: TOP
next_url: developer.html
next_string: Developer
---

# Building
The build method for k2hdkc and k2hdkclinetool is explained below.

## 1. Install prerequisites before compiling
### Debian / Ubuntu
```
$ sudo aptitude update
$ sudo aptitude install git autoconf autotools-dev gcc g++ make gdb dh-make fakeroot dpkg-dev devscripts libtool pkg-config libssl-dev libyaml-dev
```

### Fedora / CentOS
```
$ sudo yum install git autoconf automake gcc libstdc++-devel gcc-c++ make libtool openssl-devel libyaml-devel
```

## 2. Building and installing fullock
```
$ git clone https://github.com/yahoojapan/fullock.git
$ cd fullock
$ ./autogen.sh
$ ./configure --prefix=/usr
$ make
$ sudo make install
```

## 3. Building and installing k2hash
```
$ git clone https://github.com/yahoojapan/k2hash.git
$ cd k2hash
$ ./autogen.sh
$ ./configure --prefix=/usr
$ make
$ sudo make install
```

## 4. Building and installing chmpx
```
$ git clone https://github.com/yahoojapan/chmpx.git
$ cd chmpx
$ ./autogen.sh
$ ./configure --prefix=/usr
$ make
$ sudo make install
```

## 5. Clone source codes from Github
```
$ git clone git@github.com:yahoojapan/k2hdkc.git
```

## 6. Building and installing k2hdkc and k2hdkclinetool
```
$ ./autogen.sh
$ ./configure --prefix=/usr
$ make
$ sudo make install
```
