# Instructions to reproduce the preliminary experiments for Tonic with MinDegreePredictor Updated (Sections B.8.1 in and B.8.2 in Appendix)

---

Here are the instructions to reproduce the preliminary experiments for *Tonic* combined with *Unbiased Space Saving (USS)* to update the *MinDegreePredictor* for sequences of graph streams. Please make sure to read the instructions in `../README.md` before continuing. Following the previous instructions ensures that the code and data are ready for running the scripts in this folder.

Preliminary experiments include *MinDegreePredictor* similarity (*Section B.8.1 in Appendix*, *Figure B7*) and *USS* (*Section B.8.2 in Appendix*, *Figure B8*) experiments. We provide the scripts to reproduce each method (including the parameter setting) shown in the figures separately, as well as meta scripts to reproduce all the results. All scripts should be run from the `root/scripts/experiments/tonic_with_mdp_updated/preliminary_analysis_experiments` folder (the folder where this README.md is located), where root refers to the root directory of the GitHub repository. If you do not want the instructions to run each method separately, please skip to [Meta scripts](#meta-scripts) for a simple procedure to reproduce all the results from the preliminary analysis.

---

# Single experiments scripts

1. *MinDegreePredictor* similarity experiments scripts should be run as follows:
   <br><br>
   `python <script_name>.py -o <oracle_min_degree_folder> -n <name>`
   <br><br>
   where *script_name* is the name of the script to be run (*exec_first_snapshot_mdp_similarity_experiments* or *exec_previous_snapshot_mdp_similarity_experiments*), *oracle_min_degree_folder* is the folder containing the *MinDegreePredictor* files with `\bar{n}_{i}` node-degree pairs for snapshot *i*, and *name* is the base name for the output results.
   <br><br>

2. *USS* experiments script should be run as follows:
   <br><br>
   `python exec_uss_experiments.py -d <dataset_folder> -o <oracle_min_degree_folder> -c <multiplier> -t <n_trials> -n <name>`
   <br><br>
   where *dataset_folder* is the path to the folder containing preprocessed snapshot files, *oracle_min_degree_folder* is the folder containing the *MinDegreePredictor* files with `\bar{n}_{i}` node-degree pairs for snapshot *i*, *multiplier* is an integer that determines the *USS* capacity per snapshot (parameter *c* in the paper), *n_trials* is the number of independent trials to run per snapshot, and *name* is the base name under which the output results will be stored.

---

# Meta scripts

Meta scripts parameters are managed by the corresponding configuration (.yaml) files as explained in `../README.md`. Each configuration file provides an example setup with parameter descriptions. To run the meta scripts, example configurations need to be replaced with real parameter values, based on your project structure. For example, *oracle_min_degree_folders* parameter contains the entry '/path/to/oracles/as_733/min_degree_predictor_folder', which should be replaced with the path where the *MinDegreePredictor* oracles for the *AS-733* dataset were saved in your project. A similar procedure should be followed for all other parameters.

1. *MinDegreePredictor* similarity meta script parameters are managed by the `./config/mdp-similarity.yaml` configuration file. Running the script once (with the parameters based on your project structure) reproduces all the experiments for one method. To reproduce experiments for both methods, run the script once per method (managed by the *script_name* parameter in the configuration file). Since all the parameters are managed by the configuration file, the script should be run as follows:
   <br><br>
   `python meta_script_mdp_similarity.py`
   <br><br>
   
2. *USS* experiments are managed by the `./config/uss-experiments.yaml `configuration file. To reproduce all the results, replace the configuration examples in the .yaml file with real parameter values (based on your project structure) and run the script as follows:
   <br><br>
   `python meta_script_uss.py`
   <br><br>

