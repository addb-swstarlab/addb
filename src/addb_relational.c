#include "addb_relational.h"
#include "sds.h"
#include "util.h"
#include <math.h>
#include <assert.h>

/*
 * 2018.3.23
 * hanseung sung
 * Parsing Information about datakey
 */



// addb Parsing Key information from arg[1]
NewDataKeyInfo * parsingDataKeyInfo(sds dataKeyString){

  assert(dataKeyString != NULL);
  serverLog(LL_DEBUG,"Create DATAKEYINFO START");

  NewDataKeyInfo *ret = zcalloc(sizeof(NewDataKeyInfo));
  char copyStr[MAX_TMPBUF_SIZE];
  char* saveptr = NULL;
  char* token = NULL;
  size_t size = sdslen(dataKeyString) + 1;

  assert( size == (strlen(dataKeyString ) + 1));
  assert( size < MAX_TMPBUF_SIZE);

  memcpy(copyStr, dataKeyString, size);

  //parsing the prefix
  if ((token = strtok_r(copyStr, RELMODEL_DELIMITER, &saveptr)) == NULL){
	  serverLog(LL_WARNING, "Fatal: DataKey broken Error: [%s]", dataKeyString);
	  zfree(ret);
	  return NULL;
  }
  /*skip IDX prefix ("I") */
  if (strcasecmp(token, RELMODEL_INDEX_PREFIX) == 0 ) {
	  // skip idxType field
	  strtok_r(NULL, RELMODEL_DELIMITER, &saveptr);
  }
  // parsing tableId
  if ((token = strtok_r(NULL, RELMODEL_DELIMITER, &saveptr)) == NULL){
	  serverLog(LL_WARNING, "Fatal: DataKey broken Error: [%s]", dataKeyString);
	  zfree(ret);
	  return NULL;
  }
  ret->tableId = atoi(token + strlen(RELMODEL_BRACE_PREFIX));
  //parsing partitionInfo
  if ((token = strtok_r(NULL, RELMODEL_BRACE_SUFFIX, &saveptr)) == NULL){
	  serverLog(LL_WARNING, "Fatal: DataKey broken Error: [%s]", dataKeyString);
	  zfree(ret);
	  return NULL;
  }
  size_t part_size = strlen(token)+1;
  assert(part_size <= PARTITION_KEY_MAX_SIZE);
  memcpy(ret->partitionInfo.partitionString, token, part_size);
  ret->isPartitionString = true;

  //parsing rowgroup number
  if (saveptr != NULL && (token = strtok_r(NULL, RELMODEL_ROWGROUPID_PREFIX, &saveptr)) != NULL)
	  ret->rowGroupId = atoi(token);
  else
	  ret->rowGroupId = 0;

  serverLog(LL_DEBUG,"TOKEN : %s, SAVEPTR: %s, table_number : %d, partitionInfo : %s, rowgroup : %d",
              token, saveptr, ret->tableId,ret->partitionInfo.partitionString, ret->rowGroupId);
  return ret;
}

// addb parsing metakey information
MetaKeyInfo *parseMetaKeyInfo(sds metakey) {
    // Metakey format = "M:{tableId:partitionInfo}"
    // example = "M:{100:3:10000:5:3000}"
    assert(metakey != NULL);

    MetaKeyInfo *metaKeyInfo = (MetaKeyInfo *) zmalloc(sizeof(MetaKeyInfo));
    char copyStr[MAX_TMPBUF_SIZE];
    char *savePtr = NULL;
    char *token = NULL;
    size_t size = sdslen(metakey) + 1;

    assert(size == (strlen(metakey) + 1));
    assert(size < MAX_TMPBUF_SIZE);

    memcpy(copyStr, metakey, size);

    // Parse the meta prefix
    if ((token = strtok_r(copyStr, RELMODEL_DELIMITER, &savePtr)) == NULL) {
        serverLog(LL_WARNING, "Fatal: MetaKey broken error: [%s]", metakey);
        zfree(metaKeyInfo);
        return NULL;
    }

    if (strcmp(token, "M") != 0) {
        serverLog(LL_WARNING, "Wrong metakey requested: [%s]", metakey);
        zfree(metaKeyInfo);
        return NULL;
    }

    // Parse tableId
    if ((token = strtok_r(NULL, RELMODEL_DELIMITER, &savePtr)) == NULL) {
        serverLog(LL_WARNING, "Fatal: MetaKey broken error: [%s]", metakey);
        zfree(metaKeyInfo);
        return NULL;
    }
    metaKeyInfo->tableId = atoi(token + strlen(RELMODEL_BRACE_PREFIX));

    // Parse partitionInfo
    if ((token = strtok_r(NULL, RELMODEL_BRACE_SUFFIX, &savePtr)) == NULL) {
        serverLog(LL_WARNING, "Fatal: MetaKey broken error: [%s]", metakey);
        zfree(metaKeyInfo);
        return NULL;
    }
    size_t partitionSize = strlen(token) + 1;
    assert(partitionSize <= PARTITION_KEY_MAX_SIZE);
    memcpy(metaKeyInfo->partitionInfo.partitionString, token, partitionSize);
    metaKeyInfo->isPartitionString = true;

    return metaKeyInfo;
}

/*addb get dictEntry, a candidate for evict*/

dictEntry *getCandidatedictFirstEntry(client *c, NewDataKeyInfo *dataKeyInfo){
	dictEntry *candidateEntry = NULL;
	robj *dataKey = generateDataKeyForFirstEntry(dataKeyInfo);
	serverLog(LL_DEBUG, "getCandidatedictEntry Find DATAKEY : %s", (char *)dataKey->ptr);

	candidateEntry = dictFind(c->db->dict, dataKey->ptr);
	robj *hashDict = dictGetVal(candidateEntry);
	if(hashDict->type != OBJ_HASH){
		serverLog(LL_NOTICE, "CandidateEntry's value is not Hash");
	}
	decrRefCount(dataKey);
	return candidateEntry;
}

