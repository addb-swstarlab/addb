# Use case
# ./scanTest.sh ${TABLE_ID} ${RANDOM_PARITION} ${COLUMN_COUNT} ${SCAN_LOOK_UP_COLUMNS} ${ITERATION_COUNT}
# Example
# ./scanTest.sh 100 yes 4 "1,2,3"
#   TABLE_ID=100
#   RANDOM_PARTITION=yes
#   COLUMN_COUNT=4
#   SCAN_LOOK_UP_COLUMNS="1,2,3"
#   ITERATION_COUNT=200

TABLE_ID=$1
RANDOM_PARTITION=$2
COLUMN_COUNT=$3
SCAN_LOOK_UP_COLUMNS=$4
ITERATION_COUNT=$5

# RANDOM_PARTITION
if [ $RANDOM_PARTITION == 'yes' ]
then
  RANDOM_PARTITION_COLUMN=$(((RANDOM % COLUMN_COUNT) + 1))
  RANDOM_PARTITION_VALUE=$(((RANDOM % 100000) + 1))
  PARTITION="$RANDOM_PARTITION_COLUMN:$RANDOM_PARTITION_VALUE"
else
  PARTITION="1:2"
fi

# Varaible counter
counter=1
for i in $(seq 1 $ITERATION_COUNT); do
  values=$counter
  if [ $COLUMN_COUNT -ge 2 ]
  then
    for j in $(seq $(($counter + 1)) $(($counter + $COLUMN_COUNT - 1))); do
      values="$values $j"
    done
  fi
  counter=$(($counter + $COLUMN_COUNT))
  CMD="./src/redis-cli fpwrite D:{$TABLE_ID:$PARTITION} $PARTITION $COLUMN_COUNT 0 $values"
  echo $CMD
  $CMD
done

CMD="./src/redis-cli fpscan D:{$TABLE_ID:$PARTITION} $SCAN_LOOK_UP_COLUMNS"
echo $CMD
$CMD