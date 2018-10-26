#!/bin/bash

#echo "(1/3) Set Bind IP"
#for host in cluster01 cluster05 cluster06 cluster07 cluster09 cluster13 cluster16
#do
#	ssh $host ./addb-RR/scripts/setBindIP.sh
#	echo "$host is done."
#done

echo "(1/2) Set Maxmemory"
echo "== Enter max memory  (ex 5GB) =="
read maxmemory
for host in cluster01 cluster05 cluster06 cluster07 cluster09 cluster13 cluster16
do
	ssh $host ./addb-RR/scripts/setMaxMem.sh $maxmemory
	echo "$host is done."
done
sleep 3
echo ""
echo "(2/2) Set rowgroup_size"
echo "== Enter row group size  (ex 100) =="
read rowgroupsize
for host in cluster01 cluster05 cluster06 cluster07 cluster09 cluster13 cluster16
do
	ssh $host ./addb-RR/scripts/setRowGroupSize.sh $rowgroupsize
	echo "$host is done."
done
