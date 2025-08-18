import subprocess
import time
import yaml

def launch_independent_runs(script_path, dataset_folders, oracle_folders, names, c_values, n_trials):
    """
    Launches multiple independent subprocesses to run USS
    on different datasets (graph snapshots) and parameter settings.

    Each subprocess executes the "exec_uss_experiments.py" script with parameters specified from the input lists.
    The script is launched once for each combination of (dataset, oracle, nbar, base_name) and c value.

    Args:
        script_path (str): Path to the Python script to be executed ("exec_uss_experiments.py")
        dataset_folders (list[str]): List of dataset folder paths
        oracle_folders (list[str]): List of MinDegree predictor folder paths
        names (list[str]): Base names used for output naming
        c_values (list[int]): List of `c` values to vary USS capacity
        n_trials (int): Number of trials to run per configuration
    """
    assert len(dataset_folders) == len(oracle_folders) == len(names), \
        "Mismatch in number of datasets, oracles, and names."

    processes = []

    for i in range(len(dataset_folders)):
        for c in c_values:
            name = f"{names[i]}_c{c}"
            cmd = [
                "python", script_path,
                "-d", dataset_folders[i],
                "-o", oracle_folders[i],
                "-c", str(c),
                "-t", str(n_trials),
                "-n", name
            ]

            print(f"Launching: {' '.join(cmd)}")
            p = subprocess.Popen(cmd)
            processes.append((name, p))
            time.sleep(1)  # Optional delay

    print("\nAll processes launched. Waiting for completion...\n")

    for name, p in processes:
        retcode = p.wait()
        print(f"{name} finished with exit code {retcode}")

if __name__ == "__main__":
    with open("config/uss-experiments.yaml", "r") as f:
        config = yaml.safe_load(f)

    dataset_folders = config["dataset_folders"]
    oracle_folders = config["oracle_min_degree_folders"]
    names = config["names"]
    c_values = config["c_values"]
    n_trials = config["n_trials"]

    script_path = "exec_uss_experiments.py"
    launch_independent_runs(script_path, dataset_folders, oracle_folders, names, c_values, n_trials)