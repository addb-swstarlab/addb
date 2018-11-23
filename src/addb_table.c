/*
 * 2018.3.15
 * hssung@yonsei.ac.kr
 * Add File for relational model in Redis
 */

#include "server.h"
#include "assert.h"
#include "addb_relational.h"
#include "stl.h"
#include "circular_queue.h"

/*ADDB*/
/*
 * fpWriteCommand
 *  Write relation data to ADDB.
 * --- Parameters ---
 *  arg1:  dataKeyInfo
 *  arg2:  partitionInfo
 *  arg3:  Number of columns
 *  arg4:  Filter index column (Not used now)
 *  arg5~: Column Data
 *
 * --- Usage Examples ---
 *  Parameters:
 *      dataKeyInfo:        D:{100:1:2}
 *      partitionInfo:      1:2
 *      NumberOfColumns:    4
 *      FilterIndexColumn:  0
 *      Column Data:        1 1 1 1
 *  Command:
 *      redis-cli> FPWRITE D:{100:1:2} 1:2 4 0 1 1 1 1
 *  Results:
 *      redis-cli> OK
 */
void fpWriteCommand(client *c){

    serverLog(LL_DEBUG,"FPWRITE COMMAND START");

    int fpWrite_result = C_OK;
    int i;
    long long insertedRow = 0;
    int Enroll_queue = 0;
    //long long meta_start = 0;
    //long long insert_start = 0;

    //struct redisClient *fakeClient = NULL;

    serverLog(LL_DEBUG, "fpWrite Param List ==> Key : %s, partition : %s, num_of_column : %s, indexColumn : %s",
            (char *) c->argv[1]->ptr,(char *) c->argv[2]->ptr, (char *) c->argv[3]->ptr , (char *) c->argv[4]->ptr);

    //meta_start = ustime();
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
    int prev_row = row_number;

    /*set rowNumber Info to Metadict*/
    if(row_number == 0 ){
    	incRowNumber(c->db, dataKeyInfo, 0);
    }
    /*check rowgroup size*/
    if(row_number >= server.rowgroup_size){
    	rowGroupId = IncRowgroupIdAndModifyInfo(c->db, dataKeyInfo, 1);
    	row_number = 0;
    	Enroll_queue = 1;
    }

		robj *dataKeyString = NULL;
		dataKeyString = generateDataKey(dataKeyInfo);
		//serverLog(LL_VERBOSE, "DATAKEY1 :  %s", (char *) dataKeyString->ptr);

		dictEntry *entryDict = dictFind(c->db->dict, dataKeyString->ptr);
		if (entryDict != NULL) {
			robj * val = dictGetVal(entryDict);
			//serverLog(LL_VERBOSE, "DATAKEYtest :  %d", val->location);

			// __sync_synchronize();
			if (val->location != LOCATION_REDIS_ONLY && !Enroll_queue) {
				rowGroupId = IncRowgroupIdAndModifyInfo(c->db, dataKeyInfo, 1);
				// incRowNumber(c->db, dataKeyInfo, 0);
				decrRefCount(dataKeyString);
				dataKeyString = generateDataKey(dataKeyInfo);
				row_number = 0;
				//serverLog(LL_VERBOSE, "DATAKEY2 :  %s, rowGroupId : %d  rowgroupId : %d",
				//		(char *) dataKeyString->ptr, rowGroupId, dataKeyInfo->rowGroupId);
			}
		} else if (entryDict == NULL && row_number != 0 && !Enroll_queue) {
			rowGroupId = IncRowgroupIdAndModifyInfo(c->db, dataKeyInfo, 1);
			// incRowNumber(c->db, dataKeyInfo, 0);
			decrRefCount(dataKeyString);
			dataKeyString = generateDataKey(dataKeyInfo);
			serverLog(LL_VERBOSE, "[FPWRITE] ENTRY_DICT_NULL... %s",
					dataKeyString->ptr);
			row_number = 0;
		}
//		server.stat_time_meta_update += ustime() - meta_start;

		//insert_start = ustime();

    int idx =0;
    int init =0;
    for(i = 5; i < c->argc; i++){

    	   /*TODO - pk column check & ROW MAX LIMIT, COLUMN MAX LIMIT, */

    	robj *valueObj = getDecodedObject(c->argv[i]);

    	//Create field Info
    	if(row_number < 0 || row_number > server.rowgroup_size)
    		serverAssert(0);
    	int row_idx = row_number + (idx / column_number) + 1;
    	int column_idx = (idx % column_number) + 1;
    	int columnvector_idx = ((row_idx -1) / server.columnvector_size + 1);
     assert(column_idx <= MAX_COLUMN_NUMBER);

    	robj *dataField = getDataField(column_idx, columnvector_idx);
     serverLog(LL_DEBUG, "DATAFIELD KEY = %s", (char *)dataField->ptr);
     assert(dataField != NULL);


     /*check Value Type*/
     if(!(strcmp((char *)valueObj->ptr, NULLVALUE)))
        	valueObj = shared.nullValue;


     serverLog(LL_DEBUG, "insertKVpairToRelational key : %s, field : %s, value : %s",
        		(char *)dataKeyString->ptr, (char *)dataField->ptr, (char *)valueObj->ptr);

     /*insert data into dict with Relational model*/
     init = insertKVpairToRelational(c, dataKeyString, dataField, valueObj);

     if(init)
    	 Enroll_queue++;

     idx++;
     insertedRow++;
     decrRefCount(dataField);
     decrRefCount(valueObj);
    }
    //server.stat_time_data_insert += ustime() - insert_start;

    /*addb update row number info*/
    insertedRow /= column_number;
    incRowNumber(c->db, dataKeyInfo, insertedRow);

    serverLog(LL_DEBUG,"FPWRITE COMMAND END");

    serverLog(LL_DEBUG,"DictEntry Registration in a circular queue START");

    /*Enqueue Entry*/
    if(Enroll_queue){
    	if(enqueue(c->db->EvictQueue, dictFind(c->db->dict, dataKeyString->ptr)) == 0) {
    		serverLog(LL_VERBOSE, "Enqueue queue : %d --- prev_row : %d --- String : %s " ,Enroll_queue, prev_row, dataKeyString->ptr);
    		serverAssert(0);
    	}
    }

    decrRefCount(dataKeyString);
    zfree(dataKeyInfo);
    addReply(c, shared.ok);
}


