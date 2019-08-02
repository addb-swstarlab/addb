# Use case
# ./runner.sh ITER_CNT=${ITER_CNT} \
#             RESP=${RESP}
#             CLI=${CLI}

# Example 1 - Run 75000_fpwrite 10 times
# ./runner.sh ITER_CNT=100 RESP=test_scripts/75000_fpwrite.resp CLI=src/redis-cli

# Parses arguments...
for ARGUMENT in "$@"
do
  KEY=$(echo $ARGUMENT | cut -f1 -d=)
  VALUE=$(echo $ARGUMENT | cut -f2 -d=)
  case "$KEY" in
    ITER_CNT)             ITER_CNT=${VALUE} ;;
    RESP)                 RESP=${VALUE} ;;
    CLI)                  CLI=${VALUE} ;;
    *)
  esac
done

echo "ITER_CNT:             $ITER_CNT"
echo "RESP:                 $RESP"

for i in $(seq 1 $ITER_CNT); do
  response=$(cat ${RESP} | ./${CLI} --pipe)
  echo $response
done
