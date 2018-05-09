/*
 * 2018.2.23
 * doyoung kim
 */

#include "server.h"
#include "global.h"

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


NewDataKeyInfo *parsingDataKeyInfo(sds dataKeyString);
int changeDataKeyInfo(NewDataKeyInfo *dataKeyInfo, int number);


/*addb Metadict*/
int getRowGroupInfoAndSetRowGroupInfo(redisDb *db, NewDataKeyInfo *keyInfo);
int getRowgroupInfo(redisDb *db, NewDataKeyInfo *dataKeyInfo);
