import os

def load_node_frequencies_txt(filename):
    """
    Loads node frequency data from a .txt file.
    
    Assumes each line is of the form: <node_id> <frequency>.

    Args:
        filename (str): Path to the .txt file.

    Returns:
        list[tuple[int, int]]: List of (node_id, frequency) tuples sorted by descending frequency.
    """
    results = []
    with open(filename, 'r') as f:
        for line in f:
            parts = line.strip().split()
            if len(parts) >= 2:
                node_id = int(parts[0])
                freq = int(parts[1])
                results.append((node_id, freq))
    return sorted(results, key=lambda x: -x[1])


def load_node_frequencies_csv(filename):
    """
    Loads node frequency data from a .csv file.

    Assumes the file has a header and rows of the form: node_id,freq

    Args:
        filename (str): Path to the .csv file.

    Returns:
        list[tuple[int, int]]: List of (node_id, frequency) tuples sorted by descending frequency.
    """
    results = []
    with open(filename, 'r') as f:
        next(f)  # skip header
        for line in f:
            parts = line.strip().split(',')
            if len(parts) == 2:
                node_id = int(parts[0])
                freq = int(parts[1])
                results.append((node_id, freq))
    return sorted(results, key=lambda x: -x[1])

def load_node_frequencies(filename):
    """
    Automatically loads node frequency data based on file extension.

    Args:
        filename (str): Path to the file (.csv or .txt).

    Returns:
        list[tuple[int, int]]: Sorted (node_id, frequency) list.

    Raises:
        ValueError: If the file extension is not .csv or .txt.
    """
    ext = os.path.splitext(filename)[1].lower()
    if ext == ".csv":
        return load_node_frequencies_csv(filename)
    elif ext == ".txt":
        return load_node_frequencies_txt(filename)
    else:
        raise ValueError(f"Unsupported file extension: {ext}")