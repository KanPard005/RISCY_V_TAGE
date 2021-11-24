#! /bin/bash
pids=""
# for i in 1 2 3 4 9
for i in 10 11 12 13 14 15
do  
    ./run_champsim.sh $1 10 10 server_0$i.champsimtrace.xz $1 &
    pids="$pids $!"
done

for pid in $pids
do
    wait $pid 
done