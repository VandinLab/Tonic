# Instructions to reproduce the main experiments for Tonic with MinDegreePredictor Updated (Section 7.3.2 in the main paper and Section B.8.4 in Appendix)

---

Here are the instructions to reproduce the main experiments for *Tonic* combined with *Unbiased Space Saving (USS)* to update the *MinDegreePredictor* for sequences of graph streams. Please make sure to read the instructions in `../README.md` before continuing. Following the previous instructions ensures that the code and data are ready for running the scripts in this folder.

Main results for *Tonic* with *MinDegreePredictor (MDP) Updated* are reported in *Table 2* in the paper. We provide the scripts to reproduce each entry in the table separately as well as meta scripts to easily reproduce the whole table. All scripts should be run from the `root/scripts/experiments/tonic_with_mdp_updated/fair_memory_setting_experiments` folder (the folder where this README.md is located), where root refers to the root directory of the GitHub repository. If you do not want the instructions to run each method separately, please skip to [Meta scripts](#meta-scripts) for a simple procedure to reproduce all the results contained in *Table 2*.

---

# Single experiments scripts

1. *Tonic* with *MDP Updated*, *MDP IncreasedBudget* *MDP IncreasedSize*, and *MDP Split* scripts should be run as follows:
   <br><br>
   `python <script_name>.py -d <dataset_folder> -o <oracle_min_degree_path> -b <nbar_file> -c <multiplier> -t <n_trials> -n <name>`
   <br><br>
   where *script_name* is the name of the script to be run (*exec_mdp_updated*, *exec_mdp_increased_budget* *exec_mdp_increased_size*, or *exec_mdp_split*), *dataset_folder* is the path to the folder containing preprocessed snapshot files, *oracle_min_degree_path* is the path to the *MinDegreePredictor* file obtained from the first snapshot (please read the note below to correctly set this parameter), *nbar_file* is a path to the .txt file containing one oracle size per row, *multiplier* is an integer that scales the values in *nbar_file* (parameter *c* in the paper), *n_trials* is the number of independent trials to run per snapshot, and *name* is the base name for the output results.
   
   *Note*: It is important to send the proper file path for the *oracle_min_degree_path* parameter. For *MDP Updated* and *MDP IncreasedBudget* experiments (using *exec_mdp_updated.py* and *exec_mdp_increased_budget.py* scripts, respectively), it should be the *MinDegreePredictor* for the first snapshot with `\bar{n}_{1}` node-degree pairs. On the other hand, for the *MDP IncreasedSize* and *MDP Split* experiments using *exec_mdp_increased_size.py* and *exec_mdp_split.py* scripts, respectively, it should be the *MinDegreePredictor* containing all node-degree pairs for the first snapshot. All the other parameters, except for *name*, are shared across all scripts.
   <br><br>

2. *Tonic* with *MinDegreePredictor* and *OracleExact* experiments are reproduced using one script, which should be run as follows:
    <br><br>
    `python exec_mdp_and_oracle_exact.py -d <dataset_folder> -o <oracle_exact_path> -i <oracle_min_degree_path> -t <n_trials> -n <name>`
     <br><br>
    where *dataset_folder* is the path to the folder containing preprocessed snapshot files, *oracle_exact_path* is the path to the *OracleExact* predictor file generated from the first snapshot, *oracle_min_degree_path* is the path to the *MinDegreePredictor* file generated from the first snapshot, *n_trials* is the number of independent trials to run per snapshot, and *name* is the base name under which the output results will be saved.

    *Note*: To obtain the *OracleExact* predictor file for the first snapshot, run the *BuildOracle* binary with proper parameters. Instructions to do this are detailed in `root/README.md`.

---

# Meta scripts

Meta scripts parameters are managed by the corresponding configuration (.yaml) files as explained in `../README.md`. Each configuration file provides an example setup with parameter descriptions. To run the meta scripts, example configurations need to be replaced with real parameter values, based on your project structure. For example, *dataset_folders* parameter contains the entry '/path/to/datasets/as_733', which should be replaced with the path where the preprocessed *AS-733* snapshot files were saved in your project. A similar procedure should be followed for all other parameters.

1. *Tonic* with *MDP Updated*, *MDP IncreasedBudget*, *MDP IncreasedSize*, and *MDP Split* meta script parameters are managed by the `./config/mdp-updated-fair-experiments.yaml` configuration file. Running the *meta_script_mdp_updated_fair_experiments.py* script once (with the parameters based on your project structure) reproduces all the results for one predictor (e.g., for *MDP Updated*). To reproduce experiments for all the predictors, run the script once per method (managed by the *script_name* parameter in the configuration file). Since all the parameters are managed by the configuration file, the script should be run as follows:
   <br><br>
   `python meta_script_mdp_updated_fair_experiments.py`
   <br><br>
   
   *Note*: Setting all the parameters is straightforward, except for the *oracle_min_degree_paths*. Here, the same logic from (1) in the [Single experiments scripts](#single-experiments-scripts) applies.
   <br><br>

2. *Tonic* with *MinDegreePredictor* and *OracleExact* experiments meta script parameters are managed by the `./config/tonic-original-predictors.yaml` configuration file. Given that these results do not depend on the value of the parameter *c*, we can run the meta script only once to reproduce all the results contained in *Table 2* for these methods. To do that, replace the configuration examples in the .yaml file with real parameter values (based on your project structure) and run the script as follows:
   <br><br>
   `python meta_script_tonic_original_predictors.py`
   <br><br>
   
   *Note*: To obtain the *OracleExact* predictor file for the first snapshot, run the *BuildOracle* binary with proper parameters. Instructions to do this are detailed in `root/README.md`.

