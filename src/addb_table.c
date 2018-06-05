/*
 * 2018.3.15
 * hssung@yonsei.ac.kr
 * Add File for relational model in Redis
 */

#include "server.h"
#include "assert.h"
#include "addb_relational.h"
#include "stl.h"

/*ADDB*/
/*fpWrite parameter list
 * arg1 : dataKeyInfo
 * arg2 : partitionInfo
 * arg3 : filter index column
 * arg4 : */

void fpWriteCommand(client *c){

    serverLog(LL_DEBUG,"FPWRITE COMMAND START");

    int fpWrite_result = C_OK;
    int i;
    long long insertedRow=0;
    //struct redisClient *fakeClient = NULL;

    serverLog(LL_DEBUG, "fpWrite Param List ==> Key : %s, partition : %s, num_of_column : %s, indexColumn : %s",
            (char *) c->argv[1]->ptr,(char *) c->argv[2]->ptr, (char *) c->argv[3]->ptr , (char *) c->argv[4]->ptr);

    /*parsing dataInfo*/
    NewDataKeyInfo *dataKeyInfo = parsingDataKeyInfo((sds)c->argv[1]->ptr);

    /*get column number*/
    int column_number = atoi((char *) c->argv[3]->ptr);
    assert(column_number <= MAX_COLUMN_NUMBER);
    serverLog(LL_DEBUG, "fpWrite Column Number : %d", column_number);

    /*get value number*/
    int value_num = c->argc - 5;
    serverLog(LL_DEBUG ,"VALUE NUM : %d", value_num);

    /*compare with column number and arguments*/
    if((value_num % column_number) != 0 ){
    	serverLog(LL_WARNING,"column number and args number do not match");
    	addReplyError(c, "column_number Error");
    	return;
    }

    serverLog(LL_DEBUG,"VALID DATAKEYSTRING ==> tableId : %d, partitionInfo : %s, rowgroup : %d",
              dataKeyInfo->tableId, dataKeyInfo->partitionInfo.partitionString, dataKeyInfo->rowGroupId);

    /*get rowgroup info from Metadict*/
    int rowGroupId = getRowgroupInfo(c->db, dataKeyInfo);
    serverLog(LL_DEBUG, "rowGroupId = %d", rowGroupId);

    /*get rownumber info from Metadict*/
    int row_number = getRowNumberInfoAndSetRowNumberInfo(c->db, dataKeyInfo);
    serverLog(LL_DEBUG, "rowNumber = %d", row_number);

    /*set rowNumber Info to Metadict*/
    if(row_number == 0 ){
    	incRowNumber(c->db, dataKeyInfo, 0);
    }
    /*check rowgroup size*/
    if(row_number >= server.rowgroup_size){
    	rowGroupId = IncRowgroupIdAndModifyInfo(c->db, dataKeyInfo, 1);
    	row_number = 0;
    }

    robj * dataKeyString = generateDataKey(dataKeyInfo);
    serverLog(LL_DEBUG, "DATAKEY :  %s", (char *)dataKeyString->ptr);

    int idx =0;
    for(i = 5; i < c->argc; i++){

    	   /*TODO - pk column check & ROW MAX LIMIT, COLUMN MAX LIMIT, */

    	robj *valueObj = getDecodedObject(c->argv[i]);

    	//Create field Info
    	int row_idx = row_number + (idx / column_number) + 1;
    	int column_idx = (idx % column_number) + 1;
     assert(column_idx <= MAX_COLUMN_NUMBER);

    	robj *dataField = getDataField(row_idx, column_idx);
     serverLog(LL_DEBUG, "DATAFIELD KEY = %s", (char *)dataField->ptr);
     assert(dataField != NULL);


     /*check Value Type*/
     if(!(strcmp((char *)valueObj->ptr, NULLVALUE)))
        	valueObj = shared.nullValue;


     serverLog(LL_DEBUG, "insertKVpairToRelational key : %s, field : %s, value : %s",
        		(char *)dataKeyString->ptr, (char *)dataField->ptr, (char *)valueObj->ptr);

        /*insert data into dict with Relational model*/
     insertKVpairToRelational(c, dataKeyString, dataField, valueObj);

        idx++;
        insertedRow++;
        decrRefCount(dataField);
        decrRefCount(valueObj);
    }
    decrRefCount(dataKeyString);
    zfree(dataKeyInfo);

    /*addb update row number info*/
    insertedRow /= column_number;
    incRowNumber(c->db, dataKeyInfo, insertedRow);


    /*TODO - filter*/
    /*TODO - eviction insert*/

    serverLog(LL_DEBUG,"FPWRITE COMMAND END");
    addReply(c, shared.ok);
}


