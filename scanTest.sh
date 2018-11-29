# Use case
# ./scanTest.sh TABLE_ID=${TABLE_ID} RANDOM_PARTITION=${RANDOM_PARITION} \
#               PARTITION_COUNT=${PARTITION_COUNT} \
#               COLUMN_COUNT=${COLUMN_COUNT} \
#               SCAN_LOOK_UP_COLUMNS=${SCAN_LOOK_UP_COLUMNS} \
#               INSERT_ITER_CNT=${INSERT_ITER_CNT} \
#               SCAN_ITER_CNT=${SCAN_ITER_CNT}

# Example
# ./scanTest.sh TABLE_ID=100 RANDOM_PARTITION=yes PARTITION_COUNT=100 COLUMN_COUNT=4 SCAN_LOOK_UP_COLUMNS="1,2,3" INSERT_ITER_CNT=200 SCAN_ITER_CNT=30
#   TABLE_ID=100
#   RANDOM_PARTITION=yes
#   PARTITION_COUNT=100
#   COLUMN_COUNT=4
#   SCAN_LOOK_UP_COLUMNS="1,2,3"
#   INSERT_ITER_CNT=200
#   SCAN_ITER_CNT=30

# Parses arguments...
for ARGUMENT in "$@"
do
  KEY=$(echo $ARGUMENT | cut -f1 -d=)
  VALUE=$(echo $ARGUMENT | cut -f2 -d=)
  case "$KEY" in
    TABLE_ID)             TABLE_ID=${VALUE} ;;
    RANDOM_PARTITION)     RANDOM_PARTITION=${VALUE} ;;
    PARTITION_COUNT)      PARTITION_COUNT=${VALUE} ;;
    COLUMN_COUNT)         COLUMN_COUNT=${VALUE} ;;
    SCAN_LOOK_UP_COLUMNS) SCAN_LOOK_UP_COLUMNS=${VALUE} ;;
    INSERT_ITER_CNT)      INSERT_ITER_CNT=${VALUE} ;;
    SCAN_ITER_CNT)        SCAN_ITER_CNT=${VALUE} ;;
    *)
  esac
done

echo "TABLE_ID:             $TABLE_ID"
echo "RANDOM_PARTITION:     $RANDOM_PARTITION"
echo "PARTITION_COUNT:      $PARTITION_COUNT"
echo "COLUMN_COUNT:         $COLUMN_COUNT"
echo "SCAN_LOOK_UP_COLUMNS: $SCAN_LOOK_UP_COLUMNS"
echo "INSERT_ITER_CNT:      $INSERT_ITER_CNT"
echo "SCAN_ITER_CNT:        $SCAN_ITER_CNT"

# Randomizes collecting partitions
PARTITIONS=()
if [ $RANDOM_PARTITION == 'yes' ]
then
  for i in $(seq 1 $PARTITION_COUNT); do
    RANDOM_PARTITION_COLUMN=$(((RANDOM % COLUMN_COUNT) + 1))
    RANDOM_PARTITION_VALUE=$(((RANDOM % 100000) + 1))
    PARTITION="$RANDOM_PARTITION_COLUMN:$RANDOM_PARTITION_VALUE"
    PARTITIONS+=($PARTITION)
  done
else
  PARTITIONS+=("1:2")
fi

# Varaible counter
counter=1
for i in $(seq 1 $INSERT_ITER_CNT); do
  values=$counter
  if [ $COLUMN_COUNT -ge 2 ]
  then
    for j in $(seq $(($counter + 1)) $(($counter + $COLUMN_COUNT - 1))); do
      values="$values $j"
    done
  fi
  counter=$(($counter + $COLUMN_COUNT))
  SELECTED_PARTITION=${PARTITIONS[$RANDOM % $PARTITION_COUNT]}
  CMD="./src/redis-cli fpwrite D:{$TABLE_ID:$SELECTED_PARTITION} $PARTITION $COLUMN_COUNT 0 $values"
  echo $CMD
  $CMD
done

# CMD="./src/redis-cli metakeys M:{100:*} 3*2*EqualTo:2*2*EqualTo:Or:1*2*EqualTo:0*2*EqualTo:Or:Or:$"
# echo $CMD
# $CMD

for i in $(seq 1 $SCAN_ITER_CNT); do
  SCAN_PARTITION=${PARTITIONS[$RANDOM % $PARTITION_COUNT]}
  CMD="./src/redis-cli fpscan D:{$TABLE_ID:$SCAN_PARTITION} $SCAN_LOOK_UP_COLUMNS"
  echo $CMD
  $CMD
done
