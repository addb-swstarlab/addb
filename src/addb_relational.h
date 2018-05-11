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

typedef struct ColumnParameter {
    sds original;
    int columnCount;
    vector columnIdList;        // int* vector
    vector columnIdStrList;     // string vector
} ColumnParameter;

typedef struct ScanParameter {
    int startRowGroupId;
    int totalRowGroupCount;
    NewDataKeyInfo *dataKeyInfo;
    ColumnParameter *columnParam;
} ScanParameter;

NewDataKeyInfo *parsingDataKeyInfo(sds dataKeyString);
int changeDataKeyInfo(NewDataKeyInfo *dataKeyInfo, int number);

/*addb Metadict*/
/*get information function*/
int getRowGroupInfoAndSetRowGroupInfo(redisDb *db, NewDataKeyInfo *keyInfo);
int getRowgroupInfo(redisDb *db, NewDataKeyInfo *dataKeyInfo);

/*Scan*/
ColumnParameter *parseColumnParameter(const sds rawColumnIdsString);
ScanParameter *createScanParameter(const client *c);
void clearColumnParameter(ColumnParameter *param);
void clearScanParameter(ScanParameter *param);

/*lookup Metadict function*/
int lookupCompInfoForMeta(robj *metaHashdictObj,robj* metaField);

/*Inc, Dec Function*/
int IncRowgroupIdAndModifyInfo(redisDb *db, NewDataKeyInfo *dataKeyInfo, int param);
int incRowgroupId(redisDb *db, NewDataKeyInfo *dataKeyInfo, int inc_number);


void setMetaKeyForRowgroup(NewDataKeyInfo *dataKeyInfo, sds key);

/*addb key generation func*/
robj * generateRgIdKeyForRowgroup(NewDataKeyInfo *dataKeyInfo);
robj * generateDataKey(NewDataKeyInfo *dataKeyInfo);


#endif