void fpReadCommand(client *c) {
    serverLog(LL_DEBUG,"FPREAD COMMAND START");
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
    serverLog(LL_DEBUG, "FPSCAN COMMAND START");
    // serverLog(LL_DEBUG, "DEBUG: command parameter");
    // serverLog(LL_DEBUG, "first: %s, second: %s", (sds) c->argv[1]->ptr,
    //           (sds) c->argv[2]->ptr);

    /*Creates scan parameters*/
    ScanParameter *scanParam = createScanParameter(c);
    // serverLog(LL_DEBUG, "DEBUG: parse scan parameter");
    // serverLog(LL_DEBUG, "startRowGroupId: %d, totalRowGroupCount: %d",
    //           scanParam->startRowGroupId, scanParam->totalRowGroupCount);
    // serverLog(LL_DEBUG, "dataKeyInfo");
    // serverLog(LL_DEBUG,
    //           "tableId: %d, partitionInfo: %s, rowGroupId: %d, rowCnt: %d",
    //           scanParam->dataKeyInfo->tableId,
    //           scanParam->dataKeyInfo->partitionInfo.partitionString,
    //           scanParam->dataKeyInfo->rowGroupId,
    //           scanParam->dataKeyInfo->row_number);
    // serverLog(LL_DEBUG, "columnParam");
    // serverLog(LL_DEBUG, "original: %s, columnCount: %d",
    //           scanParam->columnParam->original,
    //           scanParam->columnParam->columnCount);
    // for (int i = 0; i < scanParam->columnParam->columnCount; ++i) {
    //     serverLog(LL_DEBUG, "i: %d, columnId: %ld, columnIdStr: %s",
    //               i,
    //               (long) vectorGet(&scanParam->columnParam->columnIdList, i),
    //               (sds) vectorGet(
    //                   &scanParam->columnParam->columnIdStrList, i));
    // }

    /*Populates row group information to scan parameters*/
    int totalDataCount = populateScanParameter(c->db, scanParam);
    serverLog(LL_DEBUG, "total data count: %d", totalDataCount);

    /*Load data from Redis or RocksDB*/
    Vector data;
    vectorTypeInit(&data, STL_TYPE_SDS);
    legacyScanDataFromADDB(c->db, scanParam, &data);
    // scanDataFromADDB(c->db, scanParam, &data);

    /*Scan data to client*/
    void *replylen = addDeferredMultiBulkLength(c);
    size_t numreplies = 0;
    serverLog(LL_DEBUG, "Loaded data from ADDB...");
    for (size_t i = 0; i < vectorCount(&data); ++i) {
        sds datum = sdsdup((sds) vectorGet(&data, i));
        // serverLog(LL_DEBUG, "i: %zu, value: %s", i, datum);
        addReplyBulkSds(c, datum);
        numreplies++;
    }

    freeScanParameter(scanParam);
    vectorFreeDeep(&data);
    setDeferredMultiBulkLength(c, replylen, numreplies);
}

