#!/bin/bash
ADDB_DIR=/home/addb/addb-RR
CONF_DIR=$ADDB_DIR/conf
GREP1="grep no-appendfsync-on-rewrite"
OPTIONNAME="no-appendfsync-on-rewrite"
FILENAME="setRewrite.sh"
if [ -z $1 ]; then
	echo "[ERROR] Please enter rewrite option (yes/no) in first paramter"
	echo "Ex) ./$FILENAME yes   => Set $OPTIONAME yes"
	exit 1;
fi

for port in 8000 8001 8002 8003 8004 8005
do
	REWRITE=$(cat ${CONF_DIR}/redis_tiering_${port}.conf | grep "no-appendfsync-on-rewrite")
	mv ${CONF_DIR}/redis_tiering_${port}.conf ${CONF_DIR}/redis_tiering_${port}.conf.old
	sed "s/$REWRITE/$OPTIONNAME ${1}/" "${CONF_DIR}/redis_tiering_${port}.conf.old" >> "${CONF_DIR}/redis_tiering_${port}.conf"
	if [ $? -ne 0 ]; then
		echo "[ERROR] Cannot overwrite configuration file..."
		echo "[ERROR] redis_tiering_${port}.conf.old to redis_tiering_${port}.conf"
		mv ${CONF_DIR}/redis_tiering_${port}.conf.old ${CONF_DIR}/redis_tiering_${port}.conf
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
