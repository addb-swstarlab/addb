#!/bin/bash
ADDB_DIR=/home/addb/addb-RR
CONF_DIR=$ADDB_DIR/conf
GREP1="grep loglevel"
OPTIONNAME="loglevel"
FILENAME="setLoglevel.sh"

if [ -z $1 ]; then
	echo "[ERROR] Please enter $OPTIONNAME in first paramter"
	echo "Ex) ./$FILENAME debug   => Set $OPTIONNAME debug"
	echo "Options: notice verbose debug warning"
	exit 1;
fi
for port in 8000 8001 8002 8003 8004 8005
do
	LOGLEVEL=$(cat ${CONF_DIR}/redis_tiering_${port}.conf  | $GREP1 )
	sed -i".old" "s/$LOGLEVEL/$OPTIONNAME ${1}/" "${CONF_DIR}/redis_tiering_${port}.conf"
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
