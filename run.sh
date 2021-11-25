#! /bin/bash

./build_champsim.sh $1 no no no no lru 1 $1

pids=""
for i in 1 2 3 4 9
do  
    ./run_champsim.sh $1 10 10 server_00$i.champsimtrace.xz $1 &
    pids="$pids $!"
done

for i in {10..39}
do  
    ./run_champsim.sh $1 10 10 server_0$i.champsimtrace.xz $1 &
    pids="$pids $!"
done

for pid in $pids
do
    wait $pid 
done