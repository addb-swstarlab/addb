/*
 * 2018.3.23
 * Doyoung Kim
 */

#include <stdint.h>

#define RELMODEL_DELIMITER ":"
#define RELMODEL_META_PREFIX "M"
#define RELMODEL_ROWIDX_PREFIX "C"
#define RELMODEL_DATA_PREFIX "D"
#define RELMODEL_INDEX_PREFIX "I"
#define RELMODEL_SRCINFO_PREFIX "{"
#define RELMODEL_SRCINFO_SUFFIX "}"
#define RELMODEL_ROWGROUPID_PREFIX "G:"


#define DATA_KEY_SIZE_MAX 256
#define PARTITION_KEY_SIZE_MAX 256
#define PARTITION_KEY_CNT_MAX (PARTITION_KEY_SIZE_MAX/sizeof(unsigned long long))	//32


typedef union Partition {
		char partitionString[PARTITION_KEY_SIZE_MAX];
		unsigned long long partitionInt[PARTITION_KEY_CNT_MAX];
} Partition;

typedef struct NewDataKeyInfo {
	int tableId;
	int rowGroupId;
	int rowCnt;
	uint32_t isPartitionString:1, partitionCnt:31;
	Partition partitionInfo;
	uint32_t timeStamp;
} NewDataKeyInfo;