void _addReplyMetakeysResults(client *c, Vector *metakeys) {
    void *replylen = addDeferredMultiBulkLength(c);
    size_t numreplies = 0;
    serverLog(LL_DEBUG, "Loaded data from ADDB...");
    for (size_t i = 0; i < vectorCount(metakeys); ++i) {
        sds metakey = sdsdup((sds) vectorGet(metakeys, i));
        serverLog(LL_DEBUG, "i: %zu, metakey: %s", i, metakey);
        addReplyBulkSds(c, metakey);
        numreplies++;
    }

    setDeferredMultiBulkLength(c, replylen, numreplies);
    return;
}

/*
 * metakeysCommand
 *  Lookup key in metadict
 *  Filter partition from Metadict by parsed stack structure.
 * --- Parameters ---
 *  arg1: Parsed stack structure
 *
 * --- Usage Examples ---
 * --- Example 1: Pattern search only ---
 *  Parameters:
 *      pattern: *
 *  Command:
 *      redis-cli> METAKEYS *
 *  Results:
 *      redis-cli> "M:{30:2:0}"
 *      redis-cli> "M:{1:1:3:3:1}"
 *      redis-cli> "M:{100:2:0}"
 *      redis-cli> ...
 *  --- Example 2: Statements searching ---
 *  Parameters:
 *      pattern: M:{100:*}
 *      Statements:
 *          "3*2*EqualTo:2*2*EqualTo:Or:1*2*EqualTo:0*2*EqualTo:Or:Or:$"
 *          (select * from kv where 2=0 or 2=1 or 2=2 or 2=3;)
 *          "3*2*EqualTo:$1*2*EqualTo:0*2*EqualTo:Or:$"
 *          (Double Filtering)
 *          (select * from kv where col2=3)
 *          (select * from kv where col2=1 or col2=0)
 *  Command:
 *      redis-cli> METAKEYS M:{100:*} 3*2*EqualTo:2*2*EqualTo:Or:1*2*EqualTo:0*2*EqualTo:Or:Or:$
 *  Results:
 *      redis-cli> "M:{100:2:0}" // col2 = 0
 *      redis-cli> "M:{100:2:1}" // col2 = 1
 *      redis-cli> "M:{100:2:2}" // col2 = 2
 *      redis-cli> "M:{100:2:3}" // col2 = 3
 *      ...
 */
