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
  // parsing table_number
  if ((token = strtok_r(NULL, RELMODEL_DELIMITER, &saveptr)) == NULL){
	  serverLog(LL_WARNING, "Fatal: DataKey broken Error: [%s]", dataKeyString);
	  zfree(ret);
	  return NULL;
  }
  ret->table_number = atoi(token + strlen(RELMODEL_BRACE_PREFIX));
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
	  ret->rowGroup_number = atoi(token);
  else
	  ret->rowGroup_number = 0;

  serverLog(LL_DEBUG, "BEFORE RETURN TO ADDB_TABLE");
  serverLog(LL_DEBUG,"TOKEN : %s, SAVEPTR: %s, table_number : %d, partitionInfo : %s, rowgroup : %d",
              token, saveptr, ret->table_number,ret->partitionInfo.partitionString, ret->rowGroup_number);
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
