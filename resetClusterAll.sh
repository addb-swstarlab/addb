#!/bin/bash

echo "Cluster reset hard in all redis-server"
for host in cluster01 cluster05 cluster06 cluster07 cluster09 cluster13 cluster16
do
	echo "[${host}]"
	ssh $host cd addb-RR; ./scripts/resetCluster.sh
done