void metakeysCommand(client *c){
    /*Parses stringfied stack structure to readable parameters*/
    sds pattern = (sds) c->argv[1]->ptr;
    bool allkeys = (pattern[0] == '*' && pattern[1] == '\0');

    /*Initializes Vector for Metadict keys*/
    Vector metakeys;
    vectorTypeInit(&metakeys, STL_TYPE_SDS);

    /*Pattern match searching for metakeys*/
    dictIterator *di = dictGetSafeIterator(c->db->Metadict);
    dictEntry *de = NULL;
    while ((de = dictNext(di)) != NULL) {
        sds metakey = (sds) dictGetKey(de);
        if (
                allkeys ||
                stringmatchlen(pattern, sdslen(pattern), metakey,
                               sdslen(metakey), 0)
        ) {
            vectorAdd(&metakeys, metakey);
        }
    }
    dictReleaseIterator(di);

    /* If rawStatements is null or empty, prints pattern matching
     * results only...
     */
    if (c->argc < 3) {
        /*Prints out target partitions*/
        /*Scan data to client*/
        _addReplyMetakeysResults(c, &metakeys);
        vectorFree(&metakeys);
        return;
    }

    sds rawStatementsStr = (sds) c->argv[2]->ptr;

    if (!validateStatements(rawStatementsStr)) {
        serverLog(LL_WARNING, "[FILTER] Stack structure is not valid form: [%s]",
                  rawStatementsStr);
        addReplyErrorFormat(c, "[FILTER] Stack structure is not valid form: [%s]",
                            rawStatementsStr);
        return;
    }

    char copyStr[sdslen(rawStatementsStr) + 1];
    char *savePtr = NULL;
    char *token = NULL;
    memcpy(copyStr, rawStatementsStr, sdslen(rawStatementsStr) + 1);

    token = strtok_r(copyStr, PARTITION_FILTER_STATEMENT_SUFFIX, &savePtr);
    while (token != NULL) {
        Condition *root;
        sds rawStatementStr = sdsnew(token);

        if (parseStatement(rawStatementStr, &root) == C_ERR) {
            serverLog(
                    LL_WARNING,
                    "[FILTER][FATAL] Stack condition parser failed, server would have a memory leak...: [%s]",
                    rawStatementsStr);
            addReplyErrorFormat(
                    c,
                    "[FILTER][FATAL] Stack condition parser failed, server would have a memory leak...: [%s]",
                    rawStatementsStr);
            return;
        }

        // serverLog(LL_DEBUG, "   ");
        // serverLog(LL_DEBUG, "[FILTER][PARSE] Condition Tree");
        // logCondition(root);

        Vector filteredMetakeys;
        vectorInit(&filteredMetakeys);
        for (size_t i = 0; i < vectorCount(&metakeys); ++i) {
            sds metakey = (sds) vectorGet(&metakeys, i);
            if (evaluateCondition(root, metakey)) {
                vectorAdd(&filteredMetakeys, metakey);
            }
        }
        vectorFree(&metakeys);
        metakeys = filteredMetakeys;

        sdsfree(rawStatementStr);
        freeConditions(root);

        token = strtok_r(NULL, PARTITION_FILTER_STATEMENT_SUFFIX, &savePtr);
    }

    /*Prints out target partitions*/
    /*Scan data to client*/
    _addReplyMetakeysResults(c, &metakeys);
    vectorFree(&metakeys);
}


/*Lookup the value list of field and field in dict*/
/*
 * fieldsAndValueCommand
 * Get key, value for the Dict.
 * --- Parameters ---
 *  arg1: fields
 *  arg2: DataKey
 *  arg3: column_id
 *  arg4: columnVector_id
 *
 * --- Usage Examples ---
 *  Parameters:
 *
 *          tableId: "100"
 *          partitionInfoId: "2:1"
 *          rowgroup: 1
 *          column_id: 1
 *          columnVector_id: 1
 *      MetaField Key: CURRENT_RGID(= 1)
 *  Command:
 *      redis-cli> fields D:{100:2:1}:G:1 1 1
 *  Results:
 *      Find Case
 *          redis-cli>
 *          1) "field : 1:1, value : 1"
 *          2) "field : 1:1, value : 2"
 *          3) "field : 1:1, value : 3"
 *      Non-exist Case
 *          redis-cli> (nil)
 */
