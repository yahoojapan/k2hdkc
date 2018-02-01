K2HDKC
--------
[![Build Status](https://travis-ci.org/yahoojapan/k2hdkc.svg?branch=master)](https://travis-ci.org/yahoojapan/k2hdkc)

**K2HDKC** - **K2H**ash based **D**istributed **K**vs **C**luster by Yahoo! JAPAN

### Overview
k2hdkc is a high speed automated based on [k2hash](https://github.com/yahoojapan/k2hash), [chmpx](https://github.com/yahoojapan/chmpx) Distributed Key Value Store(KVS).

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

### Doccuments
  - [WIKI](https://github.com/yahoojapan/k2hdkc/wiki)

### License
This software is released under the MIT License, see the LICENSE file.

Copyright 2018 Yahoo! JAPAN corporation.
