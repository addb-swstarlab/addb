# Test scripts
ADDB Test Help Scripts!

## Command Generator
cmdGenerator.sh

### How to use this?
#### Random Test
```bash
./cmdGenerator.sh TABLE_ID=100 MODE=random PARTITION_COUNT=100 COLUMN_COUNT=4 SCAN_LOOK_UP_COLUMNS="1,2,3" INSERT_ITER_CNT=200 SCAN_ITER_CNT=30
```

#### Static Test (Always same output)
```bash
./cmdGenerator.sh TABLE_ID=100 MODE=static PARTITION_COUNT=100 COLUMN_COUNT=4 SCAN_LOOK_UP_COLUMNS="1,2,3" INSERT_ITER_CNT=200 SCAN_ITER_CNT=30
```

These commands creates 2 files, 'fpWriteCmd.resp' and 'fpScanCmd.resp'. \
After creating resp files, you can call multiple commands by super fast! \

#### Call Multiple Commands
```bash
cat ./test_scripts/fpWriteCmd.resp | ./src/redis-cli --pipe
```

