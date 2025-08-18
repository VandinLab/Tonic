import os
import argparse
import subprocess
from utils import run_exact_algorithm

def parse_args():
    """
    Parses command-line arguments for running TONIC on graph snapshots
    using OracleExact and the original MinDegreePredictor.

    Returns:
        argparse.Namespace: Parsed command-line arguments with dataset path,
        oracle paths, number of trials, and output name.
    """
    parser = argparse.ArgumentParser(description="Run TONIC on graph snapshots with original Exact and MinDegree predictors.")
    parser.add_argument('-d', '--dataset_folder', required=True, help='Dataset folder containing graph snapshots')
    parser.add_argument('-o', '--oracle_exact_path', required=True, help='OracleExact predictor path (from the first snapshot)')
    parser.add_argument('-i', '--oracle_min_degree_path', required=True, help='MinDegreePredictor path (from the first snapshot)')
    parser.add_argument('-t', '--n_trials', type=int, required=True, help='Number of trials per snapshot')
    parser.add_argument('-n', '--name', required=True, help='Output name')
    return parser.parse_args()

def main():
    """
    Main function that runs the TONIC binary on a sequence of graph snapshots
    using both OracleExact and MinDegreePredictor as predictors.

    For each snapshot:
    - It runs the exact triangle counting algorithm to determine the total number of edges
    - Runs TONIC with both predictors for a fixed number of trials
    
    """
    args = parse_args()

    FILE_TONIC = "../../../code/Tonic-build/Tonic"
    FILE_EXACT = "../../../code/Tonic-build/RunExactAlgo"

    RANDOM_SEED = 4177
    END = RANDOM_SEED + args.n_trials - 1

    OUTPUT_FOLDER = f"output/SnapshotExperiments/{args.name}"
    os.makedirs(OUTPUT_FOLDER, exist_ok=True)

    # Clean previous outputs in the folder
    for f in os.listdir(OUTPUT_FOLDER):
        os.remove(os.path.join(OUTPUT_FOLDER, f))

    OUTPUT_PATH_TONIC = f"{OUTPUT_FOLDER}/output_tonic_{args.name}"
    OUTPUT_PATH_EXACT = f"{OUTPUT_FOLDER}/output_exact_{args.name}"

    dataset_files = sorted(os.listdir(args.dataset_folder))

    for dataset_filename in dataset_files:
        dataset_path = os.path.join(args.dataset_folder, dataset_filename)
        print(f"\nRunning for snapshot: {os.path.basename(dataset_filename)}")

        # Run Exact to get total number of edges
        total_edges = run_exact_algorithm(FILE_EXACT, dataset_path, OUTPUT_PATH_EXACT)
        print(f"Total number of edges: {total_edges}")

        # Compute the memory budget
        perc_k = 0.1
        memory_budget = int(perc_k * total_edges)
        print(f"Memory Budget: {memory_budget}")

        for r in range(RANDOM_SEED, END + 1):
            # Run TONIC with OracleExact
            subprocess.run([
                FILE_TONIC, "0", str(r), str(memory_budget), "0.05", "0.2",
                dataset_path, args.oracle_exact_path, "edges", OUTPUT_PATH_TONIC + "_exact"
            ], check=True)

            # Run TONIC with MinDegreePredictor
            subprocess.run([
                FILE_TONIC, "0", str(r), str(memory_budget), "0.05", "0.2",
                dataset_path, args.oracle_min_degree_path, "nodes", OUTPUT_PATH_TONIC + "_min_degree"
            ], check=True)

if __name__ == "__main__":
    main()