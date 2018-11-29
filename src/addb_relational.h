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

#define CONDITION_CHILD_VALUE_TYPE_NONE 0
#define CONDITION_CHILD_VALUE_TYPE_COND 1
#define CONDITION_CHILD_VALUE_TYPE_LONG 2
#define CONDITION_CHILD_VALUE_TYPE_SDS 3

#define CONDITION_OP_TYPE_NONE 0                // Default
#define CONDITION_OP_TYPE_AND 1                 // &&
#define CONDITION_OP_TYPE_OR 2                  // ||
#define CONDITION_OP_TYPE_NOT 3                 // !
#define CONDITION_OP_TYPE_EQ 4                  // ==
#define CONDITION_OP_TYPE_LT 5                  // <
#define CONDITION_OP_TYPE_LTE 6                 // <=
#define CONDITION_OP_TYPE_GT 7                  // >
#define CONDITION_OP_TYPE_GTE 8                 // >=
#define CONDITION_OP_TYPE_IS_NULL 9             // isNull
#define CONDITION_OP_TYPE_IS_NOT_NULL 10        // isNotNull
#define CONDITION_OP_TYPE_STRING_CONTAINS 11    // StringContains
#define CONDITION_OP_TYPE_STRING_ENDS_WITH 12   // StringEndsWith
#define CONDITION_OP_TYPE_STRING_STARTS_WITH 13 // StringStartsWith

/* ADDB evaluate column vector ID */
static inline int getColumnVectorId(size_t rowId) {
    return ((rowId - 1) / server.columnvector_size + 1);
}

/* ADDB evaluate column vector ID */
static inline size_t getColumnVectorIndex(size_t rowId) {
    return (rowId - 1) % server.columnvector_size;
}

/* ADDB Create Partition Filter parameter*/
static inline int _getOperatorType(char *operator) {
    if (strcmp(operator, "And") == 0) {
        return CONDITION_OP_TYPE_AND;
    } else if (strcmp(operator, "Or") == 0) {
        return CONDITION_OP_TYPE_OR;
    } else if (strcmp(operator, "Not") == 0) {
        return CONDITION_OP_TYPE_NOT;
    } else if (strcmp(operator, "EqualTo") == 0) {
        return CONDITION_OP_TYPE_EQ;
    } else if (strcmp(operator, "LessThan") == 0) {
        return CONDITION_OP_TYPE_LT;
    } else if (strcmp(operator, "LessThanOrEqual") == 0) {
        return CONDITION_OP_TYPE_LTE;
    } else if (strcmp(operator, "GreaterThan") == 0) {
        return CONDITION_OP_TYPE_GT;
    } else if (strcmp(operator, "GreaterThanOrEqual") == 0) {
        return CONDITION_OP_TYPE_GTE;
    } else if (strcmp(operator, "IsNull") == 0) {
        return CONDITION_OP_TYPE_IS_NULL;
    } else if (strcmp(operator, "IsNotNull") == 0) {
        return CONDITION_OP_TYPE_IS_NOT_NULL;
    } else if (strcmp(operator, "StringContains") == 0) {
        return CONDITION_OP_TYPE_STRING_CONTAINS;
    } else if (strcmp(operator, "StringEndsWith") == 0) {
        return CONDITION_OP_TYPE_STRING_ENDS_WITH;
    } else if (strcmp(operator, "StringStartsWith") == 0) {
        return CONDITION_OP_TYPE_STRING_STARTS_WITH;
    } else {
        return -1;
    }
}

static inline const char *_getOperatorStr(int optype) {
    if (optype == CONDITION_OP_TYPE_AND) {
        return "And";
    } else if (optype == CONDITION_OP_TYPE_OR) {
        return "Or";
    } else if (optype == CONDITION_OP_TYPE_NOT) {
        return "Not";
    } else if (optype == CONDITION_OP_TYPE_EQ) {
        return "EqualTo";
    } else if (optype == CONDITION_OP_TYPE_LT) {
        return "LessThan";
    } else if (optype == CONDITION_OP_TYPE_LTE) {
        return "LessThanOrEqual";
    } else if (optype == CONDITION_OP_TYPE_GT) {
        return "GreaterThan";
    } else if (optype == CONDITION_OP_TYPE_GTE) {
        return "GreaterThanOrEqual";
    } else if (optype == CONDITION_OP_TYPE_IS_NULL) {
        return "IsNull";
    } else if (optype == CONDITION_OP_TYPE_IS_NOT_NULL) {
        return "IsNotNull";
    } else if (optype == CONDITION_OP_TYPE_STRING_CONTAINS) {
        return "StringContains";
    } else if (optype == CONDITION_OP_TYPE_STRING_ENDS_WITH) {
        return "StringEndsWith";
    } else if (optype == CONDITION_OP_TYPE_STRING_STARTS_WITH) {
        return "StringStartsWith";
    } else {
        return NULL;
    }
}

static inline int _getOperatorOperandCount(int optype) {
    if (
            (optype < CONDITION_OP_TYPE_AND) ||
            (optype > CONDITION_OP_TYPE_STRING_STARTS_WITH)
    ) {
        return -1;
    }

    if (
            optype == CONDITION_OP_TYPE_NOT ||
            optype == CONDITION_OP_TYPE_IS_NULL ||
            optype == CONDITION_OP_TYPE_IS_NOT_NULL
    ) {
        return 1;
    }

    return 2;
}

//typedef struct DataKeyInfo {
//    char dataKeyCopy[MAX_TMPBUF_SIZE];  //the whole string
//    char *tableId;                //table name inner ptr
//    char *partitionInfo;            //partition info inner ptr
//    char *rowGroupId;               //row group id
//    int rowGroupIdTemp;
//    int rowCnt;                     // used for tiering
//} DataKeyInfo;

