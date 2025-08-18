import os
import argparse
import subprocess
import csv
from utils import run_exact_algorithm, clean_auxiliary_files
import shutil

def parse_args():
    """
    Parses command-line arguments for running TONIC with USS to update the MinDegreePredictor (MinDegreePredictr Updated in the paper).

    Returns:
        argparse.Namespace: Parsed arguments with dataset path, oracle path, and experiment parameters.
    """
    parser = argparse.ArgumentParser(description="Run TONIC on graph snapshots combined with USS to update the MinDegreePredictor")
    parser.add_argument("-d", "--dataset_folder", required=True, help="Dataset folder containing graph snapshots")
    parser.add_argument("-o", "--oracle_min_degree_path", required=True, help="MinDegreePredictor path (from the first snapshot)")
    parser.add_argument('-b', '--nbar_file', required=True, help='Path to .txt file containing one oracle size per row')
    parser.add_argument("-c", "--multiplier", type=int, required=True, help="Multiplier for oracle sizes to set the USS capacity")
    parser.add_argument("-t", "--n_trials", type=int, required=True, help="Number of trials per snapshot")
    parser.add_argument("-n", "--name", required=True, help="Output name")

    return parser.parse_args()

def run_tonic(file_tonic, r, memory_budget, dataset_path, oracle_path, output_path_tonic, update_map_capacity, next_oracle_size):
    """
    Executes TONIC algorithm with given parameters, optionally enabling USS if next_oracle_size > 0

    Args:
        file_tonic (str): Path to the compiled TONIC binary
        r (int): Random seed for the current trial
        memory_budget (int): Memory budget for TONIC
        dataset_path (str): Path to the current graph snapshot file
        oracle_path (str): Path to the oracle file
        output_path_tonic (str): Output path for TONIC
        update_map_capacity (int): USS update map capacity based on multiplier and oracle size
        next_oracle_size (int): Number of top nodes to retain in the next oracle (0 disables USS)
    """
    base_args = [
        file_tonic, "0", str(r), str(memory_budget), "0.05", "0.2",
        dataset_path, oracle_path, "nodes", output_path_tonic
    ]

    if next_oracle_size > 0:
        base_args += ["1", str(update_map_capacity), str(next_oracle_size)]
    subprocess.run(base_args, check=True)

def update_node_oracle(updated_oracle_path, node_degree_file):
    """
    Rewrites the oracle file by reading top node degrees from the USS output (CSV format).

    Args:
        updated_oracle_path (str): Output path for the updated oracle file.
        node_degree_file (str): CSV file containing top nodes and their degrees.
    """
    node_degrees = {}

    if not os.path.exists(node_degree_file):
        print(f"Warning: {node_degree_file} not found. Skipping update.")
        return

    with open(node_degree_file, "r") as f:
        csv_reader = csv.reader(f)
        next(csv_reader)  # Skip header
        for row in csv_reader:
            if len(row) == 2:
                node, degree = int(row[0]), int(row[1])
                node_degrees[node] = degree

    with open(updated_oracle_path, "w") as f:
        for node, degree in node_degrees.items():
            f.write(f"{node} {degree}\n")

def main():
    """
    Main function to run TONIC with USS (MinDegreePredictor Updated) on a sequence of graph snapshots.

    For each snapshot:
    - It runs the exact triangle counting algorithm to determine the total number of edges
    - Computes USS map capacity
    - Runs TONIC (with USS)
    - Updates the oracle using USS output
    - Cleans auxiliary outputs

    """
    args = parse_args()

    FILE_TONIC = "../../../code/Tonic-build/Tonic"
    FILE_EXACT = "../../../code/Tonic-build/RunExactAlgo"

    RANDOM_SEED = 4177
    END = RANDOM_SEED + args.n_trials - 1

    OUTPUT_FOLDER = f"output/SnapshotExperiments/{args.name}"
    os.makedirs(OUTPUT_FOLDER, exist_ok=True)
   
    # Clean previous outputs
    for f in os.listdir(OUTPUT_FOLDER):
        os.remove(os.path.join(OUTPUT_FOLDER, f))

    OUTPUT_PATH_TONIC = f"{OUTPUT_FOLDER}/output_tonic_{args.name}"
    OUTPUT_PATH_EXACT = f"{OUTPUT_FOLDER}/output_exact_{args.name}"

    TEMP_NODE_FILE = f"{OUTPUT_FOLDER}/output_tonic_{args.name}_top_nodes.csv"

    # Load snapshot files
    dataset_files = sorted(os.listdir(args.dataset_folder))

    # Load oracle sizes
    with open(args.nbar_file, "r") as f:
        oracle_sizes = [int(line.strip()) for line in f if line.strip().isdigit()]

    if len(oracle_sizes) != len(dataset_files):
        raise ValueError(f"Number of oracle sizes ({len(oracle_sizes)}) does not match number of snapshot files ({len(dataset_files)}).")

    # We shift the oracle sizes by one. We remove the size of the first oracle because it is not needed for USS updates
    if oracle_sizes:
        oracle_sizes.pop(0)

    for idx, dataset_filename in enumerate(dataset_files):
        dataset_path = os.path.join(args.dataset_folder, dataset_filename)

        # Compute capacity of the USS map (number of tracked nodes)
        # next_nbar is seto to zero for the last snapshot in the sequence to disable USS 
        next_nbar = oracle_sizes[idx] if idx < len(oracle_sizes) else 0
        update_map_capacity = args.multiplier * next_nbar

        if next_nbar == 0:
            print(f"Warning: No next oracle size for {dataset_filename}. USS will be disabled.")

        print(f"Snapshot {dataset_filename}: Using update map capacity {next_nbar if next_nbar else 'N/A'}")

        total_edges = run_exact_algorithm(FILE_EXACT, dataset_path, OUTPUT_PATH_EXACT)
        
        # Run Exact algorithm
        memory_budget = int(0.1 * total_edges)
        print(f"Snapshot {dataset_filename}: Total Edges = {total_edges}, Memory Budget = {memory_budget}")

        # Create a separate MinDegreePredictor for seed (each run of Tonic with USS), ensuring that the variability of both Tonic and USS is taken into account
        for r in range(RANDOM_SEED, END + 1):
            NODE_DEGREE_FILE = f"{OUTPUT_FOLDER}/output_tonic_{args.name}_top_nodes_seed{r}.csv"
            UPDATED_ORACLE_PATH = f"{OUTPUT_FOLDER}/updated_oracle_{args.name}_seed{r}.txt"

            if not os.path.exists(UPDATED_ORACLE_PATH):
                shutil.copy(args.oracle_min_degree_path, UPDATED_ORACLE_PATH)

            run_tonic(FILE_TONIC, r, memory_budget, dataset_path, UPDATED_ORACLE_PATH,
                      OUTPUT_PATH_TONIC, update_map_capacity, next_nbar)

            if os.path.exists(TEMP_NODE_FILE):
                shutil.move(TEMP_NODE_FILE, NODE_DEGREE_FILE)
            # This is is expected for the last snapshot as there are no updates in this case.
            else:
                print(f"Top-node output file '{TEMP_NODE_FILE}' not found. Update disabled for snapshot number {idx + 1}.")

            # Update the predictor only if the size for the next oracle is known
            if next_nbar > 0:
                update_node_oracle(UPDATED_ORACLE_PATH, NODE_DEGREE_FILE)
                print(f"Updated oracle for seed {r} using top nodes from snapshot {dataset_filename}")

    # Delete temporary files used throughout the algorithm execution
    clean_auxiliary_files(OUTPUT_FOLDER)

if __name__ == "__main__":
    main()