void fpReadCommand(client *c) {
    serverLog(LL_VERBOSE,"FPREAD COMMAND START");
    getGenericCommand(c);
}

/*
 * fpScanCommand
 * Scan data from the database(Redis & RocksDB)
 * --- Parameters ---
 *  arg1: Key(Table ID & PartitionInfo ID)
 *  arg2: Column IDs to find
 *
 * --- Usage Examples ---
 *  Parameters:
 *      key: "D:{3:1:2}"
 *          tableId: "3"
 *          partitionInfoId: "1:2"
 *      columnIds: ["2", "3", "4"]
 *  Command:
 *      redis-cli> FPSCAN D:{3:2:1} 2,3,4
 *  Results:
 *      redis-cli> "20180509"
 *      redis-cli> "Do young Kim"
 *      redis-cli> "Yonsei Univ"
 *      ...
 */
void fpScanCommand(client *c) {
    serverLog(LL_VERBOSE, "FPSCAN COMMAND START");
    serverLog(LL_DEBUG, "DEBUG: command parameter");
    serverLog(LL_DEBUG, "first: %s, second: %s", (sds) c->argv[1]->ptr,
              (sds) c->argv[2]->ptr);

    /*Creates scan parameters*/
    ScanParameter *scanParam = createScanParameter(c);
    serverLog(LL_DEBUG, "DEBUG: parse scan parameter");
    serverLog(LL_DEBUG, "startRowGroupId: %d, totalRowGroupCount: %d",
              scanParam->startRowGroupId, scanParam->totalRowGroupCount);
    serverLog(LL_DEBUG, "dataKeyInfo");
    serverLog(LL_DEBUG,
              "tableId: %d, partitionInfo: %s, rowGroupId: %d, rowCnt: %d",
              scanParam->dataKeyInfo->tableId,
              scanParam->dataKeyInfo->partitionInfo.partitionString,
              scanParam->dataKeyInfo->rowGroupId,
              scanParam->dataKeyInfo->row_number);
    serverLog(LL_DEBUG, "columnParam");
    serverLog(LL_DEBUG, "original: %s, columnCount: %d",
              scanParam->columnParam->original,
              scanParam->columnParam->columnCount);
    for (int i = 0; i < scanParam->columnParam->columnCount; ++i) {
        serverLog(LL_DEBUG, "i: %d, columnId: %ld, columnIdStr: %s",
                  i,
                  (long) vectorGet(&scanParam->columnParam->columnIdList, i),
                  (sds) vectorGet(
                      &scanParam->columnParam->columnIdStrList, i));
    }

    /*Populates row group information to scan parameters*/
    int totalDataCount = populateScanParameter(c->db, scanParam);
    serverLog(LL_DEBUG, "total data count: %d", totalDataCount);

    /*Load data from Redis or RocksDB*/
    Vector data;
    vectorTypeInit(&data, VECTOR_TYPE_SDS);
    scanDataFromADDB(c->db, scanParam, &data);

    /*Scan data to client*/
    void *replylen = addDeferredMultiBulkLength(c);
    size_t numreplies = 0;
    serverLog(LL_DEBUG, "Loaded data from ADDB...");
    for (size_t i = 0; i < vectorCount(&data); ++i) {
        sds datum = sdsdup((sds) vectorGet(&data, i));
        serverLog(LL_DEBUG, "i: %zu, value: %s", i, datum);
        addReplyBulkSds(c, datum);
        numreplies++;
    }

    freeScanParameter(scanParam);
    vectorFree(&data);
    setDeferredMultiBulkLength(c, replylen, numreplies);
}

