/*
 * persistent_store.c
 *
 *  Created on: 2017. 9. 5.
 *      Author: Jaehyung Kim
 */
#include "redisassert.h"
#include "persistent_store.h"
#include "zmalloc.h"
#include "util.h"

#include <stdio.h>

void createPersistentStoreOptions(persistent_store_t *ps) {
    persistent_store_options_t* ps_options = (persistent_store_options_t*)zmalloc(sizeof(persistent_store_options_t));
    ps_options->options = rocksdb_options_create();
    ps_options->roptions = rocksdb_readoptions_create();
    ps_options->woptions = rocksdb_writeoptions_create();
    ps_options->coptions = rocksdb_compactoptions_create();
    ps_options->cache = rocksdb_cache_create_lru(100000);

    rocksdb_options_set_create_if_missing(ps_options->options, 1);
    rocksdb_options_set_create_missing_column_families(ps_options->options, 1);
    rocksdb_options_set_info_log(ps_options->options, NULL);
    rocksdb_options_set_write_buffer_size(ps_options->options, 100000);
    rocksdb_options_set_paranoid_checks(ps_options->options, 1);
    rocksdb_options_set_max_open_files(ps_options->options, 10);
    rocksdb_options_set_base_background_compactions(ps_options->options, 1);
    ps_options->table_options = rocksdb_block_based_options_create();
    rocksdb_block_based_options_set_block_cache(ps_options->table_options, ps_options->cache);
    rocksdb_options_set_block_based_table_factory(ps_options->options, ps_options->table_options);

    rocksdb_options_set_compression(ps_options->options, rocksdb_no_compression);
    rocksdb_options_set_compression_options(ps_options->options, -14, -1, 0, 0);
    int compression_levels[] = {rocksdb_no_compression, rocksdb_no_compression,
                                rocksdb_no_compression, rocksdb_no_compression};
    rocksdb_options_set_compression_per_level(ps_options->options, compression_levels, 4);
    ps_options->rate_limiter = rocksdb_ratelimiter_create(1000 * 1024 * 1024, 100 * 1000, 10);
    rocksdb_options_set_ratelimiter(ps_options->options, ps_options->rate_limiter);

    rocksdb_readoptions_set_verify_checksums(ps_options->roptions, 1);
    rocksdb_readoptions_set_fill_cache(ps_options->roptions, 1);

    rocksdb_writeoptions_set_sync(ps_options->woptions, 1);

    rocksdb_compactoptions_set_exclusive_manual_compaction(ps_options->coptions, 1);
    ps->ps_options = ps_options;
}

void createPersistentStoreDb(persistent_store_t *ps) {
    char *err = NULL;
    size_t i=0;

    /* First, DB creation by opening */
//    ps->ps = rocksdb_open(ps->ps_options->options, ps->dbname, &err);
//
//    if(err != NULL)
//    {
//        panic("[PERSISTENT_STORE] Persistent DB open failed due to : %s", err);
//    }
//
//    rocksdb_column_family_handle_t* cfh;
//    cfh = rocksdb_create_column_family(ps->ps, ps->ps_options->options, "RW", &err);
//    rocksdb_column_family_handle_destroy(cfh);
//
//    rocksdb_close(ps->ps);

    /* Get list of CFs */
    size_t cflen = 2;
    char* cf_names[2] = {"default", "RW"};
//    ps->cf_names = cf_names;
//    ps->cf_names = rocksdb_list_column_families(ps->ps_options->options, (const char*)ps->dbname, &cflen, &err);
//    if(err) {
//        rocksdb_free(err);
//        panic("[PERSISTENT_STORE] list column families failed due to %s", err);
//    }
    ps->cflen = cflen;

    /* Open CFs */
    ps->ps_cf_handles = zmalloc(sizeof(rocksdb_column_family_handle_t *) * cflen);
    rocksdb_options_t **cfs_options = zmalloc(sizeof(rocksdb_options_t *) * cflen);
    for(i = 0; i < cflen; i++) {
        cfs_options[i] = ps->ps_options->options;
    }

    /* Open RocksDB */
    ps->ps = rocksdb_open_column_families((const rocksdb_options_t*)ps->ps_options->options, (const char*)ps->dbname, cflen, (const char**)cf_names, (const struct rocksdb_options_t **)cfs_options, ps->ps_cf_handles, &err);
    if(err) {
        panic("[PERSISTENT_STORE] open column families failed due to %s", err);
    }

    zfree(cfs_options);
}

persistent_store_t* createPersistentStore(int dbnum) {
    persistent_store_t *ps = (persistent_store_t *)zmalloc(sizeof(persistent_store_t));

    /* Assign the database name */
    /* TODO temporarily used name */
    char *ps_db_name = "default";
    char *ps_name = zmalloc(digits10(dbnum)+9);
    sprintf(ps_name, "%d:%s", dbnum, ps_db_name);
    ps->dbname = ps_name;

    createPersistentStoreOptions(ps);
    createPersistentStoreDb(ps);
    return ps;
}

void setPersistentKey(persistent_store_t* ps, const void *key, const int keylen, const void *val, const int vallen) {
    char *err = NULL;
    serverLog(1, "BEFORE WRITE TO ROCKSDB KEY : %s, VAL : %s",(const char*)key, (const char*)val);
    serverLog(0, "KEYlength : %d, VALlength : %d", keylen, vallen);
    rocksdb_put_cf(ps->ps, ps->ps_options->woptions, ps->ps_cf_handles[PERSISTENT_STORE_CF_RW],
    		(const char*)key, keylen, (const char*)val, vallen, &err);
    if(err) {
        panic("[PERSISTENT_STORE] putting a key failed due to %s", err);
    }
}


//TODO - Implement later
void getPersistentKey(void){


}

void destroyPersistentStoreOptions(persistent_store_t* ps) {
    rocksdb_cache_destroy(ps->ps_options->cache);
    rocksdb_block_based_options_destroy(ps->ps_options->table_options);
    rocksdb_compactoptions_destroy(ps->ps_options->coptions);
    rocksdb_ratelimiter_destroy(ps->ps_options->rate_limiter);
    rocksdb_readoptions_destroy(ps->ps_options->roptions);
    rocksdb_writeoptions_destroy(ps->ps_options->woptions);
    rocksdb_options_destroy(ps->ps_options->options);
}

void destroyPersistentStore(persistent_store_t* ps) {
    size_t i = 0;
    rocksdb_list_column_families_destroy(ps->cf_names, ps->cflen);
    for (i = 0; i < ps->cflen; i++) {
      rocksdb_column_family_handle_destroy(ps->ps_cf_handles[i]);
    }
    destroyPersistentStoreOptions(ps);
    rocksdb_close(ps->ps);
    zfree(ps->dbname);
}