dictEntry *getCandidatedictEntry(client *c, NewDataKeyInfo *dataKeyInfo){

	dictEntry *candidateEntry = NULL;
	robj *dataKey = generateDataKey(dataKeyInfo);
	serverLog(LL_DEBUG, "getCandidatedictEntry Find DATAKEY : %s", (char *)dataKey->ptr);

	candidateEntry = dictFind(c->db->dict, dataKey->ptr);
	robj *hashDict = dictGetVal(candidateEntry);
	if(hashDict->type != OBJ_HASH){
		serverLog(LL_NOTICE, "CandidateEntry's value is not Hash");
	}
	decrRefCount(dataKey);
	return candidateEntry;
}

dictEntry *getPrevCandidatedictEntry(client *c, NewDataKeyInfo *dataKeyInfo){

	dictEntry *candidateEntry = NULL;
	robj *dataKey = generatePrevDataKey(dataKeyInfo);
	serverLog(LL_DEBUG, "getPrevCandidatedictEntry Find DATAKEY : %s", (char *)dataKey->ptr);

	candidateEntry = dictFind(c->db->dict, dataKey->ptr);
	robj *hashDict = dictGetVal(candidateEntry);
	if(hashDict->type != OBJ_HASH){
		serverLog(LL_NOTICE, "CandidateEntry's value is not Hash");
	}
	decrRefCount(dataKey);
	return candidateEntry;
}


/*addb get RowNumberInfo from Metadict*/
int getRowNumberInfoAndSetRowNumberInfo(redisDb *db, NewDataKeyInfo *dataKeyInfo){
	char tmp[SDS_DATA_KEY_MAX];
	int rowNumber= 0;
	sds metaKey = sdsnewlen("", SDS_DATA_KEY_MAX);// sdsnewlen(tmp, sizeof(tmp) //sdsnew(tmp) //sdsIntialize(tmp, sizeof(tmp));
	setMetaKeyForRowgroup(dataKeyInfo, metaKey);

	robj *metaHashdictObj = lookupSDSKeyForMetadict(db, metaKey);
	if(metaHashdictObj == NULL){
		serverLog(LL_DEBUG, "METAHASHDICT OBJ IS NULL");
	}
	else{
		serverLog(LL_DEBUG, "METAHASHDICT OBJ IS NOTNULL");
	}

	robj *metaField  = createStringObjectFromLongLong((long long) dataKeyInfo->rowGroupId);
	rowNumber = lookupCompInfoForRowNumberInMeta(metaHashdictObj, metaField);
  dataKeyInfo->row_number = rowNumber;
	decrRefCount(metaField);
    sdsfree(metaKey);
	return rowNumber;
}


/*addb get RowgroupInfo from Metadict*/
int getRowGroupInfoAndSetRowGroupInfo(redisDb *db, NewDataKeyInfo *dataKeyInfo){
	char tmp[SDS_DATA_KEY_MAX];
	int rowgroup = 0;
	sds metaKey = sdsnewlen("", SDS_DATA_KEY_MAX);
	setMetaKeyForRowgroup(dataKeyInfo, metaKey);

	robj *metaHashdictObj = lookupSDSKeyForMetadict(db, metaKey);
	if(metaHashdictObj == NULL){
		serverLog(LL_DEBUG, "METAHASHDICT OBJ IS NULL");
	}
	else{
		serverLog(LL_DEBUG, "METAHASHDICT OBJ IS NOTNULL");
	}

	robj *metaField = shared.integers[0];
	rowgroup = lookupCompInfoForMeta(metaHashdictObj, metaField);
	dataKeyInfo->rowGroupId = rowgroup;
    sdsfree(metaKey);
	return rowgroup;
}


int getRowgroupInfo(redisDb *db, NewDataKeyInfo *dataKeyInfo){

    int rowgroup = getRowGroupInfoAndSetRowGroupInfo(db,dataKeyInfo);
    if(rowgroup == 0){
    	 serverLog(LL_DEBUG, "ROWGROUP 0 CASE OCCUR");
    	rowgroup = IncRowgroupIdAndModifyInfo(db, dataKeyInfo, 1);

    }
    return rowgroup;
}


int lookupCompInfoForRowNumberInMeta(robj *metaHashdictObj,robj* metaField){

    if (metaHashdictObj == NULL){
        return 0;
    }
    robj *decodedField = getDecodedObject(metaField);
    int retVal = 0;
    robj *ret = hashTypeGetValueObject(metaHashdictObj, (sds) decodedField->ptr);
    if(ret == NULL){
        decrRefCount(decodedField);
        return 0;
    }
    if (!sdsEncodedObject(ret)) {
        retVal = (int) (long) ret->ptr;
    } else {
        retVal = atoi((char *) ret->ptr);
    }
    decrRefCount(decodedField);
    return retVal;
}

int lookupCompInfoForMeta(robj *metaHashdictObj,robj* metaField){

    if (metaHashdictObj == NULL){
        return 0;
    }
    robj *decodedField = getDecodedObject(metaField);
    int retVal = 0;
    robj *ret = hashTypeGetValueObject(metaHashdictObj, (sds) decodedField->ptr);

    if (!sdsEncodedObject(ret)) {
        retVal = (int) (long) ret->ptr;
    } else {
        retVal = atoi((char *) ret->ptr);
    }
    decrRefCount(decodedField);
    return retVal;
}


void setMetaKeyForRowgroup(NewDataKeyInfo *dataKeyInfo, sds key){

	/*Typecheck PartitionInfo*/
	if(dataKeyInfo->isPartitionString != true) {
		unsigned long long buf[PARTITION_KEY_MAX_CNT];
		memcpy(buf, dataKeyInfo->partitionInfo.partitionInt, sizeof(unsigned long long) * dataKeyInfo->partitionCnt);


		char *ptr = (char *) dataKeyInfo->partitionInfo.partitionString;
		for (int i = 0; i < dataKeyInfo->partitionCnt; i++) {
			if (i != (dataKeyInfo->partitionCnt - 1))
				ptr += sprintf(ptr, "%llu:", buf[i]);
			else
				ptr += sprintf(ptr, "%llu", buf[i]);
		}
		int partitionSize = ptr - dataKeyInfo->partitionInfo.partitionString + 1;	//NULL character
		assert(partitionSize <= PARTITION_KEY_MAX_SIZE);
		dataKeyInfo->isPartitionString = true;
	}

	size_t keySize = sprintf(key, "%s:{%d:%s}",
			RELMODEL_META_PREFIX, dataKeyInfo->tableId, dataKeyInfo->partitionInfo.partitionString);
	assert(keySize < DATA_KEY_MAX_SIZE);
	sdsupdatelen(key);
}


