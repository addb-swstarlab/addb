#!/bin/bash

ADDB_DIR=$(pwd)
echo "1) Check own"
sudo chown -R ${USER}:${USER} dir/*
echo

echo "2) Make dir directories"
mkdir -p dir
mkdir -p dir/log
mkdir -p dir/run
mkdir -p dir/ssd1
mkdir -p dir/ssd2
mkdir -p dir/ssd1/8000
mkdir -p dir/ssd1/8001
mkdir -p dir/ssd1/8002
mkdir -p dir/ssd1/hadoopData
mkdir -p dir/ssd2/8003
mkdir -p dir/ssd2/8004
mkdir -p dir/ssd2/8005
mkdir -p dir/ssd2/hadoopData
echo

# Change redis configuration
## Bind IP
FILENAME="setBindIP.sh"
echo "3) Edit ${FILENAME}"
mv scripts/${FILENAME} scripts/${FILENAME}.old
OLD=$(cat scripts/${FILENAME}.old | grep ADDB_DIR | grep -v CONF_DIR)
sed -e "s@$OLD@ADDB_DIR=${ADDB_DIR}@" "scripts/${FILENAME}.old"  >> "scripts/${FILENAME}"
if [ $? -ne 0 ]; then
	echo "[ERROR] Cannot overwrite configuration file"
	mv scripts/${FILENAME}.old scripts/${FILENAME}
else
	chmod +x scripts/${FILENAME}
	rm scripts/${FILENAME}.old
fi
echo "3-1) Run ${FILENAME}"
./scripts/${FILENAME}
echo

## Max memory
FILENAME="setMaxMem.sh"
echo "4) Edit ${FILENAME}"
mv scripts/${FILENAME} scripts/${FILENAME}.old
OLD=$(cat scripts/${FILENAME}.old | grep ADDB_DIR | grep -v CONF_DIR)
sed -e "s@$OLD@ADDB_DIR=${ADDB_DIR}@" "scripts/${FILENAME}.old"  >> "scripts/${FILENAME}"
if [ $? -ne 0 ]; then
	echo "[ERROR] Cannot overwrite configuration file"
	mv scripts/${FILENAME}.old scripts/${FILENAME}
else
	chmod +x scripts/${FILENAME}
	rm scripts/${FILENAME}.old
fi
echo "4-1) Run ${FILENAME}"
echo "Enter maxmemory size  ex) 5GB"
read maxmemory
./scripts/${FILENAME} $maxmemory
echo

## Rowgroup size
FILENAME="setRowGroupSize.sh"
echo "5) Edit ${FILENAME}"
mv scripts/${FILENAME} scripts/${FILENAME}.old
OLD=$(cat scripts/${FILENAME}.old | grep ADDB_DIR | grep -v CONF_DIR)
sed -e "s@$OLD@ADDB_DIR=${ADDB_DIR}@" "scripts/${FILENAME}.old"  >> "scripts/${FILENAME}"
if [ $? -ne 0 ]; then
	echo "[ERROR] Cannot overwrite configuration file"
	mv scripts/${FILENAME}.old scripts/${FILENAME}
else
	chmod +x scripts/${FILENAME}
	rm scripts/${FILENAME}.old
fi
echo "5-1) Run ${FILENAME}"
echo "Enter row group size  ex) 5"
read rowgroupsize
./scripts/${FILENAME} $rowgroupsize
echo

## hash-max-ziplist-entries
FILENAME="setZiplist.sh"
echo "6) Edit ${FILENAME}"
mv scripts/${FILENAME} scripts/${FILENAME}.old
OLD=$(cat scripts/${FILENAME}.old | grep ADDB_DIR | grep -v CONF_DIR)
sed -e "s@$OLD@ADDB_DIR=${ADDB_DIR}@" "scripts/${FILENAME}.old"  >> "scripts/${FILENAME}"
if [ $? -ne 0 ]; then
	echo "[ERROR] Cannot overwrite configuration file"
	mv scripts/${FILENAME}.old scripts/${FILENAME}
else
	chmod +x scripts/${FILENAME}
	rm scripts/${FILENAME}.old
fi
echo "6-1) Run ${FILENAME}"
echo "Enter hash-max-ziplist-entries  ex) 100000"
read ziplist
./scripts/${FILENAME} $ziplist
echo

## AOF
FILENAME="setAOF.sh"
echo "7) Edit ${FILENAME}"
mv scripts/${FILENAME} scripts/${FILENAME}.old
OLD=$(cat scripts/${FILENAME}.old | grep ADDB_DIR | grep -v CONF_DIR)
sed -e "s@$OLD@ADDB_DIR=${ADDB_DIR}@" "scripts/${FILENAME}.old"  >> "scripts/${FILENAME}"
if [ $? -ne 0 ]; then
	echo "[ERROR] Cannot overwrite configuration file"
	mv scripts/${FILENAME}.old scripts/${FILENAME}
else
	chmod +x scripts/${FILENAME}
	rm scripts/${FILENAME}.old
fi
echo "7-1) Run ${FILENAME}"
echo "Enter AOF option (yes/no)  ex) yes"
read AOF
./scripts/${FILENAME} $AOF
echo
