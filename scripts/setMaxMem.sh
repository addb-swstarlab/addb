#!/bin/bash
ADDB_DIR=/home/addb/addb-RR
CONF_DIR=$ADDB_DIR/conf
GREP1="grep maxmemory"
GREP2="grep -v #"
GREP3="grep -v policy"
OPTIONNAME=maxmemory
FILENAME=setMaxMem.sh
if [ -z $1 ]; then
	echo "[ERROR] Please enter $OPTIONNAME size in first paramter"
	echo "Ex) ./$FILENAME 5GB   => Set $OPTIONNAME 5GB"
	exit 1;
fi
for port in 8000 8001 8002 8003 8004 8005
do
	MAXMEMORY=$(cat ${CONF_DIR}/redis_tiering_${port}.conf | $GREP1 | $GREP2 | $GREP3)
	sed -i".old" "s/$MAXMEMORY/$OPTIONNAME ${1}/" "${CONF_DIR}/redis_tiering_${port}.conf"
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
	echo $(cat ${CONF_DIR}/redis_tiering_${port}.conf | $GREP1 | $GREP2 | $GREP3 )
done