/*addb INC,DEC FUNCTION*/

int IncRowgroupIdAndModifyInfo(redisDb *db, NewDataKeyInfo *dataKeyInfo, int cnt){
	int result = incRowgroupId(db, dataKeyInfo, cnt);
	dataKeyInfo->rowGroupId = result;
	return result;

}

int incRowgroupId(redisDb *db, NewDataKeyInfo *dataKeyInfo, int inc_number){
	robj *rowgroupKey= generateRgIdKeyForRowgroup(dataKeyInfo);
	robj *metaRgField = shared.integers[0];

	int ret = IncDecCount(db, rowgroupKey, metaRgField, (long long) inc_number);
	decrRefCount(rowgroupKey);
	return ret;
}

int incRowNumber(redisDb *db, NewDataKeyInfo *dataKeyInfo, int inc_number){
	/*check rowgroup info*/
	if(dataKeyInfo->rowGroupId == 0)
		assert(0);
	robj *rowgroupKey = generateRgIdKeyForRowgroup(dataKeyInfo);
	robj *metaRgField = createStringObjectFromLongLong((long long) dataKeyInfo->rowGroupId);
	int ret = IncDecCount(db, rowgroupKey, metaRgField, (long long) inc_number);
	decrRefCount(rowgroupKey);
	decrRefCount(metaRgField);
	return ret;
}


//TO-DO MODIFY LATER -HS
int IncDecCount(redisDb *db, robj *key, robj *field, long long cnt){  // int flags ??

    long long value, oldvalue;
    robj *o, *new, *cur_obj;

    o = lookupKeyWriteForMetadict(db,key);
    if(o == NULL){

    	o = createHashObject();  //ziplist object
    	dbAddForMetadict(db, key, o);
    }
    else{

    	if(o->type != OBJ_HASH){
    		serverLog(LL_ERROR, "The value of the Metadict, hashdictobj's type, must be hash ");
    		assert(0);
    	}
    }

    if((cur_obj = hashTypeGetValueRObject(o, field)) != NULL){
    	if (getLongLongFromObject(cur_obj, &value) != C_OK) {
    		serverLog(LL_ERROR, "the value of meta must be INTEGER");
    		assert(0);
		}
		decrRefCount(cur_obj);
	}
    else{
    	value = 0;
    }
    oldvalue = value;
    if ((cnt < 0 && oldvalue < 0 && cnt < (LLONG_MIN-oldvalue)) ||
        (cnt > 0 && oldvalue > 0 && cnt > (LLONG_MAX-oldvalue))) {
    	serverLog(LL_ERROR,"increment or decrement would overflow");
    	assert(0);
    }
    value += cnt;

    new = createStringObjectFromLongLong(value);
    hashTypeTryObjectEncoding(o,&field,NULL);
    hashTypeSetWithNoFlags(o, field, new);
    decrRefCount(new);

    signalModifiedKey(db,key);
    notifyKeyspaceEvent(NOTIFY_HASH,"Hash incrby",key,db->id);
    server.dirty++;

    return value;
}


/*addb key generation func*/

robj * generateRgIdKeyForRowgroup(NewDataKeyInfo *dataKeyInfo){

	char rgKey[DATA_KEY_MAX_SIZE];
	if(dataKeyInfo->isPartitionString){
		sprintf(rgKey, "%s:{%d:%s}", RELMODEL_META_PREFIX, dataKeyInfo->tableId, dataKeyInfo->partitionInfo.partitionString);
	}else{
		char *p = (char *)rgKey;
		sprintf(p, "%s:{%d", RELMODEL_META_PREFIX, dataKeyInfo->tableId);
		p += strlen(RELMODEL_DATA_PREFIX);
		p += 2; // for :{
		p += digits10(dataKeyInfo->tableId);
		for(int i=0;i<dataKeyInfo->partitionCnt;i++){
			sprintf(p, ":%llu", dataKeyInfo->partitionInfo.partitionInt[i]);
			p += digits10(dataKeyInfo->partitionInfo.partitionInt[i]);
			p += 1; //for :
		}
		sprintf(p, "}");
	}
	serverLog(LL_DEBUG, "RowGroup KEY :  %s", (char *)rgKey);
	return createStringObject(rgKey, strlen(rgKey));
}


robj *generateDataKey(NewDataKeyInfo *dataKeyInfo) {
    sds dataKeySds = generateDataKeySds(dataKeyInfo);
    robj *dataKeyObj = createStringObject(dataKeySds, sdslen(dataKeySds));
    sdsfree(dataKeySds);
    return dataKeyObj;
}

sds generateDataKeySds(NewDataKeyInfo *dataKeyInfo) {
    char dataKey[DATA_KEY_MAX_SIZE];
	if(dataKeyInfo->isPartitionString){
		sprintf(dataKey, "%s:{%d:%s}:%s%d",
				RELMODEL_DATA_PREFIX, dataKeyInfo->tableId, dataKeyInfo->partitionInfo.partitionString, RELMODEL_ROWGROUPID_PREFIX,
				dataKeyInfo->rowGroupId);
	}else{
		char *p = (char *)dataKey;
		sprintf(p, "%s:{%d", RELMODEL_DATA_PREFIX, dataKeyInfo->tableId);
		p += strlen(RELMODEL_DATA_PREFIX);
		p += 2; // for :{
		p += digits10(dataKeyInfo->tableId);
		for(int i=0;i<dataKeyInfo->partitionCnt;i++){
			sprintf(p, ":%llu", dataKeyInfo->partitionInfo.partitionInt[i]);
			p += digits10(dataKeyInfo->partitionInfo.partitionInt[i]);
			p += 1; //for :
		}
		sprintf(p, "}:%s%d", RELMODEL_ROWGROUPID_PREFIX, dataKeyInfo->rowGroupId);
	}
	serverLog(LL_DEBUG, "DATAKEY :  %s", (char *)dataKey);
    return sdsnew(dataKey);
}

