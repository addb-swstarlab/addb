/*
 * persistent_store.h
 *
 *  Created on: 2017. 9. 5.
 *      Author: canpc815
 */

#ifndef SRC_PERSISTENT_STORE_H_
#define SRC_PERSISTENT_STORE_H_

#include "rocksdb/c.h"

#define LOG_MAX_LEN    1024 /* Default maximum length of syslog messages */

/* Log levels */
#define PERSISTENT_STORE_DEBUG 0
#define PERSISTENT_STORE_VERBOSE 1
#define PERSISTENT_STORE_NOTICE 2
#define PERSISTENT_STORE_WARNING 3

/* COLUMN FAMILIES */
#define PERSISTENT_STORE_CF_DEFAULT 0
#define PERSISTENT_STORE_CF_RW      1
#define PERSISTENT_STORE_CF_NUM     2

typedef struct _persistent_store_options {
    rocksdb_options_t* options;
    rocksdb_readoptions_t* roptions;
    rocksdb_writeoptions_t* woptions;
    rocksdb_compactoptions_t* coptions;
    rocksdb_block_based_table_options_t* table_options;
    rocksdb_cache_t* cache;
    rocksdb_ratelimiter_t* rate_limiter;
} persistent_store_options_t;

typedef struct _persistent_store {
    rocksdb_t *ps;                                    /* RocksDB instance */
    persistent_store_options_t* ps_options;           /* DB options */
    rocksdb_column_family_handle_t** ps_cf_handles;    /* ColumnFamily handles */
    size_t cflen;
    char** cf_names;
    char* dbname;
} persistent_store_t;

void createPersistentStoreOptions(persistent_store_t *ps);
void createPersistentStoreDb(persistent_store_t *ps);
persistent_store_t* createPersistentStore(int dbnum);

#endif /* SRC_PERSISTENT_STORE_H_ */
