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

- You might want to follow instructions of the Champsim repository [here](https://github.com/ChampSim/ChampSim)

- Execute the script ***run.sh*** to generate logs of all the traces for the specified branch predictor
```
# Usage : ./run.sh [-b BUILD] [-r RUN] [trace_dir] [branch_predictor] [binary_name OPTIONAL] [results_directory OPTIONAL]
# example: 
$ ./run.sh -br ../traces ltage ltage_binary ltage_results 
```

- The results will be created for each trace in the results directory specified.
- To compare results from two branch predictors, execute the following command

```
# Usage : python3 generate_results.py [...DIR_NAMES] [RESULT_DIR] where DIR_NAMES is a list of directories where logs have been stored
# Example :
$ python3 generate_results.py ./tage ./ltage ./hashed_perceptron ./results
```
- To generate plots run:
```
$ python3 generate_accuracy_plots.py 
```
# Code Structure

Since the implementation is made specific to Champsim simulator, the code structure is the same as the source code of Champsim.   

Below is the description of files containing the implementation of the branch predictors.

| File                                          | Description                                                                           |
| ---                                           | ---                                                                                   |                         
| [branch/loop_pred.h](branch/loop_pred.h)      | It implements loop predictor                                                          |
| [branch/loop.h](branch/loop.h)                | It implements a variant of loop predictor component above                             |
| [branch/ltage.bpred](branch/ltage.bpred)      | It implements the Champsim functions associated with branch prediction for LTAGE      |
| [branch/tage.bpred](branch/tage.bpred)        | It implements the Champsim functions associated with branch prediction for TAGE       |
| [branch/tage.h](branch/tage.h)                | It implements TAGE predictor                                                          |

Below is the description of folders containing the results of the branch predictors.

| Folder                                        | Description                                                                           |
| ---                                           | ---                                                                                   | 
| [ltage](ltage)                                | contains results associated with ltage predictor for each server trace file.          |
| [tage](tage)                                  | contains results associated with tage predictor for each server trace file.           |
| [hashed_perceptron](hashed_perceptron)        | contains results associated with hashed perceptron predictor for each server trace file|
| [results](results)                            | contains performance scores of each predictor                                         |


# References

- [LTAGE Branch Predictor](https://jilp.org/vol9/v9paper6.pdf)
- [TAGE Branch Predictor](https://jilp.org/vol9/v9paper6.pdf)
- [CBP2 Source Files](https://hpca23.cse.tamu.edu/taco/camino/cbp2/source.html)
- [CBP4 TAGE-SC-L Branch Predictor](https://jilp.org/cbp2014/program.html)
