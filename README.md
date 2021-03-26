K2HDKC
--------
[![C/C++ AntPickax CI](https://github.com/yahoojapan/k2hdkc/workflows/C/C++%20AntPickax%20CI/badge.svg)](https://github.com/yahoojapan/k2hdkc/actions)
[![GitHub license](https://img.shields.io/badge/license-MIT-blue.svg)](https://raw.githubusercontent.com/yahoojapan/k2hdkc/master/COPYING)
[![GitHub forks](https://img.shields.io/github/forks/yahoojapan/k2hdkc.svg)](https://github.com/yahoojapan/k2hdkc/network)
[![GitHub stars](https://img.shields.io/github/stars/yahoojapan/k2hdkc.svg)](https://github.com/yahoojapan/k2hdkc/stargazers)
[![GitHub issues](https://img.shields.io/github/issues/yahoojapan/k2hdkc.svg)](https://github.com/yahoojapan/k2hdkc/issues)
[![debian packages](https://img.shields.io/badge/deb-packagecloud.io-844fec.svg)](https://packagecloud.io/antpickax/stable)
[![RPM packages](https://img.shields.io/badge/rpm-packagecloud.io-844fec.svg)](https://packagecloud.io/antpickax/stable)

**K2HDKC** - **K2H**ash based **D**istributed **K**vs **C**luster

### Overview
k2hdkc is a high speed automated based on [k2hash](https://github.com/yahoojapan/k2hash), [chmpx](https://github.com/yahoojapan/chmpx) Distributed Key Value Store(KVS).  

![K2HDKC](https://k2hdkc.antpick.ax/images/top_k2hdkc.png)

### Feature
k2hdkc is a highly available and scalable distributed KVS clustering system with the following features and features.
- **Consistency** (Automatic Data Synchronization between Server Nodes)  
Provides automatic merging function of data due to failure/recovery of server nodes in the cluster.
- **Automatic Scaling**  
Server nodes can be added/deleted to the cluster, and the data automatic merging function at this time is also provided.
- **Nested key structure**  
Provides the association function of key and subkey which is the feature of k2hash as distributed KVS.
- **Queue(FIFO/LIFO)**  
Provides the queuing function which is the feature of k2hash as distributed KVS.
- **Transaction Plugins**  
Provides a function that can perform arbitrary processing using data update processing which is the feature of k2hash as a trigger of transaction.
- **Encryption**  
Provides encryption function as distributed KVS for the key's data held as the feature of k2hash.
- **Expiration**  
Provides the key expiration function which is the feature of k2hash as distributed KVS.

### Documents
- [Document top page](https://k2hdkc.antpick.ax/)
- [Github wiki page](https://github.com/yahoojapan/k2hdkc/wiki)
- [About AntPickax](https://antpick.ax/)

### Packages
- [RPM packages(packagecloud.io)](https://packagecloud.io/antpickax/stable)
- [Debian packages(packagecloud.io)](https://packagecloud.io/antpickax/stable)

### Related products
- [K2HDKC Database As A Service(DBaaS)](https://dbaas.k2hdkc.antpick.ax/)

### License
This software is released under the MIT License, see the license file.

### AntPickax
k2hdkc is one of [AntPickax](https://antpick.ax/) products.

Copyright(C) 2018 Yahoo Japan Corporation.