/*Scan Parameters*/
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

/*Partition Filter Parameters*/
typedef union _ConditionValue {
    void *cond;
    long l;
    sds s;
} ConditionValue;

typedef struct _ConditionChild {
    unsigned type:2;
    ConditionValue value;
} ConditionChild;

typedef struct _Condition {
    unsigned op:4;  // Operator
    int opCount;
    bool isLeaf;
    ConditionChild *first;
    ConditionChild *second;
} Condition;

typedef struct _PartitionValue {
    long l;
    sds s;
} PartitionValue;

typedef struct _PartitionParameter {
    int columnId;
    unsigned type:2;
    PartitionValue value;
} PartitionParameter;

NewDataKeyInfo *parsingDataKeyInfo(sds dataKeyString);
MetaKeyInfo *parseMetaKeyInfo(sds metakey);
int changeDataKeyInfo(NewDataKeyInfo *dataKeyInfo, int number);

dictEntry *getCandidatedictFirstEntry(client *c, NewDataKeyInfo *dataKeyInfo);
dictEntry *getCandidatedictEntry(client *c, NewDataKeyInfo *dataKeyInfo);
dictEntry *getPrevCandidatedictEntry(client *c, NewDataKeyInfo *dataKeyInfo);


/*addb Metadict*/
/*get information function*/
int getRowNumberInfoAndSetRowNumberInfo(redisDb *db, NewDataKeyInfo *dataKeyInfo);
int getRowGroupInfoAndSetRowGroupInfo(redisDb *db, NewDataKeyInfo *keyInfo);
int getRowgroupInfo(redisDb *db, NewDataKeyInfo *dataKeyInfo);

/*lookup Metadict function*/
int lookupCompInfoForMeta(robj *metaHashdictObj,robj* metaField);
int lookupCompInfoForRowNumberInMeta(robj *metaHashdictObj,robj* metaField);


/*Inc, Dec Function*/
int IncRowgroupIdAndModifyInfo(redisDb *db, NewDataKeyInfo *dataKeyInfo, int param);
int incRowgroupId(redisDb *db, NewDataKeyInfo *dataKeyInfo, int inc_number);
int incRowNumber(redisDb *db, NewDataKeyInfo *dataKeyInfo, int inc_number);

void setMetaKeyForRowgroup(NewDataKeyInfo *dataKeyInfo, sds key);

/*addb key generation func*/
robj * generateRgIdKeyForRowgroup(NewDataKeyInfo *dataKeyInfo);
robj * generateDataKey(NewDataKeyInfo *dataKeyInfo);
sds generateDataKeySds(NewDataKeyInfo *dataKeyInfo);
robj *generateDataRocksKey(NewDataKeyInfo *dataKeyInfo, int rowId,
                           int columnId);
sds generateDataRocksKeySds(NewDataKeyInfo *dataKeyInfo, int rowId,
                            int columnId);

robj * generateDataKeyForFirstEntry(NewDataKeyInfo *dataKeyInfo);
robj * generatePrevDataKey(NewDataKeyInfo *dataKeyInfo);

/*addb data field function*/
robj *getDataField(int row, int column);
sds getDataFieldSds(int rowId, int columnId);

/*Insert function*/
int insertKVpairToRelational(client *c, robj *dataKeyString, robj *dataField, robj *valueObj);
void prepareWriteToRocksDB(redisDb *db, robj *keyobj, robj *targetVal);

/*Scan*/
ColumnParameter *parseColumnParameter(const sds rawColumnIdsString);
ScanParameter *createScanParameter(const client *c);
void freeColumnParameter(ColumnParameter *param);
void freeScanParameter(ScanParameter *param);
int populateScanParameter(redisDb *db, ScanParameter *scanParam);
RowGroupParameter createRowGroupParameter(redisDb *db, robj *dataKey);
void scanDataFromADDB(redisDb *db, ScanParameter *scanParam, Vector *data);
void _cachedScan(redisDb *db, size_t rowGroupId, ScanParameter *scanParam,
                 Vector *data, robj **cachedColumnVectorObjs,
                 int *cachedColumnVectorIds);
Vector *getColumnVectorFromRocksDB(redisDb *db, sds dataRocksKey);
// Legacy Functions
void legacyScanDataFromADDB(redisDb *db, ScanParameter *scanParam, Vector *data);
void _legacyCachedScan(redisDb *db, size_t rowGroupId, ScanParameter *scanParam,
                       Vector *data);
void _legacyCachedScanOnRocksDB(redisDb *db, size_t rowGroupId,
                                ScanParameter *scanParam, Vector *data);
// Non-vector scan functions
size_t scanDataFromADDB_non_vector(client *c, redisDb *db,
                                   ScanParameter *scanParam);
size_t _cachedScan_non_vector(client *c, redisDb *db, size_t rowGroupId,
                              ScanParameter *scanParam);
size_t _cachedScanOnRocksDB_non_vector(client *c, redisDb *db,
                                       size_t rowGroupId,
                                       ScanParameter *scanParam);

/*Partition Filter*/
bool validateStatements(const sds rawStatementsStr);
bool validateStatement(const sds rawStatementStr);
int parseStatement(const sds rawStatementStr, Condition **root);
int createCondition(const char *rawConditionStr, Stack *s, Condition **cond);
bool _evaluateLeafOperator(const int optype, const ConditionChild *first,
                           const ConditionChild *second, Vector *partitions);
bool _evaluateNonleafOperator(const int optype, const ConditionChild *first,
                              const ConditionChild *second, Vector *partitions);
bool _evaluateCondition(const Condition *cond, Vector *partitions);
bool evaluateCondition(const Condition *cond, const sds metakey);
void logCondition(const Condition *cond);
void freeConditions(Condition *cond);

#endif
