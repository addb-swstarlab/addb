#!/bin/bash

ADDB_DIR=$(pwd)
echo "1) Check own"
sudo chown -R ${USER}:${USER} dir/*
echo

echo "2) Make dir directories"
./scripts/init.sh
echo

# Change redis configuration
## setConf
FILENAME="setConf.sh"
echo "3) Edit ${FILENAME}"
OLD=$(cat scripts/${FILENAME} | grep ADDB_DIR | grep -v CONF_DIR | grep -v SCRIPT_DIR )
sed -i".old" -e "s@$OLD@ADDB_DIR=${ADDB_DIR}@" "scripts/${FILENAME}"  >> "scripts/${FILENAME}"
if [ $? -ne 0 ]; then
	echo "[ERROR] Cannot overwrite configuration file"
else
	chmod +x scripts/${FILENAME}
	rm scripts/${FILENAME}.old
fi
echo

echo "4) Run ${FILENAME}"
./scripts/${FILENAME} help
echo

