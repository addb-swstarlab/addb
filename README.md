<<<<<<< HEAD
[![Protobuf Version](https://img.shields.io/badge/protobuf-v3.9.0-yellow.svg)](https://github.com/protocolbuffers/protobuf)
[![Protobuf-c Version](https://img.shields.io/badge/protobuf--c-v1.3.2-yellow.svg)](https://github.com/protobuf-c/protobuf-c)
=======
>>>>>>> 7677178... [TRIVIAL] Updates README.md
[![ADDB Version](https://img.shields.io/badge/version-v1.1.0-brightgreen.svg)](https://github.com/addb-swstarlab/addb/tree/v1.1.0)

# Analytic Distributed DBMS for the project SW-StarLab

We improve analytic DBMS, a special kind of DBMS which stores big data for analytic purposes. 
We are developing �ADDB (Analytic Distributed DBMS)�, an in-memory, distributed version of analytic DBMS based on flash memory storage.
In this project, emerging persistent storage such as NVRAM will also be concerned.

This research was supported by the MSIT(Ministry of Science and ICT), Korea, under the SW Starlab support program(IITP-2017-0-00477) supervised by the IITP(Institute for Information & communications Technology Promotion).

## Prerequisites
<<<<<<< HEAD
Please install these libraries \
protobuf  v3.9.0 \
protobuf-c v1.3.2

After install these libraries, please execute theses commands

```bash
echo "export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig" >> ~/.bash_profile
echo "export LD_LIBRARY_PATH=\$LD_LIBRARY_PATH:/usr/local/lib" >> ~/.bash_profile
source ~/.bash_profile
=======
Before compile ADDB, please execute these commands on ADDB path.
```bash
echo "export ADDB_HOME=$(pwd)" >> ~/.bash_profile
echo "export LD_LIBRARY_PATH=\$LD_LIBRARY_PATH:\$ADDB_HOME/deps/rocksdb" >> ~/.bash_profile
echo "export LD_LIBRARY_PATH=\$LD_LIBRARY_PATH:\$ADDB_HOME/deps/jemalloc/lib" >> ~/.bash_profile
source ~/.bash_profile
```

## Run ADDB-RR
```bash
# Compile
make -j8

# Run server (with 'redis_tiering.conf')
./src/redis-server redis_tiering.conf

# Run client
./src/redis-cli
```

## Run test_scripts
```bash
# Create fpWrite, fpScan commands
cd test_scripts/
./cmdGenerator.sh TABLE_ID=100 MODE=static PARTITION_COUNT=100 COLUMN_COUNT=4 SCAN_LOOK_UP_COLUMNS="1,2,3" INSERT_ITER_CNT=200 SCAN_ITER_CNT=30

# Run multiple fpWrite commands
cat ./test_scripts/fpWriteCmd.resp | ./src/redis-cli --pipe

# (Option) Watch server status by client
./src/redis-cli --stat
```

## Leak check
Please check memory leaks by this method.

### How to use?
Step 1: Install 'valgrind' software \
http://www.valgrind.org/

Step 2: Re-compile 'redis-server' for using valgrind.
```bash
make valgrind -j8
```

Step 3: Run below commands when starts 'redis-server'. \
Valgrind log file will be stored on '${log_file_path}'.
```bash
valgrind --leak-check=full --trace-children=yes --log-file=${log_file_path} ./src/redis-server redis_tiering.conf
>>>>>>> 7677178... [TRIVIAL] Updates README.md
```