robj *generateDataRocksKey(NewDataKeyInfo *dataKeyInfo, int rowId,
                           int columnId) {
    sds dataFieldKeySds = generateDataRocksKeySds(dataKeyInfo, rowId, columnId);
    robj *dataFieldKeyObj = createStringObject(dataFieldKeySds,
                                               sdslen(dataFieldKeySds));
    sdsfree(dataFieldKeySds);
    return dataFieldKeyObj;
}

sds generateDataRocksKeySds(NewDataKeyInfo *dataKeyInfo, int rowId,
                            int columnId) {
    sds dataKey = generateDataKeySds(dataKeyInfo);
    sds fieldKey = getDataFieldSds(rowId, columnId);

    char strBuf[DATA_KEY_MAX_SIZE];
    sprintf(strBuf, "%s:%s%s", dataKey, REL_MODEL_FIELD_PREFIX, fieldKey);
    sdsfree(dataKey);
    sdsfree(fieldKey);
    return sdsnew(strBuf);
}

robj * generateDataKeyForFirstEntry(NewDataKeyInfo *dataKeyInfo){

	char dataKey[DATA_KEY_MAX_SIZE];
	int FirstRgid = 1;
	if(dataKeyInfo->isPartitionString){
		sprintf(dataKey, "%s:{%d:%s}:%s%d",
				RELMODEL_DATA_PREFIX, dataKeyInfo->tableId, dataKeyInfo->partitionInfo.partitionString, RELMODEL_ROWGROUPID_PREFIX,
				FirstRgid);
	}else{
		char *p = (char *)dataKey;
		sprintf(p, "%s:{%d", RELMODEL_DATA_PREFIX, dataKeyInfo->tableId);
		p += strlen(RELMODEL_DATA_PREFIX);
		p += 2; // for :{
		p += digits10(dataKeyInfo->tableId);
		for(int i=0;i<dataKeyInfo->partitionCnt;i++){
			sprintf(p, ":%llu", dataKeyInfo->partitionInfo.partitionInt[i]);
			p += digits10(dataKeyInfo->partitionInfo.partitionInt[i]);
			p += 1; //for :
		}
		sprintf(p, "}:%s%d", RELMODEL_ROWGROUPID_PREFIX, FirstRgid);
	}
	serverLog(LL_DEBUG, "DATAKEY :  %s", (char *)dataKey);
	return createStringObject(dataKey, strlen(dataKey));
}

robj * generatePrevDataKey(NewDataKeyInfo *dataKeyInfo){

	char dataKey[DATA_KEY_MAX_SIZE];
	int prevRgId = dataKeyInfo->rowGroupId-1;
	if(dataKeyInfo->isPartitionString){
		sprintf(dataKey, "%s:{%d:%s}:%s%d",
				RELMODEL_DATA_PREFIX, dataKeyInfo->tableId, dataKeyInfo->partitionInfo.partitionString, RELMODEL_ROWGROUPID_PREFIX,
				prevRgId);
	}else{
		char *p = (char *)dataKey;
		sprintf(p, "%s:{%d", RELMODEL_DATA_PREFIX, dataKeyInfo->tableId);
		p += strlen(RELMODEL_DATA_PREFIX);
		p += 2; // for :{
		p += digits10(dataKeyInfo->tableId);
		for(int i=0;i<dataKeyInfo->partitionCnt;i++){
			sprintf(p, ":%llu", dataKeyInfo->partitionInfo.partitionInt[i]);
			p += digits10(dataKeyInfo->partitionInfo.partitionInt[i]);
			p += 1; //for :
		}
		sprintf(p, "}:%s%d", RELMODEL_ROWGROUPID_PREFIX, prevRgId);
	}
	serverLog(LL_DEBUG, "DATAKEY :  %s", (char *)dataKey);
	return createStringObject(dataKey, strlen(dataKey));
}


/*addb generate datafield string
 * dataField ==> row:column format
 */
robj *getDataField(int row, int column){

	char dataField[DATA_KEY_MAX_SIZE];
	sprintf(dataField, "%d:%d", row, column);
	serverLog(LL_DEBUG, "DATAFIELD :  %s", (char *)dataField);
	return createStringObject(dataField, strlen(dataField));
}

sds getDataFieldSds(int rowId, int columnId) {
    char dataField[DATA_KEY_MAX_SIZE];
    sprintf(dataField, "%d:%d", rowId, columnId);
    return sdsnew(dataField);
}

/*addb Data Insertion func*/

void insertKVpairToRelational(client *c, robj *dataKeyString, robj *dataField, robj *valueObj){

	assert(dataKeyString != NULL);
	assert(dataField != NULL);

	robj *dataHashdictObj = NULL;

	if( (dataHashdictObj = lookupDictAndGetHashdictObj(c,dataKeyString)) == NULL ){

		serverLog(LL_WARNING, "Can't Find dataHashdict in dict, Because of Creation Error");
		serverPanic("insertKVpairToRelational ERROR");
	}

	dict *hashDict = (dict *)dataHashdictObj->ptr;
	dictEntry *de = NULL;

	if((de = dictFind(hashDict, dataField->ptr)) == NULL){

		int ret = dictAdd(hashDict, sdsdup(dataField->ptr), sdsdup(valueObj->ptr));

		if(!ret){
			serverLog(LL_DEBUG, "DATA INSERTION SUCCESS. dataKey : %s, dataField : %s, value :%s"
					, (char *)dataKeyString->ptr, (char*)dataField->ptr, (char *)valueObj->ptr);
		}
		else {
			serverLog(LL_WARNING, "DATA INSERTION FAIL");
			serverPanic("DATA INSERTION ERROR in insertKVpairToRelational");
		}
	}
	notifyKeyspaceEvent(NOTIFY_HASH,"hset", dataKeyString,c->db->id);
	server.dirty++;

}


