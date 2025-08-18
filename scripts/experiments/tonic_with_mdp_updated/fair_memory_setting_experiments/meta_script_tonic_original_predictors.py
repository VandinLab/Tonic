import subprocess
import time
import yaml

def launch_independent_runs(script_path, dataset_folders, oracle_exact_paths, oracle_min_paths, names, n_trials):
    """
    Launches multiple independent runs of the "exec_mdp_and_oracle_exact.py" script using subprocesses.

    Each run is configured with a specific dataset folder, oracle paths, and name.
    Ensures the number of datasets, oracles, and names are aligned.

    Args:
        script_path (str): Path to the script to be executed (exec_mdp_and_oracle_exact.py)
        dataset_folders (list[str]): List of dataset folders containing graph snapshots, one per experiment
        oracle_exact_paths (list[str]): List of paths to exact oracles, one per experiment
        oracle_min_paths (list[str]): List of paths to MinDegreePredictor oracles, one per experiment
        names (list[str]): List of experiment identifiers (used in naming outputs)
        n_trials (int): Number of trials to be passed to each script
    """
    assert len(dataset_folders) == len(oracle_exact_paths) == len(oracle_min_paths) == len(names), \
        "All dataset-related lists must be the same length."

    processes = []

    for i in range(len(dataset_folders)):
        dataset_folder = dataset_folders[i]
        oracle_exact = oracle_exact_paths[i]
        oracle_min = oracle_min_paths[i]
        name = names[i]

        cmd = [
            "python", script_path,
            "-d", dataset_folder,
            "-o", oracle_exact,
            "-i", oracle_min,
            "-t", str(n_trials),
            "-n", name
        ]

        print(f"Launching: {' '.join(cmd)}")
        p = subprocess.Popen(cmd)
        processes.append((name, p))
        time.sleep(1)  # Optional delay to avoid overloading

    print("\nAll processes launched. Waiting for completion...\n")

    for name, p in processes:
        retcode = p.wait()
        print(f"{name} finished with exit code {retcode}")


if __name__ == "__main__":
    with open("config/tonic-original-predictors.yaml", "r") as f:
        config = yaml.safe_load(f)

    dataset_folders = config["dataset_folders"]
    oracle_exact_paths = config["oracle_exact_paths"]
    oracle_min_paths = config["oracle_min_degree_paths"]
    names = config["names"]
    n_trials = config["n_trials"]

    script_path = "exec_mdp_and_oracle_exact.py"

    launch_independent_runs(script_path, dataset_folders, oracle_exact_paths, oracle_min_paths, names, n_trials)