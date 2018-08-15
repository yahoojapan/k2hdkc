---
layout: contents
language: ja
title: Tools
short_desc: K2Hash based Distributed Kvs Cluster
lang_opp_file: tools.html
lang_opp_word: To English
prev_url: environmentsja.html
prev_string: Environments
top_url: indexja.html
top_string: TOP
next_url: 
next_string: 
---

# ツール類
**K2HDKC** にて提供するツールには、以下のものがあります。

## k2hdkclinetool
**K2HDKC**クライアントノード上で動作する**K2HDKC**クライアントプログラムです。  
**K2HDKC**クラスタの持つデータへの更新、削除、移動など全てのテスト、操作ができます。  
**k2hlinetool** の **K2HDKC** 版であり同様の機能を提供します。

## デバッグ方法
**K2HDKC**クラスタ、**K2HDKC**クライアントライブラリは、[K2HASH](https://k2hash.antpick.ax/indexja.html)、[CHMPX](https://chmpx.antpick.ax/indexja.html) を含む構成であるため、**k2hdkclinetool** だけではデバッグできないケースがあります。

**K2HDKC**クラスタ、**K2HDKC**クライアントライブラリの動作確認、デバッグを行う場合には以下のツール、設定などを利用するようにしてください。

| tools/settings | component | description |
|-|-|-|
| [k2hdkclinetool](k2hdkclinetoolja.html)               | **K2HDKC**                                       | テストクライアントノードツールです。<br />**K2HDKC**クライアントノードからの**K2HDKC**クラスタのデータへの操作などをテストできます。 |
| [chmpxstatus](https://chmpx.antpick.ax/toolsja.html)  | [CHMPX](https://chmpx.antpick.ax/indexja.html)   | [CHMPX](https://chmpx.antpick.ax/indexja.html)が提供するツールです。<br />[CHMPX](https://chmpx.antpick.ax/indexja.html)で構成されたクラスタの情報を取得・表示などできます。 |
| CHMDBGMODE と CHMDBGFILE 環境変数                     | [CHMPX](https://chmpx.antpick.ax/indexja.html)   | [CHMPX](https://chmpx.antpick.ax/indexja.html)プロセス、ライブラリの動作ログを出力するための環境変数です。<br />これらの値を設定することで、[CHMPX](https://chmpx.antpick.ax/indexja.html)の動作確認、ログメッセージを確認できます。 <br />シグナルUSR1を使いCHMDBGMODEのレベル変更もできます。<br />[環境変数](environmentsja.html)を参照してください。|
| [k2hlinetool](https://k2hash.antpick.ax/toolsja.html) | [K2HASH](https://k2hash.antpick.ax/indexja.html) | ローカル[K2HASH](https://k2hash.antpick.ax/indexja.html)ファイルのテストを行うことができるツールです。<br />**K2HDKC**クラスタのサーバーノードの保持するデータ（[K2HASH](https://k2hash.antpick.ax/indexja.html)ファイル）の状態を確認できます。<br />また、データ変更・参照などできます。|
| K2HDBGMODE と K2HDBGFILE 環境変数                     | [K2HASH](https://k2hash.antpick.ax/indexja.html) | [K2HASH](https://k2hash.antpick.ax/indexja.html)ライブラリの動作ログを出力するための環境変数です。<br />これらの値を設定することで、[K2HASH](https://k2hash.antpick.ax/indexja.html)ライブラリを通した操作の動作確認、ログメッセージを確認できます。<br />シグナルUSR1を使いK2HDBGMODEのレベル変更できます。<br />[環境変数](environmentsja.html)を参照してください。|
