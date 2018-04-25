/*
 * 2018.2.23
 * doyoung kim
 */

#include "server.h"
#include "global.h"

#define MAX_TMPBUF_SIZE 128


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
