#!/bin/bash

while getopts "bg" flag
do
    case "$flag" in
        b) BUILD="1";;
        g) GENERATE="1";;
    esac
done

shift $(($OPTIND - 1))
if [ -z $1 ]
then
    echo "Usage : ./run.sh [-b BUILD] [-g GENERATE_RESULTS] binary_name"
    exit 1
fi

if [ ! -z $BUILD ]
then
    ./build_champsim.sh $1 no no no no lru 1 $1
fi

pids=""

for i in 1 2 3 4 9 # {10..39}
do  
    ./run_champsim.sh $1 10 10 server_00$i.champsimtrace.xz $1 &
    pids="$pids $!"
done

for pid in $pids
do
    wait $pid 
done

if [ ! -z $GENERATE ]
then
    python3 generate_results.py
fi