void fieldsAndValueCommand(client *c){

    sds pattern = sdsnew(c->argv[1]->ptr);
    int column = atoi(c->argv[2]->ptr);
    int vector_id = atoi(c->argv[3]->ptr);
    

    robj *hashdict = lookupSDSKeyFordict(c->db, pattern);
    
    dictEntry *de;
    
    if(hashdict == NULL){
    	 addReply(c, shared.nullbulk);
    }
    else {

     	char str_buf[1024];
     	unsigned long numkeys = 0;
     	void *replylen = addDeferredMultiBulkLength(c);

     	dict *hashdictObj = (dict *) hashdict->ptr;
     	robj *dataField = getDataField(column, vector_id);

    	if((de = dictFind(hashdictObj, dataField->ptr)) != NULL){
    	  sds key = dictGetKey(de);
     		robj *obj = dictGetVal(de);
     		Vector *v = (Vector *)obj->ptr;
     		int number = v->count;
     		int j=0;
     		for(j=0; j < number; j++){
     			sprintf(str_buf, "field : %s, value : %s", key, (sds)vectorGet(v, j));
     			addReplyBulkCString(c, str_buf);
     			numkeys++;
     		}
     }
    	else {
    		serverLog(LL_VERBOSE ,"Cant find vector");
     }
     	sdsfree(pattern);
     	decrRefCount(dataField);
     	setDeferredMultiBulkLength(c, replylen, numkeys);
    }

}

/*
 * rocksdbkeyCommand
 * Command to write redis data into rocksdb
 * Find redis data. if data existed, then write into rocksdb
 * Please check log file
 * --- Parameters ---
 *  arg1: rockskey
 *  arg2: DataKey
 *
 * --- Usage Examples ---
 *  Parameters:
 *
 *          tableId: "100"
 *          partitionInfoId: "2:1"
 *          rowgroup: 1
 *  Command:
 *      redis-cli> rockskey D:{100:2:1}:G:1
 *  Results:
 *      Find Case
 *          redis-cli>
 *          OK
 */
void rocksdbkeyCommand(client *c){
    sds pattern = sdsnew(c->argv[1]->ptr);
    robj *key = createStringObject(pattern, sdslen(pattern));
    robj *hashdict = lookupSDSKeyFordict(c->db, pattern);
    prepareWriteToRocksDB(c->db, key,hashdict);
    sdsfree(pattern);
    decrRefCount(key);
    addReply(c, shared.ok);
}

/*
 * getQueueStatusCommand
 * Lookup dictEntry currently registered in Queue.
 * --- Parameters ---
 *  arg1: queuestatus
 *  arg2: *
 *
 * --- Usage Examples ---
 *  Command:
 *      redis-cli> queuestatus *
 *  Results:
 *      Find Case
 *          redis-cli>
 *           1) "[Rear : 0, Front : 2, idx : 0]DataKey : D:{100:2:1}:G:1 Field : 3:2 ]"
 *           2) "[Rear : 0, Front : 2, idx : 0]DataKey : D:{100:2:1}:G:1 Field : 2:2 ]"
 *           3) "[Rear : 0, Front : 2, idx : 0]DataKey : D:{100:2:1}:G:1 Field : 1:1 ]"
 *           4) "[Rear : 0, Front : 2, idx : 0]DataKey : D:{100:2:1}:G:1 Field : 2:1 ]"
 *           5) "[Rear : 0, Front : 2, idx : 0]DataKey : D:{100:2:1}:G:1 Field : 4:1 ]"
 *           6) "[Rear : 0, Front : 2, idx : 0]DataKey : D:{100:2:1}:G:1 Field : 4:2 ]"
 *           7) "[Rear : 0, Front : 2, idx : 0]DataKey : D:{100:2:1}:G:1 Field : 1:2 ]"
 *           8) "[Rear : 0, Front : 2, idx : 0]DataKey : D:{100:2:1}:G:1 Field : 3:1 ]"
 *           9) "[Rear : 0, Front : 2, idx : 1]DataKey : D:{100:2:1}:G:2 Field : 3:2 ]"
 *          10) "[Rear : 0, Front : 2, idx : 1]DataKey : D:{100:2:1}:G:2 Field : 2:2 ]"
 *
 *      Empty Case
 *          redis-cli>
 *          1) "Queue is empty, front : 0, rear : 0"
 *
 */
