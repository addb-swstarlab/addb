#!/bin/bash
#sudo killall redis-server

# Check permission
if [ "${EUID}" -ne 0 ]; then
  echo "[Error] please run as sudo"
  exit 1
fi

#sudo killall redis-server
RESULT=$(ps -ef | grep redis-server | tr -s ' ' | cut -d ' ' -f 2)
for port in ${RESULT[@]}; do
  sudo kill -9 $port
done

echo $(ps -ef | grep redis-server)

