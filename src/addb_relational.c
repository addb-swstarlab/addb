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

  serverLog(LL_DEBUG, "BEFORE RETURN TO ADDB_TABLE");
  serverLog(LL_DEBUG,"TOKEN : %s, SAVEPTR: %s, table_number : %d, partitionInfo : %s, rowgroup : %d",
              token, saveptr, ret->tableId,ret->partitionInfo.partitionString, ret->rowGroupId);
  serverLog(LL_DEBUG,"Create DATAKEYINFO END");

  return ret;
}

/*addb get RowgroupInfo from Metadict*/
int getRowGroupInfoAndSetRowGroupInfo(redisDb *db, NewDataKeyInfo *dataKeyInfo){
	char tmp[SDS_DATA_KEY_MAX];
	int rowgroup = 0;
	sds metaKey = sdsnewlen("", SDS_DATA_KEY_MAX);// sdsnewlen(tmp, sizeof(tmp) //sdsnew(tmp) //sdsIntialize(tmp, sizeof(tmp));
	setMetaKeyForRowgroup(dataKeyInfo, metaKey);

	robj *metaHashdictObj = lookupSDSKeyForMetadict(db, metaKey);
	robj *metaField = shared.integers[0];
	rowgroup = lookupCompInfoForMeta(metaHashdictObj, metaField);
	dataKeyInfo->rowGroupId = rowgroup;

	serverLog(LL_VERBOSE, "FIND METAKEY :  %s", metaKey);
	serverLog(LL_VERBOSE, "FIND ROWGROUP :  %d", dataKeyInfo->rowGroupId);
	return rowgroup;
}


int getRowgroupInfo(redisDb *db, NewDataKeyInfo *dataKeyInfo){
    serverLog(LL_VERBOSE,"GET ROWGROUP INFO START");
    int rowgroup = getRowGroupInfoAndSetRowGroupInfo(db,dataKeyInfo);

	serverLog(LL_VERBOSE, "FIND ROWGROUP :  %d", dataKeyInfo->rowGroupId);
    if(rowgroup == 0){
    	rowgroup = IncRowgroupIdAndModifyInfo(db, dataKeyInfo, 1);
    }
    return rowgroup;

}


int lookupCompInfoForMeta(robj *metaHashdictObj,robj* metaField){
    if (metaHashdictObj == NULL)
        return 0;

    sds field = sdsnew((char *) metaField->ptr);
    int retVal = 0;
    robj *ret = hashTypeGetValueObject(metaHashdictObj, field);
    if (!sdsEncodedObject(ret)) {
        retVal = (int) (long) ret->ptr;
    } else {
        retVal = atoi((char *) ret->ptr);
    }
    sdsfree(field);
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

}

int incRowgroupId(redisDb *db, NewDataKeyInfo *dataKeyInfo, int inc_number){
	robj *rowgroupKey= generateRgIdKeyForRowgroup(dataKeyInfo);
	serverLog(LL_VERBOSE, "RowgroupKEY :  %s", (char *)rowgroupKey->ptr);
	robj *metaRgField = shared.integers[0];


	return 1;
}

//TO-DO MODIFY LATER -HS
//int IncDecCount(redisDb *db, robj *key, robj *){
//
//    long long value, oldvalue;
//    robj *o, *new;
//
//    o = lookupKeyWriteForMetadict(db,key);
//    if (o != NULL && checkType(c,o,OBJ_STRING)) return;
//    if (getLongLongFromObjectOrReply(c,o,&value,NULL) != C_OK) return;
//
//    oldvalue = value;
//    if ((incr < 0 && oldvalue < 0 && incr < (LLONG_MIN-oldvalue)) ||
//        (incr > 0 && oldvalue > 0 && incr > (LLONG_MAX-oldvalue))) {
//        addReplyError(c,"increment or decrement would overflow");
//        return;
//    }
//    value += incr;
//
//    if (o && o->refcount == 1 && o->encoding == OBJ_ENCODING_INT &&
//        (value < 0 || value >= OBJ_SHARED_INTEGERS) &&
//        value >= LONG_MIN && value <= LONG_MAX)
//    {
//        new = o;
//        o->ptr = (void*)((long)value);
//    } else {
//        new = createStringObjectFromLongLong(value);
//        if (o) {
//            dbOverwrite(c->db,c->argv[1],new);
//        } else {
//            dbAdd(c->db,c->argv[1],new);
//        }
//    }
//    signalModifiedKey(c->db,c->argv[1]);
//    notifyKeyspaceEvent(NOTIFY_STRING,"incrby",c->argv[1],c->db->id);
//    server.dirty++;
//
//
//}


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
	serverLog(LL_VERBOSE, "RGKEY :  %s", (char *)rgKey);
	return createStringObject(rgKey, strlen(rgKey));
}


robj * generateDataKey(NewDataKeyInfo *dataKeyInfo){

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
	serverLog(LL_VERBOSE, "DATAKEY :  %s", (char *)dataKey);
	return createStringObject(dataKey, strlen(dataKey));
}

/* ADDB Create Scan parameter*/
ColumnParameter *parseColumnParameter(const sds rawColumnIdsString) {
    ColumnParameter *param = (ColumnParameter *) zmalloc(
            sizeof(ColumnParameter));
    param->original = sdsnew(rawColumnIdsString);

    vectorTypeInit(&param->columnIdStrList, VECTOR_TYPE_SDS);
    vectorTypeInit(&param->columnIdList, VECTOR_TYPE_INT);

    int tokenCounts;
    sds *tokens = sdssplit(param->original, RELMODEL_COLUMN_DELIMITER,
                           &tokenCounts);

    if (tokens == NULL || tokenCounts == 0) {
        serverLog(LL_WARNING, "Fatal: column parameter is broken: no data");
        serverPanic("column parameter is broken");
    }

    for (int i = 0; i < tokenCounts; ++i) {
        // TODO(totoro): Checks that column ID is valid.
        vectorAddSds(&param->columnIdStrList, tokens[i]);
        vectorAddInt(&param->columnIdList, atoi(tokens[i]));
    }

    param->columnCount = vectorCount(&param->columnIdList);
    return param;
}

ScanParameter *createScanParameter(const client *c) {
    ScanParameter *param = (ScanParameter *) zmalloc(sizeof(ScanParameter));
    param->startRowGroupId = 0;
    param->dataKeyInfo = parsingDataKeyInfo((sds) c->argv[1]->ptr);
    param->totalRowGroupCount = getRowGroupInfoAndSetRowGroupInfo(
            c->db, param->dataKeyInfo);
    param->columnParam = parseColumnParameter((sds) c->argv[2]->ptr);
    return param;
}

void clearColumnParameter(ColumnParameter *param) {
    sdsfree(param->original);
    vectorFreeDeep(&param->columnIdList);
    vectorFreeDeep(&param->columnIdStrList);
}

void clearScanParameter(ScanParameter *param) {
    zfree(param->dataKeyInfo);
    clearColumnParameter(param->columnParam);
    zfree(param->columnParam);
    zfree(param);
}

/*
 * populateScanParameter
 * - Description
 *      Populates row group data to scan parameter by looking up MetaDict
 * - Return
 *      Returns total data count scaned by scan parameter.
 */
int populateScanParameter(ScanParameter *scanParam) {
    int totalDataCount = 0;
    // TODO(totoro): Implements critical logics for populateScanParameter.
    return totalDataCount;
}

