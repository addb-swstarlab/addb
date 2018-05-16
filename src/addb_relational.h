/*
 * 2018.2.23
 * doyoung kim
 */

#ifndef __ADDB_RELATIONAL
#define __ADDB_RELATIONAL

#include "server.h"
#include "global.h"
#include "stl.h"

#define MAX_TMPBUF_SIZE 128
#define SDS_DATA_KEY_MAX (sizeof(struct sdshdr) + DATA_KEY_MAX_SIZE)

//typedef struct DataKeyInfo {
//    char dataKeyCopy[MAX_TMPBUF_SIZE];  //the whole string
//    char *tableId;                //table name inner ptr
//    char *partitionInfo;            //partition info inner ptr
//    char *rowGroupId;               //row group id
//    int rowGroupIdTemp;
//    int rowCnt;                     // used for tiering
//} DataKeyInfo;

typedef struct _RowGroupParameter {
    robj *dictObj;      // RowGroup dict table object
    uint64_t isInRocksDb:1, rowCount:63;
} RowGroupParameter;

typedef struct _ColumnParameter {
    sds original;
    int columnCount;
    Vector columnIdList;        // int* vector
    Vector columnIdStrList;     // string vector
} ColumnParameter;

typedef struct _ScanParameter {
    int startRowGroupId;
    int totalRowGroupCount;
    NewDataKeyInfo *dataKeyInfo;
    RowGroupParameter *rowGroupParams;
    ColumnParameter *columnParam;
} ScanParameter;

NewDataKeyInfo *parsingDataKeyInfo(sds dataKeyString);
int changeDataKeyInfo(NewDataKeyInfo *dataKeyInfo, int number);

/*addb Metadict*/
/*get information function*/
int getRowGroupInfoAndSetRowGroupInfo(redisDb *db, NewDataKeyInfo *keyInfo);
int getRowgroupInfo(redisDb *db, NewDataKeyInfo *dataKeyInfo);

/*lookup Metadict function*/
int lookupCompInfoForMeta(robj *metaHashdictObj,robj* metaField);

/*Inc, Dec Function*/
int IncRowgroupIdAndModifyInfo(redisDb *db, NewDataKeyInfo *dataKeyInfo, int param);
int incRowgroupId(redisDb *db, NewDataKeyInfo *dataKeyInfo, int inc_number);


void setMetaKeyForRowgroup(NewDataKeyInfo *dataKeyInfo, sds key);

/*addb key generation func*/
robj * generateRgIdKeyForRowgroup(NewDataKeyInfo *dataKeyInfo);
robj * generateDataKey(NewDataKeyInfo *dataKeyInfo);

/*Scan*/
ColumnParameter *parseColumnParameter(const sds rawColumnIdsString);
ScanParameter *createScanParameter(const client *c);
void freeColumnParameter(ColumnParameter *param);
void freeScanParameter(ScanParameter *param);
int populateScanParameter(redisDb *db, ScanParameter *scanParam);
RowGroupParameter createRowGroupParameter(redisDb *db, robj *dataKey);

#endif
