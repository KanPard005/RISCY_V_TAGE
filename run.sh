#!/bin/bash

while getopts "bgr" flag
do
    case "$flag" in
        b) BUILD="1";;
        r) RUN="1";;
    esac
done

shift $(($OPTIND - 1))

if [[ -z $1 || -z $2 ]]
then
    echo "Usage : ./run.sh [-b BUILD] [-r RUN] [trace_dir] [branch_predictor] [binary_name OPTIONAL] [results_directory OPTIONAL]"
    exit 1
fi

trace_dir=$1
bpred=$2
bin_name=$3
results_dir=$4

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
        ./run_champsim.sh $bin_name 10 10 server_00$i.champsimtrace.xz $results_dir $trace_dir &
        pids="$pids $!"
    done

    for i in {10..39}
    do  
        ./run_champsim.sh $bin_name 10 10 server_0$i.champsimtrace.xz $results_dir $trace_dir &
        pids="$pids $!"
    done

    for pid in $pids
    do
        wait $pid 
    done
fi