#!/bin/bash

for I in $(ps -ef | grep redis-server | tr -s ' '| cut -d ' ' -f 2); do
  PID=$(ps -ef | grep -m1 redis-server | tr -s ' ' | cut -d ' ' -f 2)
  PROCESS=$(ps -ef | grep -m1 redis-server | tr -s ' ' | cut -d ' ' -f 8)
  if [ "${PROCESS}" == "./src/redis-server" ]; then
    sudo kill -9 ${PID}
  fi
done

echo $(ps -ef | grep redis-server)