void getQueueStatusCommand(client *c){
 	char str_buf[1024];
	if(c->db->EvictQueue->front == c->db->EvictQueue->rear){
		serverLog(LL_DEBUG, "EMPTY QUEUE");
     	unsigned long numkeys = 0;
     	void *replylen = addDeferredMultiBulkLength(c);
     	sprintf(str_buf, "Queue is empty, front : %d, rear : %d",c->db->EvictQueue->front ,c->db->EvictQueue->rear);
     addReplyBulkCString(c, str_buf);
     numkeys++;
     setDeferredMultiBulkLength(c, replylen, numkeys);
	}
	else {

     	unsigned long numkeys = 0;
     	void *replylen = addDeferredMultiBulkLength(c);

		int idx = c->db->EvictQueue->rear;
		int end = c->db->EvictQueue->front;
		while(idx != end){
		    dictIterator *di;
		    dictEntry *de = c->db->EvictQueue->buf[idx];

		    if(de == NULL){
		    	serverLog(LL_VERBOSE, "QUEUESTATUS COMMAND ENTRY IS NULL(front : %d, rear : %d, idx : %d)"
		    			,c->db->EvictQueue->front, c->db->EvictQueue->rear, idx);
		    }
		    sds key = dictGetKey(de);
		    robj *value = dictGetVal(de);
		    dict *hashdict = (dict *)value->ptr;

		    di = dictGetSafeIterator(hashdict);
		    dictEntry *de2;
		    while((de2 = dictNext(di)) != NULL){
		    	sds field_key = dictGetKey(de2);
		    	serverLog(LL_DEBUG, "GET QUEUE DataKey : %s Field : %s", key ,field_key);
		    	sprintf(str_buf, "[Rear : %d, Front : %d, idx : %d]DataKey : %s Field : %s ]",
		    			c->db->EvictQueue->rear, c->db->EvictQueue->front ,idx,key,field_key);	     	
		    	addReplyBulkCString(c, str_buf);
	     		numkeys++;
		    }
		    idx++;
		 	dictReleaseIterator(di);
		}
     	setDeferredMultiBulkLength(c, replylen, numkeys);
	}
}

/*
 * dequeueCommand
 * Command for checking dequeue operation
 * --- Parameters ---
 *  arg1: dequeueentry
 *  arg2: *
 *
 * --- Usage Examples ---
 *  Command:
 *      redis-cli> dequeueentry *
 *  Results:
 *      Success Case
 *          redis-cli>
 *          OK
 *
 *      Fail Case
 *          redis-cli>
 *          ERR
 *
 */
void dequeueCommand(client *c){
	dictEntry *de = dequeue(c->db->EvictQueue);
	if(de == NULL){
		addReply(c, shared.err);
	}
	else {
		addReply(c, shared.ok);
		}
}


/*
 * chooseBestKeyCommand
 * Command to check chooseBestKeyFromQueue function
 * --- Parameters ---
 *  arg1: evictbestkey
 *  arg2: *
 *
 * --- Usage Examples ---
 *  Command:
 *      redis-cli> evictbestkey *
 *  Results:
 *      Success Case
 *          redis-cli>
 *          1) "[Rear : 0]DataKey : D:{100:2:1}:G:1 Field : 2:2 ]"
 *          2) "[Rear : 0]DataKey : D:{100:2:1}:G:1 Field : 4:2 ]"
 *          3) "[Rear : 0]DataKey : D:{100:2:1}:G:1 Field : 3:2 ]"
 *          4) "[Rear : 0]DataKey : D:{100:2:1}:G:1 Field : 3:1 ]"
 *          5) "[Rear : 0]DataKey : D:{100:2:1}:G:1 Field : 1:2 ]"
 *          6) "[Rear : 0]DataKey : D:{100:2:1}:G:1 Field : 2:1 ]"
 *          7) "[Rear : 0]DataKey : D:{100:2:1}:G:1 Field : 4:1 ]"
 *          8) "[Rear : 0]DataKey : D:{100:2:1}:G:1 Field : 1:1 ]"
 *
 *      Empty Case
 *          redis-cli>
 *          1) "Queue is empty, front : 0, rear : 0"
 *
 */
