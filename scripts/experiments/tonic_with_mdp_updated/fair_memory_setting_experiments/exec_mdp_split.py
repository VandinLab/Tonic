import os
import argparse
import subprocess
import math
from utils import run_exact_algorithm, read_top_k_lines

def parse_args():
    """
    Parses command-line arguments for running TONIC with partial split of additional entries:
    half of the additional entries go to the memory budget, and half to the 
    MinDegreePredictor size (MinDegreePredictor Split in the paper).

    Returns:
        argparse.Namespace: Parsed arguments including dataset folder, oracle path,
        nbar file, multiplier, number of trials, and output name.
    """
    parser = argparse.ArgumentParser(description="Run TONIC on graph snapshots with half of the additional entries to the memory budget and half to the MinDegreePredictor from the first snapshot")
    parser.add_argument('-d', '--dataset_folder', required=True, help='Dataset folder containing the graph snapshots')
    parser.add_argument('-o', '--oracle_min_degree_path', required=True, help='All node-degree pairs for the first snapshot in descending order')
    parser.add_argument('-b', '--nbar_file', required=True, help='Path to .txt file containing one oracle size per row')
    parser.add_argument('-c', '--c_multiplier', type=int, required=True, help='Multiplier for the additional memory budget')
    parser.add_argument('-t', '--n_trials', type=int, required=True, help='Number of trials per snapshot')
    parser.add_argument('-n', '--name', required=True, help='Output name')
    return parser.parse_args()

def main():
    """
    Main function to run TONIC on a sequence of graph snapshots using a MinDegreePredictor with the following setting:
    - half of the additional entries are used to increase the MinDegreePredictor size
    - the other half is used to increase the memory budget
    
    For each snapshot:
    - The additional budget (c × next_nbar + current_nbar - first_nbar) is split equally
    - Takes n_bar plus the half of additional entries from the file with all node-degree pairs
    - Computes the increased memory budget
    - TONIC is executed for multiple trials using the constructed oracle and budget
    
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
    TEMP_ORACLE_PATH = f"{OUTPUT_FOLDER}/temp_oracle.txt"

    dataset_files = sorted(os.listdir(args.dataset_folder))

    with open(args.nbar_file, 'r') as f:
        nbar_values = [int(line.strip()) for line in f if line.strip()]

    if len(dataset_files) != len(nbar_values):
        raise ValueError("Mismatch between number of snapshots and number of entries in nbar_file.")

    first_nbar = nbar_values[0]

    for idx, dataset_filename in enumerate(dataset_files):
        dataset_path = os.path.join(args.dataset_folder, dataset_filename)
        print(f"\nRunning for snapshot: {os.path.basename(dataset_filename)}")

        current_nbar = nbar_values[idx]
        next_nbar = nbar_values[idx + 1] if idx + 1 < len(nbar_values) else 0

        # Compute the number of additional entries to expand the MinDegreePredictor size and add it to n_bar (original size).
        size_increase = math.ceil((current_nbar + args.c_multiplier * next_nbar - first_nbar) / 2)
        oracle_size = first_nbar + size_increase

        # Take the calculated number of entries from the file that contains all node-degree pairs
        top_lines = read_top_k_lines(args.oracle_min_degree_path, oracle_size)
        with open(TEMP_ORACLE_PATH, 'w') as f:
            f.writelines(top_lines)

        # Run Exact algorithm
        total_m = run_exact_algorithm(FILE_EXACT, dataset_path, OUTPUT_PATH_EXACT)
        print(f"Total number of edges: {total_m}")

        # Compute the base memory budget
        perc_k = 0.1
        base_mem = int(perc_k * total_m)
        
        # Compute the additional memory budget and add it to the base
        extra_mb = (current_nbar + args.c_multiplier * next_nbar - first_nbar) // 2
        memory_budget = base_mem + extra_mb

        print(f"Base Memory Budget: {base_mem}")
        print(f"Final Memory Budget: {memory_budget}")
        print(f"Increased Oracle Size: {oracle_size}")

        for r in range(RANDOM_SEED, END + 1):
            subprocess.run([
                FILE_TONIC, "0", str(r), str(memory_budget), "0.05", "0.2",
                dataset_path, TEMP_ORACLE_PATH, "nodes", OUTPUT_PATH_TONIC
            ], check=True)

if __name__ == "__main__":
    main()