import os
import argparse
from itertools import combinations

def parse_args():
    """
    Parses command-line arguments for computing n_bar values from graph snapshots.

    Returns:
        argparse.Namespace: Parsed arguments with dataset_folder, degrees_folder, and output_file.
    """
    parser = argparse.ArgumentParser(description="Compute n_bar values from graph snapshots.")
    parser.add_argument('-d', '--dataset_folder', required=True, help='Dataset folder containing graph snapshots')
    parser.add_argument('-g', '--degrees_folder', required=True, help='Folder containing node degree files (containing all node degree-pairs)')
    parser.add_argument('-o', '--output_file', required=True, help='Path to output .txt file for storing n_bar values')
    return parser.parse_args()

def load_degrees(degrees_file):
    """
    Loads node degrees from a file.

    Args:
        degrees_file (str): Path to the file containing node degrees. Each line should be: <node_id> <degree>

    Returns:
        dict: Mapping from node ID (int) to degree (int).
    """
    degrees = {}
    with open(degrees_file, 'r') as df:
        for line in df:
            parts = line.strip().split()
            if len(parts) == 2:
                node_id, node_degree = map(int, parts)
                degrees[node_id] = node_degree
    return degrees

def load_edges(edges_file):
    """
    Loads edges from a file, ignoring timestamps.

    Args:
        edges_file (str): Path to the file containing edges. Each line should be: <node1> <node2> <timestamp>

    Returns:
        set: A set of edges represented as tuples (node1, node2).
    """
    edges = set()
    with open(edges_file, 'r') as ef:
        for line in ef:
            parts = line.strip().split()
            if len(parts) == 3:
                node1, node2, _ = map(int, parts)
                edges.add((node1, node2))
    return edges

def compute_n_bar(degrees_file, edges_file):
    """
    Computes the n_bar value, which is the number of unique nodes
    involved in the top 10% of all the possible edges ranked by minimum degree. Refer to the
    paper for full details on the computation of n_bar values.

    Args:
        degrees_file (str): Path to the file with node degrees.
        edges_file (str): Path to the file with original edges.

    Returns:
        int: The computed n_bar value.
    """
    degrees = load_degrees(degrees_file)
    original_edges = load_edges(edges_file)
    nodes = list(degrees.keys())

    all_possible_edges = [(node1, node2, min(degrees[node1], degrees[node2]))
                          for node1, node2 in combinations(nodes, 2)]

    all_possible_edges.sort(key=lambda x: x[2], reverse=True)

    top_10_percent = int(len(original_edges) * 0.1)
    top_edges = all_possible_edges[:top_10_percent]

    unique_nodes = set(node for edge in top_edges for node in edge[:2])
    n_bar = len(unique_nodes)

    print(f"MinDegree predictor size: {n_bar}")
    return n_bar

def process_folders(dataset_folder, degrees_folder, output_file_path):
    """
    Processes all snapshot files and computes n_bar values for each snapshot.

    Args:
        dataset_folder (str): Path to the folder containing dataset files.
        degrees_folder (str): Path to the folder containing node degree files.
        output_file_path (str): Path to the output file where n_bar values will be written.
    """
    os.makedirs(os.path.dirname(output_file_path), exist_ok=True)

    edges_files = sorted(os.listdir(dataset_folder))
    degrees_files = sorted(os.listdir(degrees_folder))

    if len(edges_files) != len(degrees_files):
        raise ValueError("Mismatch in the number of files in dataset and degrees folders.")

    with open(output_file_path, 'w') as out_file:
        for edge_file, degree_file in zip(edges_files, degrees_files):
            edges_path = os.path.join(dataset_folder, edge_file)
            degrees_path = os.path.join(degrees_folder, degree_file)

            n_bar = compute_n_bar(degrees_path, edges_path)
            out_file.write(f"{n_bar}\n")

            print(f"Wrote n_bar={n_bar} for snapshot {edge_file}")

def main():
    args = parse_args()
    process_folders(args.dataset_folder, args.degrees_folder, args.output_file)

if __name__ == "__main__":
    main()