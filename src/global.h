/*
 * 2018.3.23
 * Doyoung Kim
 */
#ifndef __ADDB_GLOBAL
#define __ADDB_GLOBAL



#include <stdint.h>

#define RELMODEL_DELIMITER ":"
#define RELMODEL_DATA_PREFIX "D"
#define RELMODEL_META_PREFIX "M"
#define RELMODEL_INDEX_PREFIX "I"
#define RELMODEL_BRACE_PREFIX "{"
#define RELMODEL_BRACE_SUFFIX "}"
#define RELMODEL_ROWGROUPID_PREFIX "G:"
#define RELMODEL_COLUMN_DELIMITER ","
#define REL_MODEL_FIELD_PREFIX "F:"
#define RELMODEL_VECTOR_PREFIX "V:"
#define RELMODEL_VECTOR_COUNT_PREFIX "N:"
#define RELMODEL_VECTOR_TYPE_PREFIX "T:"
#define RELMODEL_VECTOR_DATA_PREFIX "D:["
#define VECTOR_DATA_PREFIX "["
#define VECTOR_DATA_SUFFIX "]"

#define PARTITION_FILTER_OPERAND_DELIMITER "*"
#define PARTITION_FILTER_OPERATOR_DELIMITER ":"
#define PARTITION_FILTER_STATEMENT_SUFFIX "$"

#define MAX_COLUMN_NUMBER 100  /*Modify later*/

#define NULLVALUE "null"
#define EMPTYSTRING "\0"

#define DATA_KEY_MAX_SIZE 256
#define PARTITION_KEY_MAX_SIZE 256
#define PARTITION_KEY_MAX_CNT (PARTITION_KEY_MAX_SIZE/sizeof(unsigned long long))	//32


typedef union Partition {
		char partitionString[PARTITION_KEY_MAX_SIZE];
		unsigned long long partitionInt[PARTITION_KEY_MAX_CNT];
} Partition;

typedef struct NewDataKeyInfo {
	int tableId;
	int rowGroupId;
	int row_number;
	uint32_t isPartitionString:1, partitionCnt:31;
	Partition partitionInfo;
	uint32_t timeStamp;
} NewDataKeyInfo;

typedef struct _MetaKeyInfo {
    int tableId;
    unsigned isPartitionString:1;
    Partition partitionInfo;
} MetaKeyInfo;

#endif
