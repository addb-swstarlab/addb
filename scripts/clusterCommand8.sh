# Get IP
IFS=$'\n' res=(`ifconfig | grep -Eo 'inet (addr:)?([0-9]*\.){3}[0-9]*' | grep -Eo '([0-9]*\.){3}[0-9]*' | grep -v '127.0.0.1'`)

if test "$#" -ne 0; then
	echo "[8000]"
	./src/redis-cli -c -p 8000 -h ${res[0]} $@
	echo "[8001]"
	./src/redis-cli -c -p 8001 -h ${res[0]} $@
	echo "[8002]"
	./src/redis-cli -c -p 8002 -h ${res[0]} $@
	echo "[8003]"
	./src/redis-cli -c -p 8003 -h ${res[0]} $@
	echo "[8004]"
	./src/redis-cli -c -p 8004 -h ${res[0]} $@
	echo "[8005]"
	./src/redis-cli -c -p 8005 -h ${res[0]} $@
	exit 1
fi