/* ADDB Create Scan parameter*/
ColumnParameter *parseColumnParameter(const sds rawColumnIdsString) {
    ColumnParameter *param = (ColumnParameter *) zmalloc(
            sizeof(ColumnParameter));
    param->original = sdsnew(rawColumnIdsString);

    vectorTypeInit(&param->columnIdStrList, STL_TYPE_SDS);
    vectorTypeInit(&param->columnIdList, STL_TYPE_LONG);

    int tokenCounts;
    sds *tokens = sdssplit(param->original, RELMODEL_COLUMN_DELIMITER,
                           &tokenCounts);

    if (tokens == NULL || tokenCounts == 0) {
        serverLog(LL_WARNING, "Fatal: column parameter is broken: no data");
        serverPanic("column parameter is broken");
    }

    for (int i = 0; i < tokenCounts; ++i) {
        // TODO(totoro): Checks that column ID is valid.
        vectorAdd(&param->columnIdStrList, sdsdup(tokens[i]));
        vectorAdd(&param->columnIdList, (void *) (long) atoi(tokens[i]));
    }

    param->columnCount = vectorCount(&param->columnIdList);
    sdsfreesplitres(tokens, tokenCounts);
    return param;
}

ScanParameter *createScanParameter(const client *c) {
    ScanParameter *param = (ScanParameter *) zmalloc(sizeof(ScanParameter));
    param->startRowGroupId = 0;
    param->dataKeyInfo = parsingDataKeyInfo((sds) c->argv[1]->ptr);
    param->totalRowGroupCount = getRowGroupInfoAndSetRowGroupInfo(
            c->db, param->dataKeyInfo);
    param->rowGroupParams = (RowGroupParameter *) zmalloc(
            sizeof(RowGroupParameter) * param->totalRowGroupCount);
    param->columnParam = parseColumnParameter((sds) c->argv[2]->ptr);
    return param;
}

void freeColumnParameter(ColumnParameter *param) {
    sdsfree(param->original);
    vectorFreeDeep(&param->columnIdList);
    vectorFreeDeep(&param->columnIdStrList);
    zfree(param);
}

void freeScanParameter(ScanParameter *param) {
    zfree(param->dataKeyInfo);
    zfree(param->rowGroupParams);
    freeColumnParameter(param->columnParam);
    zfree(param);
}

/*
 * populateScanParameter
 * - Description
 *      Populates row group data to scan parameter by looking up MetaDict
 * - Return
 *      Returns total data count scaned by scan parameter.
 */
int populateScanParameter(redisDb *db, ScanParameter *scanParam) {
    int totalDataCount = 0;

    for (size_t i = 0; i < scanParam->totalRowGroupCount; ++i) {
        int rowGroupId = i + 1;
        scanParam->dataKeyInfo->rowGroupId = rowGroupId;
        robj *dataKey = generateDataKey(scanParam->dataKeyInfo);
        scanParam->rowGroupParams[i] = createRowGroupParameter(db, dataKey);

        int rowCount = getRowNumberInfoAndSetRowNumberInfo(
               db, scanParam->dataKeyInfo);
        scanParam->rowGroupParams[i].rowCount = rowCount;
        totalDataCount += scanParam->columnParam->columnCount * rowCount;
        decrRefCount(dataKey);
    }
    return totalDataCount;
}

RowGroupParameter createRowGroupParameter(redisDb *db, robj *dataKey) {
    RowGroupParameter param;
    expireIfNeeded(db, dataKey);
    param.dictObj = lookupKey(db, dataKey, LOOKUP_NONE);

    if (
            param.dictObj == NULL ||
            param.dictObj->location == LOCATION_PERSISTED
       ) {
        param.isInRocksDb = true;
    } else {
        param.isInRocksDb = false;
    }

    return param;
}

void scanDataFromADDB(redisDb *db, ScanParameter *scanParam, Vector *data) {
    size_t startRowGroupIdx = scanParam->startRowGroupId;
    RowGroupParameter *rowGroupParams = scanParam->rowGroupParams;
    ColumnParameter *columnParam = scanParam->columnParam;

    for (size_t i = startRowGroupIdx; i < scanParam->totalRowGroupCount; ++i) {
        size_t rowGroupId = i + 1;
        scanParam->dataKeyInfo->rowGroupId = rowGroupId;

        // Scan on RocksDB.
        if (rowGroupParams[i].isInRocksDb) {
            serverLog(LL_VERBOSE, "[DEBUG][scanDataFromADDB] Scan on RocksDB");
            scanDataFromRocksDB(db, scanParam->dataKeyInfo, columnParam,
                                rowGroupParams[i], data);
            continue;
        }

        // Scan on Redis.
        robj *hashDictObj = rowGroupParams[i].dictObj;
        dict *hashDict = (dict *) hashDictObj->ptr;
        for (size_t j = 0; j < rowGroupParams[i].rowCount; ++j) {
            size_t rowId = j + 1;
            for (size_t k = 0; k < columnParam->columnCount; ++k) {
                size_t columnId = (long) vectorGet(&columnParam->columnIdList,
                                                   k);
                // Data field key (Row & Column pair)
                // Ex) "1:2"
                sds dataFieldKey = getDataFieldSds(rowId, columnId);
                dictEntry *entry = dictFind(hashDict, dataFieldKey);
                sds value = dictGetVal(entry);
                vectorAdd(data, (void *) value);
                sdsfree(dataFieldKey);
            }
        }
    }
}

