#!/bin/bash

while getopts "bgr" flag
do
    case "$flag" in
        b) BUILD="1";;
        g) GENERATE="1";;
        r) RUN="1";;
    esac
done

shift $(($OPTIND - 1))

if [ -z $1 ]
then
    echo "Usage : ./run.sh [-b BUILD] [-g GENERATE_RESULTS] [-r RUN] branch_predictor [binary_name] [results_directory]"
    exit 1
fi

bpred=$1
bin_name=$2
results_dir=$3

if [ -z $bin_name ]
then
    bin_name=$bpred
fi

if [ -z $results_dir ]
then
   results_dir=$bin_name
fi


if [ ! -z $BUILD ]
then
    ./build_champsim.sh $bpred no no no no lru 1 $bin_name
fi

if [ ! -z $RUN ]
then
    pids=""

    for i in 1 2 3 4 9
    do  
        ./run_champsim.sh $bin_name 10 10 server_00$i.champsimtrace.xz $results_dir &
        pids="$pids $!"
    done

    for i in {10..39}
    do  
        ./run_champsim.sh $bin_name 10 10 server_0$i.champsimtrace.xz $results_dir &
        pids="$pids $!"
    done

    for pid in $pids
    do
        wait $pid 
    done
fi

if [ ! -z $GENERATE ]
then
    python3 generate_results.py
fi