#!/bin/bash
ADDB_DIR=/home/addb/addb-RR
CONF_DIR=$ADDB_DIR/conf

# Get IP
IFS=$'\n' res=(`ifconfig | grep -Eo 'inet (addr:)?([0-9]*\.){3}[0-9]*' | grep -Eo '([0-9]*\.){3}[0-9]*' | grep -v '127.0.0.1'`)

for port in 8000 8001 8002 8003 8004 8005
do
	BIND=$(cat ${CONF_DIR}/redis_tiering_${port}.conf | grep "bind " | grep -v "#")
	sed -i".old" "s/$BIND/bind ${res[0]}/" "${CONF_DIR}/redis_tiering_${port}.conf"
	if [ $? -ne 0 ]; then
		echo "[ERROR] Cannot overwrite configuration file..."
		exit 1 
	fi
done

echo "Change is done !"
echo "Finally, check bind IP"
for port in 8000 8001 8002 8003 8004 8005
do
	echo "[redis_tiering_${port}.conf]"
	cat ${CONF_DIR}/redis_tiering_${port}.conf | grep "bind " | grep -v "#"
done
