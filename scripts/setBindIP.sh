#!/bin/bash
ADDB_DIR=/home/addb/addb-RR
CONF_DIR=$ADDB_DIR/conf
if [ -z $1 ]; then
	echo "[ERROR] Please enter bind IP in first paramter"
	echo "Ex) ./setBindIP.sh 165.132.172.60   => Set bind 165.132.172.60"
	exit 1;
fi
for port in 8000 8001 8002 8003 8004 8005
do
	BIND=$(cat ${CONF_DIR}/redis_tiering_${port}.conf | grep "bind " | grep -v "#")
	sed "s/$BIND/bind ${1}/" "${CONF_DIR}/redis_tiering_${port}.conf" >> "${CONF_DIR}/redis_tiering_${port}_new.conf"
	if [ $? -eq 0 ]; then
		mv ${CONF_DIR}/redis_tiering_${port}_new.conf ${CONF_DIR}/redis_tiering_${port}.conf
	else
		echo "[ERROR] Cannot overwrite configuration file..."
		echo "[ERROR] redis_tiering_${port}_new.conf to redis_tiering_${port}.conf"
	fi
done

echo "Change is done !"
echo "Finally, check bind IP"
for port in 8000 8001 8002 8003 8004 8005
do
	echo "[redis_tiering_${port}.conf]"
	cat ${CONF_DIR}/redis_tiering_${port}.conf | grep "bind " | grep -v "#"
done