void chooseBestKeyCommand(client *c){

	char str_buf[1024];

	if(c->db->EvictQueue->front == c->db->EvictQueue->rear){
     	unsigned long numkeys = 0;
     	void *replylen = addDeferredMultiBulkLength(c);
     	sprintf(str_buf, "Queue is empty, front : %d, rear : %d",c->db->EvictQueue->front ,c->db->EvictQueue->rear);
     addReplyBulkCString(c, str_buf);
     numkeys++;
     setDeferredMultiBulkLength(c, replylen, numkeys);

	}
	else {
     	unsigned long numkeys = 0;
     	void *replylen = addDeferredMultiBulkLength(c);

     	dictEntry *de = chooseBestKeyFromQueue(c->db->EvictQueue);
     	if(de != NULL){
     		dictIterator *di;
     		sds key = dictGetKey(de);
     		robj *value = dictGetVal(de);
     		dict *hashdict = (dict *)value->ptr;
     		di = dictGetSafeIterator(hashdict);
     		dictEntry *de2;

		    while((de2 = dictNext(di)) != NULL){
		    	sds field_key = dictGetKey(de2);
		    	serverLog(LL_DEBUG, "BestKey From Queue DataKey : %s Field : %s", key ,field_key);
		    	sprintf(str_buf, "[Rear : %d]DataKey : %s Field : %s ]",
		    			c->db->EvictQueue->rear,key,field_key);
		 		addReplyBulkCString(c, str_buf);
		 		numkeys++;
		    }
		 	dictReleaseIterator(di);
		 	setDeferredMultiBulkLength(c, replylen, numkeys);
     	}
	}
}

/*
 * queueEmptyCommand
 * Command to check initializeQueue function
 * --- Parameters ---
 *  arg1: emptyqueue
 *  arg2: *
 *
 * --- Usage Examples ---
 *  Command:
 *      redis-cli> emptyqueue *
 *  Results:
 *      Success Case
 *          redis-cli>
 *          OK
 *
 */
void queueEmptyCommand(client *c){
	int j =0;

    for (j = 0; j < server.dbnum; j++){

    	redisDb *db = server.db + j;
    	initializeQueue(db->EvictQueue);
    	initializeQueue(db->FreeQueue);
   }
	addReply(c, shared.ok);
}



/*
 * serializeCommand
 * Serialize test command to save the value(Vector) in redis to rocksdb
 * --- Parameters ---
 *  arg1: cvserial
 *  arg2: DataKey
 *
 * --- Usage Examples ---
 *  Parameters:
 *
 *          tableId: "100"
 *          partitionInfoId: "2:1"
 *          rowgroup: 1
 *          field: "1:1" to "column_id:columnVector_id"
 *          value: 1,2,3...
 *  Command:
 *      redis-cli> cvserial D:{100:2:1}:G:1
 *  Results:
 *      Success Case
 *          redis-cli>
 *          1) "Datakey : D:{100:2:1}:G:1, Field : 4:2, SerializeVector : V:{T:1:N:3}:D:[4:5:6]"
 *          2) "Datakey : D:{100:2:1}:G:1, Field : 2:1, SerializeVector : V:{T:1:N:3}:D:[1:2:3]"
 *          3) "Datakey : D:{100:2:1}:G:1, Field : 1:1, SerializeVector : V:{T:1:N:3}:D:[1:2:3]"
 *          4) "Datakey : D:{100:2:1}:G:1, Field : 1:2, SerializeVector : V:{T:1:N:3}:D:[4:5:6]"
 *          5) "Datakey : D:{100:2:1}:G:1, Field : 2:2, SerializeVector : V:{T:1:N:3}:D:[4:5:6]"
 *          6) "Datakey : D:{100:2:1}:G:1, Field : 4:1, SerializeVector : V:{T:1:N:3}:D:[1:2:3]"
 *          7) "Datakey : D:{100:2:1}:G:1, Field : 3:2, SerializeVector : V:{T:1:N:3}:D:[4:5:6]"
 *          8) "Datakey : D:{100:2:1}:G:1, Field : 3:1, SerializeVector : V:{T:1:N:3}:D:[1:2:3]"
 *
 *      Non-exist Case
 *          redis-cli>
 *          (nil)
 */
