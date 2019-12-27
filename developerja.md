---
layout: contents
language: ja
title: Developer
short_desc: K2Hash based Distributed Kvs Cluster
lang_opp_file: developer.html
lang_opp_word: To English
prev_url: buildja.html
prev_string: Build
top_url: indexja.html
top_string: TOP
next_url: environmentsja.html
next_string: Environments
---

<!-- -----------------------------------------------------------　-->
# 開発者向け

### [前提](#PRECONDITION)
[仕様について](#SPECIFICATIONS)  
[C++言語の永続/非永続chmpx接続について](#PERSISTENT)  
[サンプルコードについて](#ABOUTSAMPLES)

### [C言語インタフェース](#CAPI)
[デバッグ関連(C I/F)](#DEBUG)  
[chmpx関連(C I/F)](#CHMPX)  
[Last Response Code関連(C I/F)](#LASTRESCODE)  
[Get関連(C I/F)](#GET)  
[Get Direct関連(C I/F)](#GETDIRECT)  
[Get Subkey関連(C I/F)](#GETSUB)  
[Get Attribute関連(C I/F)](#GETATTR)  
[Set関連(C I/F)](#SET)  
[Set Direct関連(C I/F)](#SETDIRECT)  
[Set Subkey関連(C I/F)](#SETSUB)  
[Clear Subkey関連(C I/F)](#CLEARSUB)  
[Add Subkey関連(C I/F)](#ADDSUB)  
[Set All関連(C I/F)](#SETALL)  
[Remove Key関連(C I/F)](#REMOVE)  
[Remove Subkey関連(C I/F)](#REMOVESUB)  
[Rename Key関連(C I/F)](#RENAME)  
[Queue(KeyQUeue) Push関連(C I/F)](#QUEUEPUSH)  
[Queue(KeyQUeue) Pop関連(C I/F)](#QUEUEPOP)  
[Queue(KeyQUeue) Remove関連(C I/F)](#QUEUEREMOVE)  
[CAS(Compare And Swap) Init関連(C I/F)](#CASINIT)  
[CAS(Compare And Swap) Get関連(C I/F)](#CASGET)  
[CAS(Compare And Swap) Set関連(C I/F)](#CASSET)  
[CAS(Compare And Swap) Increment/Decrement関連(C I/F)](#CASINCDEC)

### [C++言語インタフェース](#CPP)
[デバッグ関連(C++ I/F)](#DEBUGCPP)  
[コマンドクラスファクトリ関連(C++ I/F)](#COMFACTORY)  
[K2hdkcComGetクラス](#GETCPP)  
[K2hdkcComGetDirectクラス](#GETDIRECTCPP)  
[K2hdkcComGetSubkeysクラス](#GETSUBSCPP)  
[K2hdkcComGetAttrsクラス](#GETATTRSCPP)  
[K2hdkcComGetAttrクラス](#GETATTRCPP)  
[K2hdkcComSetクラス](#SETAPP)  
[K2hdkcComSetDirectクラス](#SETDIRECTCPP)  
[K2hdkcComSetSubkeysクラス](#SETSUBSCPP)  
[K2hdkcComSetAllクラス](#SETALLCPP)  
[K2hdkcComAddSubkeysクラス](#ADDSUBSCPP)  
[K2hdkcComAddSubkeyクラス](#ADDSUBCPP)  
[K2hdkcComDelクラス](#DELCPP)  
[K2hdkcComDelSubkeysクラス](#DELSUBSCPP)  
[K2hdkcComDelSubkeyクラス](#DELSUBCPP)  
[K2hdkcComRenクラス](#RENCPP)  
[K2hdkcComQPushクラス](#QUEUEPUSHCPP)  
[K2hdkcComQPopクラス](#QUEUEPOPCPP)  
[K2hdkcComQDelクラス](#QUEUEDELCPP)  
[K2hdkcComCasInitクラス](#CASINITCPP)  
[K2hdkcComCasGetクラス](#CASGETCPP)  
[K2hdkcComCasSetクラス](#CASSETCPP)  
[K2hdkcComCasIncDecクラス](#CASINCDECCPP)  
[K2hdkcComStateクラス](#STATECPP)

<!-- -----------------------------------------------------------　-->
***

## <a name="PRECONDITION"> 前提
k2hdkcライブラリを利用したプログラムを作成するための前提条件を説明します。

### <a name="SPECIFICATIONS"> 仕様について
k2hdkcライブラリを利用する前に、k2hdkcライブラリの仕様の理解が必要となります。
k2hdkcクラスタは、通信ミドルウエア（[CHMPX](https://chmpx.antpick.ax/indexja.html)）とKey Value Store（KVS）ライブラリ（[K2HASH](https://k2hash.antpick.ax/indexja.html)）を組み合わせて実現されています。
k2hdkcクラスタへアクセスするためのプログラムをk2hdkcライブラリを利用し、作成することができます。

k2hdkcクラスタデータの各操作API（C言語/C++言語）は、k2hdkcクラスタにアクセスするため通信ミドルウエア（chmpx）を通して、内部通信コマンドを送受信します。
各APIは、この通信を行う際に以下の2種類の方法があります。
- 通信コマンド毎に毎回chmpxに接続を行い、通信完了後切断する
- 予め永続的利用を行うchmpx接続を行い、この接続を1回以上（プロセス終了時まで）利用・再利用する

各APIの動作は同じですが、操作（k2hdkcクラスタへの通信）の前後でchmpxに対して、毎回接続する処理を行うか否かの違いがあります。
毎回接続するタイプは、クライアントプロセスが永続的な接続を維持できない場合に利用します。（例：HTTPプロセスのハンドラとして実装されている場合など）
永続的な接続を維持するタイプは、デーモンプロセスとして動作するクライアントプロセスのケースなどで利用できます。

### <a name="PERSISTENT"> C++言語の永続/非永続chmpx接続について
C++言語では、chmpxとの永続/非永続接続において、以下に示す手順の違いがあります。
ただし、それぞれにおいて手順は異なりますが、chmpxへの接続処理、切断処理を除く、通信コマンド（専用通信コマンドクラス）の処理はは同じです。
永続/非永続接続の違いは、専用通信コマンドクラスオブジェクトを作成するグローバルメソッド（マクロ）の呼び出しだけの違いとなっています。

#### 非永続chmpx接続
C++言語ライブラリで、非永続chmpx接続を使う場合は、以下の手順でライブラリを呼び出します。
1. グローバルメソッド（マクロ）：GetOtSlaveK2hdkcCom...() を使い専用通信コマンドクラスオブジェクトを取得します。
1. 専用通信コマンドクラスオブジェクトの CommandSend()系メソッドを呼び出し、処理を行います。
1. 処理結果は、専用通信コマンドクラスオブジェクトのGetResponseData()メソッドで取得できます。（CommandSend()系メソッドでも同時取得可能）
1. 最後に、専用通信コマンドクラスオブジェクトを破棄します。

#### 永続chmpx接続
C++言語ライブラリで、永続chmpx接続を使う場合は、以下の手順でライブラリを呼び出します。
1. K2hdkcSlaveクラスインスタンスを作成し、Initialize()メソッドで初期化します。
1. K2hdkcSlaveクラスインスタンスのOpen()メソッドを呼び出し、chmpxへ接続します。
1. グローバルメソッド（マクロ）：GetPmSlaveK2hdkcCom...() にK2hdkcSlaveクラスインスタンスを渡し、専用通信コマンドクラスオブジェクトを取得します。
1. 専用通信コマンドクラスオブジェクトの CommandSend()系メソッドを呼び出し、処理を行います。
1. 処理結果は、専用通信コマンドクラスオブジェクトのGetResponseData()メソッドで取得できます。（CommandSend()系メソッドでも同時取得可能）
1. 専用通信コマンドクラスオブジェクトを破棄します。
1. 上記、3～6の繰り返し
1. K2hdkcSlaveクラスインスタンスのClose()メソッドを呼び出し、chmpxから切断します。

### <a name="ABOUTSAMPLES"> サンプルについて
k2hdkcライブラリインターフェースのC、C++ APIの説明には、簡単なサンプルコードが付属しています。
詳細な使い方はk2hdkcテストツール（k2hdkclinetool）のソースコードを参照することをお勧めします。
このテストツールには、ほぼ全てのサンプルコードに匹敵するコードが含まれています。
このソースコード内で目的の関数、メソッドの検索をすることでサンプルコードとして利用できます。

<!-- -----------------------------------------------------------　-->
***

## <a name="CAPI"> C言語インタフェース
C言語用のインタフェースです。
開発時には以下のヘッダファイルをインクルードしてください。
```
#include <k2hdkc/k2hdkc.h>
```

リンク時には以下をオプションとして指定してください。
```
-lk2hdkc
```

以下にC言語用の関数の説明をします。
<!-- -----------------------------------------------------------　-->
***

### <a name="DEBUG"> デバッグ関連(C I/F)
k2hdkcライブラリは、内部動作およびAPIの動作の確認をするためにメッセージ出力を行うことができます。
本関数群は、メッセージ出力を制御するための関数群です。

#### 書式
- void k2hdkc_bump_debug_level(void)
- void k2hdkc_set_debug_level_silent(void)
- void k2hdkc_set_debug_level_error(void)
- void k2hdkc_set_debug_level_warning(void)
- void k2hdkc_set_debug_level_message(void)
- void k2hdkc_set_debug_level_dump(void)
- bool k2hdkc_set_debug_file(const char* filepath)
- bool k2hdkc_unset_debug_file(void)
- bool k2hdkc_load_debug_env(void)
- bool k2hdkc_is_enable_comlog(void)
- void k2hdkc_enable_comlog(void)
- void k2hdkc_disable_comlog(void)
- void k2hdkc_toggle_comlog(void)

#### 説明
- k2hdkc_bump_debug_level  
  デバッグメッセージの出力レベルを１段階アップします。（SILENT→ERROR→WARNING→INFO→DUMP→SILENT→・・・）
- k2hdkc_set_debug_level_silent  
  デバッグメッセージの出力レベルをSILENTにします。
- k2hdkc_set_debug_level_error  
  デバッグメッセージの出力レベルをERRORにします。
- k2hdkc_set_debug_level_warning  
  デバッグメッセージの出力レベルをWARNINGにします。
- k2hdkc_set_debug_level_message  
  デバッグメッセージの出力レベルをMESSAGE（INFO)にします。
- k2hdkc_set_debug_level_dump  
  デバッグメッセージの出力レベルをDUMPにします。
- k2hdkc_set_debug_file  
  デバッグメッセージの出力先をstderrから指定したファイルに切り替えます。
- k2hdkc_unset_debug_file  
  デバッグメッセージの出力先をstderrに戻します。
- k2hdkc_load_debug_env  
  環境変数DKCDBGMODE、DKCDBGFILEを読み込み設定します。
- k2hdkc_is_enable_comlog  
  通信コマンドの送受信ログ出力が有効か調べます。
- k2hdkc_enable_comlog  
  通信コマンドの送受信ログ出力を有効とします。
- k2hdkc_disable_comlog  
  通信コマンドの送受信ログ出力を無効とします。
- k2hdkc_toggle_comlog  
  通信コマンドの送受信ログ出力を有効/無効で切り替えます。

#### パラメータ
- filepath  
  デバッグメッセージの出力先のファイルパスを指定します。

#### 返り値
k2hdkc_set_debug_file、k2hdkc_unset_debug_file、k2hdkc_load_debug_env、k2hdkc_is_enable_comlogは成功した場合には、trueを返します。失敗した場合にはfalseを返します。

#### 注意
環境変数DKCDBGMODE、DKCDBGFILEについては、[環境変数](environmentsja.html)を参照してください。

#### サンプル
```
k2hdkc_bump_debug_level();
k2hdkc_set_debug_file("/var/log/k2hdkc/error.log");
k2hdkc_set_debug_level_message();
```

<!-- -----------------------------------------------------------　-->
***

### <a name="CHMPX"> chmpx関連(C I/F)
本関数群は、[CHMPX](https://chmpx.antpick.ax/indexja.html)に永続的に接続してk2hdkcライブラリを利用する場合に必要となるchmpxへの接続を行います。
本関数群で返されるk2hdkc_chmpx_hハンドルは、他のk2hdkcライブラリC APIで利用することができるハンドルです。

#### 書式
- k2hdkc_chmpx_h k2hdkc_open_chmpx(const char* config)
- k2hdkc_chmpx_h k2hdkc_open_chmpx_full(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, bool is_clean_bup)
- bool k2hdkc_close_chmpx(k2hdkc_chmpx_h handle)
- bool k2hdkc_close_chmpx_ex(k2hdkc_chmpx_h handle, bool is_clean_bup)

#### 説明
- k2hdkc_open_chmpx  
  chmpxスレーブノードへ接続します。
- k2hdkc_open_chmpx_ex  
  chmpxスレーブノードへ詳細なオプションを指定して接続します。
- k2hdkc_close_chmpx  
  chmpxスレーブノードから切断します。
- k2hdkc_close_chmpx_ex  
  chmpxスレーブノードから詳細なオプションを指定して切断します。

#### パラメータ
- config  
  chmpxスレーブノードのコンフィグレーション（INI形式、YAML形式、JSON形式ファイルへのパス、もしくはJSON文字列）を指定します。
- ctlport  
  chmpxスレーブノードの制御ポート番号を指定します。（同一サーバ上で複数のchmpxが起動している場合には指定する必要があります）
- cuk  
  chmpxスレーブノードのCUK文字列を指定します。（同一サーバ上で複数のchmpxが起動している場合には指定する必要があります）
- is_auto_rejoin  
  chmpxとの接続が切断された場合に自動的に再接続するか指定します。
- no_giveup_rejoin  
  chmpxとの接続が切断された場合に自動的に再接続する指定をしている場合に、その上限をなくすか指定します。
- is_clean_bup  
  chmpxと切断した場合に、不要な情報ファイルなどの後始末をするか指定します。
- handle  
  k2hdkcライブラリで用いるchmpx接続のハンドルです。

#### 返り値
- k2hdkc_open_chmpx / k2hdkc_open_chmpx_ex  
  成功時には、接続したk2hdkc_chmpx_hハンドルを返します。  
  失敗時には、K2HDKC_INVALID_HANDLEを返します。
- k2hdkc_close_chmpx / k2hdkc_close_chmpx_ex  
  成功時にはtrue、失敗時にはfalseを返します。

#### サンプル
```
k2hdkc_chmpx_h    chmpxhandle;
if(K2HDKC_INVALID_HANDLE == (chmpxhandle = k2hdkc_open_chmpx_ex(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin, isCleanupBup))){
    exit(EXIT_FAILURE);
}
...
...
...

if(!k2hdkc_close_chmpx_ex(chmpxhandle, isCleanupBup)){
    fprintf(stderr, "Could not close(leave and close msgid) slave node chmpx, but continue for cleanup...\n");
}
chmpxhandle = K2HDKC_INVALID_HANDLE;        // force
```

<!-- -----------------------------------------------------------　-->
***

### <a name="LASTRESCODE"> Last Response Code関連(C I/F)
k2hdkcライブラリのC APIを利用後、APIのレスポンスコードを取得することができます。
レスポンスコードは、64ビットであり上位32ビットで成功、失敗を示し、下位32ビット（サブコード）で詳細なエラーコードを示しています。
エラーコードの判別などには、マクロが準備されています。（k2hdkccomstructure.hを参照）
エラーコードについては、C言語のerrnoと同等の取り扱いとなります。

#### 書式
- dkcres_type_t k2hdkc_get_lastres_code(void)
- dkcres_type_t k2hdkc_get_lastres_subcode(void)
- bool k2hdkc_is_lastres_success(void)
- dkcres_type_t k2hdkc_get_res_code(k2hdkc_chmpx_h handle)
- dkcres_type_t k2hdkc_get_res_subcode(k2hdkc_chmpx_h handle)
- bool k2hdkc_is_res_success(k2hdkc_chmpx_h handle)

#### 説明
- k2hdkc_get_lastres_code  
  chmpxへ非永続接続をしているケースにおいて、最後のレスポンスコードを取得できます。
- k2hdkc_get_lastres_subcode  
  chmpxへ非永続接続をしているケースにおいて、最後のレスポンスコードのサブコードを取得できます。
- k2hdkc_is_lastres_success  
  chmpxへ非永続接続をしているケースにおいて、最後の処理結果を判定します。
- k2hdkc_get_res_code  
  chmpxへ永続接続をしているケースにおいて、指定したchmpxのハンドルの最後のレスポンスコードを取得できます。
- k2hdkc_get_res_subcode  
  chmpxへ永続接続をしているケースにおいて、指定したchmpxのハンドルの最後のレスポンスコードのサブコードを取得できます。
- k2hdkc_is_res_success  
  chmpxへ永続接続をしているケースにおいて、指定したchmpxのハンドルの最後の処理結果を判定します。

#### パラメータ
- handle  
  k2hdkcライブラリで用いるchmpx接続のハンドルです

#### 返り値
- k2hdkc_get_lastres_code / k2hdkc_get_res_code  
  レスポンスコードを返します。
- k2hdkc_get_lastres_subcode / k2hdkc_get_res_subcode  
  レスポンスコードのサブコードを返します。
- k2hdkc_is_lastres_success / k2hdkc_is_res_success  
  レスポンスの成功時にはtrue、失敗時にはfalseを返します。

#### サンプル
```
// get response code
dkcres_type_t    rescode = k2hdkc_get_lastres_code();

// check result
if(!result){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}
```

<!-- -----------------------------------------------------------　-->
***

### <a name="GET"> Get関連(C I/F)
k2hdkcクラスタからキーを指定して、値を取得する関数群です。

#### 書式
- bool k2hdkc_get_value(const char* config, const unsigned char* pkey, size_t keylength, unsigned char** ppval, size_t* pvallength);
- unsigned char* k2hdkc_get_direct_value(const char* config, const unsigned char* pkey, size_t keylength, size_t* pvallength);
- bool k2hdkc_get_str_value(const char* config, const char* pkey, char** ppval);
- char* k2hdkc_get_str_direct_value(const char* config, const char* pkey);
 
- bool k2hdkc_get_value_wp(const char* config, const unsigned char* pkey, size_t keylength, const char* encpass, unsigned char** ppval, size_t* pvallength);
- unsigned char* k2hdkc_get_direct_value_wp(const char* config, const unsigned char* pkey, size_t keylength, const char* encpass, size_t* pvallength);
- bool k2hdkc_get_str_value_wp(const char* config, const char* pkey, const char* encpass, char** ppval);
- char* k2hdkc_get_str_direct_value_wp(const char* config, const char* pkey, const char* encpass);
 
- bool k2hdkc_get_value_np(const char* config, const unsigned char* pkey, size_t keylength, unsigned char** ppval, size_t* pvallength);
- unsigned char* k2hdkc_get_direct_value_np(const char* config, const unsigned char* pkey, size_t keylength, size_t* pvallength);
- bool k2hdkc_get_str_value_np(const char* config, const char* pkey, char** ppval);
- char* k2hdkc_get_str_direct_value_np(const char* config, const char* pkey);
 
- bool k2hdkc_full_get_value(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, unsigned char** ppval, size_t* pvallength);
- unsigned char* k2hdkc_full_get_direct_value(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, size_t* pvallength);
- bool k2hdkc_full_get_str_value(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, char** ppval);
- char* k2hdkc_full_get_str_direct_value(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey);
 
- bool k2hdkc_full_get_value_wp(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, const char* encpass, unsigned char** ppval, size_t* pvallength);
- unsigned char* k2hdkc_full_get_direct_value_wp(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, const char* encpass, size_t* pvallength);
- bool k2hdkc_full_get_str_value_wp(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, const char* encpass, char** ppval);
- char* k2hdkc_full_get_str_direct_value_wp(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, const char* encpass);
 
- bool k2hdkc_full_get_value_np(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, unsigned char** ppval, size_t* pvallength);
- unsigned char* k2hdkc_full_get_direct_value_np(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, size_t* pvallength);
- bool k2hdkc_full_get_str_value_np(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, char** ppval);
- char* k2hdkc_full_get_str_direct_value_np(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey);
 
- bool k2hdkc_pm_get_value(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, unsigned char** ppval, size_t* pvallength);
- unsigned char* k2hdkc_pm_get_direct_value(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, size_t* pvallength);
- bool k2hdkc_pm_get_str_value(k2hdkc_chmpx_h handle, const char* pkey, char** ppval);
- char* k2hdkc_pm_get_str_direct_value(k2hdkc_chmpx_h handle, const char* pkey);
 
- bool k2hdkc_pm_get_value_wp(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, const char* encpass, unsigned char** ppval, size_t* pvallength);
- unsigned char* k2hdkc_pm_get_direct_value_wp(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, const char* encpass, size_t* pvallength);
- bool k2hdkc_pm_get_str_value_wp(k2hdkc_chmpx_h handle, const char* pkey, const char* encpass, char** ppval);
- char* k2hdkc_pm_get_str_direct_value_wp(k2hdkc_chmpx_h handle, const char* pkey, const char* encpass);
 
- bool k2hdkc_pm_get_value_np(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, unsigned char** ppval, size_t* pvallength);
- unsigned char* k2hdkc_pm_get_direct_value_np(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, size_t* pvallength);
- bool k2hdkc_pm_get_str_value_np(k2hdkc_chmpx_h handle, const char* pkey, char** ppval);
- char* k2hdkc_pm_get_str_direct_value_np(k2hdkc_chmpx_h handle, const char* pkey);

#### 説明
関数は、以下の規則に沿って展開されており、基本は以下4種類となっています。
- k2hdkc_get_value系  
  処理結果の値をバイナリ列として引数に示すバッファに格納して返すタイプ
- k2hdkc_get_direct_value系  
  処理結果の値をバイナリ列として返り値として返すタイプ
- k2hdkc_get_str_value系  
  処理結果の値を文字列として引数に示すバッファに格納して返すタイプ（指定するキーも文字列として呼び出します）
- k2hdkc_get_str_direct_value系  
  処理結果の値を文字列として返り値として返すタイプ（指定するキーも文字列として呼び出します）

上記の4種類に対して、以下の規則でそれぞれのタイプに対して引数などの違いのある類似関数があります。
- k2hdkc_...  
  サフィックスに何もない関数であり、これは基本となる引数列を取ります。
- k2hdkc_..._np  
  サフィックスに **_np** がある関数です。これらの関数は属性チェック（パーミッションチェック）を行わないタイプの関数群です。
- k2hdkc_..._wp  
  サフィックスに **_wp** がある関数です。これらの関数はパスフレーズを指定するタイプの関数群です。

さらに、以下のタイプが上記までのタイプに加え、以下の形式が存在します。
- k2hdkc_...  
  予め設定されているデフォルトのchmpxに関する引数で、chmpxへ接続し、本コマンド処理を行う関数群です。簡単に非永続のchmpxを利用する場合に利用できる関数群です。
- k2hdkc_full_...  
  chmpxに関する引数を呼び出し毎で指定して、chmpxへ接続し、本コマンド処理を行う関数群です。詳細な設定を行いつつ、非永続のchmpxを利用する場合に利用できる関数群です。
- k2hdkc_pm_...  
  永続的なchmpxへのハンドルを指定して、本コマンド処理を行う関数群です。

#### パラメータ
- config  
  chmpxスレーブノードのコンフィグレーション（INI形式、YAML形式、JSON形式ファイルへのパス、もしくはJSON文字列）を指定します。
- ctlport  
  chmpxスレーブノードの制御ポート番号を指定します。（同一サーバ上で複数のchmpxが起動している場合には指定する必要があります）
- cuk  
  chmpxスレーブノードのCUK文字列を指定します。（同一サーバ上で複数のchmpxが起動している場合には指定する必要があります）
- is_auto_rejoin  
  chmpxとの接続が切断された場合に自動的に再接続するか指定します。
- no_giveup_rejoin  
  chmpxとの接続が切断された場合に自動的に再接続する指定をしている場合に、その上限をなくすか指定します。
- pkey  
  バイナリ列の場合にはキーのバイナリ列へのポインタ、文字列の場合にはキー文字列へのポインタを指定します。
- keylength  
  バイナリ列の場合にキーのバイナリ列長を指定します。
- ppval  
  バイナリ列の値を取得する場合には値のバイナリ列を格納するポインタ、文字列の場合には値の文字列を格納するためのポインタを指定します。
- pvallength  
  バイナリ列の値を取得する場合には取得した値のバイナリ列長を格納するポインタを指定します。
- encpass  
  複合するためのパスフレーズを指定します。
- handle  
  k2hdkcライブラリで用いるchmpx接続のハンドルです。

#### 返り値
- k2hdkc_get_value系 / k2hdkc_get_str_value系  
  値が存在しない、エラーの場合にはfalseが返されます。成功した場合にはtrueが返されます。  
  失敗した理由を取得するにはLast Response Codeを参照してください。
- k2hdkc_get_direct_value系  
  成功した場合には取得した値のバイナリ列のポインタが返されます。失敗した理由を取得するにはLast Response Codeを参照してください。
- k2hdkc_get_str_direct_value系  
  成功した場合には取得した値の文字列のポインタが返されます。失敗した理由を取得するにはLast Response Codeを参照してください。

#### サンプル
```
// do command
unsigned char* pval = NULL;
size_t         vallength = 0;
bool           result = k2hdkc_full_get_value(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin, pkey, keylength, &pval, &vallength);
dkcres_type_t  rescode = k2hdkc_get_lastres_code();

// check result
if(!result){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}
free(pval);
```

<!-- -----------------------------------------------------------　-->
***

### <a name="GETDIRECT"> Get Direct関連(C I/F)
k2hdkcクラスタからキーを指定して、値を取得する関数群です。

#### 書式
- bool k2hdkc_da_get_value(const char* config, const unsigned char* pkey, size_t keylength, off_t getpos, size_t val_length, unsigned char** ppval, size_t* pvallength)
- unsigned char* k2hdkc_da_get_direct_value(const char* config, const unsigned char* pkey, size_t keylength, off_t getpos, size_t val_length, size_t* pvallength)
- bool k2hdkc_da_get_str_value(const char* config, const char* pkey, off_t getpos, size_t val_length, char** ppval)
- char* k2hdkc_da_get_str_direct_value(const char* config, const char* pkey, off_t getpos, size_t val_length)
 
- bool k2hdkc_full_da_get_value(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, off_t getpos, size_t val_length, unsigned char** ppval, size_t* pvallength)
- unsigned char* k2hdkc_full_da_get_direct_value(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, off_t getpos, size_t val_length, size_t* pvallength)
- bool k2hdkc_full_da_get_str_value(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, off_t getpos, size_t val_length, char** ppval)
- char* k2hdkc_full_da_get_str_direct_value(const char* conffile, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, off_t getpos, size_t val_length)
 
- bool k2hdkc_pm_da_get_value(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, off_t getpos, size_t val_length, unsigned char** ppval, size_t* pvallength)
- unsigned char* k2hdkc_pm_da_get_direct_value(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, off_t getpos, size_t val_length, size_t* pvallength)
- bool k2hdkc_pm_da_get_str_value(k2hdkc_chmpx_h handle, const char* pkey, off_t getpos, size_t val_length, char** ppval)
- char* k2hdkc_pm_da_get_str_direct_value(k2hdkc_chmpx_h handle, const char* pkey, off_t getpos, size_t val_length)

#### 説明
関数は、以下の規則に沿って展開されており、基本は以下4種類となっています。
- k2hdkc_da_get_value系  
  バイナリ列のキー、読み取り開始オフセットを指定して、バイナリ列の値を取得する関数です。値は引数で渡したバッファに設定され返されます。
- k2hdkc_da_get_direct_value系  
  バイナリ列のキー、読み取り開始オフセットを指定して、バイナリ列の値を取得する関数です。値は返り値で返されます。
- k2hdkc_da_get_str_value系  
  文字列のキー、読み取り開始オフセットを指定して、文字列の値を取得する関数です。値は引数で渡したバッファに設定され返されます。
- k2hdkc_da_get_str_direct_value系  
  文字列のキー、読み取り開始オフセットを指定して、文字列の値を取得する関数です。値は返り値で返されます。

上記の4種類に対して、以下の規則でそれぞれのタイプに対して引数などの違いのある類似関数があります。
- k2hdkc_...  
  予め設定されているデフォルトのchmpxに関する引数で、chmpxへ接続し、本コマンド処理を行う関数群です。簡単に非永続のchmpxを利用する場合に利用できる関数群です。
- k2hdkc_full_...  
  chmpxに関する引数を呼び出し毎で指定して、chmpxへ接続し、本コマンド処理を行う関数群です。詳細な設定を行いつつ、非永続のchmpxを利用する場合に利用できる関数群です。
- k2hdkc_pm_...  
  永続的なchmpxへのハンドルを指定して、本コマンド処理を行う関数群です。

#### パラメータ
- config  
  chmpxスレーブノードのコンフィグレーション（INI形式、YAML形式、JSON形式ファイルへのパス、もしくはJSON文字列）を指定します。
- ctlport  
  chmpxスレーブノードの制御ポート番号を指定します。（同一サーバ上で複数のchmpxが起動している場合には指定する必要があります）
- cuk  
  chmpxスレーブノードのCUK文字列を指定します。（同一サーバ上で複数のchmpxが起動している場合には指定する必要があります）
- is_auto_rejoin  
  chmpxとの接続が切断された場合に自動的に再接続するか指定します。
- no_giveup_rejoin  
  chmpxとの接続が切断された場合に自動的に再接続する指定をしている場合に、その上限をなくすか指定します。
- pkey  
  バイナリ列の場合にはキーのバイナリ列へのポインタ、文字列の場合にはキー文字列へのポインタを指定します。
- keylength  
  バイナリ列の場合にキーのバイナリ列長を指定します。
- getpos  
  値の読み出しオフセットを指定します。
- ppval  
  バイナリ列の値を取得する場合には値のバイナリ列を格納するポインタ、文字列の場合には値の文字列を格納するためのポインタを指定します。
- pvallength  
  バイナリ列の値を取得する場合には取得した値のバイナリ列長を格納するポインタを指定します。
- handle  
  k2hdkcライブラリで用いるchmpx接続のハンドルです。

#### 返り値
- k2hdkc_da_get_value系 / k2hdkc_da_get_str_value系  
  値が存在しない、エラーの場合にはfalseが返されます。成功した場合にはtrueが返されます。  
  失敗した理由を取得するにはLast Response Codeを参照してください。
- k2hdkc_da_get_direct_value系  
  成功した場合には取得した値のバイナリ列のポインタが返されます。失敗した理由を取得するにはLast Response Codeを参照してください。
- k2hdkc_da_get_str_direct_value系  
  成功した場合には取得した値の文字列のポインタが返されます。失敗した理由を取得するにはLast Response Codeを参照してください。

#### 注意
Get Direct系にはGet系のように、属性チェック（パーミッションチェック）を行わないタイプ、パスフレーズを指定するタイプのI/Fはありません。

#### サンプル
```
// get direct
unsigned char**    ppval    = NULL;
size_t        vallen    = 0;
bool        result    = k2hdkc_full_da_get_value(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin, pkey, keylen, offset, length, ppval, &vallen);
dkcres_type_t    rescode    = k2hdkc_get_lastres_code();

// check result
if(!result){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}
```


<!-- -----------------------------------------------------------　-->
***

### <a name="GETSUB"> Get Subkey関連(C I/F)
k2hdkcクラスタからキーを指定して、Subkeyリストを取得する関数群です。

#### 書式
- bool k2hdkc_get_subkeys(const char* config, const unsigned char* pkey, size_t keylength, PK2HDKCKEYPCK* ppskeypck, int* pskeypckcnt)
- PK2HDKCKEYPCK k2hdkc_get_direct_subkeys(const char* config, const unsigned char* pkey, size_t keylength, int* pskeypckcnt)
- int k2hdkc_get_str_subkeys(const char* config, const char* pkey, char*** ppskeyarray)
- char** k2hdkc_get_str_direct_subkeys(const char* config, const char* pkey)
 
- bool k2hdkc_get_subkeys_np(const char* config, const unsigned char* pkey, size_t keylength, PK2HDKCKEYPCK* ppskeypck, int* pskeypckcnt)
- PK2HDKCKEYPCK k2hdkc_get_direct_subkeys_np(const char* config, const unsigned char* pkey, size_t keylength, int* pskeypckcnt)
- int k2hdkc_get_str_subkeys_np(const char* config, const char* pkey, char*** ppskeyarray)
- char** k2hdkc_get_str_direct_subkeys_np(const char* config, const char* pkey)
 
- bool k2hdkc_full_get_subkeys(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, PK2HDKCKEYPCK* ppskeypck, int* pskeypckcnt)
- PK2HDKCKEYPCK k2hdkc_full_get_direct_subkeys(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, int* pskeypckcnt)
- int k2hdkc_full_get_str_subkeys(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, char*** ppskeyarray)
- char** k2hdkc_full_get_str_direct_subkeys(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey)
 
- bool k2hdkc_full_get_subkeys_np(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, PK2HDKCKEYPCK* ppskeypck, int* pskeypckcnt)
- PK2HDKCKEYPCK k2hdkc_full_get_direct_subkeys_np(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, int* pskeypckcnt)
- int k2hdkc_full_get_str_subkeys_np(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, char*** ppskeyarray)
- char** k2hdkc_full_get_str_direct_subkeys_np(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey)
 
- bool k2hdkc_pm_get_subkeys(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, PK2HDKCKEYPCK* ppskeypck, int* pskeypckcnt)
- PK2HDKCKEYPCK k2hdkc_pm_get_direct_subkeys(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, int* pskeypckcnt)
- int k2hdkc_pm_get_str_subkeys(k2hdkc_chmpx_h handle, const char* pkey, char*** ppskeyarray)
- char** k2hdkc_pm_get_str_direct_subkeys(k2hdkc_chmpx_h handle, const char* pkey)
 
- bool k2hdkc_pm_get_subkeys_np(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, PK2HDKCKEYPCK* ppskeypck, int* pskeypckcnt)
- PK2HDKCKEYPCK k2hdkc_pm_get_direct_subkeys_np(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, int* pskeypckcnt)
- int k2hdkc_pm_get_str_subkeys_np(k2hdkc_chmpx_h handle, const char* pkey, char*** ppskeyarray)
- char** k2hdkc_pm_get_str_direct_subkeys_np(k2hdkc_chmpx_h handle, const char* pkey)

#### 説明
関数は、以下の規則に沿って展開されており、基本は以下4種類となっています。
- k2hdkc_get_subkeys系  
  バイナリ列のキーを指定して、PK2HDKCKEYPCKを取得する関数です。値は引数で渡したバッファに設定され返されます。
- k2hdkc_get_direct_subkeys系  
  バイナリ列のキーを指定して、PK2HDKCKEYPCKを取得する関数です。値は返り値で返されます。
- k2hdkc_get_str_subkeys系  
  文字列のキーを指定して、文字列配列を取得する関数です。値は引数で渡したバッファに設定され返されます。
- k2hdkc_get_str_direct_subkeys系  
  文字列のキーを指定して、文字列配列（NULL終端）を取得する関数です。値は返り値で返されます。

上記の4種類に対して、以下の規則でそれぞれのタイプに対して引数などの違いのある類似関数があります。
- k2hdkc_...  
  サフィックスに何もない関数であり、これは基本となる引数列を取ります。
- k2hdkc_..._np  
  サフィックスに"_np"があるタイプであり、これらの関数は属性チェック（パーミッションチェック）を行わないタイプの関数群です。

さらに、以下のタイプが上記までのタイプに加えて（積算）、存在します。
- k2hdkc_...  
  予め設定されているデフォルトのchmpxに関する引数で、chmpxへ接続し、本コマンド処理を行う関数群です。簡単に非永続のchmpxを利用する場合に利用できる関数群です。
- k2hdkc_full_...  
  chmpxに関する引数を呼び出し毎で指定して、chmpxへ接続し、本コマンド処理を行う関数群です。詳細な設定を行いつつ、非永続のchmpxを利用する場合に利用できる関数群です。
- k2hdkc_pm_...  
  永続的なchmpxへのハンドルを指定して、本コマンド処理を行う関数群です。

#### パラメータ
- config  
  chmpxスレーブノードのコンフィグレーション（INI形式、YAML形式、JSON形式ファイルへのパス、もしくはJSON文字列）を指定します。
- ctlport  
  chmpxスレーブノードの制御ポート番号を指定します。（同一サーバ上で複数のchmpxが起動している場合には指定する必要があります）
- cuk  
  chmpxスレーブノードのCUK文字列を指定します。（同一サーバ上で複数のchmpxが起動している場合には指定する必要があります）
- is_auto_rejoin  
  chmpxとの接続が切断された場合に自動的に再接続するか指定します。
- no_giveup_rejoin  
  chmpxとの接続が切断された場合に自動的に再接続する指定をしている場合に、その上限をなくすか指定します。
- pkey  
  バイナリ列の場合にはキーのバイナリ列へのポインタ、文字列の場合にはキー文字列へのポインタを指定します。
- keylength  
  バイナリ列の場合にキーのバイナリ列長を指定します。
- ppskeypck  
  取得したSubkeyのリストをバイナリ列配列として取得するためのポインタです。
- pskeypckcnt  
   取得したSubkeyリストの数を返すためのポインタです。
- ppskeyarray  
  取得したSubkeyのリストを文字列配列として取得するためのポインタです。
- handle  
  k2hdkcライブラリで用いるchmpx接続のハンドルです。

#### 返り値
- k2hdkc_get_subkeys系 / k2hdkc_get_str_subkeys系  
  値が存在しない、エラーの場合にはfalseが返されます。成功した場合にはtrueが返されます。  
  失敗した理由を取得するにはLast Response Codeを参照してください。
- k2hdkc_get_direct_subkeys系  
  成功した場合には取得したSubkeyリストのPK2HDKCKEYPCKポインタが返されます。失敗した理由を取得するにはLast Response Codeを参照してください。
- k2hdkc_get_str_direct_subkeys系  
  成功した場合には取得した値の文字列配列のポインタが返されます。失敗した理由を取得するにはLast Response Codeを参照してください。

#### 構造体について
引数および返り値に指定されるK2HDKCKEYPCK構造体について説明します。
この構造体は、[K2HASH](https://k2hash.antpick.ax/indexja.html)にて定義されており、k2hdkcでtypedefされた構造体です。実態は以下のようになっています。
また、領域確保されたこの構造体へのポインタ、構造体の配列へのポインタの開放については、以下のマクロ（k2hdkc提供）もしくは[K2HASH](https://k2hash.antpick.ax/indexja.html)のAPIを利用してください。
```
// structure
typedef struct k2h_key_pack{
    unsigned char*    pkey;
    size_t            length;
}K2HKEYPCK, *PK2HKEYPCK;
 
// typedefs by k2hdkc
typedef K2HKEYPCK                   K2HDKCKEYPCK;
typedef PK2HKEYPCK                  PK2HDKCKEYPCK;
 
// Free function
extern bool k2h_free_keypack(PK2HKEYPCK pkeys, int keycnt);
extern bool k2h_free_keyarray(char** pkeys);
 
// Free macro presented by k2hdkc
#define    DKC_FREE_KEYPACK            k2h_free_keypack
#define    DKC_FREE_KEYARRAY           k2h_free_keyarray
```

#### サンプル
```
PK2HDKCKEYPCK    pskeypck = NULL;
int        skeypckcnt = 0;
bool        result = k2hdkc_full_get_subkeys(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin, pkey, keylength, &pskeypck, &skeypckcnt);
dkcres_type_t    rescode = k2hdkc_get_lastres_code();
 
// check result
if(!result){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}
```

<!-- -----------------------------------------------------------　-->
***

### <a name="GETATTR"> Get Attribute関連(C I/F)
k2hdkcクラスタからキーを指定して、属性（Attribute）を取得する関数群です。

#### 書式
- bool k2hdkc_get_attrs(const char* config, const unsigned char* pkey, size_t keylength, PK2HDKCATTRPCK* ppattrspck, int* pattrspckcnt)
- PK2HDKCATTRPCK k2hdkc_get_direct_attrs(const char* config, const unsigned char* pkey, size_t keylength, int* pattrspckcnt)
- PK2HDKCATTRPCK k2hdkc_get_str_direct_attrs(const char* config, const char* pkey, int* pattrspckcnt)
 
- bool k2hdkc_full_get_attrs(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, PK2HDKCATTRPCK* ppattrspck, int* pattrspckcnt)
- PK2HDKCATTRPCK k2hdkc_full_get_direct_attrs(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, int* pattrspckcnt)
- PK2HDKCATTRPCK k2hdkc_full_get_str_direct_attrs(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, int* pattrspckcnt)
 
- bool k2hdkc_pm_get_attrs(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, PK2HDKCATTRPCK* ppattrspck, int* pattrspckcnt)
- PK2HDKCATTRPCK k2hdkc_pm_get_direct_attrs(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, int* pattrspckcnt)
- PK2HDKCATTRPCK k2hdkc_pm_get_str_direct_attrs(k2hdkc_chmpx_h handle, const char* pkey, int* pattrspckcnt)

#### 説明
関数は、以下の規則に沿って展開されており、基本は以下4種類となっています。
- k2hdkc_get_attrs系  
  バイナリ列のキーを指定して、PK2HDKCATTRPCKを取得する関数です。値は引数で渡したバッファに設定され返されます。
- k2hdkc_get_direct_attrs系  
  バイナリ列のキーを指定して、PK2HDKCATTRPCKを取得する関数です。値は返り値で返されます。
- k2hdkc_get_str_direct_attrs系  
  文字列のキーを指定して、PK2HDKCATTRPCKを取得する関数です。値は返り値で返されます。


上記の3種類に対して、以下の規則でそれぞれのタイプに対して引数などの違いのある類似関数があります。
- k2hdkc_...  
  予め設定されているデフォルトのchmpxに関する引数で、chmpxへ接続し、本コマンド処理を行う関数群です。簡単に非永続のchmpxを利用する場合に利用できる関数群です。
- k2hdkc_full_...  
  chmpxに関する引数を呼び出し毎で指定して、chmpxへ接続し、本コマンド処理を行う関数群です。詳細な設定を行いつつ、非永続のchmpxを利用する場合に利用できる関数群です。
- k2hdkc_pm_...  
  永続的なchmpxへのハンドルを指定して、本コマンド処理を行う関数群です。

#### パラメータ
- config  
  chmpxスレーブノードのコンフィグレーション（INI形式、YAML形式、JSON形式ファイルへのパス、もしくはJSON文字列）を指定します。
- ctlport  
  chmpxスレーブノードの制御ポート番号を指定します。（同一サーバ上で複数のchmpxが起動している場合には指定する必要があります）
- cuk  
  chmpxスレーブノードのCUK文字列を指定します。（同一サーバ上で複数のchmpxが起動している場合には指定する必要があります）
- is_auto_rejoin  
  chmpxとの接続が切断された場合に自動的に再接続するか指定します。
- no_giveup_rejoin  
  chmpxとの接続が切断された場合に自動的に再接続する指定をしている場合に、その上限をなくすか指定します。
- pkey  
  バイナリ列の場合にはキーのバイナリ列へのポインタ、文字列の場合にはキー文字列へのポインタを指定します。
- keylength  
  バイナリ列の場合にキーのバイナリ列長を指定します。
- ppattrspck  
  取得したAttributesのリストをバイナリ列配列として取得するためのポインタです。
- pattrspckcnt  
  取得したAttributesリストの数を返すためのポインタです。
- handle  
  k2hdkcライブラリで用いるchmpx接続のハンドルです。

#### 返り値
- k2hdkc_get_attrs系  
  値が存在しない、エラーの場合にはfalseが返されます。成功した場合にはtrueが返されます。  
  失敗した理由を取得するにはLast Response Codeを参照してください。
- k2hdkc_get_direct_attrs系 / k2hdkc_get_str_direct_attrs系  
  成功した場合には取得した属性リストのPK2HDKCATTRPCKポインタが返されます。失敗した理由を取得するにはLast Response Codeを参照してください。

#### 構造体について
引数および返り値に指定されるK2HDKCATTRPCK構造体について説明します。
この構造体は、[K2HASH](https://k2hash.antpick.ax/indexja.html)にて定義されており、k2hdkcでtypedefされた構造体です。実態は以下のようになっています。
また、領域確保されたこの構造体へのポインタ、構造体の配列へのポインタの開放については、以下のマクロ（k2hdkc提供）もしくは[K2HASH](https://k2hash.antpick.ax/indexja.html)のAPIを利用してください。
```
// for attributes list
typedef struct k2h_attr_pack{
    unsigned char*    pkey;
    size_t            keylength;
    unsigned char*    pval;
    size_t            vallength;
}K2HATTRPCK, *PK2HATTRPCK;
 
// typedefs by k2hdkc
typedef K2HATTRPCK                    K2HDKCATTRPCK;
typedef PK2HATTRPCK                    PK2HDKCATTRPCK;
 
// Free attributes array
extern bool k2h_free_attrpack(PK2HATTRPCK pattrs, int attrcnt);
 
// Free attributes macro presented by k2hdkc
#define    DKC_FREE_ATTRPACK    k2h_free_attrpack
```

#### サンプル
```
PK2HDKCATTRPCK    pattrspck    = NULL;
int        attrspckcnt    = 0;
bool        result = k2hdkc_full_get_attrs(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, &pattrspck, &attrspckcnt);
dkcres_type_t    rescode= k2hdkc_get_lastres_code();
 
// check result
if(!result){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
    DKC_FREE_ATTRPACK(pattrspck, attrspckcnt);
}
```

<!-- -----------------------------------------------------------　-->
***

### <a name="SET"> Set関連(C I/F)
k2hdkcクラスタからキーを指定して、値を設定する関数群です。

#### 書式
- bool k2hdkc_set_value(const char* config, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength)
- bool k2hdkc_set_str_value(const char* config, const char* pkey, const char* pval)
 
- bool k2hdkc_full_set_value(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength)
- bool k2hdkc_full_set_str_value(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, const char* pval)
 
- bool k2hdkc_pm_set_value(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength)
- bool k2hdkc_pm_set_str_value(k2hdkc_chmpx_h handle, const char* pkey, const char* pval)
 
- bool k2hdkc_set_value_wa(const char* config, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, bool rmsubkeylist, const char* encpass, const time_t* expire)
- bool k2hdkc_set_str_value_wa(const char* config, const char* pkey, const char* pval, bool rmsubkeylist, const char* encpass, const time_t* expire)
 
- bool k2hdkc_full_set_value_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, bool rmsubkeylist, const char* encpass, const time_t* expire)
- bool k2hdkc_full_set_str_value_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, const char* pval, bool rmsubkeylist, const char* encpass, const time_t* expire)
 
- bool k2hdkc_pm_set_value_wa(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, bool rmsubkeylist, const char* encpass, const time_t* expire)
- bool k2hdkc_pm_set_str_value_wa(k2hdkc_chmpx_h handle, const char* pkey, const char* pval, bool rmsubkeylist, const char* encpass, const time_t* expire)

#### 説明
関数は、以下の規則に沿って展開されており、基本は以下2種類となっています。
- k2hdkc_set_value系  
  バイナリ列のキーを指定して、バイナリ列の値を設定する関数です。
- k2hdkc_set_str_value系  
  文字列のキーを指定して、文字列の値を設定する関数です。

上記の2種類に対して、以下の規則でそれぞれのタイプに対して引数などの違いのある類似関数があります。
- k2hdkc_...  
  サフィックスに何もない関数であり、これは基本となる引数列を取ります。
- k2hdkc_..._wa  
  サフィックスに"_wa"があるタイプであり、これらの関数はパスフレーズ、有効期限を指定するタイプの関数群です。

さらに、以下のタイプが上記までのタイプに加えて（積算）、存在します。
- k2hdkc_...  
  予め設定されているデフォルトのchmpxに関する引数で、chmpxへ接続し、本コマンド処理を行う関数群です。簡単に非永続のchmpxを利用する場合に利用できる関数群です。
- k2hdkc_full_...  
  chmpxに関する引数を呼び出し毎で指定して、chmpxへ接続し、本コマンド処理を行う関数群です。詳細な設定を行いつつ、非永続のchmpxを利用する場合に利用できる関数群です。
- k2hdkc_pm_...  
  永続的なchmpxへのハンドルを指定して、本コマンド処理を行う関数群です。

#### パラメータ
- config  
  chmpxスレーブノードのコンフィグレーション（INI形式、YAML形式、JSON形式ファイルへのパス、もしくはJSON文字列）を指定します。
- ctlport  
  chmpxスレーブノードの制御ポート番号を指定します。（同一サーバ上で複数のchmpxが起動している場合には指定する必要があります）
- cuk  
  chmpxスレーブノードのCUK文字列を指定します。（同一サーバ上で複数のchmpxが起動している場合には指定する必要があります）
- is_auto_rejoin  
  chmpxとの接続が切断された場合に自動的に再接続するか指定します。
- no_giveup_rejoin  
  chmpxとの接続が切断された場合に自動的に再接続する指定をしている場合に、その上限をなくすか指定します。
- pkey  
  バイナリ列の場合にはキーのバイナリ列へのポインタ、文字列の場合にはキー文字列へのポインタを指定します。
- keylength  
  バイナリ列の場合にキーのバイナリ列長を指定します。
- pval  
  値へのポインタです。バイナリ列の場合、文字列の場合いずれもポインタで指定します。
- vallength  
  バイナリ列の値を設定する場合のバイナリ列長を指定します。
- rmsubkeylist  
  設定するキーがサブキーを持つ場合には、サブキーリストを削除するか否か（サブキー自体の削除ではない）を指定します。
- encpass  
  暗号化するためのパスフレーズを指定します。
- expire  
  値に有効期限を設定する場合に有効期限を指定します。
- handle  
  k2hdkcライブラリで用いるchmpx接続のハンドルです。

#### 返り値
成功した場合にはtrue、失敗した場合にはfalseを返します。

#### サンプル
```
bool        result = k2hdkc_full_set_value(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin, pKey, KeyLen, pVal, ValLen);
dkcres_type_t    rescode = k2hdkc_get_lastres_code();
 
// check result
if(!result){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}
```

<!-- -----------------------------------------------------------　-->
***

### <a name="SETDIRECT"> Set Direct関連(C I/F)
k2hdkcクラスタからキーを指定して、値を設定する関数群です。

#### 書式
- bool k2hdkc_da_set_value(const char* config, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, const off_t setpos)
- bool k2hdkc_da_set_str_value(const char* config, const char* pkey, const char* pval, const off_t setpos)
 
- bool k2hdkc_full_da_set_value(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, const off_t setpos)
- bool k2hdkc_full_da_set_str_value(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, const char* pval, const off_t setpos)
 
- bool k2hdkc_pm_da_set_value(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, const off_t setpos)
- bool k2hdkc_pm_da_set_str_value(k2hdkc_chmpx_h handle, const char* pkey, const char* pval, const off_t setpos)

#### 説明
関数は、以下の規則に沿って展開されており、基本は以下2種類となっています。
- k2hdkc_da_set_value系  
  バイナリ列のキー、値の設定オフセットを指定して、バイナリ列の値を設定する関数です。
- k2hdkc_da_set_str_value系  
  文字列のキー、値の設定オフセットを指定して、文字列の値を設定する関数です。

上記の2種類に対して、以下の規則でそれぞれのタイプに対して引数などの違いのある類似関数があります。
- k2hdkc_...  
  予め設定されているデフォルトのchmpxに関する引数で、chmpxへ接続し、本コマンド処理を行う関数群です。簡単に非永続のchmpxを利用する場合に利用できる関数群です。
- k2hdkc_full_...  
  chmpxに関する引数を呼び出し毎で指定して、chmpxへ接続し、本コマンド処理を行う関数群です。詳細な設定を行いつつ、非永続のchmpxを利用する場合に利用できる関数群です。
- k2hdkc_pm_...  
  永続的なchmpxへのハンドルを指定して、本コマンド処理を行う関数群です。

#### パラメータ
- config  
  chmpxスレーブノードのコンフィグレーション（INI形式、YAML形式、JSON形式ファイルへのパス、もしくはJSON文字列）を指定します。
- ctlport  
  chmpxスレーブノードの制御ポート番号を指定します。（同一サーバ上で複数のchmpxが起動している場合には指定する必要があります）
- cuk  
  chmpxスレーブノードのCUK文字列を指定します。（同一サーバ上で複数のchmpxが起動している場合には指定する必要があります）
- is_auto_rejoin  
  chmpxとの接続が切断された場合に自動的に再接続するか指定します。
- no_giveup_rejoin  
  chmpxとの接続が切断された場合に自動的に再接続する指定をしている場合に、その上限をなくすか指定します。
- pkey  
  バイナリ列の場合にはキーのバイナリ列へのポインタ、文字列の場合にはキー文字列へのポインタを指定します。
- keylength  
  バイナリ列の場合にキーのバイナリ列長を指定します。
- pval  
  値へのポインタです。バイナリ列の場合、文字列の場合いずれもポインタで指定します。
- vallength  
  バイナリ列の値を設定する場合のバイナリ列長を指定します。
- setpos  
  設定する値のオフセットを指定します。
- handle  
  k2hdkcライブラリで用いるchmpx接続のハンドルです。

#### 返り値
成功した場合にはtrue、失敗した場合にはfalseを返します。

#### サンプル
```
bool        result    = k2hdkc_full_da_set_value(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin, pkey, keylen, pval, vallen, offset);
dkcres_type_t    rescode    = k2hdkc_get_lastres_code();
 
// check result
if(!result){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}
```

<!-- -----------------------------------------------------------　-->
***

### <a name="SETSUB"> Set Subkey関連(C I/F)
k2hdkcクラスタからキーを指定して、サブキーリストを設定する関数群です。

#### 書式
- bool k2hdkc_set_subkeys(const char* config, const unsigned char* pkey, size_t keylength, const PK2HDKCKEYPCK pskeypck, int skeypckcnt)
- bool k2hdkc_set_str_subkeys(const char* config, const char* pkey, const char** pskeyarray)
 
- bool k2hdkc_full_set_subkeys(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, const PK2HDKCKEYPCK pskeypck, int skeypckcnt)
- bool k2hdkc_full_set_str_subkeys(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, const char** pskeyarray)
 
- bool k2hdkc_pm_set_subkeys(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, const PK2HDKCKEYPCK pskeypck, int skeypckcnt)
- bool k2hdkc_pm_set_str_subkeys(k2hdkc_chmpx_h handle, const char* pkey, const char** pskeyarray)

#### 説明
関数は、以下の規則に沿って展開されており、基本は以下2種類となっています。
- k2hdkc_set_subkeys系  
  バイナリのキーを指定して、PK2HDKCKEYPCKポインタ（サブキーリストのバイナリ列配列）でサブキーリストを設定する関数です。
- k2hdkc_set_str_subkeys系  
  文字列のキーを指定して、文字列配列のサブキーリストを設定する関数です。

上記の2種類に対して、以下の規則でそれぞれのタイプに対して引数などの違いのある類似関数があります。
- k2hdkc_...  
  予め設定されているデフォルトのchmpxに関する引数で、chmpxへ接続し、本コマンド処理を行う関数群です。簡単に非永続のchmpxを利用する場合に利用できる関数群です。
- k2hdkc_full_...  
  chmpxに関する引数を呼び出し毎で指定して、chmpxへ接続し、本コマンド処理を行う関数群です。詳細な設定を行いつつ、非永続のchmpxを利用する場合に利用できる関数群です。
- k2hdkc_pm_...  
  永続的なchmpxへのハンドルを指定して、本コマンド処理を行う関数群です。

#### パラメータ
- config  
  chmpxスレーブノードのコンフィグレーション（INI形式、YAML形式、JSON形式ファイルへのパス、もしくはJSON文字列）を指定します。
- ctlport  
  chmpxスレーブノードの制御ポート番号を指定します。（同一サーバ上で複数のchmpxが起動している場合には指定する必要があります）
- cuk  
  chmpxスレーブノードのCUK文字列を指定します。（同一サーバ上で複数のchmpxが起動している場合には指定する必要があります）
- is_auto_rejoin  
  chmpxとの接続が切断された場合に自動的に再接続するか指定します。
- no_giveup_rejoin  
  chmpxとの接続が切断された場合に自動的に再接続する指定をしている場合に、その上限をなくすか指定します。
- pkey  
  バイナリ列の場合にはキーのバイナリ列へのポインタ、文字列の場合にはキー文字列へのポインタを指定します。
- keylength  
  バイナリ列の場合にキーのバイナリ列長を指定します。
- pskeypck  
  バイナリのサブキーリストをPK2HDKCKEYPCKポインタで指定します。
- skeypckcnt  
  サブキーリストの数を指定します。
- pskeyarray  
  文字列のサブキーリストを文字列配列（NULL終端）で指定します。
- handle  
  k2hdkcライブラリで用いるchmpx接続のハンドルです。

#### 返り値
成功した場合にはtrue、失敗した場合にはfalseを返します。

#### サンプル
```
bool        result    = k2hdkc_full_set_subkeys(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin, pPKey, PKeyLen, pskeypck, skeypckcnt);
dkcres_type_t    rescode = k2hdkc_get_lastres_code();
 
// check result
if(!result){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}
```

<!-- -----------------------------------------------------------　-->
***

### <a name="CLEARSUB"> Clear Subkey関連(C I/F)
k2hdkcクラスタからキーを指定して、サブキーリストをクリアする関数群です。

#### 書式
- bool k2hdkc_clear_subkeys(const char* config, const unsigned char* pkey, size_t keylength)
- bool k2hdkc_clear_str_subkeys(const char* config, const char* pkey)
 
- bool k2hdkc_full_clear_subkeys(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength)
- bool k2hdkc_full_clear_str_subkeys(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey)
 
- bool k2hdkc_pm_clear_subkeys(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength)
- bool k2hdkc_pm_clear_str_subkeys(k2hdkc_chmpx_h handle, const char* pkey)

#### 説明
関数は、以下の規則に沿って展開されており、基本は以下2種類となっています。
- k2hdkc_clear_subkeys系  
  バイナリのキーを指定して、そのキーの持つサブキーリストをクリアします。
- k2hdkc_clear_str_subkeys系  
  文字列のキーを指定して、そのキーの持つサブキーリストをクリアします。

上記の2種類に対して、以下の規則でそれぞれのタイプに対して引数などの違いのある類似関数があります。
- k2hdkc_...  
  予め設定されているデフォルトのchmpxに関する引数で、chmpxへ接続し、本コマンド処理を行う関数群です。簡単に非永続のchmpxを利用する場合に利用できる関数群です。
- k2hdkc_full_...  
  chmpxに関する引数を呼び出し毎で指定して、chmpxへ接続し、本コマンド処理を行う関数群です。詳細な設定を行いつつ、非永続のchmpxを利用する場合に利用できる関数群です。
- k2hdkc_pm_...  
  永続的なchmpxへのハンドルを指定して、本コマンド処理を行う関数群です。

#### パラメータ
- config  
  chmpxスレーブノードのコンフィグレーション（INI形式、YAML形式、JSON形式ファイルへのパス、もしくはJSON文字列）を指定します。
- ctlport  
  chmpxスレーブノードの制御ポート番号を指定します。（同一サーバ上で複数のchmpxが起動している場合には指定する必要があります）
- cuk  
  chmpxスレーブノードのCUK文字列を指定します。（同一サーバ上で複数のchmpxが起動している場合には指定する必要があります）
- is_auto_rejoin  
  chmpxとの接続が切断された場合に自動的に再接続するか指定します。
- no_giveup_rejoin  
  chmpxとの接続が切断された場合に自動的に再接続する指定をしている場合に、その上限をなくすか指定します。
- pkey  
  バイナリ列の場合にはキーのバイナリ列へのポインタ、文字列の場合にはキー文字列へのポインタを指定します。
- keylength  
  バイナリ列の場合にキーのバイナリ列長を指定します。
- handle  
  k2hdkcライブラリで用いるchmpx接続のハンドルです。

#### 返り値
成功した場合にはtrue、失敗した場合にはfalseを返します。

#### サンプル
```
bool        result    = k2hdkc_full_clear_subkeys(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin, pPKey, PKeyLen);
dkcres_type_t    rescode = k2hdkc_get_lastres_code();
 
// check result
if(!result){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}
```

<!-- -----------------------------------------------------------　-->
***

### <a name="ADDSUB"> Add Subkey関連(C I/F)
k2hdkcクラスタから親キー、サブキーを指定して、サブキーの作成・設定する関数群です。

#### 書式
- bool k2hdkc_set_subkey(const char* config, const unsigned char* pkey, size_t keylength, const unsigned char* psubkey, size_t subkeylength, const unsigned char* pskeyval, size_t skeyvallength)
- bool k2hdkc_set_str_subkey(const char* config, const char* pkey, const char* psubkey, const char* pskeyval)
 
- bool k2hdkc_full_set_subkey(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, const unsigned char* psubkey, size_t subkeylength, const unsigned char* pskeyval, size_t skeyvallength)
- bool k2hdkc_full_set_str_subkey(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, const char* psubkey, const char* pskeyval)
 
- bool k2hdkc_pm_set_subkey(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, const unsigned char* psubkey, size_t subkeylength, const unsigned char* pskeyval, size_t skeyvallength)
- bool k2hdkc_pm_set_str_subkey(k2hdkc_chmpx_h handle, const char* pkey, const char* psubkey, const char* pskeyval)
 
- bool k2hdkc_set_subkey_wa(const char* config, const unsigned char* pkey, size_t keylength, const unsigned char* psubkey, size_t subkeylength, const unsigned char* pskeyval, size_t skeyvallength, bool checkattr, const char* encpass, const time_t* expire)
- bool k2hdkc_set_str_subkey_wa(const char* config, const char* pkey, const char* psubkey, const char* pskeyval, bool checkattr, const char* encpass, const time_t* expire)
 
- bool k2hdkc_full_set_subkey_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, const unsigned char* psubkey, size_t subkeylength, const unsigned char* pskeyval, size_t skeyvallength, bool checkattr, const char* encpass, const time_t* expire)
- bool k2hdkc_full_set_str_subkey_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, const char* psubkey, const char* pskeyval, bool checkattr, const char* encpass, const time_t* expire)
 
- bool k2hdkc_pm_set_subkey_wa(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, const unsigned char* psubkey, size_t subkeylength, const unsigned char* pskeyval, size_t skeyvallength, bool checkattr, const char* encpass, const time_t* expire)
- bool k2hdkc_pm_set_str_subkey_wa(k2hdkc_chmpx_h handle, const char* pkey, const char* psubkey, const char* pskeyval, bool checkattr, const char* encpass, const time_t* expire)

#### 説明
関数は、以下の規則に沿って展開されており、基本は以下2種類となっています。
- k2hdkc_set_subkey系  
  バイナリの親キー、サブキーを指定して、バイナリの値をサブキーに設定し、親キーにはサブキーを追加する関数です。
- k2hdkc_set_str_subkey系  
  文字列の親キー、サブキーを指定して、文字列をサブキーに設定し、親キーにはサブキーを追加する関数です。

上記の2種類に対して、以下の規則でそれぞれのタイプに対して引数などの違いのある類似関数があります。
- k2hdkc_...  
  サフィックスに何もない関数であり、これは基本となる引数列を取ります。
- k2hdkc_..._wa  
  サフィックスに"_wa"があるタイプであり、これらの関数はパスフレーズ、有効期限を指定するタイプの関数群です。

さらに、以下のタイプが上記までのタイプに加えて（積算）、存在します。
- k2hdkc_...  
  予め設定されているデフォルトのchmpxに関する引数で、chmpxへ接続し、本コマンド処理を行う関数群です。簡単に非永続のchmpxを利用する場合に利用できる関数群です。
- k2hdkc_full_...  
  chmpxに関する引数を呼び出し毎で指定して、chmpxへ接続し、本コマンド処理を行う関数群です。詳細な設定を行いつつ、非永続のchmpxを利用する場合に利用できる関数群です。
- k2hdkc_pm_...  
  永続的なchmpxへのハンドルを指定して、本コマンド処理を行う関数群です。

#### パラメータ
- config  
  chmpxスレーブノードのコンフィグレーション（INI形式、YAML形式、JSON形式ファイルへのパス、もしくはJSON文字列）を指定します。
- ctlport  
  chmpxスレーブノードの制御ポート番号を指定します。（同一サーバ上で複数のchmpxが起動している場合には指定する必要があります）
- cuk  
  chmpxスレーブノードのCUK文字列を指定します。（同一サーバ上で複数のchmpxが起動している場合には指定する必要があります）
- is_auto_rejoin  
  chmpxとの接続が切断された場合に自動的に再接続するか指定します。
- no_giveup_rejoin  
  chmpxとの接続が切断された場合に自動的に再接続する指定をしている場合に、その上限をなくすか指定します。
- pkey  
  バイナリ列の場合にはキーのバイナリ列へのポインタ、文字列の場合にはキー文字列へのポインタを指定します。
- keylength  
  バイナリ列の場合にキーのバイナリ列長を指定します。
- psubkey  
  バイナリの場合にはバイナリ列、文字列の場合には文字列で示すサブキーを指定します。
- subkeylength  
  バイナリのサブキー長を指定します。
- pskeyval  
  バイナリの場合にはバイナリ列、文字列の場合には文字列で示すサブキーの値を指定します。
- skeyvallength  
  バイナリのサブキー値長を指定します。
- checkattr  
  親キーへサブキーを追加する際に、親キーの属性（パスフレーズ、有効期限）をチェックするか示します。
- encpass  
  暗号化するためのパスフレーズを指定します。
- expire  
  値に有効期限を設定する場合に有効期限を指定します。
- handle  
  k2hdkcライブラリで用いるchmpx接続のハンドルです。

#### 返り値
成功した場合にはtrue、失敗した場合にはfalseを返します。

#### サンプル
```
bool        result    = k2hdkc_full_set_subkeys(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin, pPKey, PKeyLen, pskeypck, skeypckcnt);
dkcres_type_t    rescode = k2hdkc_get_lastres_code();
 
// check result
if(!result){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}
```

<!-- -----------------------------------------------------------　-->
***

### <a name="SETALL"> Set All関連(C I/F)
k2hdkcクラスタからキーを指定して、値を設定する関数群です。

#### 書式
- bool k2hdkc_set_all(const char* config, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, const PK2HDKCKEYPCK pskeypck, int skeypckcnt)
- bool k2hdkc_set_str_all(const char* config, const char* pkey, const char* pval, const char** pskeyarray)
 
- bool k2hdkc_full_set_all(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, const PK2HDKCKEYPCK pskeypck, int skeypckcnt)
- bool k2hdkc_full_set_str_all(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, const char* pval, const char** pskeyarray)
 
- bool k2hdkc_pm_set_all(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, const PK2HDKCKEYPCK pskeypck, int skeypckcnt)
- bool k2hdkc_pm_set_str_all(k2hdkc_chmpx_h handle, const char* pkey, const char* pval, const char** pskeyarray)
 
- bool k2hdkc_set_all_wa(const char* config, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, const PK2HDKCKEYPCK pskeypck, int skeypckcnt, const char* encpass, const time_t* expire)
- bool k2hdkc_set_str_all_wa(const char* config, const char* pkey, const char* pval, const char** pskeyarray, const char* encpass, const time_t* expire)
 
- bool k2hdkc_full_set_all_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, const PK2HDKCKEYPCK pskeypck, int skeypckcnt, const char* encpass, const time_t* expire)
- bool k2hdkc_full_set_str_all_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, const char* pval, const char** pskeyarray, const char* encpass, const time_t* expire)
 
- bool k2hdkc_pm_set_all_wa(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, const PK2HDKCKEYPCK pskeypck, int skeypckcnt, const char* encpass, const time_t* expire)
- bool k2hdkc_pm_set_str_all_wa(k2hdkc_chmpx_h handle, const char* pkey, const char* pval, const char** pskeyarray, const char* encpass, const time_t* expire)

#### 説明
関数は、以下の規則に沿って展開されており、基本は以下2種類となっています。
- k2hdkc_set_all系  
  バイナリ列のキーを指定して、バイナリ列の値、PK2HDKCKEYPCKで示すサブキーリストを設定する関数です。
- k2hdkc_set_str_all系  
  文字列のキーを指定して、文字列の値、文字列配列で示すサブキーリストを設定する関数です。

上記の2種類に対して、以下の規則でそれぞれのタイプに対して引数などの違いのある類似関数があります。
- k2hdkc_...  
  サフィックスに何もない関数であり、これは基本となる引数列を取ります。
- k2hdkc_..._wa  
  サフィックスに"_wa"があるタイプであり、これらの関数はパスフレーズ、有効期限を指定するタイプの関数群です。

さらに、以下のタイプが上記までのタイプに加えて（積算）、存在します。
- k2hdkc_...  
  予め設定されているデフォルトのchmpxに関する引数で、chmpxへ接続し、本コマンド処理を行う関数群です。簡単に非永続のchmpxを利用する場合に利用できる関数群です。
- k2hdkc_full_...  
  chmpxに関する引数を呼び出し毎で指定して、chmpxへ接続し、本コマンド処理を行う関数群です。詳細な設定を行いつつ、非永続のchmpxを利用する場合に利用できる関数群です。
- k2hdkc_pm_...  
  永続的なchmpxへのハンドルを指定して、本コマンド処理を行う関数群です。

#### パラメータ
- config  
  chmpxスレーブノードのコンフィグレーション（INI形式、YAML形式、JSON形式ファイルへのパス、もしくはJSON文字列）を指定します。
- ctlport  
  chmpxスレーブノードの制御ポート番号を指定します。（同一サーバ上で複数のchmpxが起動している場合には指定する必要があります）
- cuk  
  chmpxスレーブノードのCUK文字列を指定します。（同一サーバ上で複数のchmpxが起動している場合には指定する必要があります）
- is_auto_rejoin  
  chmpxとの接続が切断された場合に自動的に再接続するか指定します。
- no_giveup_rejoin  
  chmpxとの接続が切断された場合に自動的に再接続する指定をしている場合に、その上限をなくすか指定します。
- pkey  
  バイナリ列の場合にはキーのバイナリ列へのポインタ、文字列の場合にはキー文字列へのポインタを指定します。
- keylength  
  バイナリ列の場合にキーのバイナリ列長を指定します。
- pval  
  値へのポインタです。バイナリ列の場合、文字列の場合いずれもポインタで指定します。
- vallength  
  バイナリ列の値を設定する場合のバイナリ列長を指定します。
- pskeypck  
  PK2HDKCKEYPCKで示すサブキーリストのポインタを指定します。
- skeypckcnt  
  PK2HDKCKEYPCKで示すサブキーリストの要素数を指定します。
- pskeyarray  
  文字列配列（NULL終端）で示すサブキーリストを指定します。
- encpass  
  暗号化するためのパスフレーズを指定します。
- expire  
  値に有効期限を設定する場合に有効期限を指定します。
- handle  
  k2hdkcライブラリで用いるchmpx接続のハンドルです。

#### 返り値
成功した場合にはtrue、失敗した場合にはfalseを返します。

#### サンプル
```
bool        result = k2hdkc_full_set_all(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin, pKey, KeyLen, pVal, ValLen, NULL, 0);
dkcres_type_t    rescode = k2hdkc_get_lastres_code();
 
// check result
if(!result){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}
```

<!-- -----------------------------------------------------------　-->
***

### <a name="REMOVE"> Remove Key関連(C I/F)
k2hdkcクラスタからキーを指定して、キー（およびキーに属するサブキー）を削除する関数群です。

#### 書式
- bool k2hdkc_remove_all(const char* config, const unsigned char* pkey, size_t keylength)
- bool k2hdkc_remove_str_all(const char* config, const char* pkey)
 
- bool k2hdkc_full_remove_all(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength)
- bool k2hdkc_full_remove_str_all(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey)
 
- bool k2hdkc_pm_remove_all(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength)
- bool k2hdkc_pm_remove_str_all(k2hdkc_chmpx_h handle, const char* pkey)
 
- bool k2hdkc_remove(const char* config, const unsigned char* pkey, size_t keylength)
- bool k2hdkc_remove_str(const char* config, const char* pkey)
 
- bool k2hdkc_full_remove(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength)
- bool k2hdkc_full_remove_str(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey)
 
- bool k2hdkc_pm_remove(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength)
- bool k2hdkc_pm_remove_str(k2hdkc_chmpx_h handle, const char* pkey)

#### 説明
関数は、以下の規則に沿って展開されており、基本は以下2種類（4タイプ）となっています。
- k2hdkc_remove_all系  
  バイナリのキーを指定して、そのキーの持つサブキーリストのサブキーを含み全ての削除をします。
- k2hdkc_remove系  
  バイナリのキーを指定して、そのキーのみ削除をします。
- k2hdkc_remove_str_all系  
  文字列のキーを指定して、そのキーの持つサブキーリストのサブキーを含み全ての削除をします。
- k2hdkc_remove_str系  
  文字列のキーを指定して、そのキーのみ削除をします。

上記の2種類に対して、以下の規則でそれぞれのタイプに対して引数などの違いのある類似関数があります。
- k2hdkc_...  
  予め設定されているデフォルトのchmpxに関する引数で、chmpxへ接続し、本コマンド処理を行う関数群です。簡単に非永続のchmpxを利用する場合に利用できる関数群です。
- k2hdkc_full_...  
  chmpxに関する引数を呼び出し毎で指定して、chmpxへ接続し、本コマンド処理を行う関数群です。詳細な設定を行いつつ、非永続のchmpxを利用する場合に利用できる関数群です。
- k2hdkc_pm_...  
  永続的なchmpxへのハンドルを指定して、本コマンド処理を行う関数群です。

#### パラメータ
- config  
   chmpxスレーブノードのコンフィグレーション（INI形式、YAML形式、JSON形式ファイルへのパス、もしくはJSON文字列）を指定します。
- ctlport  
  chmpxスレーブノードの制御ポート番号を指定します。（同一サーバ上で複数のchmpxが起動している場合には指定する必要があります）
- cuk  
  chmpxスレーブノードのCUK文字列を指定します。（同一サーバ上で複数のchmpxが起動している場合には指定する必要があります）
- is_auto_rejoin  
  chmpxとの接続が切断された場合に自動的に再接続するか指定します。
- no_giveup_rejoin  
  chmpxとの接続が切断された場合に自動的に再接続する指定をしている場合に、その上限をなくすか指定します。
- pkey  
  バイナリ列の場合にはキーのバイナリ列へのポインタ、文字列の場合にはキー文字列へのポインタを指定します。
- keylength  
  バイナリ列の場合にキーのバイナリ列長を指定します。
- handle  
  k2hdkcライブラリで用いるchmpx接続のハンドルです。

#### 返り値
成功した場合にはtrue、失敗した場合にはfalseを返します。

#### サンプル
```
bool        result = k2hdkc_full_remove(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin, pkey, keylen);
dkcres_type_t    rescode = k2hdkc_get_lastres_code();
 
// check result
if(!result){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}
```


<!-- -----------------------------------------------------------　-->
***

### <a name="REMOVESUB"> Remove Subkey関連(C I/F)
k2hdkcクラスタから親キーを指定して、サブキーを削除する関数群です。

#### 書式
- bool k2hdkc_remove_subkey(const char* config, const unsigned char* pkey, size_t keylength, const unsigned char* psubkey, size_t subkeylength, bool is_nest)
- bool k2hdkc_remove_str_subkey(const char* config, const char* pkey, const char* psubkey, bool is_nest)
 
- bool k2hdkc_full_remove_subkey(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, const unsigned char* psubkey, size_t subkeylength, bool is_nest)
- bool k2hdkc_full_remove_str_subkey(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, const char* psubkey, bool is_nest)
 
- bool k2hdkc_pm_remove_subkey(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, const unsigned char* psubkey, size_t subkeylength, bool is_nest)
- bool k2hdkc_pm_remove_str_subkey(k2hdkc_chmpx_h handle, const char* pkey, const char* psubkey, size_t subkeylength, bool is_nest)
 
- bool k2hdkc_remove_subkey_np(const char* config, const unsigned char* pkey, size_t keylength, const unsigned char* psubkey, size_t subkeylength, bool is_nest)
- bool k2hdkc_remove_str_subkey_np(const char* config, const char* pkey, const char* psubkey, bool is_nest)
 
- bool k2hdkc_full_remove_subkey_np(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, const unsigned char* psubkey, size_t subkeylength, bool is_nest)
- bool k2hdkc_full_remove_str_subkey_np(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, const char* psubkey, bool is_nest)
 
- bool k2hdkc_pm_remove_subkey_np(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, const unsigned char* psubkey, size_t subkeylength, bool is_nest)
- bool k2hdkc_pm_remove_str_subkey_np(k2hdkc_chmpx_h handle, const char* pkey, const char* psubkey, size_t subkeylength, bool is_nest)

#### 説明
関数は、以下の規則に沿って展開されており、基本は以下2種類となっています。
- k2hdkc_remove_subkey系  
  バイナリ列の親キー、サブキーを指定して、そのサブキーを削除します。
- k2hdkc_remove_str_subkey系  
  文字列列の親キー、サブキーを指定して、そのサブキーを削除します。

上記の4種類に対して、以下の規則でそれぞれのタイプに対して引数などの違いのある類似関数があります。
- k2hdkc_...  
  サフィックスに何もない関数であり、これは基本となる引数列を取ります。
- k2hdkc_..._np  
  サフィックスに"_np"があるタイプであり、これらの関数は属性チェック（パーミッションチェック）を行わないタイプの関数群です。

さらに、以下のタイプが上記までのタイプに加えて（積算）、存在します。
- k2hdkc_...  
  予め設定されているデフォルトのchmpxに関する引数で、chmpxへ接続し、本コマンド処理を行う関数群です。簡単に非永続のchmpxを利用する場合に利用できる関数群です。
- k2hdkc_full_...  
  chmpxに関する引数を呼び出し毎で指定して、chmpxへ接続し、本コマンド処理を行う関数群です。詳細な設定を行いつつ、非永続のchmpxを利用する場合に利用できる関数群です。
- k2hdkc_pm_...  
  永続的なchmpxへのハンドルを指定して、本コマンド処理を行う関数群です。

#### パラメータ
- config  
  chmpxスレーブノードのコンフィグレーション（INI形式、YAML形式、JSON形式ファイルへのパス、もしくはJSON文字列）を指定します。
- ctlport  
  chmpxスレーブノードの制御ポート番号を指定します。（同一サーバ上で複数のchmpxが起動している場合には指定する必要があります）
- cuk  
  chmpxスレーブノードのCUK文字列を指定します。（同一サーバ上で複数のchmpxが起動している場合には指定する必要があります）
- is_auto_rejoin  
  chmpxとの接続が切断された場合に自動的に再接続するか指定します。
- no_giveup_rejoin  
  chmpxとの接続が切断された場合に自動的に再接続する指定をしている場合に、その上限をなくすか指定します。
- pkey  
  バイナリ列の場合にはキーのバイナリ列へのポインタ、文字列の場合には親キー文字列へのポインタを指定します。
- keylength  
  バイナリ列の場合に親キーのバイナリ列長を指定します。
- psubkey  
  バイナリ列の場合にはサブキーのバイナリ列へのポインタ、文字列の場合にはサブキー文字列へのポインタを指定します。
- subkeylength  
  バイナリ列の場合にサブキーのバイナリ列長を指定します。
- is_nest  
  サブキーにサブキーが存在する場合にネストしてサブキーの削除を行うか指定します。
- handle  
  k2hdkcライブラリで用いるchmpx接続のハンドルです。

#### 返り値
成功した場合にはtrue、失敗した場合にはfalseを返します。

#### サンプル
```
bool        result = k2hdkc_full_remove_subkey(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin, pkey, keylen, psubkey, subkeylen, true);
dkcres_type_t    rescode = k2hdkc_get_lastres_code();
 
// check result
if(!result){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}
```

<!-- -----------------------------------------------------------　-->
***

### <a name="RENAME"> Rename Key関連(C I/F)
k2hdkcクラスタからキーを指定して、キーをリネームする関数群です。

#### 書式
- bool k2hdkc_rename(const char* config, const unsigned char* poldkey, size_t oldkeylength, const unsigned char* pnewkey, size_t newkeylength)
- bool k2hdkc_rename_str(const char* config, const char* poldkey, const char* pnewkey)
 
- bool k2hdkc_full_rename(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* poldkey, size_t oldkeylength, const unsigned char* pnewkey, size_t newkeylength)
- bool k2hdkc_full_rename_str(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* poldkey, const char* pnewkey)
 
- bool k2hdkc_pm_rename(k2hdkc_chmpx_h handle, const unsigned char* poldkey, size_t oldkeylength, const unsigned char* pnewkey, size_t newkeylength)
- bool k2hdkc_pm_rename_str(k2hdkc_chmpx_h handle, const char* poldkey, const char* pnewkey)
 
- bool k2hdkc_rename_wa(const char* config, const unsigned char* poldkey, size_t oldkeylength, const unsigned char* pnewkey, size_t newkeylength, bool checkattr, const char* encpass, const time_t* expire)
- bool k2hdkc_rename_str_wa(const char* config, const char* poldkey, const char* pnewkey, bool checkattr, const char* encpass, const time_t* expire)
 
- bool k2hdkc_full_rename_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* poldkey, size_t oldkeylength, const unsigned char* pnewkey, size_t newkeylength, bool checkattr, const char* encpass, const time_t* expire)
- bool k2hdkc_full_rename_str_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* poldkey, const char* pnewkey, bool checkattr, const char* encpass, const time_t* expire)
 
- bool k2hdkc_pm_rename_wa(k2hdkc_chmpx_h handle, const unsigned char* poldkey, size_t oldkeylength, const unsigned char* pnewkey, size_t newkeylength, bool checkattr, const char* encpass, const time_t* expire)
- bool k2hdkc_pm_rename_str_wa(k2hdkc_chmpx_h handle, const char* poldkey, const char* pnewkey, bool checkattr, const char* encpass, const time_t* expire)
 
- bool k2hdkc_rename_with_parent(const char* config, const unsigned char* poldkey, size_t oldkeylength, const unsigned char* pnewkey, size_t newkeylength, const unsigned char* pparentkey, size_t parentkeylength)
- bool k2hdkc_rename_with_parent_str(const char* config, const char* poldkey, const char* pnewkey, const char* pparentkey)
 
- bool k2hdkc_full_rename_with_parent(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* poldkey, size_t oldkeylength, const unsigned char* pnewkey, size_t newkeylength, const unsigned char* pparentkey, size_t parentkeylength)
- bool k2hdkc_full_rename_with_parent_str(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* poldkey, const char* pnewkey, const char* pparentkey)
 
- bool k2hdkc_pm_rename_with_parent(k2hdkc_chmpx_h handle, const unsigned char* poldkey, size_t oldkeylength, const unsigned char* pnewkey, size_t newkeylength, const unsigned char* pparentkey, size_t parentkeylength)
- bool k2hdkc_pm_rename_with_parent_str(k2hdkc_chmpx_h handle, const char* poldkey, const char* pnewkey, const char* pparentkey)
 
- bool k2hdkc_rename_with_parent_wa(const char* config, const unsigned char* poldkey, size_t oldkeylength, const unsigned char* pnewkey, size_t newkeylength, const unsigned char* pparentkey, size_t parentkeylength, bool checkattr, const char* encpass, const time_t* expire)
- bool k2hdkc_rename_with_parent_str_wa(const char* config, const char* poldkey, const char* pnewkey, const char* pparentkey, bool checkattr, const char* encpass, const time_t* expire)
 
- bool k2hdkc_full_rename_with_parent_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* poldkey, size_t oldkeylength, const unsigned char* pnewkey, size_t newkeylength, const unsigned char* pparentkey, size_t parentkeylength, bool checkattr, const char* encpass, const time_t* expire)
- bool k2hdkc_full_rename_with_parent_str_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* poldkey, const char* pnewkey, const char* pparentkey, bool checkattr, const char* encpass, const time_t* expire)
 
- bool k2hdkc_pm_rename_with_parent_wa(k2hdkc_chmpx_h handle, const unsigned char* poldkey, size_t oldkeylength, const unsigned char* pnewkey, size_t newkeylength, const unsigned char* pparentkey, size_t parentkeylength, bool checkattr, const char* encpass, const time_t* expire)
- bool k2hdkc_pm_rename_with_parent_str_wa(k2hdkc_chmpx_h handle, const char* poldkey, const char* pnewkey, const char* pparentkey, bool checkattr, const char* encpass, const time_t* expire)

#### 説明
関数は、以下の規則に沿って展開されており、基本は以下2種類（4タイプ）となっています。
- k2hdkc_rename系  
  バイナリ列として、旧キー、新キーを指定してリネームする関数です。
- k2hdkc_rename_with_parent系  
  バイナリ列として、旧キー、新キー、親キーを指定してリネームする関数です。親キーのサブキーリストにあるキー名の変更も同時に行われます。
- k2hdkc_rename_str系  
  文字列として、旧キー、新キーを指定してリネームする関数です。
- k2hdkc_rename_with_parent_str系  
  文字列として、旧キー、新キー、親キーを指定してリネームする関数です。親キーのサブキーリストにあるキー名の変更も同時に行われます。

上記の2種類に対して、以下の規則でそれぞれのタイプに対して引数などの違いのある類似関数があります。
- k2hdkc_...  
  サフィックスに何もない関数であり、これは基本となる引数列を取ります。
- k2hdkc_..._wa  
  サフィックスに"_wa"があるタイプであり、これらの関数はパスフレーズ、有効期限を指定するタイプの関数群です。

さらに、以下のタイプが上記までのタイプに加えて（積算）、存在します。
- k2hdkc_...  
  予め設定されているデフォルトのchmpxに関する引数で、chmpxへ接続し、本コマンド処理を行う関数群です。簡単に非永続のchmpxを利用する場合に利用できる関数群です。
- k2hdkc_full_...  
  chmpxに関する引数を呼び出し毎で指定して、chmpxへ接続し、本コマンド処理を行う関数群です。詳細な設定を行いつつ、非永続のchmpxを利用する場合に利用できる関数群です。
- k2hdkc_pm_...  
  永続的なchmpxへのハンドルを指定して、本コマンド処理を行う関数群です。

#### パラメータ
- config  
  chmpxスレーブノードのコンフィグレーション（INI形式、YAML形式、JSON形式ファイルへのパス、もしくはJSON文字列）を指定します。
- ctlport  
  chmpxスレーブノードの制御ポート番号を指定します。（同一サーバ上で複数のchmpxが起動している場合には指定する必要があります）
- cuk  
  chmpxスレーブノードのCUK文字列を指定します。（同一サーバ上で複数のchmpxが起動している場合には指定する必要があります）
- is_auto_rejoin  
  chmpxとの接続が切断された場合に自動的に再接続するか指定します。
- no_giveup_rejoin  
  chmpxとの接続が切断された場合に自動的に再接続する指定をしている場合に、その上限をなくすか指定します。
- poldkey  
  バイナリ列の場合には旧キーのバイナリ列へのポインタ、文字列の場合には旧キー文字列へのポインタを指定します。
- oldkeylength  
  バイナリ列の場合に旧キーのバイナリ列長を指定します。
- pnewkey  
  バイナリ列の場合には新キーのバイナリ列へのポインタ、文字列の場合には新キー文字列へのポインタを指定します。
- newkeylength  
  バイナリ列の場合に新キーのバイナリ列長を指定します。
- pparentkey  
  バイナリ列の場合には親キーのバイナリ列へのポインタ、文字列の場合には親キー文字列へのポインタを指定します。
- parentkeylength  
  バイナリ列の場合に親キーのバイナリ列長を指定します。
- checkattr  
  親キーのサブキーリストの変更の際に、親キーの属性（Attribute）の確認をするか否かを指定します。
- encpass  
  暗号化するためのパスフレーズを指定します。
- expire  
  値に有効期限を設定する場合に有効期限を指定します。
- handle  
  k2hdkcライブラリで用いるchmpx接続のハンドルです。

#### 返り値
成功した場合にはtrue、失敗した場合にはfalseを返します。

#### 注意

#### サンプル
```
bool        result = k2hdkc_full_rename_with_parent(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin, pOldKey, OldKeyLen, pNewKey, NewkeyLen, pParentKey, pParentKeyLen);
dkcres_type_t    rescode = k2hdkc_get_lastres_code();
 
// check result
if(!result){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}
```

<!-- -----------------------------------------------------------　-->
***

### <a name="QUEUEPUSH"> Queue(KeyQUeue) Push関連(C I/F)
k2hdkcクラスタからキューを指定して、キューにPUSHする関数群です。

#### 書式
- bool k2hdkc_q_push(const char* config, const unsigned char* pprefix, size_t prefixlength, const unsigned char* pval, size_t vallength, bool is_fifo)
- bool k2hdkc_q_str_push(const char* config, const char* pprefix, const char* pval, bool is_fifo)
 
- bool k2hdkc_full_q_push(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pprefix, size_t prefixlength, const unsigned char* pval, size_t vallength, bool is_fifo)
- bool k2hdkc_full_q_str_push(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pprefix, const char* pval, bool is_fifo)
 
- bool k2hdkc_pm_q_push(k2hdkc_chmpx_h handle, const unsigned char* pprefix, size_t prefixlength, const unsigned char* pval, size_t vallength, bool is_fifo)
- bool k2hdkc_pm_q_str_push(k2hdkc_chmpx_h handle, const char* pprefix, const char* pval, bool is_fifo)
 
- bool k2hdkc_q_push_wa(const char* config, const unsigned char* pprefix, size_t prefixlength, const unsigned char* pval, size_t vallength, bool is_fifo, bool checkattr, const char* encpass, const time_t* expire)
- bool k2hdkc_q_str_push_wa(const char* config, const char* pprefix, const char* pval, bool is_fifo, bool checkattr, const char* encpass, const time_t* expire)
 
- bool k2hdkc_full_q_push_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pprefix, size_t prefixlength, const unsigned char* pval, size_t vallength, bool is_fifo, bool checkattr, const char* encpass, const time_t* expire)
- bool k2hdkc_full_q_str_push_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pprefix, const char* pval, bool is_fifo, bool checkattr, const char* encpass, const time_t* expire)
 
- bool k2hdkc_pm_q_push_wa(k2hdkc_chmpx_h handle, const unsigned char* pprefix, size_t prefixlength, const unsigned char* pval, size_t vallength, bool is_fifo, bool checkattr, const char* encpass, const time_t* expire)
- bool k2hdkc_pm_q_str_push_wa(k2hdkc_chmpx_h handle, const char* pprefix, const char* pval, bool is_fifo, bool checkattr, const char* encpass, const time_t* expire)
 
- bool k2hdkc_keyq_push(const char* config, const unsigned char* pprefix, size_t prefixlength, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, bool is_fifo)
- bool k2hdkc_keyq_str_push(const char* config, const char* pprefix, const char* pkey, const char* pval, bool is_fifo)
 
- bool k2hdkc_full_keyq_push(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pprefix, size_t prefixlength, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, bool is_fifo)
- bool k2hdkc_full_keyq_str_push(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pprefix, const char* pkey, const char* pval, bool is_fifo)
 
- bool k2hdkc_pm_keyq_push(k2hdkc_chmpx_h handle, const unsigned char* pprefix, size_t prefixlength, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, bool is_fifo)
- bool k2hdkc_pm_keyq_str_push(k2hdkc_chmpx_h handle, const char* pprefix, const char* pkey, const char* pval, bool is_fifo)
 
- bool k2hdkc_keyq_push_wa(const char* config, const unsigned char* pprefix, size_t prefixlength, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, bool is_fifo, bool checkattr, const char* encpass, const time_t* expire)
- bool k2hdkc_keyq_str_push_wa(const char* config, const char* pprefix, const char* pkey, const char* pval, bool is_fifo, bool checkattr, const char* encpass, const time_t* expire)
 
- bool k2hdkc_full_keyq_push_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pprefix, size_t prefixlength, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, bool is_fifo, bool checkattr, const char* encpass, const time_t* expire)
- bool k2hdkc_full_keyq_str_push_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pprefix, const char* pkey, const char* pval, bool is_fifo, bool checkattr, const char* encpass, const time_t* expire)
 
- bool k2hdkc_pm_keyq_push_wa(k2hdkc_chmpx_h handle, const unsigned char* pprefix, size_t prefixlength, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, bool is_fifo, bool checkattr, const char* encpass, const time_t* expire)
- bool k2hdkc_pm_keyq_str_push_wa(k2hdkc_chmpx_h handle, const char* pprefix, const char* pkey, const char* pval, bool is_fifo, bool checkattr, const char* encpass, const time_t* expire)

#### 説明
関数は、以下の規則に沿って展開されており、基本は以下2種類（4タイプ）となっています。
- k2hdkc_q_push系  
  バイナリ列としてキューを指定して、値(バイナリ）をキューにPUSHする関数です。（QUEUE)
- k2hdkc_keyq_push系  
  バイナリ列としてキューを指定して、キー(バイナリ）と値(バイナリ）をキューにPUSHする関数です。（KEYQUEUE）
- k2hdkc_q_str_push系  
  文字列としてキューを指定して、値(文字列）をキューにPUSHする関数です。（QUEUE)
- k2hdkc_keyq_str_push系  
  文字列としてキューを指定して、キー(文字列）と値(文字列）をキューにPUSHする関数です。（KEYQUEUE）

上記の2種類に対して、以下の規則でそれぞれのタイプに対して引数などの違いのある類似関数があります。
- k2hdkc_...  
  サフィックスに何もない関数であり、これは基本となる引数列を取ります。
- k2hdkc_..._wa  
  サフィックスに"_wa"があるタイプであり、これらの関数はパスフレーズ、有効期限を指定するタイプの関数群です。

さらに、以下のタイプが上記までのタイプに加えて（積算）、存在します。
- k2hdkc_...  
  予め設定されているデフォルトのchmpxに関する引数で、chmpxへ接続し、本コマンド処理を行う関数群です。簡単に非永続のchmpxを利用する場合に利用できる関数群です。
- k2hdkc_full_...  
  chmpxに関する引数を呼び出し毎で指定して、chmpxへ接続し、本コマンド処理を行う関数群です。詳細な設定を行いつつ、非永続のchmpxを利用する場合に利用できる関数群です。
- k2hdkc_pm_...  
  永続的なchmpxへのハンドルを指定して、本コマンド処理を行う関数群です。

#### パラメータ
- config  
  chmpxスレーブノードのコンフィグレーション（INI形式、YAML形式、JSON形式ファイルへのパス、もしくはJSON文字列）を指定します。
- ctlport  
  chmpxスレーブノードの制御ポート番号を指定します。（同一サーバ上で複数のchmpxが起動している場合には指定する必要があります）
- cuk  
  chmpxスレーブノードのCUK文字列を指定します。（同一サーバ上で複数のchmpxが起動している場合には指定する必要があります）
- is_auto_rejoin  
  chmpxとの接続が切断された場合に自動的に再接続するか指定します。
- no_giveup_rejoin  
  chmpxとの接続が切断された場合に自動的に再接続する指定をしている場合に、その上限をなくすか指定します。
- pprefix  
  バイナリ列の場合にはキューのプレフィックスのバイナリ列へのポインタ、文字列の場合にはキューのプレフィックス文字列へのポインタを指定します。
- prefixlength  
  バイナリ列の場合にキューのプレフィックスのバイナリ列長を指定します。
- pkey  
  バイナリ列の場合にはキューに蓄積するキーのバイナリ列へのポインタ、文字列の場合にはキューに蓄積するキーの文字列へのポインタを指定します。
- keylength  
  バイナリ列の場合にキューに蓄積するキーのバイナリ列長を指定します。
- pval  
  バイナリ列の場合にはキューに蓄積する値のバイナリ列へのポインタ、文字列の場合にはキューに蓄積する値の文字列へのポインタを指定します。
- vallength  
  バイナリ列の場合にキューに蓄積する値のバイナリ列長を指定します。
- is_fifo  
  PUSHするときにFIFO/LIFOを指定します。
- checkattr  
  キューへのPUSHの際に、キューの属性（Attribute）の確認をするか否かを指定します。
- encpass  
  暗号化するためのパスフレーズを指定します。
- expire  
  値に有効期限を設定する場合に有効期限を指定します。
- handle  
  k2hdkcライブラリで用いるchmpx接続のハンドルです。

#### 返り値
成功した場合にはtrue、失敗した場合にはfalseを返します。

#### サンプル
```
bool        result = k2hdkc_full_keyq_push(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin, pName, NameLen, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, reinterpret_cast<const unsigned char*>(strValue.c_str()), strValue.length() + 1, is_Fifo);
dkcres_type_t    rescode = k2hdkc_get_lastres_code();
 
// check result
if(!result){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}
```

<!-- -----------------------------------------------------------　-->
***

### <a name="QUEUEPOP"> Queue(KeyQUeue) Pop関連(C I/F)
k2hdkcクラスタからキューを指定して、キューからPOPする関数群です。

#### 書式
- bool k2hdkc_q_pop(const char* config, const unsigned char* pprefix, size_t prefixlength, bool is_fifo, unsigned char** ppval, size_t* pvallength)
- bool k2hdkc_q_pop_wp(const char* config, const unsigned char* pprefix, size_t prefixlength, bool is_fifo, const char* encpass, unsigned char** ppval, size_t* pvallength)
- bool k2hdkc_q_str_pop(const char* config, const char* pprefix, bool is_fifo, const char** ppval)
- bool k2hdkc_q_str_pop_wp(const char* config, const char* pprefix, bool is_fifo, const char* encpass, const char** ppval)
 
- bool k2hdkc_full_q_pop(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pprefix, size_t prefixlength, bool is_fifo, unsigned char** ppval, size_t* pvallength)
- bool k2hdkc_full_q_pop_wp(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pprefix, size_t prefixlength, bool is_fifo, const char* encpass, unsigned char** ppval, size_t* pvallength)
- bool k2hdkc_full_q_str_pop(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pprefix, bool is_fifo, const char** ppval)
- bool k2hdkc_full_q_str_pop_wp(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pprefix, bool is_fifo, const char* encpass, const char** ppval)
 
- bool k2hdkc_full_keyq_pop(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pprefix, size_t prefixlength, bool is_fifo, unsigned char** ppkey, size_t* pkeylength, unsigned char** ppval, size_t* pvallength)
- bool k2hdkc_full_keyq_pop_wp(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pprefix, size_t prefixlength, bool is_fifo, const char* encpass, unsigned char** ppkey, size_t* pkeylength, unsigned char** ppval, size_t* pval- bool k2hdkc_full_keyq_str_pop(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pprefix, bool is_fifo, const char** ppkey, const char** ppval)
length)
- bool k2hdkc_full_keyq_str_pop_wp(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pprefix, bool is_fifo, const char* encpass, const char** ppkey, const char** ppval)
 
- bool k2hdkc_pm_q_pop(k2hdkc_chmpx_h handle, const unsigned char* pprefix, size_t prefixlength, bool is_fifo, unsigned char** ppval, size_t* pvallength)
- bool k2hdkc_pm_q_pop_wp(k2hdkc_chmpx_h handle, const unsigned char* pprefix, size_t prefixlength, bool is_fifo, const char* encpass, unsigned char** ppval, size_t* pvallength)
- bool k2hdkc_pm_q_str_pop(k2hdkc_chmpx_h handle, const char* pprefix, bool is_fifo, const char** ppval)
- bool k2hdkc_pm_q_str_pop_wp(k2hdkc_chmpx_h handle, const char* pprefix, bool is_fifo, const char* encpass, const char** ppval)
 
- bool k2hdkc_pm_keyq_pop(k2hdkc_chmpx_h handle, const unsigned char* pprefix, size_t prefixlength, bool is_fifo, unsigned char** ppkey, size_t* pkeylength, unsigned char** ppval, size_t* pvallength)
- bool k2hdkc_pm_keyq_pop_wp(k2hdkc_chmpx_h handle, const unsigned char* pprefix, size_t prefixlength, bool is_fifo, const char* encpass, unsigned char** ppkey, size_t* pkeylength, unsigned char** ppval, size_t* pvallength)
- bool k2hdkc_pm_keyq_str_pop(k2hdkc_chmpx_h handle, const char* pprefix, bool is_fifo, const char** ppkey, const char** ppval)
- bool k2hdkc_pm_keyq_str_pop_wp(k2hdkc_chmpx_h handle, const char* pprefix, bool is_fifo, const char* encpass, const char** ppkey, const char** ppval)


#### 説明
関数は、以下の規則に沿って展開されており、基本は以下2種類（4タイプ）となっています。
- k2hdkc_q_pop系  
  バイナリ列としてキューを指定して、値(バイナリ）をキューからPOPする関数です。（QUEUE)
- k2hdkc_keyq_pop系  
  バイナリ列としてキューを指定して、キー(バイナリ）と値(バイナリ）をキューからPOPする関数です。（KEYQUEUE）
- k2hdkc_q_str_pop系  
  文字列としてキューを指定して、値(文字列）をキューからPOPする関数です。（QUEUE)
- k2hdkc_keyq_str_pop系  
  文字列としてキューを指定して、キー(文字列）と値(文字列）をキューからPOPする関数です。（KEYQUEUE）

上記の2種類に対して、以下の規則でそれぞれのタイプに対して引数などの違いのある類似関数があります。
- k2hdkc_...  
  サフィックスに何もない関数であり、これは基本となる引数列を取ります。
- k2hdkc_..._wp  
  サフィックスに"_wp"があるタイプであり、これらの関数はパスフレーズを指定するタイプの関数群です。

さらに、以下のタイプが上記までのタイプに加えて（積算）、存在します。
- k2hdkc_...  
  予め設定されているデフォルトのchmpxに関する引数で、chmpxへ接続し、本コマンド処理を行う関数群です。簡単に非永続のchmpxを利用する場合に利用できる関数群です。
- k2hdkc_full_...  
  chmpxに関する引数を呼び出し毎で指定して、chmpxへ接続し、本コマンド処理を行う関数群です。詳細な設定を行いつつ、非永続のchmpxを利用する場合に利用できる関数群です。
- k2hdkc_pm_...  
  永続的なchmpxへのハンドルを指定して、本コマンド処理を行う関数群です。

#### パラメータ
- config  
  chmpxスレーブノードのコンフィグレーション（INI形式、YAML形式、JSON形式ファイルへのパス、もしくはJSON文字列）を指定します。
- ctlport  
  chmpxスレーブノードの制御ポート番号を指定します。（同一サーバ上で複数のchmpxが起動している場合には指定する必要があります）
- cuk  
  chmpxスレーブノードのCUK文字列を指定します。（同一サーバ上で複数のchmpxが起動している場合には指定する必要があります）
- is_auto_rejoin  
  chmpxとの接続が切断された場合に自動的に再接続するか指定します。
- no_giveup_rejoin  
  chmpxとの接続が切断された場合に自動的に再接続する指定をしている場合に、その上限をなくすか指定します。
- pprefix  
  バイナリ列の場合にはキューのプレフィックスのバイナリ列へのポインタ、文字列の場合にはキューのプレフィックス文字列へのポインタを指定します。
- prefixlength  
  バイナリ列の場合にキューのプレフィックスのバイナリ列長を指定します。
- ppkey  
  バイナリ列の場合にはキューから取り出したキーのバイナリ列へのポインタを保管するポインターを指定します。
- pkeylength  
  バイナリ列の場合にキューから取り出したキー長を保管するポインタを指定します。
- ppval  
  バイナリ列の場合にはキューから取り出した値のバイナリ列へのポインタを保管するポインターを指定します。
- pvallength  
  バイナリ列の場合にキューから取り出した値長を保管するポインタを指定します。
- is_fifo  
  POPするときにFIFO/LIFOを指定します。
- encpass  
  暗号化するためのパスフレーズを指定します。
- handle  
  k2hdkcライブラリで用いるchmpx接続のハンドルです。

#### 返り値
成功した場合にはtrue、失敗した場合にはfalseを返します。

#### サンプル
```
unsigned char*    pkey        = NULL;
size_t        keylength    = 0;
unsigned char*    pval        = NULL;
size_t        vallength    = 0;
bool        result        = k2hdkc_full_keyq_pop(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin, pName, NameLen, is_Fifo, &pkey, &keylength, &pval, &vallength);
dkcres_type_t    rescode        = k2hdkc_get_lastres_code();
 
// check result
if(!result){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}
```

<!-- -----------------------------------------------------------　-->
***

### <a name="QUEUEREMOVE"> Queue(KeyQUeue) Remove関連(C I/F)
k2hdkcクラスタからキューを指定して、キューから指定した数蓄積データを削除する関数群です。

#### 書式
- bool k2hdkc_q_remove(const char* config, const unsigned char* pprefix, size_t prefixlength, int count, bool is_fifo)
- bool k2hdkc_q_str_remove(const char* config, const char* pprefix, int count, bool is_fifo)
 
- bool k2hdkc_full_q_remove(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pprefix, size_t prefixlength, int count, bool is_fifo)
- bool k2hdkc_full_q_str_remove(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pprefix, int count, bool is_fifo)
 
- bool k2hdkc_pm_q_remove(k2hdkc_chmpx_h handle, const unsigned char* pprefix, size_t prefixlength, int count, bool is_fifo)
- bool k2hdkc_pm_q_str_remove(k2hdkc_chmpx_h handle, const char* pprefix, int count, bool is_fifo)
 
- bool k2hdkc_q_remove_wp(const char* config, const unsigned char* pprefix, size_t prefixlength, int count, bool is_fifo, const char* encpass)
- bool k2hdkc_q_str_remove_wp(const char* config, const char* pprefix, int count, bool is_fifo, const char* encpass)
 
- bool k2hdkc_full_q_remove_wp(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pprefix, size_t prefixlength, int count, bool is_fifo, const char* encpass)
- bool k2hdkc_full_q_str_remove_wp(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pprefix, int count, bool is_fifo, const char* encpass)
 
- bool k2hdkc_pm_q_remove_wp(k2hdkc_chmpx_h handle, const unsigned char* pprefix, size_t prefixlength, int count, bool is_fifo, const char* encpass)
- bool k2hdkc_pm_q_str_remove_wp(k2hdkc_chmpx_h handle, const char* pprefix, int count, bool is_fifo, const char* encpass)
 
- bool k2hdkc_keyq_remove(const char* config, const unsigned char* pprefix, size_t prefixlength, int count, bool is_fifo)
- bool k2hdkc_keyq_str_remove(const char* config, const char* pprefix, int count, bool is_fifo)
 
- bool k2hdkc_full_keyq_remove(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pprefix, size_t prefixlength, int count, bool is_fifo)
- bool k2hdkc_full_keyq_str_remove(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pprefix, int count, bool is_fifo)
 
- bool k2hdkc_pm_keyq_remove(k2hdkc_chmpx_h handle, const unsigned char* pprefix, size_t prefixlength, int count, bool is_fifo)
- bool k2hdkc_pm_keyq_str_remove(k2hdkc_chmpx_h handle, const char* pprefix, int count, bool is_fifo)
 
- bool k2hdkc_keyq_remove_wp(const char* config, const unsigned char* pprefix, size_t prefixlength, int count, bool is_fifo, const char* encpass)
- bool k2hdkc_keyq_str_remove_wp(const char* config, const char* pprefix, int count, bool is_fifo, const char* encpass)
 
- bool k2hdkc_full_keyq_remove_wp(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pprefix, size_t prefixlength, int count, bool is_fifo, const char* encpass)
- bool k2hdkc_full_keyq_str_remove_wp(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pprefix, int count, bool is_fifo, const char* encpass)
 
- bool k2hdkc_pm_keyq_remove_wp(k2hdkc_chmpx_h handle, const unsigned char* pprefix, size_t prefixlength, int count, bool is_fifo, const char* encpass)
- bool k2hdkc_pm_keyq_str_remove_wp(k2hdkc_chmpx_h handle, const char* pprefix, int count, bool is_fifo, const char* encpass)

#### 説明
関数は、以下の規則に沿って展開されており、基本は以下2種類（4タイプ）となっています。
- k2hdkc_q_remove系  
  バイナリ列としてキューを指定して、値(バイナリ）を指定数分キューから削除する関数です。（QUEUE)
- k2hdkc_keyq_remove系  
  バイナリ列としてキューを指定して、キー(バイナリ）と値(バイナリ）を指定数分キューから削除する関数です。（KEYQUEUE）
- k2hdkc_q_str_remove系  
  文字列としてキューを指定して、値(文字列）を指定数分キューから削除する関数です。（QUEUE)
- k2hdkc_keyq_str_remove系  
  文字列としてキューを指定して、キー(文字列）と値(文字列）を指定数分キューから削除する関数です。（KEYQUEUE）

上記の2種類に対して、以下の規則でそれぞれのタイプに対して引数などの違いのある類似関数があります。
- k2hdkc_...  
  サフィックスに何もない関数であり、これは基本となる引数列を取ります。
- k2hdkc_..._wp  
  サフィックスに"_wp"があるタイプであり、これらの関数はパスフレーズを指定するタイプの関数群です。

さらに、以下のタイプが上記までのタイプに加えて（積算）、存在します。
- k2hdkc_...  
  予め設定されているデフォルトのchmpxに関する引数で、chmpxへ接続し、本コマンド処理を行う関数群です。簡単に非永続のchmpxを利用する場合に利用できる関数群です。
- k2hdkc_full_...  
  chmpxに関する引数を呼び出し毎で指定して、chmpxへ接続し、本コマンド処理を行う関数群です。詳細な設定を行いつつ、非永続のchmpxを利用する場合に利用できる関数群です。
- k2hdkc_pm_...  
  永続的なchmpxへのハンドルを指定して、本コマンド処理を行う関数群です。

#### パラメータ
- config  
  chmpxスレーブノードのコンフィグレーション（INI形式、YAML形式、JSON形式ファイルへのパス、もしくはJSON文字列）を指定します。
- ctlport  
  chmpxスレーブノードの制御ポート番号を指定します。（同一サーバ上で複数のchmpxが起動している場合には指定する必要があります）
- cuk  
  chmpxスレーブノードのCUK文字列を指定します。（同一サーバ上で複数のchmpxが起動している場合には指定する必要があります）
- is_auto_rejoin  
  chmpxとの接続が切断された場合に自動的に再接続するか指定します。
- no_giveup_rejoin  
  chmpxとの接続が切断された場合に自動的に再接続する指定をしている場合に、その上限をなくすか指定します。
- pprefix  
  バイナリ列の場合にはキューのプレフィックスのバイナリ列へのポインタ、文字列の場合にはキューのプレフィックス文字列へのポインタを指定します。
- prefixlength  
  バイナリ列の場合にキューのプレフィックスのバイナリ列長を指定します。
- is_fifo  
  REMOVEするときにFIFO/LIFOを指定します。
- encpass  
  暗号化するためのパスフレーズを指定します。
- handle  
  k2hdkcライブラリで用いるchmpx接続のハンドルです。

#### 返り値
成功した場合にはtrue、失敗した場合にはfalseを返します。

#### サンプル
```
bool        result = k2hdkc_full_keyq_remove(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin, pName, NameLen, RmCount, is_Fifo);
dkcres_type_t    rescode = k2hdkc_get_lastres_code();
 
// check result
if(!result){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}
```

<!-- -----------------------------------------------------------　-->
***

### <a name="CASINIT"> CAS(Compare And Swap) Init関連(C I/F)
k2hdkcクラスタにキー名を指定して、そのキーをCAS（Compare And Swap）コマンドとして利用できるように初期化する関数群です。

#### 書式
- bool k2hdkc_cas64_init(const char* config, const unsigned char* pkey, size_t keylength, uint64_t val)
- bool k2hdkc_cas64_str_init(const char* config, const char* pkey, uint64_t val)
- bool k2hdkc_full_cas64_init(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint64_t val)
- bool k2hdkc_full_cas64_str_init(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint64_t val)
- bool k2hdkc_pm_cas64_init(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint64_t val)
- bool k2hdkc_pm_cas64_str_init(k2hdkc_chmpx_h handle, const char* pkey, uint64_t val)
- bool k2hdkc_cas64_init_wa(const char* config, const unsigned char* pkey, size_t keylength, uint64_t val, const char* encpass, const time_t* expire)
- bool k2hdkc_cas64_str_init_wa(const char* config, const char* pkey, uint64_t val, const char* encpass, const time_t* expire)
- bool k2hdkc_full_cas64_init_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint64_t val, const char* encpass, const time_t* expire)
- bool k2hdkc_full_cas64_str_init_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint64_t val, const char* encpass, const time_t* expire)
- bool k2hdkc_pm_cas64_init_wa(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint64_t val, const char* encpass, const time_t* expire)
- bool k2hdkc_pm_cas64_str_init_wa(k2hdkc_chmpx_h handle, const char* pkey, uint64_t val, const char* encpass, const time_t* expire)
 
- bool k2hdkc_cas32_init(const char* config, const unsigned char* pkey, size_t keylength, uint32_t val)
- bool k2hdkc_cas32_str_init(const char* config, const char* pkey, uint32_t val)
- bool k2hdkc_full_cas32_init(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint32_t val)
- bool k2hdkc_full_cas32_str_init(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint32_t val)
- bool k2hdkc_pm_cas32_init(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint32_t val)
- bool k2hdkc_pm_cas32_str_init(k2hdkc_chmpx_h handle, const char* pkey, uint32_t val)
- bool k2hdkc_cas32_init_wa(const char* config, const unsigned char* pkey, size_t keylength, uint32_t val, const char* encpass, const time_t* expire)
- bool k2hdkc_cas32_str_init_wa(const char* config, const char* pkey, uint32_t val, const char* encpass, const time_t* expire)
- bool k2hdkc_full_cas32_init_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint32_t val, const char* encpass, const time_t* expire)
- bool k2hdkc_full_cas32_str_init_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint32_t val, const char* encpass, const time_t* expire)
- bool k2hdkc_pm_cas32_init_wa(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint32_t val, const char* encpass, const time_t* expire)
- bool k2hdkc_pm_cas32_str_init_wa(k2hdkc_chmpx_h handle, const char* pkey, uint32_t val, const char* encpass, const time_t* expire)
 
- bool k2hdkc_cas16_init(const char* config, const unsigned char* pkey, size_t keylength, uint16_t val)
- bool k2hdkc_cas16_str_init(const char* config, const char* pkey, uint16_t val)
- bool k2hdkc_full_cas16_init(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint16_t val)
- bool k2hdkc_full_cas16_str_init(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint16_t val)
- bool k2hdkc_pm_cas16_init(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint16_t val)
- bool k2hdkc_pm_cas16_str_init(k2hdkc_chmpx_h handle, const char* pkey, uint16_t val)
- bool k2hdkc_cas16_init_wa(const char* config, const unsigned char* pkey, size_t keylength, uint16_t val, const char* encpass, const time_t* expire)
- bool k2hdkc_cas16_str_init_wa(const char* config, const char* pkey, uint16_t val, const char* encpass, const time_t* expire)
- bool k2hdkc_full_cas16_init_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint16_t val, const char* encpass, const time_t* expire)
- bool k2hdkc_full_cas16_str_init_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint16_t val, const char* encpass, const time_t* expire)
- bool k2hdkc_pm_cas16_init_wa(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint16_t val, const char* encpass, const time_t* expire)
- bool k2hdkc_pm_cas16_str_init_wa(k2hdkc_chmpx_h handle, const char* pkey, uint16_t val, const char* encpass, const time_t* expire)
 
- bool k2hdkc_cas8_init(const char* config, const unsigned char* pkey, size_t keylength, uint8_t val)
- bool k2hdkc_cas8_str_init(const char* config, const char* pkey, uint8_t val)
- bool k2hdkc_full_cas8_init(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint8_t val)
- bool k2hdkc_full_cas8_str_init(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint8_t val)
- bool k2hdkc_pm_cas8_init(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint8_t val)
- bool k2hdkc_pm_cas8_str_init(k2hdkc_chmpx_h handle, const char* pkey, uint8_t val)
- bool k2hdkc_cas8_init_wa(const char* config, const unsigned char* pkey, size_t keylength, uint8_t val, const char* encpass, const time_t* expire)
- bool k2hdkc_cas8_str_init_wa(const char* config, const char* pkey, uint8_t val, const char* encpass, const time_t* expire)
- bool k2hdkc_full_cas8_init_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint8_t val, const char* encpass, const time_t* expire)
- bool k2hdkc_full_cas8_str_init_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint8_t val, const char* encpass, const time_t* expire)
- bool k2hdkc_pm_cas8_init_wa(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint8_t val, const char* encpass, const time_t* expire)
- bool k2hdkc_pm_cas8_str_init_wa(k2hdkc_chmpx_h handle, const char* pkey, uint8_t val, const char* encpass, const time_t* expire)

#### 説明
関数は、以下の規則に沿って展開されており、基本は以下2種類となっています。
- k2hdkc_cas..._init系  
  バイナリ列としてキーを指定して、指定された値（データ長依存）でCAS用値の初期化を行います。
- k2hdkc_cas..._str_init系  
  文字列としてキーを指定して、指定された値（データ長依存）でCAS用値の初期化を行います。

上記の2種類は、それぞれデータ長に応じて、以下の4種類に分化しています。
- k2hdkc_cas8_...  
  データ長が8ビット（1byte）
- k2hdkc_cas16_...  
  データ長が16ビット（2byte）
- k2hdkc_cas32_...  
  データ長が32ビット（4byte）
- k2hdkc_cas64_...  
  データ長が64ビット（8byte）

上記の2種類×4データ長に対して、以下の規則でそれぞれのタイプに対して引数などの違いのある類似関数があります。
- k2hdkc_...  
  サフィックスに何もない関数であり、これは基本となる引数列を取ります。
- k2hdkc_..._wa  
  サフィックスに"_wa"があるタイプであり、これらの関数はパスフレーズ、有効期限を指定するタイプの関数群です。

さらに、以下のタイプが上記までのタイプに加えて（積算）、存在します。
- k2hdkc_...  
  予め設定されているデフォルトのchmpxに関する引数で、chmpxへ接続し、本コマンド処理を行う関数群です。簡単に非永続のchmpxを利用する場合に利用できる関数群です。
- k2hdkc_full_...  
  chmpxに関する引数を呼び出し毎で指定して、chmpxへ接続し、本コマンド処理を行う関数群です。詳細な設定を行いつつ、非永続のchmpxを利用する場合に利用できる関数群です。
- k2hdkc_pm_...  
  永続的なchmpxへのハンドルを指定して、本コマンド処理を行う関数群です。

#### パラメータ
- config  
  chmpxスレーブノードのコンフィグレーション（INI形式、YAML形式、JSON形式ファイルへのパス、もしくはJSON文字列）を指定します。
- ctlport  
  chmpxスレーブノードの制御ポート番号を指定します。（同一サーバ上で複数のchmpxが起動している場合には指定する必要があります）
- cuk  
  chmpxスレーブノードのCUK文字列を指定します。（同一サーバ上で複数のchmpxが起動している場合には指定する必要があります）
- is_auto_rejoin  
  chmpxとの接続が切断された場合に自動的に再接続するか指定します。
- no_giveup_rejoin  
  chmpxとの接続が切断された場合に自動的に再接続する指定をしている場合に、その上限をなくすか指定します。
- pkey  
  バイナリ列の場合にはキーのバイナリ列へのポインタ、文字列の場合にはキーの文字列へのポインタを指定します。
- keylength  
  バイナリ列の場合にキーのバイナリ列長を指定します。
- val  
  初期化に利用する値を指定します。データ長は各々の関数に応じて、8/16/32/64ビット長になります。
- encpass  
  暗号化するためのパスフレーズを指定します。
- expire  
  値に有効期限を設定する場合に有効期限を指定します。
- handle  
  k2hdkcライブラリで用いるchmpx接続のハンドルです。

#### 返り値
成功した場合にはtrue、失敗した場合にはfalseを返します。

#### サンプル
```
bool        result    = k2hdkc_full_cas64_init(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, val);
dkcres_type_t    rescode = k2hdkc_get_lastres_code();
 
// check result
if(!result){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}
```

<!-- -----------------------------------------------------------　-->
***

### <a name="CASGET"> CAS(Compare And Swap) Get関連(C I/F)
k2hdkcクラスタにキー名を指定して、そのキーの値を取得する関数群です。

#### 書式
- bool k2hdkc_cas64_get(const char* config, const unsigned char* pkey, size_t keylength, uint64_t* pval)
- bool k2hdkc_cas64_str_get(const char* config, const char* pkey, uint64_t* pval)
- bool k2hdkc_full_cas64_get(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint64_t* pval)
- bool k2hdkc_full_cas64_str_get(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint64_t* pval)
- bool k2hdkc_pm_cas64_get(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint64_t* pval)
- bool k2hdkc_pm_cas64_str_get(k2hdkc_chmpx_h handle, const char* pkey, uint64_t* pval)
- bool k2hdkc_cas64_get_wa(const char* config, const unsigned char* pkey, size_t keylength, const char* encpass, uint64_t* pval)
- bool k2hdkc_cas64_str_get_wa(const char* config, const char* pkey, const char* encpass, uint64_t* pval)
- bool k2hdkc_full_cas64_get_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, const char* encpass, uint64_t* pval)
- bool k2hdkc_full_cas64_str_get_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, const char* encpass, uint64_t* pval)
- bool k2hdkc_pm_cas64_get_wa(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, const char* encpass, uint64_t* pval)
- bool k2hdkc_pm_cas64_str_get_wa(k2hdkc_chmpx_h handle, const char* pkey, const char* encpass, uint64_t* pval)
 
- bool k2hdkc_cas32_get(const char* config, const unsigned char* pkey, size_t keylength, uint32_t* pval)
- bool k2hdkc_cas32_str_get(const char* config, const char* pkey, uint32_t* pval)
- bool k2hdkc_full_cas32_get(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint32_t* pval)
- bool k2hdkc_full_cas32_str_get(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint32_t* pval)
- bool k2hdkc_pm_cas32_get(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint32_t* pval)
- bool k2hdkc_pm_cas32_str_get(k2hdkc_chmpx_h handle, const char* pkey, uint32_t* pval)
- bool k2hdkc_cas32_get_wa(const char* config, const unsigned char* pkey, size_t keylength, const char* encpass, uint32_t* pval)
- bool k2hdkc_cas32_str_get_wa(const char* config, const char* pkey, const char* encpass, uint32_t* pval)
- bool k2hdkc_full_cas32_get_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, const char* encpass, uint32_t* pval)
- bool k2hdkc_full_cas32_str_get_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, const char* encpass, uint32_t* pval)
- bool k2hdkc_pm_cas32_get_wa(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, const char* encpass, uint32_t* pval)
- bool k2hdkc_pm_cas32_str_get_wa(k2hdkc_chmpx_h handle, const char* pkey, const char* encpass, uint32_t* pval)
 
- bool k2hdkc_cas16_get(const char* config, const unsigned char* pkey, size_t keylength, uint16_t* pval)
- bool k2hdkc_cas16_str_get(const char* config, const char* pkey, uint16_t* pval)
- bool k2hdkc_full_cas16_get(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint16_t* pval)
- bool k2hdkc_full_cas16_str_get(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint16_t* pval)
- bool k2hdkc_pm_cas16_get(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint16_t* pval)
- bool k2hdkc_pm_cas16_str_get(k2hdkc_chmpx_h handle, const char* pkey, uint16_t* pval)
- bool k2hdkc_cas16_get_wa(const char* config, const unsigned char* pkey, size_t keylength, const char* encpass, uint16_t* pval)
- bool k2hdkc_cas16_str_get_wa(const char* config, const char* pkey, const char* encpass, uint16_t* pval)
- bool k2hdkc_full_cas16_get_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, const char* encpass, uint16_t* pval)
- bool k2hdkc_full_cas16_str_get_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, const char* encpass, uint16_t* pval)
- bool k2hdkc_pm_cas16_get_wa(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, const char* encpass, uint16_t* pval)
- bool k2hdkc_pm_cas16_str_get_wa(k2hdkc_chmpx_h handle, const char* pkey, const char* encpass, uint16_t* pval)
 
- bool k2hdkc_cas8_get(const char* config, const unsigned char* pkey, size_t keylength, uint8_t* pval)
- bool k2hdkc_cas8_str_get(const char* config, const char* pkey, uint8_t* pval)
- bool k2hdkc_full_cas8_get(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint8_t* pval)
- bool k2hdkc_full_cas8_str_get(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint8_t* pval)
- bool k2hdkc_pm_cas8_get(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint8_t* pval)
- bool k2hdkc_pm_cas8_str_get(k2hdkc_chmpx_h handle, const char* pkey, uint8_t* pval)
- bool k2hdkc_cas8_get_wa(const char* config, const unsigned char* pkey, size_t keylength, const char* encpass, uint8_t* pval)
- bool k2hdkc_cas8_str_get_wa(const char* config, const char* pkey, const char* encpass, uint8_t* pval)
- bool k2hdkc_full_cas8_get_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, const char* encpass, uint8_t* pval)
- bool k2hdkc_full_cas8_str_get_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, const char* encpass, uint8_t* pval)
- bool k2hdkc_pm_cas8_get_wa(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, const char* encpass, uint8_t* pval)
- bool k2hdkc_pm_cas8_str_get_wa(k2hdkc_chmpx_h handle, const char* pkey, const char* encpass, uint8_t* pval)

#### 説明
関数は、以下の規則に沿って展開されており、基本は以下2種類となっています。
- k2hdkc_cas..._get系  
  バイナリ列としてキーを指定して、値（データ長依存）を取得します。
- k2hdkc_cas..._str_get系  
  文字列としてキーを指定して、値（データ長依存）を取得します。

上記の2種類は、それぞれデータ長に応じて、以下の4種類に分化しています。
- k2hdkc_cas8_...  
  データ長が8ビット（1byte）
- k2hdkc_cas16_...  
  データ長が16ビット（2byte）
- k2hdkc_cas32_...  
  データ長が32ビット（4byte）
- k2hdkc_cas64_...  
  データ長が64ビット（8byte）

上記の2種類×4データ長に対して、以下の規則でそれぞれのタイプに対して引数などの違いのある類似関数があります。
- k2hdkc_...  
  サフィックスに何もない関数であり、これは基本となる引数列を取ります。
- k2hdkc_..._wa  
  サフィックスに"_wa"があるタイプであり、これらの関数はパスフレーズ、有効期限を指定するタイプの関数群です。

さらに、以下のタイプが上記までのタイプに加えて（積算）、存在します。
- k2hdkc_...  
  予め設定されているデフォルトのchmpxに関する引数で、chmpxへ接続し、本コマンド処理を行う関数群です。簡単に非永続のchmpxを利用する場合に利用できる関数群です。
- k2hdkc_full_...  
  chmpxに関する引数を呼び出し毎で指定して、chmpxへ接続し、本コマンド処理を行う関数群です。詳細な設定を行いつつ、非永続のchmpxを利用する場合に利用できる関数群です。
- k2hdkc_pm_...  
  永続的なchmpxへのハンドルを指定して、本コマンド処理を行う関数群です。

#### パラメータ
- config  
  chmpxスレーブノードのコンフィグレーション（INI形式、YAML形式、JSON形式ファイルへのパス、もしくはJSON文字列）を指定します。
- ctlport  
  chmpxスレーブノードの制御ポート番号を指定します。（同一サーバ上で複数のchmpxが起動している場合には指定する必要があります）
- cuk  
  chmpxスレーブノードのCUK文字列を指定します。（同一サーバ上で複数のchmpxが起動している場合には指定する必要があります）
- is_auto_rejoin  
  chmpxとの接続が切断された場合に自動的に再接続するか指定します。
- no_giveup_rejoin  
  chmpxとの接続が切断された場合に自動的に再接続する指定をしている場合に、その上限をなくすか指定します。
- pkey  
  バイナリ列の場合にはキーのバイナリ列へのポインタ、文字列の場合にはキーの文字列へのポインタを指定します。
- keylength  
  バイナリ列の場合にキーのバイナリ列長を指定します。
- pval  
  取得する値を格納するポインタを指定します。データ長は各々の関数に応じて、8/16/32/64ビット長になります。
- encpass  
  暗号化するためのパスフレーズを指定します。
- handle  
  k2hdkcライブラリで用いるchmpx接続のハンドルです。

#### 返り値
成功した場合にはtrue、失敗した場合にはfalseを返します。

#### サンプル
```
uint64_t    val64    = 0;
bool        result    = k2hdkc_full_cas64_get(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, &val64);
dkcres_type_t    rescode = k2hdkc_get_lastres_code();
 
// check result
if(!result){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}
```


<!-- -----------------------------------------------------------　-->
***

### <a name="CASSET"> CAS(Compare And Swap) Set関連(C I/F)
k2hdkcクラスタにキー名と期待する値を指定して、期待する値と同じであれば、新たな値でキーの値を設定する関数群です。

#### 書式
- bool k2hdkc_cas64_set(const char* config, const unsigned char* pkey, size_t keylength, uint64_t oldval, uint64_t newval)
- bool k2hdkc_cas64_str_set(const char* config, const char* pkey, uint64_t oldval, uint64_t newval)
- bool k2hdkc_full_cas64_set(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint64_t oldval, uint64_t newval)
- bool k2hdkc_full_cas64_str_set(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint64_t oldval, uint64_t newval)
- bool k2hdkc_pm_cas64_set(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint64_t oldval, uint64_t newval)
- bool k2hdkc_pm_cas64_str_set(k2hdkc_chmpx_h handle, const char* pkey, uint64_t oldval, uint64_t newval)
- bool k2hdkc_cas64_set_wa(const char* config, const unsigned char* pkey, size_t keylength, uint64_t oldval, uint64_t newval, const char* encpass, const time_t* expire)
- bool k2hdkc_cas64_str_set_wa(const char* config, const char* pkey, uint64_t oldval, uint64_t newval, const char* encpass, const time_t* expire)
- bool k2hdkc_full_cas64_set_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint64_t oldval, uint64_t newval, const char* encpass, const time_t* expire)
- bool k2hdkc_full_cas64_str_set_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint64_t oldval, uint64_t newval, const char* encpass, const time_t* expire)
- bool k2hdkc_pm_cas64_set_wa(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint64_t oldval, uint64_t newval, const char* encpass, const time_t* expire)
- bool k2hdkc_pm_cas64_str_set_wa(k2hdkc_chmpx_h handle, const char* pkey, uint64_t oldval, uint64_t newval, const char* encpass, const time_t* expire)
 
- bool k2hdkc_cas32_set(const char* config, const unsigned char* pkey, size_t keylength, uint32_t oldval, uint32_t newval)
- bool k2hdkc_cas32_str_set(const char* config, const char* pkey, uint32_t oldval, uint32_t newval)
- bool k2hdkc_full_cas32_set(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint32_t oldval, uint32_t newval)
- bool k2hdkc_full_cas32_str_set(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint32_t oldval, uint32_t newval)
- bool k2hdkc_pm_cas32_set(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint32_t oldval, uint32_t newval)
- bool k2hdkc_pm_cas32_str_set(k2hdkc_chmpx_h handle, const char* pkey, uint32_t oldval, uint32_t newval)
- bool k2hdkc_cas32_set_wa(const char* config, const unsigned char* pkey, size_t keylength, uint32_t oldval, uint32_t newval, const char* encpass, const time_t* expire)
- bool k2hdkc_cas32_str_set_wa(const char* config, const char* pkey, uint32_t oldval, uint32_t newval, const char* encpass, const time_t* expire)
- bool k2hdkc_full_cas32_set_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint32_t oldval, uint32_t newval, const char* encpass, const time_t* expire)
- bool k2hdkc_full_cas32_str_set_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint32_t oldval, uint32_t newval, const char* encpass, const time_t* expire)
- bool k2hdkc_pm_cas32_set_wa(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint32_t oldval, uint32_t newval, const char* encpass, const time_t* expire)
- bool k2hdkc_pm_cas32_str_set_wa(k2hdkc_chmpx_h handle, const char* pkey, uint32_t oldval, uint32_t newval, const char* encpass, const time_t* expire)
 
- bool k2hdkc_cas16_set(const char* config, const unsigned char* pkey, size_t keylength, uint16_t oldval, uint16_t newval)
- bool k2hdkc_cas16_str_set(const char* config, const char* pkey, uint16_t oldval, uint16_t newval)
- bool k2hdkc_full_cas16_set(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint16_t oldval, uint16_t newval)
- bool k2hdkc_full_cas16_str_set(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint16_t oldval, uint16_t newval)
- bool k2hdkc_pm_cas16_set(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint16_t oldval, uint16_t newval)
- bool k2hdkc_pm_cas16_str_set(k2hdkc_chmpx_h handle, const char* pkey, uint16_t oldval, uint16_t newval)
- bool k2hdkc_cas16_set_wa(const char* config, const unsigned char* pkey, size_t keylength, uint16_t oldval, uint16_t newval, const char* encpass, const time_t* expire)
- bool k2hdkc_cas16_str_set_wa(const char* config, const char* pkey, uint16_t oldval, uint16_t newval, const char* encpass, const time_t* expire)
- bool k2hdkc_full_cas16_set_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint16_t oldval, uint16_t newval, const char* encpass, const time_t* expire)
- bool k2hdkc_full_cas16_str_set_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint16_t oldval, uint16_t newval, const char* encpass, const time_t* expire)
- bool k2hdkc_pm_cas16_set_wa(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint16_t oldval, uint16_t newval, const char* encpass, const time_t* expire)
- bool k2hdkc_pm_cas16_str_set_wa(k2hdkc_chmpx_h handle, const char* pkey, uint16_t oldval, uint16_t newval, const char* encpass, const time_t* expire)
 
- bool k2hdkc_cas8_set(const char* config, const unsigned char* pkey, size_t keylength, uint8_t oldval, uint8_t newval)
- bool k2hdkc_cas8_str_set(const char* config, const char* pkey, uint8_t oldval, uint8_t newval)
- bool k2hdkc_full_cas8_set(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint8_t oldval, uint8_t newval)
- bool k2hdkc_full_cas8_str_set(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint8_t oldval, uint8_t newval)
- bool k2hdkc_pm_cas8_set(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint8_t oldval, uint8_t newval)
- bool k2hdkc_pm_cas8_str_set(k2hdkc_chmpx_h handle, const char* pkey, uint8_t oldval, uint8_t newval)
- bool k2hdkc_cas8_set_wa(const char* config, const unsigned char* pkey, size_t keylength, uint8_t oldval, uint8_t newval, const char* encpass, const time_t* expire)
- bool k2hdkc_cas8_str_set_wa(const char* config, const char* pkey, uint8_t oldval, uint8_t newval, const char* encpass, const time_t* expire)
- bool k2hdkc_full_cas8_set_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, uint8_t oldval, uint8_t newval, const char* encpass, const time_t* expire)
- bool k2hdkc_full_cas8_str_set_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, uint8_t oldval, uint8_t newval, const char* encpass, const time_t* expire)
- bool k2hdkc_pm_cas8_set_wa(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, uint8_t oldval, uint8_t newval, const char* encpass, const time_t* expire)
- bool k2hdkc_pm_cas8_str_set_wa(k2hdkc_chmpx_h handle, const char* pkey, uint8_t oldval, uint8_t newval, const char* encpass, const time_t* expire)

#### 説明
関数は、以下の規則に沿って展開されており、基本は以下2種類となっています。
- k2hdkc_cas..._set系  
  バイナリ列としてキーを指定し、期待する値（現状の値）と新たに設定する値を指定し、現状の値と同じであれば、新たな値でキーの値を設定します。
- k2hdkc_cas..._str_set系  
  文字列としてキーを指定し、期待する値（現状の値）と新たに設定する値を指定し、現状の値と同じであれば、新たな値でキーの値を設定します。

上記の2種類は、それぞれデータ長に応じて、以下の4種類に分化しています。
- k2hdkc_cas8_...  
  データ長が8ビット（1byte）
- k2hdkc_cas16_...  
  データ長が16ビット（2byte）
- k2hdkc_cas32_...  
  データ長が32ビット（4byte）
- k2hdkc_cas64_...  
  データ長が64ビット（8byte）

上記の2種類×4データ長に対して、以下の規則でそれぞれのタイプに対して引数などの違いのある類似関数があります。
- k2hdkc_...  
  サフィックスに何もない関数であり、これは基本となる引数列を取ります。
- k2hdkc_..._wa  
  サフィックスに"_wa"があるタイプであり、これらの関数はパスフレーズ、有効期限を指定するタイプの関数群です。

さらに、以下のタイプが上記までのタイプに加えて（積算）、存在します。
- k2hdkc_...  
  予め設定されているデフォルトのchmpxに関する引数で、chmpxへ接続し、本コマンド処理を行う関数群です。簡単に非永続のchmpxを利用する場合に利用できる関数群です。
- k2hdkc_full_...  
  chmpxに関する引数を呼び出し毎で指定して、chmpxへ接続し、本コマンド処理を行う関数群です。詳細な設定を行いつつ、非永続のchmpxを利用する場合に利用できる関数群です。
- k2hdkc_pm_...  
  永続的なchmpxへのハンドルを指定して、本コマンド処理を行う関数群です。

#### パラメータ
- config  
  chmpxスレーブノードのコンフィグレーション（INI形式、YAML形式、JSON形式ファイルへのパス、もしくはJSON文字列）を指定します。
- ctlport  
  chmpxスレーブノードの制御ポート番号を指定します。（同一サーバ上で複数のchmpxが起動している場合には指定する必要があります）
- cuk  
  chmpxスレーブノードのCUK文字列を指定します。（同一サーバ上で複数のchmpxが起動している場合には指定する必要があります）
- is_auto_rejoin  
  chmpxとの接続が切断された場合に自動的に再接続するか指定します。
- no_giveup_rejoin  
  chmpxとの接続が切断された場合に自動的に再接続する指定をしている場合に、その上限をなくすか指定します。
- pkey  
  バイナリ列の場合にはキーのバイナリ列へのポインタ、文字列の場合にはキーの文字列へのポインタを指定します。
- keylength  
  バイナリ列の場合にキーのバイナリ列長を指定します。
- oldval  
  期待する値（現状の値）を指定します。データ長は各々の関数に応じて、8/16/32/64ビット長になります。
- newval  
  新たに設定する値を指定します。データ長は各々の関数に応じて、8/16/32/64ビット長になります。
- encpass  
  暗号化するためのパスフレーズを指定します。
- expire  
  キーの有効期限を指定します。
- handle  
  k2hdkcライブラリで用いるchmpx接続のハンドルです。

#### 返り値
成功した場合にはtrue、失敗した場合にはfalseを返します。

#### サンプル
```
bool        result    = k2hdkc_full_cas64_set(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, oldval, newval);
dkcres_type_t    rescode = k2hdkc_get_lastres_code();
 
// check result
if(!result){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}
```

<!-- -----------------------------------------------------------　-->
***

### <a name="CASINCDEC"> CAS(Compare And Swap) Increment/Decrement関連(C I/F)
k2hdkcクラスタにキー名を指定して、その値をインクリメント/デクリメントする関数群です。

#### 書式
- bool k2hdkc_cas_increment(const char* config, const unsigned char* pkey, size_t keylength)
- bool k2hdkc_cas_str_increment(const char* config, const char* pkey)
- bool k2hdkc_full_cas_increment(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength)
- bool k2hdkc_full_cas_str_increment(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey)
- bool k2hdkc_pm_cas_increment(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength)
- bool k2hdkc_pm_cas_str_increment(k2hdkc_chmpx_h handle, const char* pkey)
- bool k2hdkc_cas_increment_wa(const char* config, const unsigned char* pkey, size_t keylength, const char* encpass, const time_t* expire)
- bool k2hdkc_cas_str_increment_wa(const char* config, const char* pkey, const char* encpass, const time_t* expire)
- bool k2hdkc_full_cas_increment_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, const char* encpass, const time_t* expire)
- bool k2hdkc_full_cas_str_increment_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, const char* encpass, const time_t* expire)
- bool k2hdkc_pm_cas_increment_wa(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, const char* encpass, const time_t* expire)
- bool k2hdkc_pm_cas_str_increment_wa(k2hdkc_chmpx_h handle, const char* pkey, const char* encpass, const time_t* expire)
 
- bool k2hdkc_cas_decrement(const char* config, const unsigned char* pkey, size_t keylength)
- bool k2hdkc_cas_str_decrement(const char* config, const char* pkey)
- bool k2hdkc_full_cas_decrement(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength)
- bool k2hdkc_full_cas_str_decrement(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey)
- bool k2hdkc_pm_cas_decrement(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength)
- bool k2hdkc_pm_cas_str_decrement(k2hdkc_chmpx_h handle, const char* pkey)
- bool k2hdkc_cas_decrement_wa(const char* config, const unsigned char* pkey, size_t keylength, const char* encpass, const time_t* expire)
- bool k2hdkc_cas_str_decrement_wa(const char* config, const char* pkey, const char* encpass, const time_t* expire)
- bool k2hdkc_full_cas_decrement_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const unsigned char* pkey, size_t keylength, const char* encpass, const time_t* expire)
- bool k2hdkc_full_cas_str_decrement_wa(const char* config, short ctlport, const char* cuk, bool is_auto_rejoin, bool no_giveup_rejoin, const char* pkey, const char* encpass, const time_t* expire)
- bool k2hdkc_pm_cas_decrement_wa(k2hdkc_chmpx_h handle, const unsigned char* pkey, size_t keylength, const char* encpass, const time_t* expire)
- bool k2hdkc_pm_cas_str_decrement_wa(k2hdkc_chmpx_h handle, const char* pkey, const char* encpass, const time_t* expire)

#### 説明
関数は、以下の規則に沿って展開されており、基本は以下2種類（4タイプ）となっています。
- k2hdkc_cas_increment系  
  バイナリ列としてキーを指定し、その値をインクリメントします。インクリメント処理中に他スレッドなどによる処理は排他制御され、確実にインクリメントされることを保証します。値のデータ長の指定はする必要はなく、k2hdkcにて自動判別されます。
- k2hdkc_cas_decrement系  
  バイナリ列としてキーを指定し、その値をデクリメントします。デクリメント処理中に他スレッドなどによる処理は排他制御され、確実にデクリメントされることを保証します。値のデータ長の指定はする必要はなく、k2hdkcにて自動判別されます。
- k2hdkc_cas_str_increment系  
  文字列としてキーを指定し、その値をインクリメントします。インクリメント処理中に他スレッドなどによる処理は排他制御され、確実にインクリメントされることを保証します。値のデータ長の指定はする必要はなく、k2hdkcにて自動判別されます。
- k2hdkc_cas_str_decrement系  
  文字列としてキーを指定し、その値をデクリメントします。デクリメント処理中に他スレッドなどによる処理は排他制御され、確実にデクリメントされることを保証します。値のデータ長の指定はする必要はなく、k2hdkcにて自動判別されます。

上記の2種類（4タイプ）に対して、以下の規則でそれぞれのタイプに対して引数などの違いのある類似関数があります。
- k2hdkc_...  
  サフィックスに何もない関数であり、これは基本となる引数列を取ります。
- k2hdkc_..._wa  
  サフィックスに"_wa"があるタイプであり、これらの関数はパスフレーズ、有効期限を指定するタイプの関数群です。

さらに、以下のタイプが上記までのタイプに加えて（積算）、存在します。
- k2hdkc_...  
  予め設定されているデフォルトのchmpxに関する引数で、chmpxへ接続し、本コマンド処理を行う関数群です。簡単に非永続のchmpxを利用する場合に利用できる関数群です。
- k2hdkc_full_...  
  chmpxに関する引数を呼び出し毎で指定して、chmpxへ接続し、本コマンド処理を行う関数群です。詳細な設定を行いつつ、非永続のchmpxを利用する場合に利用できる関数群です。
- k2hdkc_pm_...  
  永続的なchmpxへのハンドルを指定して、本コマンド処理を行う関数群です。

#### パラメータ
- config  
  chmpxスレーブノードのコンフィグレーション（INI形式、YAML形式、JSON形式ファイルへのパス、もしくはJSON文字列）を指定します。
- ctlport  
  chmpxスレーブノードの制御ポート番号を指定します。（同一サーバ上で複数のchmpxが起動している場合には指定する必要があります）
- cuk  
  chmpxスレーブノードのCUK文字列を指定します。（同一サーバ上で複数のchmpxが起動している場合には指定する必要があります）
- is_auto_rejoin  
  chmpxとの接続が切断された場合に自動的に再接続するか指定します。
- no_giveup_rejoin  
  chmpxとの接続が切断された場合に自動的に再接続する指定をしている場合に、その上限をなくすか指定します。
- pkey  
  バイナリ列の場合にはキーのバイナリ列へのポインタ、文字列の場合にはキーの文字列へのポインタを指定します。
- keylength  
  バイナリ列の場合にキーのバイナリ列長を指定します。
- encpass  
  暗号化するためのパスフレーズを指定します。
- expire  
  キーの有効期限を指定します。
- handle  
  k2hdkcライブラリで用いるchmpx接続のハンドルです。

#### 返り値
成功した場合にはtrue、失敗した場合にはfalseを返します。

#### サンプル
```
bool        result = k2hdkc_full_cas_increment_wa(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), (-1 == Expire ? NULL : &Expire));
dkcres_type_t    rescode = k2hdkc_get_lastres_code();
 
// check result
if(!result){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
    printf("RESULT CODE(%s - %s)\n", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
}
```

<!-- -----------------------------------------------------------　-->
***

## <a name="CPP"> C++言語インタフェース
C++言語用のインタフェースです。
開発時には以下のヘッダファイルをインクルードしてください。
```
#include <k2hdkc/k2hdkcslave.h>
```

リンク時には以下をオプションとして指定してください。
```
-lk2hdkc
```

以下にC++言語用の利用方法、クラス、メソッドの説明をします。
<!-- -----------------------------------------------------------　-->
***

### <a name="DEBUGCPP"> デバッグ関連(C++ I/F)
k2hdkcライブラリは、内部動作およびAPIの動作の確認をするためにメッセージ出力を行うことができます。
本関数群は、メッセージ出力を制御するためのC++言語用の関数群です。

#### 書式
- K2hdkcDbgMode SetK2hdkcDbgMode(K2hdkcDbgMode mode)
- K2hdkcDbgMode BumpupK2hdkcDbgMode(void)
- K2hdkcDbgMode GetK2hdkcDbgMode(void)
- bool LoadK2hdkcDbgEnv(void)
- bool SetK2hdkcDbgFile(const char* filepath)
- bool UnsetK2hdkcDbgFile(void)
- const char* GetK2hdkcDbgFile(void)
- void SetK2hdkcComLog(bool enable)
- void EnableK2hdkcComLog(void)
- void DsableK2hdkcComLog(void)
- bool IsK2hdkcComLog(void)

#### 説明
- SetK2hdkcDbgMode  
  デバッグメッセージの出力レベルを指定して変更します。
- BumpupK2hdkcDbgMode  
  デバッグメッセージの出力レベルを1段階Bumpupさせる。
- GetK2hdkcDbgMode  
  現在のデバッグメッセージの出力レベルを取得します。
- LoadK2hdkcDbgEnv  
  環境変数DKCDBGMODE、DKCDBGFILEを読み込み設定します。
- SetK2hdkcDbgFile  
  デバッグメッセージの出力先をstderrから指定したファイルに切り替えます。
- UnsetK2hdkcDbgFile  
  デバッグメッセージの出力先をstderrに戻します。
- GetK2hdkcDbgFile  
  デバッグメッセージの出力先を指定している場合には、そのファイルパスを取得します。
- SetK2hdkcComLog  
  通信コマンドの送受信ログ出力の有効/無効を設定します。
- k2hdkc_load_debug_env  
  環境変数DKCDBGMODE、DKCDBGFILEを読み込み設定します。
- IsK2hdkcComLog  
  通信コマンドの送受信ログ出力が有効か調べます。
- EnableK2hdkcComLog  
  通信コマンドの送受信ログ出力を有効とします。
- DsableK2hdkcComLog  
  通信コマンドの送受信ログ出力を無効とします。

#### パラメータ
- mode  
  デバッグメッセージの出力レベル（DKCDBG_SILENT、DKCDBG_ERR、DKCDBG_WARN、DKCDBG_MSG、DKCDBG_DUMP）を指定します。
- filepath  
  デバッグメッセージの出力先のファイルパスを指定します。
- enable  
  通信コマンドの送受信ログ出力の有効/無効を設定します。

#### 返り値
- SetK2hdkcDbgMode / BumpupK2hdkcDbgMode  
  旧デバッグメッセージの出力レベル（DKCDBG_SILENT、DKCDBG_ERR、DKCDBG_WARN、DKCDBG_MSG、DKCDBG_DUMP）を返します。
- GetK2hdkcDbgMode  
  現在のデバッグメッセージの出力レベル（DKCDBG_SILENT、DKCDBG_ERR、DKCDBG_WARN、DKCDBG_MSG、DKCDBG_DUMP）を返します。
- LoadK2hdkcDbgEnv / SetK2hdkcDbgFile / UnsetK2hdkcDbgFile / IsK2hdkcComLog  
  成功した場合にはtrue、失敗した場合にはfalseを返します。
- GetK2hdkcDbgFile  
  デバッグメッセージの出力先を指定している場合には、そのファイルパスを返します。

#### 注意
環境変数DKCDBGMODE、DKCDBGFILEについては、[環境変数](environmentsja.html)を参照してください。

<!-- -----------------------------------------------------------　-->
***

### <a name="COMFACTORY"> コマンドクラスファクトリ関連(C++ I/F)
クラスファクトリは、通信コマンドクラスオブジェクトを作成します。
クラスファクトリには、非永続chmpx接続用と永続chmpx接続用の2種類があります。

#### 説明
2つのクラスファクトリは、非永続chmpx接続用と永続chmpx接続用となっています。（これらのクラスファクトリは、実装はマクロであり、実装処理は隠蔽しています。）
クラスファクトリタイプの種類と、返される通信コマンドクラスについて以下にまとめます。

#### 非永続chmpx接続用クラスファクトリ
これらの関数（マクロ）は、GetOt...のプレフィックスを持っています。

- GetOtSlaveK2hdkcComGet  
  キーに対する値を取得するための K2hdkcComGet クラスオブジェクトを取得します。
- GetOtSlaveK2hdkcComGetDirect  
  キーに対する値をオフセットを指定して取得するための K2hdkcComGetDirect クラスオブジェクトを取得します。
- GetOtSlaveK2hdkcComGetSubkeys  
  キーに対するサブキーのリストを取得するための K2hdkcComGetSubkeys クラスオブジェクトを取得します。
- GetOtSlaveK2hdkcComGetAttrs  
  キーに対する属性データを取得するための K2hdkcComGetAttrs クラスオブジェクトを取得します。
- GetOtSlaveK2hdkcComGetAttr  
  キーに対する属性名を指定してその値を取得するための K2hdkcComGetAttr クラスオブジェクトを取得します。
- GetOtSlaveK2hdkcComSet  
  キーに対する値を設定するための K2hdkcComSet クラスオブジェクトを取得します。
- GetOtSlaveK2hdkcComSetDirect  
  キーに対する値をオフセットを指定して設定するための K2hdkcComSetDirect クラスオブジェクトを取得します。
- GetOtSlaveK2hdkcComSetSubkeys  
  キーに対するサブキーのリストを設定するための K2hdkcComSetSubkeys クラスオブジェクトを取得します。
- GetOtSlaveK2hdkcComSetAll  
  キーに対する値、サブキーのリストを設定するための K2hdkcComSetAll クラスオブジェクトを取得します。
- GetOtSlaveK2hdkcComAddSubkeys  
  キーに対するサブキーリストにサブキーを追加するための K2hdkcComAddSubkeys クラスオブジェクトを取得します。
- GetOtSlaveK2hdkcComAddSubkey  
  サブキーとサブキーの値を指定してサブキーを作成し、キーに対するサブキーリストにそのサブキーを追加するための K2hdkcComAddSubkey クラスオブジェクトを取得します。
- GetOtSlaveK2hdkcComDel  
  キーを削除するための K2hdkcComDel クラスオブジェクトを取得します。
- GetOtSlaveK2hdkcComDelSubkeys  
  キーに対するサブキーリストからサブキーを削除するための K2hdkcComDelSubkeys クラスオブジェクトを取得します。
- GetOtSlaveK2hdkcComDelSubkey  
  キーに対するサブキーリストからサブキーを削除し、サブキー自体も削除するための K2hdkcComDelSubkey クラスオブジェクトを取得します。
- GetOtSlaveK2hdkcComRen  
  キーをリネームするための K2hdkcComRen クラスオブジェクトを取得します。
- GetOtSlaveK2hdkcComQPush  
  キュー（およびキーキュー）に値（もしくはキーと値）をPUSHするための K2hdkcComQPush クラスオブジェクトを取得します。
- GetOtSlaveK2hdkcComQPop  
  キュー（およびキーキュー）から値（もしくはキーと値）をPOPするための K2hdkcComQPop クラスオブジェクトを取得します。
- GetOtSlaveK2hdkcComQDel  
  キュー（およびキーキュー）から指定数のデータを削除するための K2hdkcComQDel クラスオブジェクトを取得します。
- GetOtSlaveK2hdkcComCasInit  
  CAS（Compare And Swap）で利用するキーの値を初期化するための K2hdkcComCasInit クラスオブジェクトを取得します。
- GetOtSlaveK2hdkcComCasGet  
  CAS（Compare And Swap）で利用する値を取得するための K2hdkcComCasGet クラスオブジェクトを取得します。
- GetOtSlaveK2hdkcComCasSet  
  CAS（Compare And Swap）で利用する値を設定するための K2hdkcComCasSet クラスオブジェクトを取得します。
- GetOtSlaveK2hdkcComCasIncDec  
  CAS（Compare And Swap）の利用する値をインクリメント/デクリメントするための K2hdkcComCasIncDec クラスオブジェクトを取得します。
- GetOtSlaveK2hdkcComState  
  k2hdkcクラスタの全ノードのステータス（k2hashデータ）を取得するための K2hdkcComState クラスオブジェクトを取得します。

#### 永続chmpx接続用クラスファクトリ
これらの関数（マクロ）は、GetPm...のプレフィックスを持っています。

- GetPmSlaveK2hdkcComGet
  キーに対する値を取得するための K2hdkcComGet クラスオブジェクトを取得します。
- GetPmSlaveK2hdkcComGetDirect
  キーに対する値をオフセットを指定して取得するための K2hdkcComGetDirect クラスオブジェクトを取得します。
- GetPmSlaveK2hdkcComGetSubkeys
  キーに対するサブキーのリストを取得するための K2hdkcComGetSubkeys クラスオブジェクトを取得します。
- GetPmSlaveK2hdkcComGetAttrs
  キーに対する属性データを取得するための K2hdkcComGetAttrs クラスオブジェクトを取得します。
- GetPmSlaveK2hdkcComGetAttr
  キーに対する属性名を指定してその値を取得するための K2hdkcComGetAttr クラスオブジェクトを取得します。
- GetPmSlaveK2hdkcComSet
  キーに対する値を設定するための K2hdkcComSet クラスオブジェクトを取得します。
- GetPmSlaveK2hdkcComSetDirect
  キーに対する値をオフセットを指定して設定するための K2hdkcComSetDirect クラスオブジェクトを取得します。
- GetPmSlaveK2hdkcComSetSubkeys
  キーに対するサブキーのリストを設定するための K2hdkcComSetSubkeys クラスオブジェクトを取得します。
- GetPmSlaveK2hdkcComSetAll
  キーに対する値、サブキーのリストを設定するための K2hdkcComSetAll クラスオブジェクトを取得します。
- GetPmSlaveK2hdkcComAddSubkeys
  キーに対するサブキーリストにサブキーを追加するための K2hdkcComAddSubkeys クラスオブジェクトを取得します。
- GetPmSlaveK2hdkcComAddSubkey
  サブキーとサブキーの値を指定してサブキーを作成し、キーに対するサブキーリストにそのサブキーを追加するための K2hdkcComAddSubkey クラスオブジェクトを取得します。
- GetPmSlaveK2hdkcComDel
  キーを削除するための K2hdkcComDel クラスオブジェクトを取得します。
- GetPmSlaveK2hdkcComDelSubkeys
  キーに対するサブキーリストからサブキーを削除するための K2hdkcComDelSubkeys クラスオブジェクトを取得します。
- GetPmSlaveK2hdkcComDelSubkey
  キーに対するサブキーリストからサブキーを削除し、サブキー自体も削除するための K2hdkcComDelSubkey クラスオブジェクトを取得します。
- GetPmSlaveK2hdkcComRen
  キーをリネームするための K2hdkcComRen クラスオブジェクトを取得します。
- GetPmSlaveK2hdkcComQPush
  キュー（およびキーキュー）に値（もしくはキーと値）をPUSHするための K2hdkcComQPush クラスオブジェクトを取得します。
- GetPmSlaveK2hdkcComQPop
  キュー（およびキーキュー）から値（もしくはキーと値）をPOPするための K2hdkcComQPop クラスオブジェクトを取得します。
- GetPmSlaveK2hdkcComQDel
  キュー（およびキーキュー）から指定数のデータを削除するための K2hdkcComQDel クラスオブジェクトを取得します。
- GetPmSlaveK2hdkcComCasInit
  CAS（Compare And Swap）で利用するキーの値を初期化するための K2hdkcComCasInit クラスオブジェクトを取得します。
- GetPmSlaveK2hdkcComCasGet
  CAS（Compare And Swap）で利用する値を取得するための K2hdkcComCasGet クラスオブジェクトを取得します。
- GetPmSlaveK2hdkcComCasSet
  CAS（Compare And Swap）で利用する値を設定するための K2hdkcComCasSet クラスオブジェクトを取得します。
- GetPmSlaveK2hdkcComCasIncDec
  CAS（Compare And Swap）の利用する値をインクリメント/デクリメントするための K2hdkcComCasIncDec クラスオブジェクトを取得します。
- GetPmSlaveK2hdkcComState
  k2hdkcクラスタの全ノードのステータス（k2hashデータ）を取得するための K2hdkcComState クラスオブジェクトを取得します。

#### パラメータ
- config  
  chmpxスレーブノードのコンフィグレーション（INI形式、YAML形式、JSON形式ファイルへのパス、もしくはJSON文字列）を指定します。  
  NULLまたは空文字列が指定されている場合には、環境変数（K2HDKCCONFFILEもしくはK2HDKCJSONCONF）の値が使用されます。
- ctlport  
  chmpxスレーブノードの制御ポート番号を指定します。（同一サーバ上で複数のchmpxが起動している場合には指定する必要があります）
- cuk  
  chmpxスレーブノードのCUK文字列を指定します。（同一サーバ上で複数のchmpxが起動している場合には指定する必要があります）
- is_auto_rejoin  
  chmpxとの接続が切断された場合に自動的に再接続するか指定します。
- no_giveup_rejoin  
  chmpxとの接続が切断された場合に自動的に再接続する指定をしている場合に、その上限をなくすか指定します。
- comnum  
  通信コマンドシーケンシャル番号を指定します。通常指定しません。（内部利用していますので指定しないでください）
- pslaveobj  
  永続chmpx接続しているケースにおける K2hdkcSlaveクラスオブジェクトのポインタを指定します。

#### 返り値
対応する通信コマンドクラスオブジェクトへのポインタを返します。  
返されたクラスオブジェクトは不要になった時点で、破棄（delete）してください。

#### サンプル
```
//-------------------------------------
// One time connection type
//-------------------------------------
K2hdkcComGet*    pComObj = GetOtSlaveK2hdkcComGet(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin);
bool        result = pComObj->CommandSend(pkey, keylength, !is_noattr, passphrase, &pconstval, &vallength, &rescode);
delete pComObj;
 
//-------------------------------------
// Permanent connection type
//-------------------------------------
// Open chmpx
K2hdkcSlave* pSlave = new K2hdkcSlave();
if(!pSlave->Initialize(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin)){
    cerr << "Could not join slave node chmpx." << endl;
    delete pSlave;
    return false;
}
if(!pSlave->Open(isNoGiveupRejoin)){
    cerr << "Could not open msgid on slave node chmpx." << endl;
    delete pSlave;
    return false;
}
for(int cnt = 0; cnt < 10; ++cnt){    // loop for example
    K2hdkcComGet*    pComObj= GetPmSlaveK2hdkcComGet(pSlave);
    bool        result = pComObj->CommandSend(pkey, keylength, !is_noattr, passphrase, &pconstval, &vallength, &rescode);
    delete pComObj;
}
if(!pSlave->Clean(isCleanupBup)){
    cerr << "Could not leave slave node chmpx, but continue for cleanup..." << endl;
}
delete pSlave;
```

<!-- -----------------------------------------------------------　-->
***

### <a name="GETCPP"> K2hdkcComGetクラス
キーを指定して、値を取得するクラスです。

#### 書式
- bool K2hdkcComGet::CommandSend(const unsigned char* pkey, size_t keylength, bool checkattr = true, const char* encpass = NULL)
- bool K2hdkcComGet::CommandSend(const unsigned char* pkey, size_t keylength, bool checkattr, const char* encpass, const unsigned char** ppval, size_t* pvallength, dkcres_type_t* prescode)
- bool K2hdkcComGet::GetResponseData(const unsigned char** ppdata, size_t* plength, dkcres_type_t* prescode) const

#### パラメータ
- pkey  
  取得するキー（バイナリ列）を指定します。
- keylength  
  キー長を指定します。
- checkattr  
  取得するキーの属性（Attribute）をチェックするか否かを指定します。
- encpass  
  取得するキーが暗号化されている場合に複合するパスフレーズを指定します。
- ppval  
  取得した値（バイナリ列）が格納されるポインタを指定します。
- pvallength  
  取得した値（バイナリ列）長が格納されるポインタを指定します。
- prescode  
  処理結果コードを格納するポインタを指定します。

#### 返り値
成功すればtrueを返し、失敗した場合にはfalseを返します。

#### 注意
CommandSendには、コマンド送信と結果取得を1度に行うメソッドタイプがあります。

#### サンプル
```
K2hdkcComGet* pComObj = GetOtSlaveK2hdkcComGet(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin);
bool result = pComObj->CommandSend(pkey, keylength, !is_noattr, passphrase, &pconstval, &vallength, &rescode);
```
<!-- -----------------------------------------------------------　-->
***

### <a name="GETDIRECTCPP"> K2hdkcComGetDirectクラス
キーと値のオフセット、値長を指定して、値を取得するクラスです。

#### 書式
- bool K2hdkcComGetDirect::CommandSend(const unsigned char* pkey, size_t keylength, off_t val_pos, size_t val_length)
- bool K2hdkcComGetDirect::CommandSend(const unsigned char* pkey, size_t keylength, off_t val_pos, size_t val_length, const unsigned char** ppval, size_t* pvallength, dkcres_type_t* prescode)
- bool K2hdkcComGetDirect::GetResponseData(const unsigned char** ppdata, size_t* plength, dkcres_type_t* prescode) const

#### パラメータ
- pkey  
  取得するキー（バイナリ列）を指定します。
- keylength  
  キー長を指定します。
- val_pos  
  取得する値のオフセットを指定します。
- val_length  
  取得する値長を指定します。
- ppval  
  取得した値（バイナリ列）が格納されるポインタを指定します。
- pvallength  
  取得した値（バイナリ列）長が格納されるポインタを指定します。
- prescode  
  処理結果コードを格納するポインタを指定します。

#### 返り値
成功すればtrueを返し、失敗した場合にはfalseを返します。

#### 注意
CommandSendには、コマンド送信と結果取得を1度に行うメソッドタイプがあります。

#### サンプル
```
K2hdkcComGetDirect*    pComObj = GetOtSlaveK2hdkcComGetDirect(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin);
bool pComObj->CommandSend(pkey, keylen, offset, length, &pconstval, &vallen, &rescode);
```

<!-- -----------------------------------------------------------　-->
***

### <a name="GETSUBSCPP"> K2hdkcComGetSubkeysクラス
キーを指定して、キーの持つサブキーリストを取得するクラスです。

#### 書式
- bool K2hdkcComGetSubkeys::CommandSend(const unsigned char* pkey, size_t keylength, bool checkattr = true)
- bool K2hdkcComGetSubkeys::CommandSend(const unsigned char* pkey, size_t keylength, bool checkattr, K2HSubKeys** ppSubKeys, dkcres_type_t* prescode)
- bool K2hdkcComGetSubkeys::GetResponseData(K2HSubKeys** ppSubKeys, dkcres_type_t* prescode) const

#### パラメータ
- pkey  
  取得するキー（バイナリ列）を指定します。
- keylength  
  キー長を指定します。
- checkattr  
  取得するキーの属性（Attribute）をチェックするか否かを指定します。
- ppSubKeys  
  取得したサブキーリストのクラスを格納するポインタを指定します。
- prescode  
  処理結果コードを格納するポインタを指定します。

#### 返り値
成功すればtrueを返し、失敗した場合にはfalseを返します。サブキーを持たない場合にもfalseを返します。

#### 注意
CommandSendには、コマンド送信と結果取得を1度に行うメソッドタイプがあります。  
サブキーリストのクラスK2HSubKeysの操作方法については、[K2HASH](https://k2hash.antpick.ax/indexja.html)のドキュメントを参照してください。

#### サンプル
```
K2HSubKeys*           pSubKeys= NULL;
K2hdkcComGetSubkeys*  pComObj = GetOtSlaveK2hdkcComGetSubkeys(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin);
bool                  result  = pComObj->CommandSend(pkey, keylength, !is_noattr, &pSubKeys, &rescode);
```

<!-- -----------------------------------------------------------　-->
***

### <a name="GETATTRSCPP"> K2hdkcComGetAttrsクラス
キーを指定して、キーの持つ属性情報を取得するクラスです。

#### 書式
- bool K2hdkcComGetAttrs::CommandSend(const unsigned char* pkey, size_t keylength)
- bool K2hdkcComGetAttrs::CommandSend(const unsigned char* pkey, size_t keylength, K2HAttrs** ppAttrsObj, dkcres_type_t* prescode)
- bool K2hdkcComGetAttrs::GetResponseData(K2HAttrs** ppAttrsObj, dkcres_type_t* prescode) const

#### パラメータ
- pkey  
  取得するキー（バイナリ列）を指定します。
- keylength  
  キー長を指定します。
- ppAttrsObj  
  取得した属性情報のクラスを格納するポインタを指定します。
- prescode  
  処理結果コードを格納するポインタを指定します。

#### 返り値
成功すればtrueを返し、失敗した場合にはfalseを返します。属性情報を持たない場合にもfalseを返します。  
k2hdkcクラスタは常に属性情報のひとつであるMTIME情報を持つための属性情報を持たない返信はありません。

#### 注意
CommandSendには、コマンド送信と結果取得を1度に行うメソッドタイプがあります。  
属性情報のクラスK2HAttrsの操作方法については、[K2HASH](https://k2hash.antpick.ax/indexja.html)のドキュメントを参照してください。

#### サンプル
```
K2HAttrs*            pAttrsObj    = NULL;
K2hdkcComGetAttrs*    pComObj     = GetOtSlaveK2hdkcComGetAttrs(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin);
bool            result = pComObj->CommandSend(reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, &pAttrsObj, &rescode);
```

<!-- -----------------------------------------------------------　-->
***

### <a name="GETATTRCPP"> K2hdkcComGetAttrクラス
キーと属性情報名（バイナリ列）を指定して、キーの持つ指定した属性情報を取得するクラスです。

#### 書式
- bool K2hdkcComGetAttr::CommandSend(const unsigned char* pkey, size_t keylength, const unsigned char* pattr, size_t attrlength)
- bool K2hdkcComGetAttr::CommandSend(const unsigned char* pkey, size_t keylength, const unsigned char* pattr, size_t attrlength, const unsigned char** ppattrval, size_t* pattrvallength, dkcres_type_t* prescode)
- bool K2hdkcComGetAttr::GetResponseData(const unsigned char** ppattrval, size_t* pattrvallength, dkcres_type_t* prescode) const

#### パラメータ
- pkey  
  取得するキー（バイナリ列）を指定します。
- keylength  
  キー長を指定します。
- pattr  
  取得する属性名（バイナリ列）を指定します。
- attrlength  
  属性名長を指定します。
- ppattrval  
  取得した属性情報の値（バイナリ列）を格納するポインタを指定します。
- pattrvallength  
  取得した属性情報の値（バイナリ列）長を格納するポインタを指定します。
- prescode  
  処理結果コードを格納するポインタを指定します。

#### 返り値
成功すればtrueを返し、失敗した場合にはfalseを返します。指定した属性情報を持たない場合にもfalseを返します。

#### 注意
CommandSendには、コマンド送信と結果取得を1度に行うメソッドタイプがあります。

#### サンプル
```
K2hdkcComGetAttrs*    pComObj     = GetOtSlaveK2hdkcComGetAttr(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin);
bool            result = pComObj->CommandSend(reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, pattrname, attrnamelen, &pAttrval, attrvallen, &rescode);
```

<!-- -----------------------------------------------------------　-->
***

### <a name="SETAPP"> K2hdkcComSetクラス
キーと値を指定して、キーに値を設定（新規作成含む）を行うクラスです。

#### 書式
- bool K2hdkcComSet::CommandSend(const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, bool rm_subkeylist = false, const char* encpass = NULL, const time_t* expire = NULL)
- bool K2hdkcComSet::CommandSend(const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, bool rm_subkeylist, const char* encpass, const time_t* expire, dkcres_type_t* prescode)
- bool K2hdkcComSet::GetResponseData(dkcres_type_t* prescode) const;

#### パラメータ
- pkey  
  作成・設定するキー（バイナリ列）を指定します。
- keylength  
  キー長を指定します。
- pval  
  設定する値（バイナリ列）を指定します。
- vallength  
  値長を指定します。
- rm_subkeylist  
  指定したキーがサブキーリストを持っている場合、そのサブキーリストを削除（サブキー自体の削除はしない）するか否かを指定します。
- encpass  
  指定するキーを暗号化する場合のパスフレーズを指定します。
- expire  
  指定するキーの有効期限を設定する場合に有効期限を指定します。
- prescode  
  処理結果コードを格納するポインタを指定します。

#### 返り値
成功すればtrueを返し、失敗した場合にはfalseを返します。

#### 注意
CommandSendには、コマンド送信と結果取得を1度に行うメソッドタイプがあります。  
本クラスには、他に2つのCommandSendメソッドがありますが、これは内部で利用しているメソッドであり、利用することはできません。

#### サンプル
```
K2hdkcComSet*    pComObj = GetOtSlaveK2hdkcComSet(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin);
bool            result = pComObj->CommandSend(pKey, KeyLen, pVal, ValLen, is_rmsublist, pPassPhrase, pExpire, &rescode);
```

<!-- -----------------------------------------------------------　-->
***

### <a name="SETDIRECTCPP"> K2hdkcComSetDirectクラス
キーと値、オフセットを指定して、キーに値を設定（新規作成含む）を行うクラスです。

#### 書式
- bool K2hdkcComSetDirect::CommandSend(const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, const off_t valpos)
- bool K2hdkcComSetDirect::CommandSend(const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, const off_t valpos, dkcres_type_t* prescode)
- bool K2hdkcComSetDirect::GetResponseData(dkcres_type_t* prescode) const

#### パラメータ
- pkey  
  作成・設定するキー（バイナリ列）を指定します。
- keylength  
  キー長を指定します。
- pval  
  設定する値（バイナリ列）を指定します。
- vallength  
  値長を指定します。
- valpos  
  値を書き込むオフセットを指定します。
- prescode  
  処理結果コードを格納するポインタを指定します。

#### 返り値
成功すればtrueを返し、失敗した場合にはfalseを返します。

#### 注意
CommandSendには、コマンド送信と結果取得を1度に行うメソッドタイプがあります。

#### サンプル
```
K2hdkcComSetDirect*    pComObj = GetOtSlaveK2hdkcComSetDirect(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin);
bool            result = pComObj->CommandSend(pkey, keylen, pval, vallen, offset, &rescode);
```

<!-- -----------------------------------------------------------　-->
***

### <a name="SETSUBSCPP"> K2hdkcComSetSubkeysクラス
キーに指定したサブキーリストを追加するクラスです。サブキーリストの全削除を行うこともできます。

#### 書式
- bool K2hdkcComSetSubkeys::CommandSend(const unsigned char* pkey, size_t keylength, const unsigned char* psubkeys, size_t subkeyslength)
- bool K2hdkcComSetSubkeys::CommandSend(const unsigned char* pkey, size_t keylength, const unsigned char* psubkeys, size_t subkeyslength, dkcres_type_t* prescode)
- bool K2hdkcComSetSubkeys::ClearSubkeysCommandSend(const unsigned char* pkey, size_t keylength)
- bool K2hdkcComSetSubkeys::ClearSubkeysCommandSend(const unsigned char* pkey, size_t keylength, dkcres_type_t* prescode)
- bool K2hdkcComSetSubkeys::GetResponseData(dkcres_type_t* prescode) const;

#### パラメータ
- pkey  
  作成・設定するキー（バイナリ列）を指定します。
- keylength  
  キー長を指定します。
- psubkeys  
  設定するサブキーリスト（バイナリ列）を指定します。サブキーリストのバイナリ列は、K2HSubKeysクラスのSerialize()メソッドにより作成されるバイナリ列です。
- subkeyslength  
  サブキーリスト長を指定します。
- prescode  
  処理結果コードを格納するポインタを指定します。

#### 返り値
成功すればtrueを返し、失敗した場合にはfalseを返します。

#### 注意
CommandSendには、コマンド送信と結果取得を1度に行うメソッドタイプがあります。
ClearSubkeysCommandSend系は、サブキーリストをクリアするメソッドです。

#### サンプル
```
unsigned char*  psubkeys = NULL;
size_t        subkeyslength = 0;
K2HSubKeys    Subkeys;
Subkeys.insert(psubkey, subkeylength);        // Add one subkey
if(!Subkeys.Serialize(&psubkeys, &subkeyslength)){
    return false;
}
 
K2hdkcComSetSubkeys*    pComObj = GetOtSlaveK2hdkcComSetSubkeys(conffile, ctlport, NULL/* cuk is NULL */, is_auto_rejoin, no_giveup_rejoin);
bool            result = pComObj->CommandSend(pPKey, PKeyLen, psubkeys, subkeyslength , &rescode);
 
DKC_FREE(psubkeys);
```

<!-- -----------------------------------------------------------　-->
***

### <a name="SETALLCPP"> K2hdkcComSetAllクラス
キーに指定した値、サブキーリスト、属性情報の全てを書き込むクラスです。

#### 書式
- bool K2hdkcComSetAll::CommandSend(const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, const unsigned char* psubkeys, size_t subkeyslength, const unsigned char* pattrs, size_t attrslength)
- bool K2hdkcComSetAll::CommandSend(const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, const unsigned char* psubkeys, size_t subkeyslength, const unsigned char* pattrs, size_t attrslength, dkcres_type_t* prescode)
- bool K2hdkcComSetAll::GetResponseData(dkcres_type_t* prescode) const

#### パラメータ
- pkey  
  作成・設定するキー（バイナリ列）を指定します。
- keylength  
  キー長を指定します。
- pval  
  設定する値（バイナリ列）を指定します。
- vallength  
  値長を指定します。
- psubkeys  
  設定するサブキーリスト（バイナリ列）を指定します。サブキーリストのバイナリ列は、K2HSubKeysクラスのSerialize()メソッドにより作成されるバイナリ列です。
- subkeyslength  
  サブキーリスト長を指定します。
- pattrs  
  設定する属性情報（バイナリ列）を指定します。属性情報のバイナリ列は、K2HAttrsクラスのSerialize()メソッドにより作成されるバイナリ列です。
- attrslength  
  属性情報（バイナリ列）長を指定します。
- prescode  
  処理結果コードを格納するポインタを指定します。

#### 返り値
成功すればtrueを返し、失敗した場合にはfalseを返します。

#### 注意
CommandSendには、コマンド送信と結果取得を1度に行うメソッドタイプがあります。

#### サンプル
```
unsigned char*  psubkeys = NULL;
size_t        subkeyslength = 0;
K2HSubKeys    Subkeys;
Subkeys.insert(psubkey, subkeylength);        // Add one subkey
if(!Subkeys.Serialize(&psubkeys, &subkeyslength)){
    return false;
}
 
K2hdkcComSetAll*    pComObj = GetOtSlaveK2hdkcComSetAll(conffile, ctlport, NULL/* cuk is NULL */, is_auto_rejoin, no_giveup_rejoin);
bool            result = pComObj->CommandSend(pPKey, PKeyLen, psubkeys, subkeyslength, NULL, 0, &rescode);
 
DKC_FREE(psubkeys);
```

<!-- -----------------------------------------------------------　-->
***

### <a name="ADDSUBSCPP"> K2hdkcComAddSubkeysクラス
キーの持つサブキーリストに指定したサブキーを追加するクラスです。

#### 書式
- bool K2hdkcComAddSubkeys::CommandSend(const unsigned char* pkey, size_t keylength, const unsigned char* psubkey, size_t subkeylength, bool checkattr = true)
- bool K2hdkcComAddSubkeys::CommandSend(const unsigned char* pkey, size_t keylength, const unsigned char* psubkey, size_t subkeylength, bool checkattr, dkcres_type_t* prescode)
- bool K2hdkcComAddSubkeys::GetResponseData(dkcres_type_t* prescode) const

#### パラメータ
- pkey  
  サブキーリストを持つキー（バイナリ列）を指定します。
- keylength  
  キー長を指定します。
- psubkey  
  設定するサブキー（バイナリ列）を指定します。
- subkeylength  
  サブキー長を指定します。
- checkattr  
  サブキーリストを持つキーの属性（Attribute）をチェックするか否かを指定します。
- prescode  
  処理結果コードを格納するポインタを指定します。

#### 返り値
成功すればtrueを返し、失敗した場合にはfalseを返します。

#### 注意
CommandSendには、コマンド送信と結果取得を1度に行うメソッドタイプがあります。

#### サンプル
```
K2hdkcComAddSubkeys*    pComObj = GetOtSlaveK2hdkcComAddSubkeys(conffile, ctlport, NULL/* cuk is NULL */, is_auto_rejoin, no_giveup_rejoin);
bool            result = pComObj->CommandSend(pPKey, PKeyLen, psubkey, subkeylen, true, &rescode);
```

<!-- -----------------------------------------------------------　-->
***

### <a name="ADDSUBCPP"> K2hdkcComAddSubkeyクラス
サブキーを作成し、指定したキーのサブキーとして登録するクラスです。

#### 書式
- bool K2hdkcComAddSubkey::CommandSend(const unsigned char* pkey, size_t keylength, const unsigned char* psubkey, size_t subkeylength, const unsigned char* pskeyval, size_t skeyvallength, bool checkattr = true, const char* encpass = NULL, const time_t* expire = NULL)
- bool K2hdkcComAddSubkey::CommandSend(const unsigned char* pkey, size_t keylength, const unsigned char* psubkey, size_t subkeylength, const unsigned char* pskeyval, size_t skeyvallength, bool checkattr, const char* encpass, const time_t* expire, dkcres_type_t* prescode)
- bool K2hdkcComAddSubkey::GetResponseData(dkcres_type_t* prescode) const

#### パラメータ
- pkey  
  サブキーリストを持つキー（バイナリ列）を指定します。
- keylength  
  キー長を指定します。
- psubkey  
  作成・更新するサブキー（バイナリ列）を指定します。
- subkeylength  
  サブキー長を指定します。
- pskeyval  
  作成・更新するサブキーに設定する値（バイナリ列）を指定します。
- skeyvallength  
  サブキーに設定する値（バイナリ列）長を指定します。
- checkattr  
  サブキーを追加するキーの属性（Attribute）をチェックするか否かを指定します。
- encpass  
  作成・更新するサブキーを暗号化する場合のパスフレーズを指定します。
- expire  
  作成・更新するサブキーに有効期限を設定する場合の有効期限を指定します。
- prescode  
  処理結果コードを格納するポインタを指定します。

#### 返り値
成功すればtrueを返し、失敗した場合にはfalseを返します。

#### 注意
CommandSendには、コマンド送信と結果取得を1度に行うメソッドタイプがあります。

#### サンプル
```
K2hdkcComAddSubkey*    pComObj = GetOtSlaveK2hdkcComAddSubkey(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin);
bool            result = pComObj->CommandSend(pParentKey, ParentKeyLen, pSubKey, SubKeyLen, pVal, ValLen, !is_noattr, pPassPhrase, pExpire, &rescode);
```

<!-- -----------------------------------------------------------　-->
***

### <a name="DELCPP"> K2hdkcComDelクラス
キーを削除するクラスです。

#### 書式
- bool K2hdkcComDel::CommandSend(const unsigned char* pkey, size_t keylength, bool is_subkeys, bool checkattr = true)
- bool K2hdkcComDel::CommandSend(const unsigned char* pkey, size_t keylength, bool is_subkeys, bool checkattr, dkcres_type_t* prescode)
- bool K2hdkcComDel::GetResponseData(dkcres_type_t* prescode) const

#### パラメータ
- pkey  
  作成・設定するキー（バイナリ列）を指定します。
- keylength  
  キー長を指定します。
- checkattr  
  削除するキーの属性（Attribute）をチェックするか否かを指定します。
- prescode  
  処理結果コードを格納するポインタを指定します。

#### 返り値
成功すればtrueを返し、失敗した場合にはfalseを返します。

#### 注意
CommandSendには、コマンド送信と結果取得を1度に行うメソッドタイプがあります。

#### サンプル
```
K2hdkcComDel*    pComObj = GetOtSlaveK2hdkcComDel(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin);
bool            result = pComObj->CommandSend(pkey, keylen, is_subkeys, true, &rescode);
```

<!-- -----------------------------------------------------------　-->
***

### <a name="DELSUBSCPP"> K2hdkcComDelSubkeysクラス
キーの持つサブキーリストから指定したサブキーを削除するクラスです。サブキー自体は削除されません。

#### 書式
- bool K2hdkcComDelSubkeys::CommandSend(const unsigned char* pkey, size_t keylength, const unsigned char* psubkey, size_t subkeylength, bool checkattr = true)
- bool K2hdkcComDelSubkeys::CommandSend(const unsigned char* pkey, size_t keylength, const unsigned char* psubkey, size_t subkeylength, bool checkattr, dkcres_type_t* prescode)
- bool K2hdkcComDelSubkeys::GetResponseData(dkcres_type_t* prescode) const

#### パラメータ
- pkey  
  サブキーを削除するキー（バイナリ列）を指定します。
- keylength  
  キー長を指定します。
- psubkey  
  削除するサブキー（バイナリ列）を指定します。
- subkeylength  
  サブキー長を指定します。
- checkattr  
  削除するサブキーリストを持つキーの属性（Attribute）をチェックするか否かを指定します。
- prescode  
  処理結果コードを格納するポインタを指定します。

#### 返り値
成功すればtrueを返し、失敗した場合にはfalseを返します。

#### 注意
CommandSendには、コマンド送信と結果取得を1度に行うメソッドタイプがあります。

#### サンプル
```
K2hdkcComDelSubkeys*    pComObj = GetOtSlaveK2hdkcComDelSubkeys(conffile, ctlport, NULL/* cuk is NULL */, is_auto_rejoin, no_giveup_rejoin);
bool            result = pComObj->CommandSend(pPKey, PKeyLen, psubkey, subkeylen, true, &rescode);
```

<!-- -----------------------------------------------------------　-->
***

### <a name="DELSUBCPP"> K2hdkcComDelSubkeyクラス
指定したキーのサブキーを削除するクラスです。サブキー自体も削除します。

#### 書式
- bool K2hdkcComDelSubkey::CommandSend(const unsigned char* pkey, size_t keylength, const unsigned char* psubkey, size_t subkeylength, bool is_subkeys, bool checkattr = true)
- bool K2hdkcComDelSubkey::CommandSend(const unsigned char* pkey, size_t keylength, const unsigned char* psubkey, size_t subkeylength, bool is_subkeys, bool checkattr, dkcres_type_t* prescode)
- bool K2hdkcComDelSubkey::GetResponseData(dkcres_type_t* prescode) const

#### パラメータ
- pkey  
  サブキーリストを持つキー（バイナリ列）を指定します。
- keylength  
  キー長を指定します。
- psubkey  
  削除するサブキー（バイナリ列）を指定します。
- subkeylength  
  サブキー長を指定します。
- checkattr  
  サブキーを削除するキーの属性（Attribute）をチェックするか否かを指定します。
- prescode  
  処理結果コードを格納するポインタを指定します。

#### 返り値
成功すればtrueを返し、失敗した場合にはfalseを返します。

#### 注意
CommandSendには、コマンド送信と結果取得を1度に行うメソッドタイプがあります。

#### サンプル
```
K2hdkcComDelSubkey*    pComObj = GetOtSlaveK2hdkcComDelSubkey(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin);
bool            result = pComObj->CommandSend(pParentKey, ParentKeyLen, pSubKey, SubKeyLen, pVal, ValLen, true, &rescode);
```

<!-- -----------------------------------------------------------　-->
***

### <a name="RENCPP"> K2hdkcComRenクラス
指定したキー名をリネームするクラスです。親キーを指定して親キーのサブキーリスト中のキー名の変更も実行することができます。

#### 書式
- bool K2hdkcComRen::CommandSend(const unsigned char* poldkey, size_t oldkeylength, const unsigned char* pnewkey, size_t newkeylength, const unsigned char* pparentkey = NULL, size_t parentkeylength = 0, bool checkattr = true, const char* encpass = NULL, const time_t* expire = NULL)
- bool K2hdkcComRen::CommandSend(const unsigned char* poldkey, size_t oldkeylength, const unsigned char* pnewkey, size_t newkeylength, bool checkattr = true, const char* encpass = NULL, const time_t* expire = NULL)
- bool K2hdkcComRen::CommandSend(const unsigned char* poldkey, size_t oldkeylength, const unsigned char* pnewkey, size_t newkeylength, const unsigned char* pparentkey, size_t parentkeylength, bool checkattr, const char* encpass, const time_t* expire, dkcres_type_t* prescode)
- bool K2hdkcComRen::CommandSend(const unsigned char* poldkey, size_t oldkeylength, const unsigned char* pnewkey, size_t newkeylength, bool checkattr, const char* encpass, const time_t* expire, dkcres_type_t* prescode)
- bool K2hdkcComRen::GetResponseData(dkcres_type_t* prescode) const;

#### パラメータ
- poldkey  
  変更前のキー（バイナリ列）を指定します。
- oldkeylength  
  変更前のキー（バイナリ列）長を指定します。
- pnewkey  
  変更後のキー（バイナリ列）を指定します。
- newkeylength  
  変更後のキー（バイナリ列）長を指定します。
- pparentkey  
  親キーを指定する場合に変更するキーをサブキーに持つ親キー（バイナリ列）を指定します。
- parentkeylength  
  親キー（バイナリ列）長を指定します。
- checkattr  
  親キーを指定した場合に、親キーの属性（Attribute）をチェックするか否かを指定します。
- encpass  
  変更後のキーを暗号化する場合のパスフレーズを指定します。
- expire  
  変更後のキーに有効期限を設定する場合の有効期限を指定します。
- prescode  
  処理結果コードを格納するポインタを指定します。

#### 返り値
成功すればtrueを返し、失敗した場合にはfalseを返します。

#### 注意
CommandSendには、コマンド送信と結果取得を1度に行うメソッドタイプがあります。

#### サンプル
```
K2hdkcComRen*    pComObj = GetOtSlaveK2hdkcComRen(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin);
bool            result = pComObj->CommandSend(pOldKey, OldKeyLen, pNewKey, NewkeyLen, pParentKey, pParentKeyLen, !is_noattr, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), (-1 == Expire ? NULL : &Expire), &rescode);
```

<!-- -----------------------------------------------------------　-->
***

### <a name="QUEUEPUSHCPP"> K2hdkcComQPushクラス
キュー（値のみ、キーと値）にデータをPUSHするクラスです。

#### 書式
- bool K2hdkcComQPush::QueueCommandSend(const unsigned char* pprefix, size_t prefixlength, const unsigned char* pval, size_t vallength, bool is_fifo, const unsigned char* pattrs = NULL, size_t attrslength = 0L, bool checkattr = true, const char* encpass = NULL, const time_t* expire = NULL)
- bool K2hdkcComQPush::QueueCommandSend(const unsigned char* pprefix, size_t prefixlength, const unsigned char* pval, size_t vallength, bool is_fifo, const unsigned char* pattrs, size_t attrslength, bool checkattr, const char* encpass, const time_t* expire, dkcres_type_t* prescode)
- bool K2hdkcComQPush::KeyQueueCommandSend(const unsigned char* pprefix, size_t prefixlength, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, bool is_fifo, const unsigned char* pattrs = NULL, size_t attrslength = 0L, bool checkattr = true, const char* encpass = NULL, const time_t* expire = NULL)
- bool K2hdkcComQPush::KeyQueueCommandSend(const unsigned char* pprefix, size_t prefixlength, const unsigned char* pkey, size_t keylength, const unsigned char* pval, size_t vallength, bool is_fifo, const unsigned char* pattrs, size_t attrslength, bool checkattr, const char* encpass, const time_t* expire, dkcres_type_t* prescode)
- bool K2hdkcComQPush::GetResponseData(dkcres_type_t* prescode) const;

#### パラメータ
- pprefix  
  キュー名のプレフィックス（バイナリ列）を指定します。
- prefixlength  
  キュー名のプレフィックス（バイナリ列）長を指定します。
- pkey  
  キーと値を設定するタイプのキューの場合に、PUSHするキー（バイナリ列）を指定します。
- keylength  
  PUSHするキー（バイナリ列）長を指定します。
- pval  
  PUSHする値（バイナリ列）を指定します。
- vallength  
  PUSHする値（バイナリ列）長を指定します。
- is_fifo  
  キューのタイプFIFO、LIFOを指定します。
- pattrs  
  キューにPUSHするデータに対して属性を設定する場合にバイナリ列で属性情報を指定します。k2hAttrsクラスのSerializeで取得したバイナリ列を指定します。（通常、この設定値は使用しません）
- attrslength  
  属性情報バイナリ列長を指定します。
- checkattr  
  キューの属性（Attribute）をチェックするか否かを指定します。
- encpass  
  キューにPUSHするデータを暗号化する場合のパスフレーズを指定します。
- expire  
  キューにPUSHするデータに有効期限を設定する場合の有効期限を指定します。
- prescode  
  処理結果コードを格納するポインタを指定します。

#### 返り値
成功すればtrueを返し、失敗した場合にはfalseを返します。

#### 注意
CommandSendには、コマンド送信と結果取得を1度に行うメソッドタイプがあります。

#### サンプル
```
K2hdkcComQPush*    pComObj = GetOtSlaveK2hdkcComQPush(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin);
bool            result = pComObj->QueueCommandSend(pName, NameLen, reinterpret_cast<const unsigned char*>(strValue.c_str()), strValue.length() + 1, is_Fifo, NULL, 0UL, !is_noattr, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), (-1 == Expire ? NULL : &Expire), &rescode);
```

<!-- -----------------------------------------------------------　-->
***

### <a name="QUEUEPOPCPP"> K2hdkcComQPopクラス
キューからデータ（値のみ、キーと値）をPOPするクラスです。

#### 書式
- bool K2hdkcComQPop::QueueCommandSend(const unsigned char* pprefix, size_t prefixlength, bool is_fifo, bool checkattr = true, const char* encpass = NULL)
- bool K2hdkcComQPop::QueueCommandSend(const unsigned char* pprefix, size_t prefixlength, bool is_fifo, bool checkattr, const char* encpass, const unsigned char** ppval, size_t* pvallength, dkcres_type_t* prescode)
- bool K2hdkcComQPop::KeyQueueCommandSend(const unsigned char* pprefix, size_t prefixlength, bool is_fifo, bool checkattr = true, const char* encpass = NULL)
- bool K2hdkcComQPop::KeyQueueCommandSend(const unsigned char* pprefix, size_t prefixlength, bool is_fifo, bool checkattr, const char* encpass, const unsigned char** ppkey, size_t* pkeylength, const unsigned char** ppval, size_t* pvallength, dkcres_type_t* prescode)
- bool K2hdkcComQPop::GetQueueResponseData(const unsigned char** ppval, size_t* pvallength, dkcres_type_t* prescode) const
- bool K2hdkcComQPop::GetKeyQueueResponseData(const unsigned char** ppkey, size_t* pkeylength, const unsigned char** ppval, size_t* pvallength, dkcres_type_t* prescode) const

#### パラメータ
- pprefix  
  キュー名のプレフィックス（バイナリ列）を指定します。
- prefixlength  
  キュー名のプレフィックス（バイナリ列）長を指定します。
- is_fifo  
  キューのタイプFIFO、LIFOを指定します。
- checkattr  
  キューの属性（Attribute）をチェックするか否かを指定します。
- encpass  
  キューからPOPするデータが暗号化されており、その値を複合化する場合のパスフレーズを指定します。
- ppkey  
  キューからPOPしたキー（バイナリ列）を保管するポインタを指定します。
- pkeylength  
  キューからPOPしたキー（バイナリ列）長を保管するポインタを指定します。
- ppval  
  キューからPOPした値（バイナリ列）を保管するポインタを指定します。
- pvallength  
  キューからPOPした値（バイナリ列）長を保管するポインタを指定します。
- prescode  
  処理結果コードを格納するポインタを指定します。

#### 返り値
成功すればtrueを返し、失敗した場合にはfalseを返します。

#### 注意
CommandSendには、コマンド送信と結果取得を1度に行うメソッドタイプがあります。

#### サンプル
```
K2hdkcComQPop*    pComObj = GetOtSlaveK2hdkcComQPop(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin);
bool            result = pComObj->KeyQueueCommandSend(pName, NameLen, is_Fifo, !is_noattr, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), &pkeytmp, &keytmplen, &pvaltmp, &valtmplen, &rescode);
```

<!-- -----------------------------------------------------------　-->
***

### <a name="QUEUEDELCPP"> K2hdkcComQDelクラス
キューからデータ（値のみ、キーと値）を指定個数削除Pするクラスです。

#### 書式
- bool K2hdkcComQDel::QueueCommandSend(const unsigned char* pprefix, size_t prefixlength, int count, bool is_fifo, bool checkattr, const char* encpass)
- bool K2hdkcComQDel::QueueCommandSend(const unsigned char* pprefix, size_t prefixlength, int count, bool is_fifo, bool checkattr, const char* encpass, dkcres_type_t* prescode)
- bool K2hdkcComQDel::KeyQueueCommandSend(const unsigned char* pprefix, size_t prefixlength, int count, bool is_fifo, bool checkattr = true, const char* encpass = NULL)
- bool K2hdkcComQDel::KeyQueueCommandSend(const unsigned char* pprefix, size_t prefixlength, int count, bool is_fifo, bool checkattr, const char* encpass, dkcres_type_t* prescode)
- bool K2hdkcComQDel::GetResponseData(dkcres_type_t* prescode) const

#### パラメータ
- pprefix  
  キュー名のプレフィックス（バイナリ列）を指定します。
- prefixlength  
  キュー名のプレフィックス（バイナリ列）長を指定します。
- is_fifo  
  キューのタイプFIFO、LIFOを指定します。
- count  
  削除する個数を指定します。
- checkattr  
  キューの属性（Attribute）をチェックするか否かを指定します。
- encpass  
  キューからPOPするデータが暗号化されており、その値を複合化する場合のパスフレーズを指定します。
- prescode  
  処理結果コードを格納するポインタを指定します。

#### 返り値
成功すればtrueを返し、失敗した場合にはfalseを返します。

#### 注意
CommandSendには、コマンド送信と結果取得を1度に行うメソッドタイプがあります。

#### サンプル
```
K2hdkcComQDel*    pComObj = GetOtSlaveK2hdkcComQDel(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin);
bool            result = pComObj->QueueCommandSend(pName, NameLen, RmCount, is_Fifo, true, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), &rescode);
```

<!-- -----------------------------------------------------------　-->
***

### <a name="CASINITCPP"> K2hdkcComCasInitクラス
CAS（Compare And Swap）としてキーの値を初期化するクラスです。値のサイズ（8/16/32/64ビット）別にメソッドが存在します。

#### 書式
- bool K2hdkcComCasInit::CommandSend(const unsigned char* pkey, size_t keylength, uint8_t val, const char* encpass = NULL, const time_t* expire = NULL)
- bool K2hdkcComCasInit::CommandSend(const unsigned char* pkey, size_t keylength, uint16_t val, const char* encpass = NULL, const time_t* expire = NULL)
- bool K2hdkcComCasInit::CommandSend(const unsigned char* pkey, size_t keylength, uint32_t val, const char* encpass = NULL, const time_t* expire = NULL)
- bool K2hdkcComCasInit::CommandSend(const unsigned char* pkey, size_t keylength, uint64_t val, const char* encpass = NULL, const time_t* expire = NULL)
- bool K2hdkcComCasInit::CommandSend(const unsigned char* pkey, size_t keylength, uint8_t val, const char* encpass, const time_t* expire, dkcres_type_t* prescode)
- bool K2hdkcComCasInit::CommandSend(const unsigned char* pkey, size_t keylength, uint16_t val, const char* encpass, const time_t* expire, dkcres_type_t* prescode)
- bool K2hdkcComCasInit::CommandSend(const unsigned char* pkey, size_t keylength, uint32_t val, const char* encpass, const time_t* expire, dkcres_type_t* prescode)
- bool K2hdkcComCasInit::CommandSend(const unsigned char* pkey, size_t keylength, uint64_t val, const char* encpass, const time_t* expire, dkcres_type_t* prescode)
- bool K2hdkcComCasInit::GetResponseData(dkcres_type_t* prescode) const

#### パラメータ
- pkey  
  CASとして利用するキー（バイナリ列）を指定します。
- keylength  
  キー（バイナリ列）長を指定します。
- val  
  初期化する値（8/16/32/64ビット）を指定します。
- encpass  
  キーを暗号化する場合、そのパスフレーズを指定します。
- expire  
  キーに有効期限を設定する場合の有効期限を指定します。
- prescode  
  処理結果コードを格納するポインタを指定します。

#### 返り値
成功すればtrueを返し、失敗した場合にはfalseを返します。

#### 注意
CommandSendには、コマンド送信と結果取得を1度に行うメソッドタイプがあります。

#### サンプル
```
K2hdkcComCasInit*    pComObj = GetOtSlaveK2hdkcComCasInit(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin);
bool                 result  = pComObj->CommandSend(reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, val, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), (-1 == Expire ? NULL : &Expire), &rescode);
```

<!-- -----------------------------------------------------------　-->
***

### <a name="CASGETCPP"> K2hdkcComCasGetクラス
CAS（Compare And Swap）としてキーの値を取得するクラスです。値のサイズ（8/16/32/64ビット）別にメソッドが存在します。

#### 書式
- bool K2hdkcComCasGet::CommandSend(const unsigned char* pkey, size_t keylength, bool checkattr, const char* encpass, const uint8_t** ppval, dkcres_type_t* prescode)
- bool K2hdkcComCasGet::CommandSend(const unsigned char* pkey, size_t keylength, bool checkattr, const char* encpass, const uint16_t** ppval, dkcres_type_t* prescode)
- bool K2hdkcComCasGet::CommandSend(const unsigned char* pkey, size_t keylength, bool checkattr, const char* encpass, const uint32_t** ppval, dkcres_type_t* prescode)
- bool K2hdkcComCasGet::CommandSend(const unsigned char* pkey, size_t keylength, bool checkattr, const char* encpass, const uint64_t** ppval, dkcres_type_t* prescode)
- bool K2hdkcComCasGet::GetResponseData(const uint8_t** ppval, dkcres_type_t* prescode) const
- bool K2hdkcComCasGet::GetResponseData(const uint16_t** ppval, dkcres_type_t* prescode) const
- bool K2hdkcComCasGet::GetResponseData(const uint32_t** ppval, dkcres_type_t* prescode) const
- bool K2hdkcComCasGet::GetResponseData(const uint64_t** ppval, dkcres_type_t* prescode) const

#### パラメータ
- pkey  
  CASとして利用するキー（バイナリ列）を指定します。
- keylength  
  キー（バイナリ列）長を指定します。
- checkattr  
  キーの属性（Attribute）をチェックするか否かを指定します。
- encpass  
  キーが暗号化されている場合、復号化するためのパスフレーズを指定します。
- ppval  
  取得した値（8/16/32/64ビット）を格納するためのポインタを指定します。
- prescode  
  処理結果コードを格納するポインタを指定します。

#### 返り値
成功すればtrueを返し、失敗した場合にはfalseを返します。

#### 注意
CommandSendには、コマンド送信と結果取得を1度に行うメソッドタイプがあります。

#### サンプル
```
K2hdkcComCasGet*    pComObj = GetOtSlaveK2hdkcComCasGet(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin);
bool                result = pComObj->CommandSend(reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, true, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), &pval8, &rescode);
```

<!-- -----------------------------------------------------------　-->
***

### <a name="CASSETCPP"> K2hdkcComCasSetクラス
CAS（Compare And Swap）としてキーの値を設定するクラスです。  
CAS操作として動作するために、変更前の値の指定も行い、変更前の値と同じ値のときの場合のみ新しい値で上書きされます。また、値のサイズ（8/16/32/64ビット）別にメソッドが存在します。

#### 書式
- bool K2hdkcComCasSet::CommandSend(const unsigned char* pkey, size_t keylength, uint8_t oldval, uint8_t newval, bool checkattr = true, const char* encpass = NULL, const time_t* expire = NULL)
- bool K2hdkcComCasSet::CommandSend(const unsigned char* pkey, size_t keylength, uint16_t oldval, uint16_t newval, bool checkattr = true, const char* encpass = NULL, const time_t* expire = NULL)
- bool K2hdkcComCasSet::CommandSend(const unsigned char* pkey, size_t keylength, uint32_t oldval, uint32_t newval, bool checkattr = true, const char* encpass = NULL, const time_t* expire = NULL)
- bool K2hdkcComCasSet::CommandSend(const unsigned char* pkey, size_t keylength, uint64_t oldval, uint64_t newval, bool checkattr = true, const char* encpass = NULL, const time_t* expire = NULL)
- bool K2hdkcComCasSet::CommandSend(const unsigned char* pkey, size_t keylength, uint8_t oldval, uint8_t newval, bool checkattr, const char* encpass, const time_t* expire, dkcres_type_t* prescode)
- bool K2hdkcComCasSet::CommandSend(const unsigned char* pkey, size_t keylength, uint16_t oldval, uint16_t newval, bool checkattr, const char* encpass, const time_t* expire, dkcres_type_t* prescode)
- bool K2hdkcComCasSet::CommandSend(const unsigned char* pkey, size_t keylength, uint32_t oldval, uint32_t newval, bool checkattr, const char* encpass, const time_t* expire, dkcres_type_t* prescode)
- bool K2hdkcComCasSet::CommandSend(const unsigned char* pkey, size_t keylength, uint64_t oldval, uint64_t newval, bool checkattr, const char* encpass, const time_t* expire, dkcres_type_t* prescode)
- bool K2hdkcComCasSet::GetResponseData(dkcres_type_t* prescode) const

#### パラメータ
- pkey  
  CASとして利用するキー（バイナリ列）を指定します。
- keylength  
  キー（バイナリ列）長を指定します。
- oldval  
  変更前の期待する値（8/16/32/64ビット）を指定します。
- newval  
  変更後の値（8/16/32/64ビット）を指定します。
- checkattr  
  キーの属性（Attribute）をチェックするか否かを指定します。
- encpass  
  キーを暗号化する場合、そのパスフレーズを指定します。
- expire  
  キーに有効期限を設定する場合の有効期限を指定します。
- prescode  
  処理結果コードを格納するポインタを指定します。

#### 返り値
成功すればtrueを返し、失敗した場合にはfalseを返します。

#### 注意
CommandSendには、コマンド送信と結果取得を1度に行うメソッドタイプがあります。

#### サンプル
```
K2hdkcComCasSet*    pComObj = GetOtSlaveK2hdkcComCasSet(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin);
bool                result  = pComObj->CommandSend(reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, oldval, newval, true, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), (-1 == Expire ? NULL : &Expire), &rescode);
```

<!-- -----------------------------------------------------------　-->
***

### <a name="CASINCDECCPP"> K2hdkcComCasIncDecクラス
CAS（Compare And Swap）としてキーの値をインクリメント/デクリメントするクラスです。  
CASとしてのインクリメント/デクリメント操作を提供するため、k2hdkcクラスタへ同時にリクエストが発行されても確実に処理が行われることを保証します。また、値のサイズ（8/16/32/64ビット）別にメソッドが存在します。

#### 書式
- bool K2hdkcComCasIncDec::IncrementCommandSend(const unsigned char* pkey, size_t keylength, bool checkattr = true, const char* encpass = NULL, const time_t* expire = NULL)
- bool K2hdkcComCasIncDec::DecrementCommandSend(const unsigned char* pkey, size_t keylength, bool checkattr = true, const char* encpass = NULL, const time_t* expire = NULL)
- bool K2hdkcComCasIncDec::IncrementCommandSend(const unsigned char* pkey, size_t keylength, bool checkattr, const char* encpass, const time_t* expire, dkcres_type_t* prescode)
- bool K2hdkcComCasIncDec::DecrementCommandSend(const unsigned char* pkey, size_t keylength, bool checkattr, const char* encpass, const time_t* expire, dkcres_type_t* prescode)
- bool K2hdkcComCasIncDec::GetResponseData(dkcres_type_t* prescode) const

#### パラメータ
- pkey  
  CASとして利用するキー（バイナリ列）を指定します。
- keylength  
  キー（バイナリ列）長を指定します。
- checkattr  
  キーの属性（Attribute）をチェックするか否かを指定します。
- encpass  
  キーを暗号化する場合、そのパスフレーズを指定します。
- expire  
  キーに有効期限を設定する場合の有効期限を指定します。
- prescode  
  処理結果コードを格納するポインタを指定します。

#### 返り値
成功すればtrueを返し、失敗した場合にはfalseを返します。

#### 注意
CommandSendには、コマンド送信と結果取得を1度に行うメソッドタイプがあります。

#### サンプル
```
K2hdkcComCasIncDec* pComObj = GetOtSlaveK2hdkcComCasIncDec(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin);
bool                result  = pComObj->pComObj->IncrementCommandSend(reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, true, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), (-1 == Expire ? NULL : &Expire), &rescode);
```

<!-- -----------------------------------------------------------　-->
***

### <a name="STATECPP"> K2hdkcComStateクラス
k2hdkcクラスタの全サーバーノードの状態を取得するクラスです。  
取得する内容は、chmpxのk2hdkcクラスタ内での番号（chmpxid、HASH値、他）および、そのサーバノードが管理しているk2hashデータのステータス情報です。

#### 書式
- bool K2hdkcComState::CommandSend(const DKC_NODESTATE** ppStates, size_t* pstatecount, dkcres_type_t* prescode)
- bool K2hdkcComState::GetResponseData(const DKC_NODESTATE** ppStates, size_t* pstatecount, dkcres_type_t* prescode) const

#### パラメータ
- ppStates  
  取得したステータス情報（配列）を格納するポインタを指定します。
- pstatecount  
  取得したステータス情報の配列数を格納するポインタを指定します。
- prescode  
  処理結果コードを格納するポインタを指定します。

#### 構造体
取得するステータス情報は以下の構造体の配列として取得します。
```
typedef struct k2hdkc_nodestate{
    chmpxid_t       chmpxid;                // See: CHMPX in chmpx/chmstructure.h
    char            name[NI_MAXHOST];       //
    chmhash_t       base_hash;              //
    chmhash_t       pending_hash;           //
    K2HSTATE        k2hstate;               // See: k2hash/k2hash.h
}DKC_NODESTATE, *PDKC_NODESTATE;
 
typedef struct k2h_state{
    char            version[K2H_VERSION_LENGTH];            // Version string as K2hash file
    char            hash_version[K2H_HASH_FUNC_VER_LENGTH]; // Version string as Hash Function
    char            trans_version[K2H_HASH_FUNC_VER_LENGTH];// Version string as Transaction Function
    int             trans_pool_count;                       // Transaction plugin thread pool count
    k2h_hash_t      max_mask;                               // Maximum value for cur_mask
    k2h_hash_t      min_mask;                               // Minimum value for cur_mask
    k2h_hash_t      cur_mask;                               // Current mask value for hash(This value is changed automatically)
    k2h_hash_t      collision_mask;                         // Mask value for collision when masked hash value by cur_mask(This value is not changed)
    unsigned long   max_element_count;                      // Maximum count for elements in collision key index structure(Increasing cur_mask when this value is over)
    size_t          total_size;                             // Total size of k2hash
    size_t          page_size;                              // Paging size(system)
    size_t          file_size;                              // k2hash file(memory) size
    size_t          total_used_size;                        // Total using size in k2hash
    size_t          total_map_size;                         // Total mapping size for k2hash
    size_t          total_element_size;                     // Total element area size
    size_t          total_page_size;                        // Total page area size
    long            total_area_count;                       // Total mmap area count( = MAX_K2HAREA_COUNT)
    long            total_element_count;                    // Total element count in k2hash
    long            total_page_count;                       // Total page count in k2hash
    long            assigned_area_count;                    // Assigned area count
    long            assigned_key_count;                     // Assigned key index count
    long            assigned_ckey_count;                    // Assigned collision key index count
    long            assigned_element_count;                 // Assigned element count
    long            assigned_page_count;                    // Assigned page count
    long            unassigned_element_count;               // Unassigned element count(free element count)
    long            unassigned_page_count;                  // Unassigned page count(free page count)
    struct timeval  last_update;                            // Last update of data
    struct timeval  last_area_update;                       // Last update of expanding area
}K2HSTATE, *PK2HSTATE;
```

#### 返り値
成功すればtrueを返し、失敗した場合にはfalseを返します。

#### サンプル
```
K2hdkcComState*    pComObj = GetOtSlaveK2hdkcComState(strConfFile.c_str(), CntlPort, NULL/* cuk is NULL */,  isAutoRejoin, isNoGiveupRejoin);
bool               result = pComObj->CommandSend(&ptmpstates, &tmpstatecount, &rescode);
```
