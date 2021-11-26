<p align="center">
    <h1 align="center"> L-TAGE Branch Predictor</h1>
    L-TAGE is a PPM based branch predictor, an extension of the TAGE branch predictor developed by Andre Seznec and Pierre Michaud. It won the second Championship Branch Predction. Here, we present an implementation of L-TAGE and TAGE branch predictors using the Champsim simulator.
</p>

# Instructions to run
- Clone the repository
```
$ git clone https://github.com/KanPard005/RISCY_V_TAGE.git
$ cd RISCY_V_TAGE 
``` 
- Download the traces from [here](https://drive.google.com/file/d/1qs8t8-YWc7lLoYbjbH_d3lf1xdoYBznf/view?usp=sharing) and extract them into a folder

- You might want to follow instructions in the Champsim repository [here](https://github.com/ChampSim/ChampSim)

- Execute the script ***run.sh*** to generate logs of all the traces for the specified branch predictor
```
# Usage : ./run.sh [-b BUILD] [-g GENERATE_RESULTS] [-r RUN] [trace_dir] [branch_predictor] [binary_name OPTIONAL] [results_directory OPTIONAL]
# example: 
$ ./run.sh -bgr ../traces ltage ltage_binary ltage_results 
```

- The results will be created for each trace in the results directory specified.
- To compare results from two branch predictors, execute the following command

```
# Usage : python3 generate_results.py [...DIR_NAMES] [RESULT_DIR] where DIR_NAMES is a list of directories where logs have been stored
# Example :
$ python3 generate_results.py ./tage ./ltage ./hashed_perceptron ./results
```

