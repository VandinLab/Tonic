import os
import argparse
import subprocess
from evaluation import evaluate_recall, evaluate_rbo
import shutil

def parse_args():
    """
    Parses command-line arguments for running and evaluating the USS algorithm
    on a sequence of graph snapshots.

    Returns:
        argparse.Namespace: Parsed arguments including dataset folder, oracle folder,
        multiplier, number of trials, and experiment name
    """
    parser = argparse.ArgumentParser(description="Run USS on graph snapshots and evaluate performance")
    parser.add_argument("-d", "--dataset_folder", required=True, help="Dataset folder containing graph snapshots")
    parser.add_argument("-o", "--oracle_min_degree_folder", required=True, help="Folder containing MinDegreePredictor files")
    parser.add_argument("-c", "--multiplier", type=int, required=True, help="Multiplier for oracle size to set USS capacity")
    parser.add_argument("-t", "--n_trials", type=int, required=True, help="Number of trials per snapshot")
    parser.add_argument("-n", "--name", required=True, help="Name for the output subfolder")
    return parser.parse_args()

def process_graph_stream(uss_binary, input_file, output_file_prefix, k, n_bar, seed):
    """
    Runs the USS on a single graph snapshot with the given parameters.
    """
    subprocess.run([uss_binary, input_file, output_file_prefix, str(k), str(seed), str(n_bar)], check=True)

def main():
    """
    Main function to run the USS algorithm across a sequence of graph snapshots and evaluate its performance.

    - For each snapshot:
        - It runs USS
        - Evaluates RBO and recall against the ground-truth oracle
        - Appends the results to a CSV file
    
    Output:
        - Global summary CSV at: output/USSExperiments/{name}/uss_rbo_recall_results.csv
    """
    args = parse_args()

    FILE_USS = "../../../code/Tonic-build/RunUSS"
    OUTPUT_ROOT = f"output/USSExperiments/{args.name}"
    os.makedirs(OUTPUT_ROOT, exist_ok=True)

    temp_root = os.path.join(OUTPUT_ROOT, "temp_runs")
    os.makedirs(temp_root, exist_ok=True)

    oracle_files = sorted([f for f in os.listdir(args.oracle_min_degree_folder) if os.path.isfile(os.path.join(args.oracle_min_degree_folder, f))])
    graph_files = sorted([f for f in os.listdir(args.dataset_folder) if os.path.isfile(os.path.join(args.dataset_folder, f))])

    assert len(oracle_files) == len(graph_files), "Mismatch in number of oracle and graph files"

    STARTING_SEED = 4177

    final_csv_path = os.path.join(OUTPUT_ROOT, "uss_rbo_recall_results.csv")
    with open(final_csv_path, "w") as out_csv:
        out_csv.write("Snapshot,Algo,c,RBO,Recall\n")

        for snapshot_idx, (oracle_file, graph_file) in enumerate(zip(oracle_files, graph_files)):
            gt_path = os.path.join(args.oracle_min_degree_folder, oracle_file)
            input_graph_path = os.path.join(args.dataset_folder, graph_file)

            n_bar = sum(1 for line in open(gt_path) if line.strip())
            k = int(args.multiplier * n_bar)

            for run_id in range(args.n_trials):
                run_output_dir = os.path.join(temp_root, f"run_{run_id}")
                os.makedirs(run_output_dir, exist_ok=True)

                output_prefix = os.path.join(run_output_dir, f"uss_snapshot{snapshot_idx}")
                output_csv = output_prefix + "_top_nodes.csv"

                process_graph_stream(FILE_USS, input_graph_path, output_prefix, k, n_bar, seed=(STARTING_SEED + run_id))
    
                recall_score = evaluate_recall(gt_path, output_csv)
                rbo_score = evaluate_rbo(gt_path, output_csv)

                snapshot_number = snapshot_idx + 1
                out_csv.write(f"{snapshot_number},USS,{args.multiplier},{rbo_score:.6f},{recall_score:.6f}\n")

    # Cleanup intermediate USS run outputs
    shutil.rmtree(temp_root)

if __name__ == "__main__":
    main()