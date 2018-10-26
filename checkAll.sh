#!/bin/bash

echo "Check all redis-server"
for host in cluster01 cluster05 cluster06 cluster07 cluster09 cluster13 cluster16
do
	echo "[${host}]"
	ssh $host ./addb-RR/scripts/checkRedisServer.sh
done
