import os
import argparse
import subprocess
from utils import run_exact_algorithm

def parse_args():
    """
    Parses command-line arguments for running TONIC with a fixed MinDegreePredictor
    and a memory budget that increases based on the next oracle size and the multiplier (MinDegreePredictor IncreasedBudget in the paper).

    Returns:
        argparse.Namespace: Parsed arguments including dataset path, oracle path,
        nbar values file, multiplier, number of trials, and output name.
    """
    parser = argparse.ArgumentParser(description="Run TONIC on graph snapshots with fixed MinDegreePredictor and increased memory budget")
    parser.add_argument('-d', '--dataset_folder', required=True, help='Dataset folder containing graph snapshots')
    parser.add_argument('-o', '--oracle_min_degree_path', required=True, help='MinDegreePredictor path (from the first snapshot)')
    parser.add_argument('-b', '--nbar_file', required=True, help='Path to .txt file containing precomputed nbar values (one per row)')
    parser.add_argument('-c', '--c_multiplier', type=int, required=True, help='Multiplier for the additional memory budget')
    parser.add_argument('-t', '--n_trials', type=int, required=True, help='Number of trials per snapshot')
    parser.add_argument('-n', '--name', required=True, help='Output name')
    return parser.parse_args()

def main():
    """
    Main function to run TONIC on a sequence of graph snapshots with increased memory budget, 
    using a MinDegreePredictor.
    
    For each snapshot:
    - It runs the exact triangle counting algorithm to determine the total number of edges
    - Sets a base memory budget as 10% of the number of edges.
    - Adds extra memory proportional to (current_nbar + c × next_nbar − first_nbar).
    - Runs TONIC for each trial using the same oracle and varying memory budgets.
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

    dataset_files = sorted(os.listdir(args.dataset_folder))

    # Read nbar values from file
    with open(args.nbar_file, "r") as f:
        nbar_values = [int(line.strip()) for line in f if line.strip()]

    if len(dataset_files) != len(nbar_values):
        raise ValueError(f"Mismatch between number of snapshot files ({len(dataset_files)}) "
                         f"and number of nbar values ({len(nbar_values)}).")
    
    first_nbar = nbar_values[0]

    for idx, dataset_filename in enumerate(dataset_files):
        dataset_path = os.path.join(args.dataset_folder, dataset_filename)
        print(f"\nRunning for snapshot: {os.path.basename(dataset_filename)}")

        current_nbar = nbar_values[idx]
        # We do not increase the memory budget for the last snapshot
        next_nbar = nbar_values[idx + 1] if idx + 1 < len(nbar_values) else 0

        # Run Exact algorithm
        total_edges = run_exact_algorithm(FILE_EXACT, dataset_path, OUTPUT_PATH_EXACT)
        print(f"Total number of edges: {total_edges}")
        
        # Compute the base memory budget
        perc_k = 0.1
        base_mem = int(perc_k * total_edges)

        # Compute the additional memory budget and add it to the base
        extra_mb = current_nbar + args.c_multiplier * next_nbar - first_nbar 
        memory_budget = base_mem + extra_mb

        print(f"Base Memory Budget: {base_mem}")
        print(f"Final Memory Budget: {memory_budget}")

        for r in range(RANDOM_SEED, END + 1):
            subprocess.run([
                FILE_TONIC, "0", str(r), str(memory_budget), "0.05", "0.2",
                dataset_path, args.oracle_min_degree_path, "nodes", OUTPUT_PATH_TONIC
            ], check=True)

if __name__ == "__main__":
    main()