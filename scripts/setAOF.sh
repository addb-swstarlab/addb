#!/bin/bash
ADDB_DIR=/home/addb/addb-RR
CONF_DIR=$ADDB_DIR/conf
GREP1="grep appendonly"
GREP2="grep -v #"
GREP3="grep -v appendfilename"
OPTIONNAME=appendonly
FILENAME="setAOF.sh"

if [ -z $1 ]; then
	echo "[ERROR] Please enter $OPTIONNAME option (yes/no) in first paramter"
	echo "Ex) ./$FILENAME yes   => Set $OPTIONNAME yes"
	exit 1;
fi
for port in 8000 8001 8002 8003 8004 8005
do
	AOF=$(cat ${CONF_DIR}/redis_tiering_${port}.conf | $GREP1 | $GREP2 | $GREP3 )
	mv ${CONF_DIR}/redis_tiering_${port}.conf ${CONF_DIR}/redis_tiering_${port}.conf.old
	sed "s/$AOF/$OPTIONNAME ${1}/" "${CONF_DIR}/redis_tiering_${port}.conf.old" >> "${CONF_DIR}/redis_tiering_${port}.conf"
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
	echo $(cat ${CONF_DIR}/redis_tiering_${port}.conf | $GREP1 | $GREP2 | $GREP3 )
done
