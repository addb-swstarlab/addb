if test "$#" -ne 0; then
	echo "[8000]"
	./src/redis-cli -c -p 8000 -h 192.168.1.5 $@
	echo "[8001]"
	./src/redis-cli -c -p 8001 -h 192.168.1.5 $@
	echo "[8002]"
	./src/redis-cli -c -p 8002 -h 192.168.1.5 $@
	echo "[8003]"
	./src/redis-cli -c -p 8003 -h 192.168.1.5 $@
	echo "[8004]"
	./src/redis-cli -c -p 8004 -h 192.168.1.5 $@
	echo "[8005]"
	./src/redis-cli -c -p 8005 -h 192.168.1.5 $@
	exit 1
fi
