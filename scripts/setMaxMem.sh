#!/bin/bash
ADDB_DIR=/home/addb/addb-RR
CONF_DIR=$ADDB_DIR/conf
if [ -z $1 ]; then
	echo "[ERROR] Please enter maxmemory size in first paramter"
	echo "Ex) ./setMaxMem.sh 5   => Set maxmemory 5GB"
	exit 1;
fi
for port in 8000 8001 8002 8003 8004 8005
do
	MAXMEMORY=$(cat ${CONF_DIR}/redis_tiering_${port}.conf | grep maxmemory | grep GB)
	sed "s/$MAXMEMORY/maxmemory ${1}GB/" "${CONF_DIR}/redis_tiering_${port}.conf" >> "${CONF_DIR}/redis_tiering_${port}_new.conf"
	if [ $? -eq 0 ]; then
		mv ${CONF_DIR}/redis_tiering_${port}_new.conf ${CONF_DIR}/redis_tiering_${port}.conf
	else
		echo "[ERROR] Cannot overwrite configuration file..."
		echo "[ERROR] redis_tiering_${port}_new.conf to redis_tiering_${port}.conf"
	fi
done

echo "Setting is done!"
echo "Finally, check maxmemory"
for port in 8000 8001 8002 8003 8004 8005
do
	echo "[redis_tiering_${port}.conf]"
	cat ${CONF_DIR}/redis_tiering_${port}.conf | grep maxmemory | grep GB
done
