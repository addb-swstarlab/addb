#!/bin/bash

echo "Check own"
sudo chown -R addb:addb dir/*

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
