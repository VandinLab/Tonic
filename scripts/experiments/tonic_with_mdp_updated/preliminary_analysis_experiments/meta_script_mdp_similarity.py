import subprocess
import yaml

def run_mdp_similarity_experiments(script_name, oracle_folders, base_names):
    """
    Runs `script_name' ("exec_first_snapshot_mdp_similarity_experiments.py" or 
    "exec_previous_snapshot_mdp_similarity_experiments.py") across a list of oracle folders (MinDegreePredictor files).
    
    Args:
        script_name (str): Path to the script that computes MDP similarity
        oracle_folders (list[str]): List of folders containing MinDegree predictor outputs
        base_names (list[str]): List of base names used to identify output results
    """
    assert len(oracle_folders) == len(base_names), "Mismatch in oracle folders and base names"

    for oracle_folder, base_name in zip(oracle_folders, base_names):
        cmd = [
            "python", script_name,
            "-o", oracle_folder,
            "-n", base_name
        ]

        print(f"Running: {' '.join(cmd)}")
        subprocess.run(cmd, check=True)

if __name__ == "__main__":
    with open("config/mdp-similarity.yaml", "r") as f:
        config = yaml.safe_load(f)

    script_name = config["script_name"]
    oracle_folders = config["oracle_min_degree_folders"]
    base_names = config["base_names"]

    run_mdp_similarity_experiments(script_name, oracle_folders, base_names)