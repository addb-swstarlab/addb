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
  serverLog(LL_VERBOSE,"Create DATAKEYINFO START");

  NewDataKeyInfo *ret = zcalloc(sizeof(NewDataKeyInfo));
  char copyStr[MAX_TMPBUF_SIZE];
  char* saveptr = NULL;
  char* token = NULL;
  size_t size = sdslen(dataKeyString) + 1;

  assert( size == (strlen(dataKeyString ) + 1));
  assert( size < MAX_TMPBUF_SIZE);

  memcpy(copyStr, dataKeyString, size);

  //parsing the prefix
  if ((token = strtok_r(copyStr, RELMODEL_DELIMITER, &saveptr)) == NULL)
          goto fail;

      /*skip IDX prefix ("I") */
      if (strcasecmp(token, RELMODEL_INDEX_PREFIX) == 0 ) {
          // skip idxType field
          strtok_r(NULL, RELMODEL_DELIMITER, &saveptr);
      }

      // parsing tableId
      if ((token = strtok_r(NULL, RELMODEL_DELIMITER, &saveptr)) == NULL)
          goto fail;
      ret->tableId = atoi(token + strlen(RELMODEL_BRACE_PREFIX));

      //parsing partitionInfo
      if ((token = strtok_r(NULL, RELMODEL_BRACE_SUFFIX, &saveptr)) == NULL)
          goto fail;

      size_t part_size = strlen(token)+1;
      assert(part_size <= PARTITION_KEY_SIZE_MAX);
      memcpy(ret->partitionInfo.partitionString, token, part_size);
      ret->isPartitionString = true;

      //parsing rowgroup id
      if (saveptr != NULL &&
          (token = strtok_r(NULL, RELMODEL_ROWGROUPID_PREFIX, &saveptr)) != NULL)
          ret->rowGroupId = atoi(token);
      else
          ret->rowGroupId = 0;

      serverLog(LL_VERBOSE, "BEFORE RETURN TO ADDB_TABLE");
      serverLog(LL_VERBOSE,"TOKEN : %s, SAVEPTR: %s, tableId : %d, partitionInfo : %s, rowgroup : %d",
              token, saveptr, ret->tableId,ret->partitionInfo.partitionString, ret->rowGroupId);
      serverLog(LL_VERBOSE,"Create DATAKEYINFO END");

      return ret;

fail:
  serverLog(LL_WARNING, "Fatal: data key is broken: [%s]", dataKeyString);
  zfree(ret);
  return NULL;

}

