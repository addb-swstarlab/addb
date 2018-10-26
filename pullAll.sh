#!/bin/bash

echo "git pull all node from Relational_Queue branch"
for host in cluster01 cluster05 cluster06 cluster07 cluster09 cluster13 cluster16
do
	ssh $host cd addb-RR; git pull origin Relational_Queue;
	echo "$host is done."
done
