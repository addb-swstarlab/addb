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
  assert(part_size <= PARTITION_KEY_SIZE_MAX);
  memcpy(ret->partitionInfo.partitionString, token, part_size);
  ret->isPartitionString = true;

  //parsing rowgroup number
  if (saveptr != NULL && (token = strtok_r(NULL, RELMODEL_ROWGROUPNUMBER_PREFIX, &saveptr)) != NULL)
	  ret->rowGroupId = atoi(token);
  else
	  ret->rowGroupId = 0;

  serverLog(LL_DEBUG, "BEFORE RETURN TO ADDB_TABLE");
  serverLog(LL_DEBUG,"TOKEN : %s, SAVEPTR: %s, tableId : %d, partitionInfo : %s, rowgroup : %d",
              token, saveptr, ret->tableId,ret->partitionInfo.partitionString, ret->rowGroupId);
  serverLog(LL_DEBUG,"Create DATAKEYINFO END");

  return ret;
}

/*addb get RowgroupInfo from Metadict*/
int getRowGroupInfoAndSetRowGroupInfo(redisDb *db, NewDataKeyInfo *keyInfo){
	char tmp[SDS_DATA_KEY_MAX];
	sds findKey = sdsIntialize(tmp, sizeof(tmp));
	return true;
}


int getRowgroupInfo(redisDb *db, NewDataKeyInfo *dataKeyInfo){
    serverLog(LL_VERBOSE,"GET ROWGROUP INFO START");
    int number =0;
    return number;

}

/* ADDB Create Scan parameter*/
ColumnParameter *parseColumnParameter(const sds rawColumnIdsString) {
    ColumnParameter *param = (ColumnParameter *) zmalloc(
            sizeof(ColumnParameter));
    param->original = sdsdup(rawColumnIdsString);

    vectorInit(&param->columnIdList);
    vectorInit(&param->columnIdStrList);

    char *savePtr = NULL;
    sds copied_original = sdsdup(param->original);
    char *token = strtok_r(copied_original, RELMODEL_COLUMN_DELIMITER,
                           &savePtr);

    if (token == NULL) {
        serverLog(LL_WARNING, "Fatal: column parameter is broken: no data");
        serverPanic("column parameter is broken");
    }

    while (token != NULL) {
        vectorAdd(&param->columnIdStrList, token);
        vectorAddInt(&param->columnIdList, atoi(token));

        // TODO(totoro): Checks that column ID is valid.
        token = strtok_r(NULL, RELMODEL_COLUMN_DELIMITER, &savePtr);
    }
    param->columnCount = vectorCount(&param->columnIdList);
    sdsfree(copied_original);
    return param;
}

ScanParameter *createScanParameter(const client *c) {
    ScanParameter *param = (ScanParameter *) zmalloc(sizeof(ScanParameter));
    param->startRowGroupId = 0;
    // TODO(totoro): Gets total row group count from dictMeta.
    param->totalRowGroupCount = 0;
    param->dataKeyInfo = parsingDataKeyInfo((sds) c->argv[1]->ptr);
    param->columnParam = parseColumnParameter((sds) c->argv[2]->ptr);
    return param;
}