void scanDataFromRocksDB(redisDb *db, NewDataKeyInfo *dataKeyInfo,
                         ColumnParameter *columnParam,
                         RowGroupParameter rowGroupParam, Vector *data) {
    for (size_t j = 0; j < rowGroupParam.rowCount; ++j) {
        size_t rowId = j + 1;
        for (size_t k = 0; k < columnParam->columnCount; ++k) {
            size_t columnId = (long) vectorGet(&columnParam->columnIdList, k);

            // Data rocks DB key (DataKey + DataField)
            // Ex) "D:{100:1:2}:G:1:F:1:2"
            sds dataRocksKey = generateDataRocksKeySds(dataKeyInfo, rowId,
                                                       columnId);
            char *err;
            size_t valueLen = 0;
            char *value;
            value = rocksdb_get_cf(
                    db->persistent_store->ps,
                    db->persistent_store->ps_options->roptions,
                    db->persistent_store->ps_cf_handles[PERSISTENT_STORE_CF_RW],
                    dataRocksKey, sdslen(dataRocksKey), &valueLen, &err);

            if (value == NULL) {
                serverLog(
                        LL_DEBUG,
                        "[DEBUG][scanDataFromRocksDB] Key: %s, value is not exist.",
                        dataRocksKey);
                sdsfree(dataRocksKey);
                continue;
            }

            vectorAdd(data, sdsnewlen(value, valueLen));
            rocksdb_free(value);
            sdsfree(dataRocksKey);
        }
    }
}

// TODO(totoro): Implements validateStatements function by regex.
bool validateStatements(const sds rawStatementsStr) {
    return true;
}

// TODO(totoro): Implements validateStatement function by regex.
bool validateStatement(const sds rawStatementStr) {
    return true;
}

int parseStatement(const sds rawStatementStr, Condition **root) {
    char copyStr[MAX_TMPBUF_SIZE];
    char *savePtr = NULL;
    char *token = NULL;
    memcpy(copyStr, rawStatementStr, sdslen(rawStatementStr) + 1);

    Stack s;
    stackInit(&s);

    // Parses raw conditions by condition unit.
    token = strtok_r(copyStr, PARTITION_FILTER_OPERATOR_DELIMITER, &savePtr);
    while (token != NULL) {
        Condition *cond;
        if (createCondition(token, &s, &cond) == C_ERR) {
            serverLog(LL_DEBUG,
                      "[FILTER][PARSE] Input Condition is invalid form: %s",
                      token);
            return C_ERR;
        }
        stackPush(&s, (void *) cond);
        token = strtok_r(NULL, PARTITION_FILTER_OPERATOR_DELIMITER, &savePtr);
    }

    // Last token must be NULL.
    // Number of stack element must be 1.
    if (token != NULL || stackCount(&s) != 1) {
        serverLog(LL_DEBUG,
                  "[FILTER][PARSE] Entire condition is invalid form: lastToken[%s], stackCount[%zu]",
                  token, stackCount(&s));
        return C_ERR;
    }

    *root = stackPop(&s);
    stackFree(&s);

    return C_OK;
}

// TODO(totoro): Needs to find and fix memory leak at this function. There are
// 8 byte memory leak...
int createCondition(const char *rawConditionStr, Stack *s,
                    Condition **cond) {
    char copyStr[MAX_TMPBUF_SIZE];
    char *savePtr = NULL;
    char *token = NULL;
    memcpy(copyStr, rawConditionStr, strlen(rawConditionStr) + 1);
    serverLog(LL_DEBUG, "[FILTER][PARSE] raw condition: %s", rawConditionStr);

    // Parses raw condition by operand unit.
    Condition *newcond = zmalloc(sizeof(Condition));
    token = strtok_r(copyStr, PARTITION_FILTER_OPERAND_DELIMITER, &savePtr);
    int leafOperandCount = 0;
    while (savePtr[0] != '\0') {
        ConditionChild *child = (ConditionChild *) zmalloc(
                sizeof(ConditionChild));
        long tokenLong;
        if (string2l(token, strlen(token), &tokenLong) == 1) {
            child->type = CONDITION_CHILD_VALUE_TYPE_LONG;
            child->value.l = tokenLong;
        } else {
            child->type = CONDITION_CHILD_VALUE_TYPE_SDS;
            child->value.s = sdsnew(token);
        }
        stackPush(s, (void *) child);
        leafOperandCount++;
        token = strtok_r(NULL, PARTITION_FILTER_OPERAND_DELIMITER, &savePtr);
    }
    serverLog(LL_DEBUG, "[FILTER][PARSE] condition operator: %s", token);

    int optype = _getOperatorType(token);
    if (optype == -1) {
        serverLog(LL_WARNING, "[FILTER][PARSE][FATAL] Invalid Operator '%s'",
                  token);
        serverPanic("[FILTER][PARSE][FATAL] Invalid Operator");
        return C_ERR;
    }
    newcond->op = optype;
    newcond->opCount = _getOperatorOperandCount(optype);

    if (leafOperandCount > 0) {
        if (leafOperandCount != newcond->opCount) {
            // Invalid condition format detected...
            return C_ERR;
        }

        newcond->isLeaf = true;
        newcond->first = (ConditionChild *) stackPop(s);
        if (newcond->first->type != CONDITION_CHILD_VALUE_TYPE_LONG) {
            // Invalid condition format detected...
            return C_ERR;
        }
        newcond->second = NULL;
        if (newcond->opCount == 2) {
            newcond->second = (ConditionChild *) stackPop(s);
        }
    } else {
        if (newcond->opCount > (int) stackCount(s)) {
            // Invalid condition format detected...
            return C_ERR;
        }

        newcond->isLeaf = false;

        ConditionChild *child = (ConditionChild *) zmalloc(
                sizeof(ConditionChild));
        child->type = CONDITION_CHILD_VALUE_TYPE_COND;
        child->value.cond = (Condition *) stackPop(s);
        newcond->first = child;

        newcond->second = NULL;
        if (newcond->opCount == 2) {
            ConditionChild *child = (ConditionChild *) zmalloc(
                    sizeof(ConditionChild));
            child->type = CONDITION_CHILD_VALUE_TYPE_COND;
            child->value.cond = (Condition *) stackPop(s);
            newcond->second = child;
        }
    }
    *cond = newcond;

    return C_OK;
}

