[![Protobuf Version](https://img.shields.io/badge/protobuf-v3.9.0-yellow.svg)](https://github.com/protocolbuffers/protobuf)
[![Protobuf-c Version](https://img.shields.io/badge/protobuf--c-v1.3.2-yellow.svg)](https://github.com/protobuf-c/protobuf-c)
[![ADDB Version](https://img.shields.io/badge/version-v1.1.0-brightgreen.svg)](https://github.com/addb-swstarlab/addb/tree/v1.1.0)

# Analytic Distributed DBMS for the project SW-StarLab

We improve analytic DBMS, a special kind of DBMS which stores big data for analytic purposes. 
We are developing �ADDB (Analytic Distributed DBMS)�, an in-memory, distributed version of analytic DBMS based on flash memory storage.
In this project, emerging persistent storage such as NVRAM will also be concerned.

This research was supported by the MSIT(Ministry of Science and ICT), Korea, under the SW Starlab support program(IITP-2017-0-00477) supervised by the IITP(Institute for Information & communications Technology Promotion).

## Prerequisites
Please install these libraries \
protobuf  v3.9.0 \
protobuf-c v1.3.2

After install these libraries, please execute theses commands

```bash
echo "export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig" >> ~/.bash_profile
echo "export LD_LIBRARY_PATH=\$LD_LIBRARY_PATH:/usr/local/lib" >> ~/.bash_profile
source ~/.bash_profile
```
