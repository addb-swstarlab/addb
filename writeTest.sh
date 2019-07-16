# Use case
# ./writeTest.sh TABLE_ID=${TABLE_ID} MODE=${MODE} \
#                PARTITION_COUNT=${PARTITION_COUNT} \
#                COLUMN_COUNT=${COLUMN_COUNT} \
#                INSERT_ITER_CNT=${INSERT_ITER_CNT}

# Example 1 - Randomize Test
# ./writeTest.sh TABLE_ID=100 MODE=random PARTITION_COUNT=100 COLUMN_COUNT=4 INSERT_ITER_CNT=200
#   TABLE_ID=100
#   MODE=random
#   PARTITION_COUNT=100
#   COLUMN_COUNT=4
#   INSERT_ITER_CNT=200

# Example 2 - Static Test (Always same output)
# ./writeTest.sh TABLE_ID=100 MODE=static PARTITION_COUNT=100 COLUMN_COUNT=4 INSERT_ITER_CNT=200
#   TABLE_ID=100
#   MODE=static
#   PARTITION_COUNT=100
#   COLUMN_COUNT=4
#   INSERT_ITER_CNT=200

# Parses arguments...
for ARGUMENT in "$@"
do
  KEY=$(echo $ARGUMENT | cut -f1 -d=)
  VALUE=$(echo $ARGUMENT | cut -f2 -d=)
  case "$KEY" in
    TABLE_ID)             TABLE_ID=${VALUE} ;;
    MODE)                 MODE=${VALUE} ;;
    PARTITION_COUNT)      PARTITION_COUNT=${VALUE} ;;
    COLUMN_COUNT)         COLUMN_COUNT=${VALUE} ;;
    INSERT_ITER_CNT)      INSERT_ITER_CNT=${VALUE} ;;
    *)
  esac
done

echo "TABLE_ID:             $TABLE_ID"
echo "MODE:                 $MODE"
echo "PARTITION_COUNT:      $PARTITION_COUNT"
echo "COLUMN_COUNT:         $COLUMN_COUNT"
echo "INSERT_ITER_CNT:      $INSERT_ITER_CNT"

# Randomizes collecting partitions
PARTITIONS=()
for i in $(seq 1 $PARTITION_COUNT); do
  if [ $MODE == 'random' ]; then
    RANDOM_PARTITION_COLUMN=$(((RANDOM % COLUMN_COUNT) + 1))
    RANDOM_PARTITION_VALUE=$(((RANDOM % 100000) + 1))
    PARTITION="$RANDOM_PARTITION_COLUMN:$RANDOM_PARTITION_VALUE"
    PARTITIONS+=($PARTITION)
  else
    # Fixes column to 1
    PARTITION="1:$i"
    PARTITIONS+=($PARTITION)
  fi
done

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
  if [ $MODE == 'random' ]; then
    SELECTED_PARTITION=${PARTITIONS[$RANDOM % $PARTITION_COUNT]}
  else
    SELECTED_PARTITION=${PARTITIONS[$i % $PARTITION_COUNT]}
  fi
  CMD="./src/redis-cli fpwrite D:{$TABLE_ID:$SELECTED_PARTITION} $PARTITION $COLUMN_COUNT 0 $values"
  echo $CMD
  $CMD
done