/*Lookup key in metadict */
void metakeysCommand(client *c){

    dictIterator *di;
    dictEntry *de;
    sds pattern = c->argv[1]->ptr;
    int plen = sdslen(pattern), allkeys;
    unsigned long numkeys = 0;
    void *replylen = addDeferredMultiBulkLength(c);

    di = dictGetSafeIterator(c->db->Metadict);
    allkeys = (pattern[0] == '*' && pattern[1] == '\0');
    while((de = dictNext(di)) != NULL) {
        sds key = dictGetKey(de);
        robj *keyobj;

        if (allkeys || stringmatchlen(pattern,plen,key,sdslen(key),0)) {
            keyobj = createStringObject(key,sdslen(key));
            if (expireIfNeeded(c->db,keyobj) == 0) {
                addReplyBulk(c,keyobj);
                numkeys++;
            }
            decrRefCount(keyobj);
        }
    }
    dictReleaseIterator(di);
    setDeferredMultiBulkLength(c,replylen,numkeys);

}

/*Lookup the value list of field and field in dict*/
void fieldsAndValueCommand(client *c){

    dictIterator *di;
    dictEntry *de;
    sds pattern = sdsnew(c->argv[1]->ptr);

    robj *hashdict = lookupSDSKeyFordict(c->db, pattern);

    if(hashdict == NULL){
    	 addReply(c, shared.nullbulk);
    }
    else {

     	char str_buf[1024];
     	unsigned long numkeys = 0;
     	void *replylen = addDeferredMultiBulkLength(c);

     	dict *hashdictObj = (dict *) hashdict->ptr;
     	di = dictGetSafeIterator(hashdictObj);
     	while((de = dictNext(di)) != NULL){

     		sds key = dictGetKey(de);
     		sds val = dictGetVal(de);
     		sprintf(str_buf, "field : %s, value : %s", key, val);
     		addReplyBulkCString(c, str_buf);
     		numkeys++;

     		serverLog(LL_VERBOSE ,"key : %s , val : %s" , key,  val);

     }
     	sdsfree(pattern);
     	dictReleaseIterator(di);
     	setDeferredMultiBulkLength(c, replylen, numkeys);
    }

}


void prepareWriteToRocksDB(redisDb *db, robj *keyobj, robj *targetVal){
serverLog(LL_DEBUG ,"PREPARING WRITE FOR ROCKSDB");
    dictIterator *di;
    dictEntry *de;
    char keystr[SDS_DATA_KEY_MAX];

 	dict *hashdictObj = (dict *) targetVal->ptr;
 	di = dictGetSafeIterator(hashdictObj);
 	while((de = dictNext(di)) != NULL){
 		sds field_key = dictGetKey(de);
 		sds val = dictGetVal(de);


 		sprintf(keystr, "%s:%s%s",keyobj->ptr,REL_MODEL_FIELD_PREFIX, field_key);
 		sds rocksKey = sdsnew(keystr);
 		robj *value = createStringObject(val, sdslen(val));


 		setPersistentKey(db->persistent_store, rocksKey, sdslen(rocksKey), value->ptr, sdslen(value->ptr));
 	}
    targetVal->location = LOCATION_PERSISTED;
 	dictReleaseIterator(di);
}



void rocksdbkeyCommand(client *c){
    sds pattern = sdsnew(c->argv[1]->ptr);
    robj *key = createStringObject(pattern, sdslen(pattern));
    robj *hashdict = lookupSDSKeyFordict(c->db, pattern);
    prepareWriteToRocksDB(c->db, key,hashdict);
    sdsfree(pattern);
    decrRefCount(key);
    addReply(c, shared.ok);
}

void getRocksDBkeyAndValueCommand(client *c){

	sds pattern = sdsnew(c->argv[1]->ptr);

	  char* err = NULL;
	  size_t val_len;
	  char* val;
	  val = rocksdb_get_cf(c->db->persistent_store->ps, c->db->persistent_store->ps_options->roptions,
			  c->db->persistent_store->ps_cf_handles[PERSISTENT_STORE_CF_RW], pattern, sdslen(pattern), &val_len, &err);

	  if(val == NULL){
		  rocksdb_free(val);
		  serverLog(LL_DEBUG, "ROCKSDB KEY : %s , VALUE NOT EXIST");
			sdsfree(pattern);
			addReply(c, shared.err);
	  }
	  else {
		  robj *value = NULL;
		  value = createStringObject(val, val_len);
		  rocksdb_free(val);
		  serverLog(LL_DEBUG, "ROCKSDB KEY : %s , VALUE : %s", pattern, (char *)value->ptr);
			sdsfree(pattern);
            decrRefCount(value);
			addReply(c, shared.ok);

	  }

}

