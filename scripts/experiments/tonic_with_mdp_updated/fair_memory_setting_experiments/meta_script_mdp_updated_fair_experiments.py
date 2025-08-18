import subprocess
import time
import yaml

def launch_independent_runs(script_name, c_values, dataset_folders, oracle_min_paths, nbar_files, base_names, n_trials):
    """
    Launches multiple independent runs of a script with different values of c and dataset configurations.

    Each subprocess executes `script_name` with parameters specified from the input lists. The script is 
    launched once for each combination of (dataset, oracle, nbar, base_name) and c value.

    Args:
        script_name (str): Path to the Python script to be executed (e.g., 'exec_mdp_updated.py')
        c_values (list[int]): List of `c` values (multiplier for oracle size)
        dataset_folders (list[str]): List of dataset folders (each containing graph snapshots)
        oracle_min_paths (list[str]): List of paths to (initial) MinDegreePredictor oracles
        nbar_files (list[str]): List of file paths with precomputed n_bar values
        base_names (list[str]): List of base names used to generate output identifiers
        n_trials (int): Number of trials to run per configuration
    """
    assert len(dataset_folders) == len(oracle_min_paths) == len(nbar_files) == len(base_names), \
        "All dataset-related lists must be the same length."

    processes = []

    for i in range(len(dataset_folders)):
        dataset_folder = dataset_folders[i]
        oracle_path = oracle_min_paths[i]
        nbar_file = nbar_files[i]
        base_name = base_names[i]

        for c in c_values:
            full_name = f"{base_name}_c{c}"

            cmd = [
                "python", script_name,
                "-d", dataset_folder,
                "-o", oracle_path,
                "-b", nbar_file,
                "-c", str(c),
                "-t", str(n_trials),
                "-n", full_name
            ]

            print(f"Launching: {' '.join(cmd)}")
            p = subprocess.Popen(cmd)
            processes.append((full_name, p))
            time.sleep(1)  # Optional delay to avoid overloading

    print("\nAll processes launched. Waiting for completion...\n")

    for name, p in processes:
        retcode = p.wait()
        print(f"{name} finished with exit code {retcode}")


if __name__ == "__main__":
    # Load from YAML
    with open("config/mdp-updated-fair-experiments.yaml", "r") as f:
        config = yaml.safe_load(f)

    script_name = config["script_name"]
    c_values = config["c_values"]
    n_trials = config["n_trials"]
    dataset_folders = config["dataset_folders"]
    oracle_min_paths = config["oracle_min_degree_paths"]
    nbar_files = config["nbar_files"]
    base_names = config["base_names"]

    launch_independent_runs(script_name, c_values, dataset_folders, oracle_min_paths, nbar_files, base_names, n_trials)