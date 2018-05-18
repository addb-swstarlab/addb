/*
 * 2018.3.15
 * hssung@yonsei.ac.kr
 * Add File for relational model in Redis
 */

#include "server.h"
#include "addb_relational.h"


//#define FPWRITE_NO_FLAGS 0
//#define FPWRITE_NX (1<<0)     /* Set if key not exists. */
//#define FPWRITE_XX (1<<1)     /* Set if key exists. */
//#define FPWRITE_EX (1<<2)     /* Set if time in seconds is given */
//#define FPWRITE_PX (1<<3)     /* Set if time in ms in given */



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

    struct redisClient *fakeClient = NULL;

    serverLog(LL_DEBUG, "fpWrite Param List ==> Key : %s, partition : %s, num_of_column : %s, indexColumn : %s",
            (char *) c->argv[1]->ptr,(char *) c->argv[2]->ptr, (char *) c->argv[3]->ptr , (char *) c->argv[4]->ptr);

    /*parsing dataInfo*/
    NewDataKeyInfo *dataKeyInfo = parsingDataKeyInfo((sds)c->argv[1]->ptr);

    /*get column number*/
    int column_number = atoi((char *) c->argv[3]->ptr);
    serverLog(LL_DEBUG, "fpWrite Column Number : %d", column_number);

    /*compare with column number and arguments*/

    if(((c->argc-5)%column_number) != 0 ){
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


    /*pk*/
    /*dict- hashdict */
    /*insert*/
    /*filter*/
    /*free*/
    /*eviction insert*/

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
        serverLog(LL_DEBUG, "i: %d, columnId: %d, columnIdStr: %s",
                  i,
                  vectorGetInt(&scanParam->columnParam->columnIdList, i),
                  vectorGetSds(
                      &scanParam->columnParam->columnIdStrList, i));
    }

    /*Populates row group information to scan parameters*/
    /*Load data from Redis or RocksDB*/
    /*Scan data to client*/

    clearScanParameter(scanParam);
    addReply(c, shared.ok);
}


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
