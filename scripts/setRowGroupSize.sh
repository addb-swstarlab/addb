#!/bin/bash
ADDB_DIR=/home/addb/addb-RR
CONF_DIR=$ADDB_DIR/conf
if [ -z $1 ]; then
	echo "[ERROR] Please enter row-group size in first paramter"
	echo "Ex) ./setRowGroupSize.sh 50   => Set rowgroup_size 50 [default=100]"
	exit 1;
fi
for port in 8000 8001 8002 8003 8004 8005
do
	ROWGROUPSIZE=$(cat ${CONF_DIR}/redis_tiering_${port}.conf | grep rowgroup_size)
	mv ${CONF_DIR}/redis_tiering_${port}.conf ${CONF_DIR}/redis_tiering_${port}.conf.old
	sed "s/$ROWGROUPSIZE/rowgroup_size ${1}/" "${CONF_DIR}/redis_tiering_${port}.conf.old" >> "${CONF_DIR}/redis_tiering_${port}.conf"
	if [ $? -ne 0 ]; then
		echo "[ERROR] Cannot overwrite configuration file..."
		echo "[ERROR] redis_tiering_${port}.conf.old to redis_tiering_${port}.conf"
	fi
done

echo "Setting is done!"
echo "Finally, check row group size"
for port in 8000 8001 8002 8003 8004 8005
do
	echo "[redis_tiering_${port}.conf]"
	cat ${CONF_DIR}/redis_tiering_${port}.conf | grep rowgroup_size
done
