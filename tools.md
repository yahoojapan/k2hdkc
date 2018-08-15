---
layout: contents
language: en-us
title: Tools
short_desc: K2Hash based Distributed Kvs Cluster
lang_opp_file: toolsja.html
lang_opp_word: To Japanese
prev_url: environments.html
prev_string: Environments
top_url: index.html
top_string: TOP
next_url: 
next_string: 
---

# Tools
## k2hdkclinetool
**k2hdkclinetool** is a client tool on a client node which provides all kinds of operations to data on the **K2HDKC** cluster.  
**k2hdkclinetool** is just like the **k2hlinetool** in [K2HASH](https://k2hash.antpick.ax/).

## How to debug
**k2hdkclinetool** is a useful tools, but many other tools and settings should be used to debug and understand the components behavior because the **K2HDKC** cluster and the **K2HDKC** client consist of several components as [K2HASH](https://k2hash.antpick.ax/), [CHMPX](https://chmpx.antpick.ax/) and **K2HDKC**.

| tools/settings | component | description |
|-|-|-|
| [k2hdkclinetool](k2hdkclinetool.html)                | **K2HDKC**                           | all kinds of operations to data on the **K2HDKC** cluster. |
| [chmpxstatus](https://chmpx.antpick.ax/tools.html)   | [CHMPX](https://chmpx.antpick.ax/)   | getting the cluster information from [CHMPX](https://chmpx.antpick.ax/).  |
| CHMDBGMODE and CHMDBGFILE environment variables      | [CHMPX](https://chmpx.antpick.ax/)   | [CHMPX](https://chmpx.antpick.ax/) log level. <br />Sending signal USR1 is another way to change the log level. <br />See [Environments](environments.html) in more details. |
| [k2hlinetool](https:///k2hash.antpick.ax/tools.html) | [K2HASH](https://k2hash.antpick.ax/) | all kinds of operation to the [K2HASH](https://k2hash.antpick.ax/) file. <br />**K2HDKC** server processes handle the [K2HASH](https://k2hash.antpick.ax/) data files. <br />You can view them with **k2hlinetool**. |
| K2HDBGMODE and K2HDBGFILE environment variables      | [K2HASH](https://k2hash.antpick.ax/) | [K2HASH](https://k2hash.antpick.ax/) log level. <br />Sending signal USR1 is another way to change the log level. <br />See [Environments](environments.html) in more details. |
