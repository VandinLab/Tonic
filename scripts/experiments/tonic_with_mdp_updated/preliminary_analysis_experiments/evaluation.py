import rbo
from utils import load_node_frequencies

def recall_at_k(true_top_k, predicted_top_k):
    """
    Computes the recall at k between two ranked lists of nodes.

    Args:
        true_top_k (list[tuple[int, int]]): Ground-truth list of (node_id, frequency), sorted by frequency
        predicted_top_k (list[tuple[int, int]]): Estimated list of (node_id, frequency), sorted by frequency

    Returns:
        float: Recall@k score (i.e., fraction of true top-k nodes that appear in the predicted list)
    """
    true_ids = set(node for node, _ in true_top_k)
    predicted_ids = set(node for node, _ in predicted_top_k)
    return len(true_ids & predicted_ids) / len(true_ids)

def evaluate_recall(ground_truth_file, estimate_file):
    """
    Loads ground-truth and estimated node frequencies and computes Recall@k.

    Args:
        ground_truth_file (str): Path to the file with ground-truth top nodes
        estimate_file (str): Path to the file with predicted top nodes

    Returns:
        float: Recall@k score between the two rankings
    """
    gt = load_node_frequencies(ground_truth_file)
    est = load_node_frequencies(estimate_file)
    return recall_at_k(gt, est)

def evaluate_rbo(ground_truth_file, estimate_file):
    """
    Loads ground-truth and estimated node frequencies and computes RBO (Rank-Biased Overlap).

    Args:
        ground_truth_file (str): Path to the file with ground-truth top nodes
        estimate_file (str): Path to the file with predicted top nodes

    Returns:
        float: RBO score between the two rankings (p=1.0 for equal weight to all overlap depths)
    """
    gt = load_node_frequencies(ground_truth_file)
    est = load_node_frequencies(estimate_file)

    true_ids = [node for node, _ in gt]
    predicted_ids = [node for node, _ in est]

    return rbo.RankingSimilarity(true_ids, predicted_ids).rbo(p=1.0)