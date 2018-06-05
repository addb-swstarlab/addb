#include "addb_relational.h"
#include "sds.h"
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

robj *generateDataFieldKey(NewDataKeyInfo *dataKeyInfo, int rowId,
                           int columnId) {
    sds dataFieldKeySds = generateDataFieldKeySds(dataKeyInfo, rowId, columnId);
    robj *dataFieldKeyObj = createStringObject(dataFieldKeySds,
                                               sdslen(dataFieldKeySds));
    sdsfree(dataFieldKeySds);
    return dataFieldKeyObj;
}

sds generateDataFieldKeySds(NewDataKeyInfo *dataKeyInfo, int rowId,
                            int columnId) {
    sds dataKey = generateDataKeySds(dataKeyInfo);
    sds fieldKey = getDataFieldSds(rowId, columnId);

    char strBuf[DATA_KEY_MAX_SIZE];
    sprintf(strBuf, "%s:%s%s", dataKey, REL_MODEL_FIELD_PREFIX, fieldKey);
    sdsfree(dataKey);
    sdsfree(fieldKey);
    return sdsnew(strBuf);
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

    vectorTypeInit(&param->columnIdStrList, VECTOR_TYPE_SDS);
    vectorTypeInit(&param->columnIdList, VECTOR_TYPE_LONG);

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

    if (param.dictObj == NULL) {
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
            // TODO(totorody): Implements scan for RocksDB.
            continue;
        }

        // Scan on Redis.
        robj *hashDictObj = rowGroupParams[i].dictObj;
        dict *hashDict = (dict *) hashDictObj->ptr;
        for (size_t j = 0; j < rowGroupParams[i].rowCount; ++j) {
            size_t rowId = j + 1;
            for(size_t k = 0; k < columnParam->columnCount; ++k) {
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

