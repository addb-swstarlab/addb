#!/bin/bash
#watch -d -n 1 "du -h dir/ssd1/8000; du -h dir/ssd2/8003; ./scripts/clusterCommand8.sh dbsize"
watch -d -n 1 "du -h dir/ssd1/8000/0:default; du -h dir/ssd1/8001/0:default; du -h dir/ssd1/8002/0:default; du -h dir/ssd2/8003/0:default; du -h dir/ssd2/8004/0:default; du -h dir/ssd2/8005/0:default; ./scripts/clusterCommand8.sh dbsize"
