import os
import argparse
from evaluation import evaluate_recall, evaluate_rbo

def parse_args():
    """
    Parses command-line arguments for evaluating the similarity between
    MinDegreePredictors across graph snapshots. Similarity (RBO) is calculated between the MinDegreePredictor
    maps of the current and the previous snapshot.

    Returns:
        argparse.Namespace: Parsed arguments with oracle folder path and output name.
    """
    parser = argparse.ArgumentParser(description="Evaluate Previous Snapshot MinDegreePredictor against all MinDegreePredictors")
    parser.add_argument("-o", "--oracle_min_degree_folder", required=True, help="Folder containing MinDegreePredictor files")
    parser.add_argument("-n", "--name", required=True, help="Output name")
    return parser.parse_args()

def main():
    """
    Evaluates the similarity between MinDegreePredictors across consecutive graph snapshots.
    Similarity (measured using RBO) is calculated between the MinDegreePredictor
    maps of the current and the previous snapshot.

    For each snapshot (excluding the first):
    - It compares the MinDegreePredictor from snapshot i-1 (previous) to snapshot i (current)
    - Computes Recall and RBO similarity between the two
    - Writes a CSV summarizing the full results

    Output:
        - summary CSV at: output/MDPredictorSimilarityExperiments/{name}/previous_snapshot_predictor_results.csv
    """
    args = parse_args()

    OUTPUT_ROOT = f"output/MDPredictorSimilarityExperiments/{args.name}"
    os.makedirs(OUTPUT_ROOT, exist_ok=True)

    oracle_files = sorted([ f for f in os.listdir(args.oracle_min_degree_folder) if os.path.isfile(os.path.join(args.oracle_min_degree_folder, f)) and f.endswith(".txt")])

    if not oracle_files:
        raise ValueError("No oracle files found in the specified folder.")

    csv_path = os.path.join(OUTPUT_ROOT, "previous_snapshot_predictor_results.csv")

    with open(csv_path, "w") as csv_file:
        csv_file.write("Snapshot,Algo,RBO,Recall\n")

        for i in range(1, len(oracle_files)):
            prev_path = os.path.join(args.oracle_min_degree_folder, oracle_files[i - 1])
            curr_path = os.path.join(args.oracle_min_degree_folder, oracle_files[i])

            recall = evaluate_recall(curr_path, prev_path)
            rbo_score = evaluate_rbo(curr_path, prev_path)

            snapshot_number = i + 1
            csv_file.write(f"{snapshot_number},PreviousSnapshotMDP,{rbo_score:.6f},{recall:.6f}\n")

if __name__ == "__main__":
    main()