int _parsePartitions(const char *partitionInfo, Vector *v) {
    char copyStr[MAX_TMPBUF_SIZE];
    char *savePtr = NULL;
    char *token = NULL;
    size_t size = strlen(partitionInfo) + 1;

    assert(size < MAX_TMPBUF_SIZE);
    memcpy(copyStr, partitionInfo, size);

    token = strtok_r(copyStr, RELMODEL_DELIMITER, &savePtr);
    while (savePtr[0] != '\0') {
        PartitionParameter *param = (PartitionParameter *) zmalloc(
                sizeof(PartitionParameter));
        param->columnId = atoi(token);
        if ((token = strtok_r(NULL, RELMODEL_DELIMITER, &savePtr)) == NULL) {
            zfree(param);
            serverLog(
                    LL_DEBUG,
                    "[FILTER][EVALUATE] PartitionInfo of key is invalid form: [%s]",
                    partitionInfo);
            return C_ERR;
        }
        long valueLong;
        if (string2l(token, strlen(token), &valueLong) == 1) {
            param->type = CONDITION_CHILD_VALUE_TYPE_LONG;
            param->value.l = valueLong;
        } else {
            param->type = CONDITION_CHILD_VALUE_TYPE_SDS;
            param->value.s = sdsnew(token);
        }
        vectorAdd(v, param);
        token = strtok_r(NULL, RELMODEL_DELIMITER, &savePtr);
    }
    return C_OK;
}

void _freePartitionParameters(Vector *partitions) {
    for (size_t i = 0; i < vectorCount(partitions); ++i) {
        PartitionParameter *param = (PartitionParameter *) vectorGet(
                partitions, i);
        if (param->type == CONDITION_CHILD_VALUE_TYPE_SDS) {
            sdsfree(param->value.s);
        }
        zfree(param);
    }
    vectorFree(partitions);
}

sds convertLikeStatementToGlobPattern(const int optype,
                                      const sds likeStatement) {
    if (optype == CONDITION_OP_TYPE_STRING_CONTAINS) {
        return sdscatfmt(sdsempty(), "%s%S%s", "*", likeStatement, "*");
    } else if (optype == CONDITION_OP_TYPE_STRING_ENDS_WITH) {
        return sdscatfmt(sdsempty(), "%s%S", "*", likeStatement);
    } else if (optype == CONDITION_OP_TYPE_STRING_STARTS_WITH) {
        return sdscatfmt(sdsempty(), "%S%s", likeStatement, "*");
    } else {
        return NULL;
    }
}

bool _evaluateLeafOperator(const int optype, const ConditionChild *first,
                           const ConditionChild *second, Vector *partitions) {
    if (
            optype != CONDITION_OP_TYPE_EQ &&
            optype != CONDITION_OP_TYPE_LT &&
            optype != CONDITION_OP_TYPE_LTE &&
            optype != CONDITION_OP_TYPE_GT &&
            optype != CONDITION_OP_TYPE_GTE &&
            optype != CONDITION_OP_TYPE_IS_NULL &&
            optype != CONDITION_OP_TYPE_IS_NOT_NULL &&
            optype != CONDITION_OP_TYPE_STRING_CONTAINS &&
            optype != CONDITION_OP_TYPE_STRING_ENDS_WITH &&
            optype != CONDITION_OP_TYPE_STRING_STARTS_WITH
    ) {
        return false;
    }

    if (partitions == NULL) {
        return false;
    }

    for (size_t i = 0; i < vectorCount(partitions); ++i) {
        PartitionParameter *param = (PartitionParameter *) vectorGet(
                partitions, i);
        if (first == NULL || first->type != CONDITION_CHILD_VALUE_TYPE_LONG) {
            return false;
        }

        if (first->value.l != param->columnId) {
            continue;
        }

        if (optype == CONDITION_OP_TYPE_IS_NULL) {
            // TODO(totoro): Implements IS_NULL operator...
            return false;
        }

        if (optype == CONDITION_OP_TYPE_IS_NOT_NULL) {
            // TODO(totoro): Implements IS_NOT_NULL operator...
            return true;
        }

        if (
                second == NULL ||
                !(
                    second->type == CONDITION_CHILD_VALUE_TYPE_LONG ||
                    second->type == CONDITION_CHILD_VALUE_TYPE_SDS
                 )
        ) {
            return false;
        }

        if (second->type != param->type) {
            continue;
        }

        if (optype == CONDITION_OP_TYPE_EQ) {
            if (second->type == CONDITION_CHILD_VALUE_TYPE_LONG) {
                return second->value.l == param->value.l;
            } else if (second->type == CONDITION_CHILD_VALUE_TYPE_SDS) {
                return sdscmp(second->value.s, param->value.s) == 0;
            }
        }

        if (
            optype == CONDITION_OP_TYPE_STRING_CONTAINS ||
            optype == CONDITION_OP_TYPE_STRING_ENDS_WITH ||
            optype == CONDITION_OP_TYPE_STRING_STARTS_WITH
        ) {
            if (second->type != CONDITION_CHILD_VALUE_TYPE_SDS) {
                return false;
            }

            sds converted = convertLikeStatementToGlobPattern(optype,
                                                              second->value.s);
            bool result = (bool) stringmatch(converted, param->value.s, 0);
            sdsfree(converted);
            return result;
        }

        if (second->type == CONDITION_CHILD_VALUE_TYPE_SDS) {
            return false;
        }

        if (optype == CONDITION_OP_TYPE_LT) {
            return param->value.l < second->value.l;
        } else if (optype == CONDITION_OP_TYPE_LTE) {
            return param->value.l <= second->value.l;
        } else if (optype == CONDITION_OP_TYPE_GT) {
            return param->value.l > second->value.l;
        } else if (optype == CONDITION_OP_TYPE_GTE) {
            return param->value.l >= second->value.l;
        } else {
            serverLog(
                    LL_WARNING,
                    "[FILTER][EVALUATE][FATAL] Invalid operator type: %s",
                    _getOperatorStr(optype));
            serverPanic("[FILTER][EVALUATE][FATAL] Invalid operator type");
            return false;
        }
    }

    return true;
}

