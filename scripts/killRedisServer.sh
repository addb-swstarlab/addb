#!/bin/bash

for I in $(ps -ef | grep redis-server | tr -s ' '| cut -d ' ' -f 2); do
  PID=$(ps -ef | grep -m1 redis-server | tr -s ' ' | cut -d ' ' -f 2)
  PROCESS1=$(ps -ef | grep -m1 redis-server | tr -s ' ' | cut -d ' ' -f 8)
  PROCESS2=$(ps -ef | grep -m1 redis-server | tr -s ' ' | cut -d ' ' -f 9)
  if [ "${PROCESS1}" == "./src/redis-server" ] || [ "${PROCESS2}" == "./src/redis-server" ]; then
    sudo kill -9 ${PID}
  fi
done

echo $(ps -ef | grep redis-server)

