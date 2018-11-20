#!/bin/bash

ADDB_DIR=/home/addb/addb-RR
CONF_DIR=$ADDB_DIR/conf
SCRIPT_DIR=$ADDB_DIR/scripts
PORTS=( 8000 8001 8002 8003 8004 8005 )

# Check first argument
if [ -z $1 ]; then
	echo "[ERROR] Please enter option in first parameter"
	echo "or type help command ( ./setConf.sh help ) "
	exit 1;
fi

# 1) Get command grep result
# Commands
#  : AOF, BindIP, ColumnVector_size, LogLevel, MaxMemory, Rewrite, RowGroup_size, Ziplist_entry
case ${1,,} in
	"help")
		echo "# Command list (Changable options) and examples"
		echo "# You can type both upper-case and lower-case"
		echo "Command:				value					Examples"
		echo "AOF				[yes|no]				./setConf.sh aof no"
		echo "IP				<AutoComplete>				./setConf.sh IP"
		echo "ColumnVector_size		[int]					./setConf.sh cv 100"
		echo "LogLevel			[notice|verbose|debug|warning]		./setConf.sh log verbose"
		echo "MaxMemory			[int][mb|gb]				./setConf.sh memory 3gb"
		echo "Rewrite				[yes|no]				./setConf.sh rewrite no"
		echo "RowGroup_size			[int]					./setConf.sh rg 500"
		echo "Ziplist_entry			[int]					./setConf.sh ziplist 1000000"
		exit 0;
	;;
	"aof")
		if [ -z $2 ]; then
			echo "[ERROR] Please enter value in second parameter"
			exit 1;
		fi
		GREP1="grep appendonly"
		GREP2="grep -v #"
		GREP3="grep -v appendfilename"
		OPTIONNAME="appendonly"
	;;
	"ip")
		OPTIONNAME="bind"
		IFS=$'\n' res=(`ifconfig | grep -Eo 'inet (addr:)?([0-9]*\.){3}[0-9]*' | grep -Eo '([0-9]*\.){3}[0-9]*' | grep -v '127.0.0.1'`)
	;;
	"cv")
		if [ -z $2 ]; then
			echo "[ERROR] Please enter value in second parameter"
			exit 1;
		fi
		GREP1="grep columnvector_size"
		OPTIONNAME="columnvector_size"
	;;
	"log")
		if [ -z $2 ]; then
			echo "[ERROR] Please enter value in second parameter"
			exit 1;
		fi
		GREP1="grep loglevel"
		OPTIONNAME="loglevel"
	;;
	"memory")
		if [ -z $2 ]; then
			echo "[ERROR] Please enter value in second parameter"
			exit 1;
		fi
		GREP1="grep maxmemory"
		GREP2="grep -v #"
		GREP3="grep -v policy"
		OPTIONNAME="maxmemory"
	;;
	"rewrite")
		if [ -z $2 ]; then
			echo "[ERROR] Please enter value in second parameter"
			exit 1;
		fi
		GREP1="grep no-appendfsync-on-rewrite"
		OPTIONNAME="no-appendfsync-on-rewrite"
	;;
	"rg")
		if [ -z $2 ]; then
			echo "[ERROR] Please enter value in second parameter"
			exit 1;
		fi
		GREP1="grep rowgroup_size"
		OPTIONNAME="rowgroup_size"
	;;
	"ziplist")
		if [ -z $2 ]; then
			echo "[ERROR] Please enter value in second parameter"
			exit 1;
		fi
		GREP1="grep hash-max-ziplist-entries"
		OPTIONNAME="hash-max-ziplist-entries"
	;;
	*)
		echo "[ERROR] Please enter correct option name in first parameter"
		echo "or type help command ( ./setConf.sh help ) "
		exit 1;
	;;
esac

# 2) replacement
for port in "${PORTS[@]}"
do
	# 2-1) grep and replace
	# no parameter
	if [ "${1,,}" == "ip" ]; then
		TotalGrep=$(cat ${CONF_DIR}/redis_tiering_${port}.conf | grep "bind " | grep -v "#" )
		sed -i".old" "s/$TotalGrep/$OPTIONNAME ${res[0]}/" "${CONF_DIR}/redis_tiering_${port}.conf"	
	# need parameters
	else
		# # of GREP = 1
		if [ "${1,,}" == "cv" ] || [ "${1,,}" == "log" ] || [ "${1,,}" == "rewrite" ] || [ "${1,,}" == "rg" ] || [ "${1,,}" == "ziplist" ]; then
			TotalGrep=$(cat ${CONF_DIR}/redis_tiering_${port}.conf | $GREP1 )
		# # of GREP = 2
		
		# # of GREP = 3
		elif [ "${1,,}" == "aof" ] || [ "${1,,}" == "memory" ]; then
			TotalGrep=$(cat ${CONF_DIR}/redis_tiering_${port}.conf | $GREP1 | $GREP2 | $GREP3 )
		fi
		sed -i".old" "s/$TotalGrep/$OPTIONNAME ${2}/" "${CONF_DIR}/redis_tiering_${port}.conf"	
	fi
	# ERROR CHECKING
	if [ $? -ne 0 ]; then
                echo "[ERROR] Cannot overwrite configuration file..."
                exit 1
        fi
	# 2-2) print result
	echo "[redis_tiering_${port}.conf]"
	if [ "${1,,}" == "cv" ] || [ "${1,,}" == "log" ] || [ "${1,,}" == "rewrite" ] || [ "${1,,}" == "rg" ] || [ "${1,,}" == "ziplist" ]; then
		echo $(cat ${CONF_DIR}/redis_tiering_${port}.conf | $GREP1 )
	elif [ "${1,,}" == "ip" ]; then
		echo $(cat ${CONF_DIR}/redis_tiering_${port}.conf | grep "bind " | grep -v "#"  )
	elif [ "${1,,}" == "aof" ] || [ "${1,,}" == "memory" ]; then
		echo $(cat ${CONF_DIR}/redis_tiering_${port}.conf | $GREP1 | $GREP2 | $GREP3 )
	fi
done