bool _evaluateNonleafOperator(const int optype, const ConditionChild *first,
                              const ConditionChild *second,
                              Vector *partitions) {
    if (
            optype != CONDITION_OP_TYPE_OR &&
            optype != CONDITION_OP_TYPE_AND &&
            optype != CONDITION_OP_TYPE_NOT
       ) {
        return false;
    }

    if (
            first == NULL ||
            first->type != CONDITION_CHILD_VALUE_TYPE_COND ||
            partitions == NULL
    ) {
        return false;
    }

    if(
            optype != CONDITION_OP_TYPE_NOT &&
            (
                second == NULL ||
                second->type != CONDITION_CHILD_VALUE_TYPE_COND
            )
    ) {
        return false;
    }

    if (optype == CONDITION_OP_TYPE_OR) {
        return _evaluateCondition(first->value.cond, partitions) ||
            _evaluateCondition(second->value.cond, partitions);
    } else if (optype == CONDITION_OP_TYPE_AND) {
        return _evaluateCondition(first->value.cond, partitions) &&
            _evaluateCondition(second->value.cond, partitions);
    } else if (optype == CONDITION_OP_TYPE_NOT) {
        return !_evaluateCondition(first->value.cond, partitions);
    } else {
        serverLog(
                LL_WARNING,
                "[FILTER][EVALUATE][FATAL] Invalid operator type: %s",
                _getOperatorStr(optype));
        serverPanic("[FILTER][EVALUATE][FATAL] Invalid operator type");
    }

    return false;
}

bool _evaluateCondition(const Condition *cond, Vector *partitions) {
    if (cond->isLeaf) {
        bool result = _evaluateLeafOperator(cond->op, cond->first, cond->second,
                                            partitions);
        serverLog(
                LL_DEBUG,
                "[FILTER][EVALUATE] _evaluateCondition leaf optype[%s], result[%d]",
                _getOperatorStr(cond->op), result);
        return result;
    }

    bool result = _evaluateNonleafOperator(cond->op, cond->first, cond->second,
                                           partitions);
    serverLog(
            LL_DEBUG,
            "[FILTER][EVALUATE] _evaluateCondition non-leaf optype[%s], result[%d]",
            _getOperatorStr(cond->op), result);
    return result;
}

bool evaluateCondition(const Condition *root, const sds metakey) {
    MetaKeyInfo *metaKeyInfo = parseMetaKeyInfo(metakey);
    if (!metaKeyInfo->isPartitionString) {
        // TODO(totoro): Needs to handle 'isPartitionInt' case...
        return false;
    }

    Vector partitions;
    vectorInit(&partitions);
    if (_parsePartitions(metaKeyInfo->partitionInfo.partitionString,
                         &partitions) == C_ERR) {
        _freePartitionParameters(&partitions);
        return false;
    }

    // Prints a log of partitions.
    serverLog(LL_DEBUG, "[FILTER][EVALUATE] Parsed partitions");
    for (size_t i = 0; i < vectorCount(&partitions); ++i) {
        PartitionParameter *param = (PartitionParameter *) vectorGet(
                &partitions, i);
        if (param->type == CONDITION_CHILD_VALUE_TYPE_SDS) {
            serverLog(LL_DEBUG,
                      "[FILTER][EVALUATE] columnId[%d] param sds value[%s]",
                      param->columnId, param->value.s);
        } else if (param->type == CONDITION_CHILD_VALUE_TYPE_LONG) {
            serverLog(LL_DEBUG,
                      "[FILTER][EVALUATE] columnId[%d] param long value[%ld]",
                      param->columnId, param->value.l);
        }
    }

    bool result = false;
    if (_evaluateCondition(root, &partitions)) {
        result = true;
    }

    _freePartitionParameters(&partitions);
    zfree(metaKeyInfo);
    return result;
}

void logCondition(const Condition *cond) {
    if (cond == NULL) {
        return;
    }

    serverLog(LL_DEBUG, "[FILTER][LOG] Operator '%s'",
              _getOperatorStr(cond->op));
    if (cond->first != NULL) {
        if (cond->first->type == CONDITION_CHILD_VALUE_TYPE_COND) {
            serverLog(LL_DEBUG, "[FILTER][LOG] Non-leaf First");
            logCondition(cond->first->value.cond);
        } else if (cond->first->type == CONDITION_CHILD_VALUE_TYPE_LONG) {
            serverLog(LL_DEBUG, "[FILTER][LOG] Leaf First Long '%ld'",
                      cond->first->value.l);
        } else if (cond->first->type == CONDITION_CHILD_VALUE_TYPE_SDS) {
            serverLog(LL_DEBUG, "[FILTER][LOG] Leaf First Sds '%s'",
                      cond->first->value.s);
        }
    }
    if (cond->second != NULL) {
        if (cond->second->type == CONDITION_CHILD_VALUE_TYPE_COND) {
            serverLog(LL_DEBUG, "[FILTER][LOG] Non-leaf Second");
            logCondition(cond->second->value.cond);
        } else if (cond->second->type == CONDITION_CHILD_VALUE_TYPE_LONG) {
            serverLog(LL_DEBUG, "[FILTER][LOG] Leaf Second Long '%ld'",
                      cond->second->value.l);
        } else if (cond->second->type == CONDITION_CHILD_VALUE_TYPE_SDS) {
            serverLog(LL_DEBUG, "[FILTER][LOG] Leaf Second Sds '%s'",
                      cond->second->value.s);
        }
    }
}

void freeConditions(Condition *cond) {
    if (cond->first != NULL) {
        if (cond->first->type == CONDITION_CHILD_VALUE_TYPE_COND) {
            freeConditions(cond->first->value.cond);
        } else if (cond->first->type == CONDITION_CHILD_VALUE_TYPE_SDS) {
            sdsfree(cond->first->value.s);
        }
        zfree(cond->first);
    }
    if (cond->second != NULL) {
        if (cond->second->type == CONDITION_CHILD_VALUE_TYPE_COND) {
            freeConditions(cond->second->value.cond);
        } else if (cond->first->type == CONDITION_CHILD_VALUE_TYPE_SDS) {
            sdsfree(cond->second->value.s);
        }
        zfree(cond->second);
    }
    zfree(cond);
}
