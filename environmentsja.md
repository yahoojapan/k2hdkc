---
layout: contents
language: ja
title: Environments
short_desc: K2Hash based Distributed Kvs Cluster
lang_opp_file: environments.html
lang_opp_word: To English
prev_url: developerja.html
prev_string: Developer
top_url: indexja.html
top_string: TOP
next_url: toolsja.html
next_string: Tools
---

# 環境変数

## k2hdkcの提供する環境変数
k2hdkcプログラムおよびライブラリは、以下の環境変数の読み込みます。

### DKCDBGMODE
起動オプションの **-d** と同じです。k2hdkcプロセスのデバッグ出力を制御します。  
パラメータにはデバッグレベル（SILENT / ERR / WARN / MSG / DUMP）を指定できます。  
環境変数と起動オプションが同時に指定された場合には起動オプションが優先されます。
### DKCDBGFILE
起動オプションの **-dfile** と同じです。  
k2hdkcプロセスのデバッグ出力をstderrから指定ファイルに置き換えます。  
環境変数と起動オプションが同時に指定された場合には起動オプションが優先されます。
### K2HDKCCONFFILE
起動オプション **-conf** および **-json** の指定をしない場合に、k2hdkcおよびchmpxサーバーノードのためのコンフィグレーションファイル（INI形式、YAML形式、JSON形式）へのパスを指定できます。
### K2HDKCJSONCONF
起動オプション **-conf** および **-json** の指定をしない場合に、k2hdkcおよびchmpxサーバーノードのためのJSON文字列のコンフィグレーションを指定できます。  
K2HDKCCONFFILE環境変数が指定されている場合には、K2HDKCCONFFILEが優先されます。

<br />
これらと同じ項目を C、C++言語のAPIで指定された場合、APIで指定した内容が優先されます。

## k2hash / chmpxの提供する環境変数
k2hdkcプログラムおよびライブラリは、通信ミドルウエア（[CHMPX](https://chmpx.antpick.ax/indexja.html)）とKey Value Store（KVS）ライブラリ（[K2HASH](https://k2hash.antpick.ax/indexja.html)）を利用してます。  
k2hdkcをデバッグする場合に、これらのミドルウエア、ライブラリのメッセージが必要となることが想定されます。  
k2hdkcと一緒に利用しているミドルウエア、ライブラリのデバッグ情報に関する環境変数を以下に紹介します。

### CHMDBGMODE/CHMDBGFILE
chmpxプロセス、ライブラリの提供する環境変数です。  
chmpxプログラム、ライブラリの動作ログを出力できます。  
これらの値を設定することで、chmpxの動作確認、ログメッセージを確認することができます。  
シグナルUSR1を使いCHMDBGMODEのレベル変更をすることもできます。
### K2HDBGMODE/K2HDBGFILE
k2hashライブラリの提供する環境変数です。  
k2hashライブラリの動作ログを出力できます。  
これらの値を設定することで、k2hashライブラリを通した操作の動作確認、ログメッセージを確認することができます。
