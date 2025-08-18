# Instructions to reproduce the experiments for Tonic with MinDegreePredictor Updated

---

Here are the instructions for running *Tonic* combined with *Unbiased Space Saving (USS)* to update the *MinDegreePredictor* for sequences of graph streams. Please make sure to read the instructions in `root/scripts/REAMDE.txt` file before continuing, where root refers to the root directory of the GitHub repository. Following the previous instructions ensures that the code is compiled and ready for running the scripts in this folder.

## Experiments structure

The experiments are split in two parts:

1. Preliminary analysis, including the results for *MinDegreePredictor Similarity* and *USS* experiments (*Sections B.8.1 and B.8.2 in Appendix*, respectively)

2. *Tonic* with the updated *MinDegreePredictor* (*Section 7.3.2 in the main paper* and *Section B.8.4 in Appendix*)

In this file, we explain the general structure of the experiments as well as the common steps required for both the preliminary analysis and the final results. Additional steps required to reproduce the results are explained in the corresponding subfolders. Scripts to reproduce the preliminary analysis can be found in the `./preliminary_analysis_experiments` subfolder, while the scripts for the final experiments are located in `./fair_memory_setting_experiments` Both folders contain two types of scripts:

1. Single-run scripts, which reproduce the result of a single experiment configuration — corresponding to one curve in a plot or one entry in a table (i.e., a specific method with a single setting of parameters). Running the single-run script automatically saves the .csv file with the results in the output folder, based on the *name* parameter.

2. Meta scripts, which automate the reproduction of all configurations required to generate an entire figure or table. These scripts internally call the single-run scripts for each setting.

All meta scripts come with a corresponding configuration file that allows the user to specify the parameters. Each configuration file follows a standard YAML format, where the user specifies dataset paths, oracle paths, parameters like the number of trials, and output name prefixes. Example configurations are provided in each experiment subfolder. This provides flexibility to save data at custom locations while keeping the configuration centralized.

*Note*: Meta scripts currently launch each experiment as an independent subprocess. This results in up to 12 concurrent processes (assuming that dataset and parameter combinations from the paper are used). Each process is typically lightweight, so this should not pose issues on modern machines. However, if you encounter resource constraints (e.g., on a shared server or low-spec laptop), you may want to modify the meta script to run experiments sequentially by replacing the subprocess launching logic with a simple loop using subprocess.run().

In the following, we describe how to use the scripts required to prepare the code and data for either the preliminary analysis or the final experiments. All scripts should be run from the `root/scripts/experiments/tonic_with_mdp_updated` folder (the folder where this README.md is located). While all steps can alternatively be performed by combining the information provided in `root/README.md`, we provide auxiliary scripts to simplify the process.

*Note*: Before we explain how to use the auxiliary scripts, we need to clarify the terminology regarding our *MinDegreePredictor*. If not stated otherwise, the *MinDegreePredictor* for snapshot *i* refers to the predictor with `\bar{n}_{i}` entries, as defined in the paper. However, for some intermediary steps, we also need all sorted node-degree pairs for snapshot *i*, i.e., a *MinDegreePredictor* containing all the nodes. While our predictor update method does not require these maps in practical applications, we need them to set up the experimental setting and to run some of the baselines. To that end, we will explicitly specify whether a given script expects the path to the actual *MinDegreePredictor* (i.e., truncated to `\bar{n}_{i}`) or to the full node-degree pair file (i.e., *MinDegreePredictor* containing all nodes), when it is not clear from the context.

## Installation

   To install the dependencies, Python `>= 3.8` version is required. Please run the following command in `root/scripts/experiments/tonic_with_mdp_updated` folder (the folder where this README.md is located) to install the required Python packages:
   <br><br>
   `pip install -r requirements.txt`
   <br><br>
   
## Scripts usage

1. Preprocess a sequence of raw snapshot dataset files stored in a folder
   <br><br>
   `python exec_preprocess_snapshots.py -i <input_folder> -o <output_folder> -d <delimiter> -s <skip>`
   <br><br>
   where *input_folder* is the path to the folder containing raw snapshot files to be preprocessed, *output_folder* is the destination folder where the preprocessed snapshot files will be stored (with the hardcoded prefix 'preprocessed_'), *delimiter* is the character used to separate the rows in each snapshot file, and *skip* is the number of lines to skip before starting to read each snapshot file.

   *Note*: To pass a **tab character** as the delimiter, use `$'\t'` in the command line (e.g., `-d $'\t'`). To pass a **space character**, enclose it in quotes (e.g., `-d ' '`).
   <br><br>

2. Build the oracle for all snapshots in a sequence 
   <br><br>
   `python exec_build_oracle_snapshots.py -d <dataset_folder> -t <oracle_type = {Exact, noWR, Node}> -p <percentage_retain> -x <prefix> -o <output_folder>`
   <br><br>
   where *dataset_folder* is the path to the folder with preprocessed snapshot files at point (1), *oracle_type* is the type of oracle to be built (Exact, noWR, Node), *percentage_retain* is the fraction of top heaviest edges/nodes to be retained in the oracle, *prefix* is the prefix for each oracle file name (read the note below for details), and *output_folder* is the destination folder where the oracles will be saved. 
   
   *Note*:
   - Setting `oracle_type = Node` and `percentage_retain = 1.0` produces a *MinDegreePredictor* containing all nodes (i.e., all node-degree pairs) for each snapshot, which are required for the next step. While our predictor update method does not require these oracles in practical applications, this step is required to set up the experimental setting from the paper.
   - The original prefix (everything before the last `_` in the input filename) is removed and replaced with *prefix*, e.g., *preprocessed_as19971108.txt* becomes *prefix_as19971108.txt*.
   <br><br>

3. Compute *MinDegreePredictor* sizes (`\bar{n}_{i}` values) for all snapshots in a sequence
   <br><br>
   `python compute_nbar_snapshots.py -d <dataset_folder> -g <degrees_folder> -o <output_file>`
   <br><br>
   where *dataset_folder* is the path to the folder with preprocessed snapshot files at point (1), *degrees_folder* is the path to the folder with files containing all node-degree pairs for each snapshot at point (2), and *output_file* is the path where the `\bar{n}_{i}` values (*MinDegreePredictor* sizes) will be saved (one per row).
   <br><br>

4. Truncate the node-degree pair files for each snapshot to the true *MinDegreePredictor* size:
   <br><br>
   `python exec_truncate_mdp_snapshots.py -i <oracle_min_degree_folder> -b <nbar_file> -x <prefix> -o <output_folder>`
   <br><br>
   where *oracle_min_degree_folder* is the path to the folder with files containing *MinDegreePredictor* files with all node-degree pairs for each snapshot at point (2), *nbar_file* is the path to .txt file with one `\bar{n}_{i}` value per snapshot at point (3), *prefix* is the prefix for each oracle file name (read the note in point (2) for details), and *output_folder* is the destination folder where the *MinDegreePredictor* oracles with `\bar{n}_{i}` entries for snapshot *i* will be stored.
   <br><br>