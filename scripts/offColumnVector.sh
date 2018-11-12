#!/bin/bash
ADDB_DIR=/home/addb/addb-RR
CONF_DIR=$ADDB_DIR/conf
GREP1="grep columnvector_size"
OPTIONNAME="columnvector_size"
FILENAME="offColumnVector.sh"

for port in 8000 8001 8002 8003 8004 8005
do
	CV=$(cat ${CONF_DIR}/redis_tiering_${port}.conf | $GREP1 )
	sed -i".old" "s/$CV/#${CV}/" "${CONF_DIR}/redis_tiering_${port}.conf.old" >> "${CONF_DIR}/redis_tiering_${port}.conf"
	if [ $? -ne 0 ]; then
		echo "[ERROR] Cannot overwrite configuration file..."
		exit 1
	fi
done

echo "Setting is done!"
echo "Finally, check $OPTIONNAME"
for port in 8000 8001 8002 8003 8004 8005
do
	echo "[redis_tiering_${port}.conf]"
	echo $(cat ${CONF_DIR}/redis_tiering_${port}.conf | $GREP1 )
done
