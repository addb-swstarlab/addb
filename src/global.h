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


#define MAX_COLUMN_NUMBER 100  /*Modify later*/

#define NULLVALUE "null"

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

#endif