void serializeCommand(client *c){

	  sds pattern = sdsnew(c->argv[1]->ptr);

    robj *hashdict = lookupSDSKeyFordict(c->db, pattern);


    dictEntry *de;

    if(hashdict == NULL){
    	 addReply(c, shared.nullbulk);
    }
    else {

     	char str_buf[1024];
     	unsigned long numkeys = 0;
     	void *replylen = addDeferredMultiBulkLength(c);

     	dict *hashdictObj = (dict *) hashdict->ptr;
     	dictIterator *di;
     	di = dictGetSafeIterator(hashdictObj);

     	while((de = dictNext(di)) != NULL){

     		sds key = dictGetKey(de);
     		robj *vectorObj = dictGetVal(de);
     		Vector *v = (Vector *)vectorObj->ptr;
     		int vector_type = v->type;
     		int vector_count = v->count;
     		int i=0;

     		sds serial_buf = sdscatfmt(sdsempty(), "%s{%s%i:%s%i}:%s:%s",RELMODEL_VECTOR_PREFIX, RELMODEL_VECTOR_TYPE_PREFIX,
     				vector_type, RELMODEL_VECTOR_COUNT_PREFIX, vector_count, RELMODEL_DATA_PREFIX,VECTOR_DATA_PREFIX);
     		for(i=0; i< vector_count; i++){
     			sds element = (sds) vectorGet(v, i);
     			serial_buf = sdscatsds(serial_buf, element);
     			if (i < vector_count -1) {
     				serial_buf =  sdscat(serial_buf, RELMODEL_DELIMITER);
     			}
     		}
     	  serial_buf =  sdscat(serial_buf, VECTOR_DATA_SUFFIX);
     		sprintf(str_buf, "Datakey : %s, Field : %s, SerializeVector : %s",
     				pattern, key, serial_buf);
     		addReplyBulkCString(c, str_buf);
     		numkeys++;

     		serverLog(LL_VERBOSE, "DATAKEY : %s , Field : %s, serial_string : %s",
     				pattern, key, serial_buf);


     	}


     	sdsfree(pattern);
     	decrRefCount(pattern);
     	setDeferredMultiBulkLength(c, replylen, numkeys);
    }
}

/*
 * deserializeCommand
 * deSerialize test command to check the operation to deserialize data which is stored in rocksdb
 * and make it a vector
 * --- Parameters ---
 *  arg1: cvdeserial
 *  arg2: RocksdbDataKey
 *
 * --- Usage Examples ---
 *  Parameters:
 *
 *          tableId: "100"
 *          partitionInfoId: "2:1"
 *          rowgroup: 1
 *          field: "1:1" to "column_id:columnVector_id"
 *          serialized data: V:{T:1:N:3}:D:[1:2:3]
 *  Command:
 *      redis-cli> cvdeserial D:{100:2:1}:G:1:F:1:1
 *  Results:
 *      Success Case
 *          redis-cli>
 *          1) "VECTOR[0], Value : 1"
 *          2) "VECTOR[1], Value : 2"
 *          3) "VECTOR[2], Value : 3"
 *
 *      Error Case
 *          redis-cli>
 *          ERR
 */
void deserializeCommand(client *c){

	sds pattern = sdsnew(c->argv[1]->ptr);

	  char* err = NULL;
	  size_t val_len;
	  char* val;
	  val = rocksdb_get_cf(c->db->persistent_store->ps, c->db->persistent_store->ps_options->roptions,
			  c->db->persistent_store->ps_cf_handles[PERSISTENT_STORE_CF_RW], pattern, sdslen(pattern), &val_len, &err);

	  if(val == NULL){
		  rocksdb_free(val);
		  serverLog(LL_VERBOSE, "ROCKSDB KEY-VALUE NOT EXIST");
			sdsfree(pattern);
			addReply(c, shared.err);
	  }
	  else {

		  char str_buf[1024];
		  unsigned long numkeys = 0;
		  void *replylen = addDeferredMultiBulkLength(c);

		  serverLog(LL_DEBUG, "VALUE STRING : %s", val);
		  Vector *v = VectordeSerialize(val);
		  int count = v->count;

		  int i =0;
		  for(i=0; i < count ; i++){

			  sprintf(str_buf, "VECTOR[%d], Value : %s", i, vectorGet(v, i));
	     		addReplyBulkCString(c, str_buf);
	     		numkeys++;
		  }

		  rocksdb_free(val);
		  sdsfree(pattern);
			vectorFreeDeep(v);
			zfree(v);
		  setDeferredMultiBulkLength(c, replylen, numkeys);
	  }

}


void deleteEntryCommand(client *c){

}

