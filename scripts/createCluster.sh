#!/bin/bash

command="./src/redis-trib.rb create"
HOST_PREFIX="165.132.172."

for host in 58 60 61 67 68 69 70 # 1 5 6 7 9 13 16
do
  for port in 8000 8001 8002 8003 8004 8005
  do
    command="$command ${HOST_PREFIX}${host}:${port}"
  done
done
$command
