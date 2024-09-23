//
// Created by X on 09/03/24.
//

#include "../include/Utils.h"

/**
 * Runs the exact algorithm for counting triangles in a insertion-only, undirected and static graph streams
 * @param dataset_filepath where the graph is stored
 * @param output_path where to write outputs
 * @return the number of triangles in the graph
 */
long Utils::run_exact_algorithm(std::string &dataset_filepath, std::string &output_path) {

    std::ifstream file(dataset_filepath);
    std::string line, su, sv;

    if (!file.is_open()) {
        std::cerr << "Error! Unable to open file " << dataset_filepath << "\n";
        return -1;
    }

    std::cout << "Running exact algorithm...\n";

    // - graph
    emhash5::HashMap<int, std::unordered_set<int>> graph_stream;
    // -- local triangles
    emhash5::HashMap<int, int> local_triangles;

    int u, v, du, dv, n_min, n_max, timestamp;
    std::unordered_set<int> min_neighbors;

    long total_T = 0, nline = 0;
    // -- check self-loops

    while (std::getline(file, line)) {
        nline++;

        std::istringstream iss(line);
        iss >> u >> v >> timestamp;
        if (u == v) continue;
        if (graph_stream[u].find(v) != graph_stream[u].end() and graph_stream[v].find(u) != graph_stream[v].end()) {
            continue;
        }

        // -- add edge to graph stream
        graph_stream[u].emplace(v);
        graph_stream[v].emplace(u);

        // -- count triangles
        du = (int) graph_stream[u].size();
        dv = (int) graph_stream[v].size();
        n_min = (du < dv) ? u : v;
        n_max = (du < dv) ? v : u;
        min_neighbors = graph_stream[n_min];
        for (const auto &neigh: min_neighbors) {
            if (graph_stream[n_max].find(neigh) != graph_stream[n_max].end()) {
                // -- triangle {n_min, neigh, n_max} discovered
                total_T += 1;

            }
        }

        if (nline % 3000000 == 0) {
            printf("Processed %ld edges | Counted %ld triangles\n", nline, total_T);
        }


    }

    long num_nodes = (long) graph_stream.size();
    printf("Processed dataset with n = %ld, m = %ld\n", num_nodes, nline);
    // -- write results
    std::ofstream out_file(output_path, std::ios::app);
    out_file << "Ground Truth:" << "\n";
    out_file << "Nodes = " << num_nodes << "\n";
    out_file << "Edges = " << nline << "\n";
    out_file << "Triangles = " << total_T << "\n";
    out_file.close();

    return total_T;
}

/**
 * Runs the exact algorithm for counting triangles in a fully dynamic graph stream streams
 * @param dataset_filepath where the graph is stored
 * @param output_path where to write outputs
 * @return the number of triangles in the graph
 */
long Utils::run_exact_algorithm_FD(std::string &dataset_filepath, std::string &output_path) {

    std::ifstream file(dataset_filepath);
    std::string line, su, sv;

    if (!file.is_open()) {
        std::cerr << "Error! Unable to open file " << dataset_filepath << "\n";
        return -1;
    }

    std::cout << "Running exact algorithm for fully dynamic streams...\n";

    // - graph
    emhash5::HashMap<int, std::unordered_set<int>> graph_stream;

    int u, v, du, dv, n_min, n_max, timestamp, src, dst;
    char sign;
    std::unordered_set<int> min_neighbors;

    emhash5::HashMap<unsigned long long, std::pair<int, int>> unique_edges;
    std::unordered_set<int> unique_nodes;

    long total_T = 0, nline = 0, cum_triangles = 0, num_edges = 0;
    long max_edges = 0, time_max_edges = 0;

    while (std::getline(file, line)) {


        std::istringstream iss(line);
        iss >> src >> dst >> timestamp >> sign;
        u = src;
        v = dst;
        if (u > v) {
            u = dst;
            v = src;
        }

        unique_nodes.emplace(u);
        unique_nodes.emplace(v);

        // -- count triangles
        // -- check if u and v are in graph
        du = (int) graph_stream[u].size();
        dv = (int) graph_stream[v].size();
        n_min = (du < dv) ? u : v;
        n_max = (du < dv) ? v : u;
        min_neighbors = graph_stream[n_min];
        cum_triangles = 0;

        for (const auto &neigh: min_neighbors) {
            if (graph_stream[n_max].find(neigh) != graph_stream[n_max].end()) {
                // -- triangle {n_min, neigh, n_max} discovered
                cum_triangles += 1;
            }
        }

        // -- check if edge is addition or removal
        if (sign == '-') {
            total_T -= cum_triangles;
            if (graph_stream[u].find(v) != graph_stream[u].end() and
                graph_stream[v].find(u) != graph_stream[v].end()) {
                num_edges--;
                // -- remove edge from graph stream
                graph_stream[u].erase(v);
                graph_stream[v].erase(u);
                // -- erase entry from map if empty
                if (graph_stream[u].empty())
                    graph_stream.erase(u);
                if (graph_stream[v].empty())
                    graph_stream.erase(v);
            }
        } else {
            // -- by default, assume addition
            total_T += cum_triangles;
            if (graph_stream[u].find(v) == graph_stream[u].end() and
                graph_stream[v].find(u) == graph_stream[v].end()) {
                num_edges++;
                // -- add edge to graph stream
                graph_stream[u].emplace(v);
                graph_stream[v].emplace(u);
            }
        }


        // -- update unique edges count
        if (unique_edges.find(edge_to_id(u, v)) == unique_edges.end()) {
            if (sign == '+')
                unique_edges[edge_to_id(u, v)] = {1, 0};
            else
                unique_edges[edge_to_id(u, v)] = {0, 1};
        } else {
            if (sign == '+')
                unique_edges[edge_to_id(u, v)].first += 1;
            else
                unique_edges[edge_to_id(u, v)].second += 1;
        }


        if (num_edges > max_edges) {
            max_edges = num_edges;
            time_max_edges = nline;
        }

        nline++;
        if (nline % 3000000 == 0) {
            printf("Processed %ld edges | Subgraph contains: %ld edges - Counted: %ld triangles\n", nline, num_edges, total_T);
        }

    }

    long num_nodes = (long) unique_nodes.size();
    printf("Processed dataset with n = %ld, m = %ld\n", num_nodes, nline);
    printf("Unique edges count: %ld\n", (long) unique_edges.size());
    // -- write results
    std::ofstream out_file(output_path, std::ios::app);
    out_file << "Ground Truth:" << "\n";
    out_file << "Number of Unique Nodes = " << num_nodes << "\n";
    out_file << "Number of Nodes at the end = " << (long) graph_stream.size() << "\n";
    out_file << "Number of Edges = " << nline << "\n";
    out_file << "Maximum Number of Edges = " << max_edges << " at time " << time_max_edges << " in the stream\n";
    out_file << "Number of Edges at the end = " << num_edges << "\n";
    out_file << "Number of Unique Edges = " << (long) unique_edges.size() << "\n";
    out_file << "Triangles = " << total_T << "\n";
    out_file.close();
    return total_T;

}

/**
 * Read node oracle
 * @param oracle_filename path for the oracle file
 * @param delimiter for rows of oracle file
 * @param skip line to skip at the beginning of oracle file
 * @param node_oracle the filled hashmap containing the node oracle
 * @return true if the oracle file is read correctly, false otherwise
 */
bool Utils::read_node_oracle(std::string &oracle_filename, char delimiter, int skip,
                             emhash5::HashMap<int, int> &node_oracle) {

    std::ifstream file(oracle_filename);
    std::string line;
    int i = 0;
    if (file.is_open()) {
        while (std::getline(file, line)) {
            if (i >= skip) {
                std::istringstream iss(line);
                std::string token;
                std::getline(iss, token, delimiter);
                int node = std::stoi(token);
                std::getline(iss, token, delimiter);
                int label = std::stoi(token);
                node_oracle.insert_unique(node, label);
            }
            i++;
        }
        file.close();
        return true;
    } else {
        std::cerr << "Error! Unable to open file " << oracle_filename << "\n";
        return false;
    }

}

/**
 * Read edge oracle
 * @param oracle_filename path for the oracle file
 * @param delimiter for rows of oracle file
 * @param skip line to skip at the beginning of oracle file
 * @param edge_id_oracle the filled hashmap containing the edge oracle
 * @return true if the oracle file is read correctly, false otherwise
 */
bool Utils::read_edge_oracle(std::string &oracle_filename, char delimiter, int skip,
                             emhash5::HashMap<long, int> &edge_id_oracle) {


    std::ifstream file(oracle_filename);
    std::string line;
    int i = 0;
    if (file.is_open()) {
        while (std::getline(file, line)) {
            if (i >= skip) {
                std::istringstream iss(line);
                std::string token;
                std::getline(iss, token, delimiter);
                int u = std::stoi(token);
                std::getline(iss, token, delimiter);
                int v = std::stoi(token);
                std::getline(iss, token, delimiter);
                int label = std::stoi(token);
                edge_id_oracle.insert_unique(edge_to_id(u, v), label);
            }
            i++;
        }
        file.close();
        return true;
    } else {
        std::cerr << "Error! Unable to open file " << oracle_filename << "\n";
        return false;
    }

}

/**
 * Function that preprocesses a graph and saves it in the correct format, i.e., (u v t) for each row, separated
 * by a space delimiter. Also, handles self-loops and multiple edges and sorts the edges by increasing time of arrival
 * in the stream.
 * @param dataset_filepath path for the graph dataset file
 * @param delimiter for rows of graph dataset file
 * @param skip line to skip at the beginning of graph dataset file
 * @param output_path where to store the preprocess graph dataset
 */
void Utils::preprocess_data(const std::string &dataset_filepath, std::string &delimiter, int skip,
                            std::string &output_path) {

    std::cout << "Preprocessing Dataset...\n";
    std::ifstream file(dataset_filepath);
    std::string line, su, sv;

    // -- edge stream
    std::unordered_map<Edge, int, hash_edge> edge_stream;

    // - graph
    std::unordered_map<int, std::unordered_set<int>> graph_stream;

    int u, v, t;
    std::unordered_set<int> min_neighbors;

    if (file.is_open()) {

        long nline = 0;
        long num_nodes;
        long num_edges = 0;

        t = 0;
        while (std::getline(file, line)) {
            nline++;
            if (nline <= skip) continue;

            std::istringstream iss(line);
            std::getline(iss, su, delimiter[0]);
            std::getline(iss, sv, delimiter[0]);

            u = stoi(su);
            v = stoi(sv);

            // -- check self-loops
            if (u == v) continue;
            t++;
            int v1 = (u < v) ? u : v;
            int v2 = (u < v) ? v : u;
            std::pair uv = std::make_pair(v1, v2);
            // -- check for multiple edges
            if (graph_stream[u].find(v) != graph_stream[u].end() and
                graph_stream[v].find(u) != graph_stream[v].end()) {
                edge_stream[uv] = t;
                continue;
            }

            // -- add edge to graph stream
            graph_stream[u].emplace(v);
            graph_stream[v].emplace(u);
            num_edges++;
            edge_stream[uv] = t;

            if (nline % 3000000 == 0) {
                std::cout << "Processed " << nline << " edges...\n";
            }

        }

        // -- eof
        num_nodes = (int) graph_stream.size();
        printf("Preprocessed dataset with n = %ld, m = %ld\n", num_nodes, num_edges);
        std::cout << "Sorting edge map...\n";
        // -- create a vector that stores all the entries <K, V> of the map edge stream
        std::vector<std::pair<Edge, int>> ordered_edge_stream(edge_stream.begin(), edge_stream.end());
        // -- sort edge by increasing time
        std::sort(ordered_edge_stream.begin(), ordered_edge_stream.end(),
                  [](const std::pair<Edge, int> &a, const std::pair<Edge, int> &b) { return a.second < b.second; });

        // -- write results
        std::cout << "Done!\nWriting results...\n";
        std::ofstream out_file(output_path);

        int cnt = 0;
        for (auto elem: ordered_edge_stream) {
            // -- also, rescale the time (not meant for Tonic)
            out_file << elem.first.first << " " << elem.first.second << " " << ++cnt << "\n";
        }

        out_file.close();

    } else {
        std::cerr << "DataPreprocessing - Error! Graph filepath not opened.\n";
    }

}

/**
 * Function that preprocesses a graph snapshot from a graph sequence, used for creating FD streams.
 * Differs from the above function beacause do not
 * include the final ordering of timestamps, which is done in later stages (also considering subsequent edge deletions)
 * @param dataset_filepath path for the graph dataset file
 * @param delimiter for rows of graph dataset file
 * @param skip line to skip at the beginning of graph dataset file
 * @param output_path where to store the preprocess graph dataset
 * @return the preprocessed edge stream and the number of edges and the current maximum timestamp
 */
std::pair<Utils::EdgeStream, long> Utils::preprocess_data_FD(const std::string &dataset_filepath,
                                                             std::string &delimiter, int skip) {

    std::cout << "Preprocessing Dataset...\n";
    std::ifstream file(dataset_filepath);
    std::string line, su, sv;

    // -- edge stream
    EdgeStream edge_stream;

    // - graph
    std::unordered_map<int, std::unordered_set<int>> graph_stream;

    int u, v;
    long t;
    std::unordered_set<int> min_neighbors;

    if (file.is_open()) {

        long nline = 0;
        long num_nodes;
        long num_edges = 0;

        t = 0;
        while (std::getline(file, line)) {
            nline++;
            if (nline <= skip) continue;

            std::istringstream iss(line);
            iss >> u >> v;

            // -- check self-loops
            if (u == v) continue;
            int v1 = (u < v) ? u : v;
            int v2 = (u < v) ? v : u;
            std::pair uv = std::make_pair(v1, v2);
            // -- check for multiple edges
            if (graph_stream[u].find(v) != graph_stream[u].end() and
                graph_stream[v].find(u) != graph_stream[v].end()) {
                edge_stream[uv] = t;
                continue;
            }

            // -- add edge to graph stream
            graph_stream[u].emplace(v);
            graph_stream[v].emplace(u);
            num_edges++;
            edge_stream[uv] = t;

            t++;

            if (nline % 3000000 == 0) {
                std::cout << "Processed " << nline << " edges...\n";
            }

        }

        // -- eof
        num_nodes = (int) graph_stream.size();
        printf("Preprocessed dataset with n = %ld, m = %ld\n", num_nodes, num_edges);
        assert(num_edges == edge_stream.size());
        return {edge_stream, t};

    } else {
        std::cerr << "DataPreprocessing - Error! Graph filepath not opened.\n";
        exit(0);
    }

}

/**
 * Merges snapshot from a sequence of graphs, computing edge deletions and additions to derive the final fully dynamic
 * stream
 * @param folder containing the graph sequences filepaths
 * @param n_snapshots the number of graphs to merge
 * @param delimiter for rows of snapshots dataset file
 * @param line_to_skip at the beginning of snapshot dataset file
 * @param output_path where to write the final FD stream
 */
void Utils::merge_snapshots_FD(std::string &folder, int n_snapshots, std::string &delimiter, int line_to_skip,
                            std::string &output_path) {

    std::vector<EdgeSigned> fd_edge_stream;
    EdgeStream edge_additions;

    // -- loop through all .txt files in the folder
    std::vector<std::string> files;
    for (const auto &entry : std::filesystem::directory_iterator(folder)) {
        if (entry.path().extension() == ".txt") {
            files.push_back(entry.path().string());
        }
    }

    // -- sort files by name
    std::sort(files.begin(), files.end());

    int idx_snap = 0;
    long current_timestamp = 0;
    // -- random int distro
    std::random_device rd;
    std::mt19937 gen(rd());


    for (const auto &file: files) {
        idx_snap += 1;
        std::cout << "Processing file #" << idx_snap << ": " << file << "\n";

        if (idx_snap > n_snapshots) break;

        // std::string out_snap_path = file.substr(0, file.find_last_of('.')) + "_preprocessed.txt";
        auto snap = Utils::preprocess_data_FD(file, delimiter, line_to_skip);
        EdgeStream snap_stream = snap.first;
        long max_timestamp = snap.second;

        if (idx_snap == 1) {
            // -- first snap: let stream = G1
            // -- add edges to fd stream
            for (auto &edge: snap_stream) {
                fd_edge_stream.push_back({{edge.first, edge.second}, 1});
            }

            edge_additions.merge(snap_stream);

            current_timestamp = max_timestamp;


            // -- print intermediate length of FD stream
            std::cout << "Length of FD stream = " << fd_edge_stream.size() << "\n";

        } else {

            // -- idx snap > 1
            printf("Merging %d and %d snapshots...\n", idx_snap -1, idx_snap);
            // -- in e_a, store snap_stream \ edge_additions
            EdgeStream e_a;
            for (const auto &edge : snap_stream) {
                if (edge_additions.find(edge.first) == edge_additions.end()) {
                    e_a[edge.first] = edge.second;
                }
            }
            // -- in e_d, store edge_additions \ snap_stream
            EdgeStream e_d;
            for (const auto &edge : edge_additions) {
                if (snap_stream.find(edge.first) == snap_stream.end()) {
                    e_d[edge.first] = edge.second;
                }
            }

            printf("|Edges in merged streams| = %d\n|Edges in %d snapshot| = %d\n", (int) edge_additions.size(),
                   idx_snap, (int) snap_stream.size());
            printf("|Edges added| = %ld\n|Edges deleted| = %ld\n", e_a.size(), e_d.size());

            // -- add to fd stream edges in e_a with 1 sign
            for (auto &edge: e_a) {
                long timestamp = current_timestamp + edge.second;
                fd_edge_stream.push_back({{edge.first, timestamp}, 1});
                edge_additions[edge.first] = timestamp;
            }

            // -- add to fd stream edges in e_d with -1 sign and random timestamps
            std::cout << "Current timestamp: " << current_timestamp << ", Max Timestamp: " << max_timestamp << "\n";
            std::uniform_int_distribution<long> dis(current_timestamp + 1, current_timestamp + max_timestamp);
            for (auto &edge: e_d) {
                long random_timestamp = dis(gen);
                fd_edge_stream.push_back({{edge.first, random_timestamp}, -1});
                edge_additions.erase(edge.first);
            }

            current_timestamp += max_timestamp;

        }

    }

    // std::cout << "Max timestamp: " << current_timestamp << "\n";

    // -- print length of fd_edge_stream
    std::cout << "Length of FD stream = " << fd_edge_stream.size() << "\n";

    std::cout << "Sorting and writing the final FD stream...\n";
    std::sort(fd_edge_stream.begin(), fd_edge_stream.end(),
              [](const EdgeSigned &a, const EdgeSigned &b) { return a.first.second < b.first.second; });

    std::ofstream out_file(output_path);
    for (auto &edge: fd_edge_stream) {
        char sign = (edge.second == 1) ? '+' : '-';
        out_file << edge.first.first.first << " " << edge.first.first.second << " "
                 << edge.first.second << " " << sign << "\n";
    }

    out_file.close();
    std::cout << "Done!\n";

}

/**
 * Function that builds OracleExact, given the graph filepath. Requires to solve the problem of counting exactly the
 * number of triangles in a graph stream.
 * @param filepath of the graph for which deriving OracleExact
 * @param percentage_retain of entries ((u,v); O_H((u, v))) to store sorted by O_H
 * @param output_path where to write OracleExact
 */
void Utils::build_edge_exact_oracle(std::string &filepath, double percentage_retain, std::string &output_path) {

    std::cout << "Building edge oracle...\n";

    std::ifstream file(filepath);
    std::string line;

    emhash5::HashMap<Edge, int, hash_edge> oracle_heaviness;

    // -- graph
    emhash5::HashMap<int, std::unordered_set<int>> graph_stream;
    std::unordered_set<int> min_neighs;

    long total_T = 0.0;
    int u, v, t;

    if (file.is_open()) {
        long nline = 0;
        while (std::getline(file, line)) {
            nline++;
            std::istringstream iss(line);
            iss >> u >> v >> t;
            if (u == v) continue;
            if (graph_stream[u].find(v) != graph_stream[u].end() and graph_stream[v].find(u) != graph_stream[v].end()) {
                continue;
            }
            graph_stream[u].emplace(v);
            graph_stream[v].emplace(u);
            int du = (int) graph_stream[u].size();
            int dv = (int) graph_stream[v].size();
            int n_min = (du < dv) ? u : v;
            int n_max = (du < dv) ? v : u;
            min_neighs = graph_stream[n_min];
            int common_neighs = 0;
            for (auto neigh: min_neighs) {
                if (graph_stream[n_max].find(neigh) != graph_stream[n_max].end()) {
                    common_neighs++;
                    int entry_11 = (n_min < neigh) ? n_min : neigh;
                    int entry_12 = (n_min < neigh) ? neigh : n_min;
                    int entry_21 = (neigh < n_max) ? neigh : n_max;
                    int entry_22 = (neigh < n_max) ? n_max : neigh;

                    oracle_heaviness[{entry_11, entry_12}] += 1;
                    oracle_heaviness[{entry_21, entry_22}] += 1;
                }
            }

            int entry_31 = (u < v) ? u : v;
            int entry_32 = (u < v) ? v : u;
            oracle_heaviness[{entry_31, entry_32}] = common_neighs;
            total_T += common_neighs;

            if (nline % 3000000 == 0) {
                printf("Processed %ld edges | Counted %ld triangles\n", nline, total_T);
            }
        }

        // -- eof: sort results

        std::cout << "Sorting the oracle and retrieving the top " << percentage_retain << " values...\n";
        std::vector<std::pair<Edge, int>> sorted_oracle;
        sorted_oracle.reserve(oracle_heaviness.size());
        for (auto &elem: oracle_heaviness) {
            sorted_oracle.emplace_back(elem.first, elem.second);
        }


        std::sort(sorted_oracle.begin(), sorted_oracle.end(),
                  [](const std::pair<Edge, int> &a, const std::pair<Edge, int> &b) { return a.second > b.second; });

        // -- write results
        std::cout << "Done!\nWriting results...\n";
        int stop_idx = (int) (percentage_retain * (int) sorted_oracle.size());

        std::ofstream out_file(output_path);
        std::cout << "Total Triangles -> " << total_T << "\n";
        std::cout << "Full Oracle Size = " << sorted_oracle.size() << "\n";

        std::cout << "Writing top " << stop_idx << " entries...\n";

        int cnt = 0;
        for (auto elem: sorted_oracle) {
            if (cnt >= stop_idx) break;
            out_file << elem.first.first << " " << elem.first.second << " " << elem.second << "\n";
            cnt++;
        }


    } else {
        std::cerr << "Error! Unable to open oracle file " << filepath << "\n";
    }
}

/**
 * Function that builds Oracle-noWR, given the graph filepath. Requires to solve the problem of counting exactly the
 * number of triangles in a graph stream
 * @param filepath of the graph for which deriving Oracle-noWR
 * @param percentage_retain of entries ((u,v); O_H((u, v))) to store sorted by O_H
 * @param output_path where to write Oracle-noWR
 * @param wr_size the dimension of the waiting room. Used to compute the triangles inside the waiting room to be
 * subtracted to the true heaviness to derive Oracle-noWR
 */
void Utils::build_edge_exact_nowr_oracle(std::string &filepath, double percentage_retain, std::string &output_path,
                                         int wr_size) {

    std::cout << "Building edge oracle...\n";

    std::ifstream file(filepath);
    std::string line;

    emhash5::HashMap<Edge, int, hash_edge> oracle_heaviness;
    emhash5::HashMap<Edge, int, hash_edge> edge_wr_heaviness;
    emhash5::HashMap<Edge, int, hash_edge> edge_time_arrival;

    // -- graph
    emhash5::HashMap<int, std::unordered_set<int>> graph_stream;
    std::unordered_set<int> min_neighs;

    long total_T = 0.0;
    int u, v, t, src, dst;

    if (file.is_open()) {
        long nline = 0;
        while (std::getline(file, line)) {
            nline++;
            std::istringstream iss(line);
            iss >> src >> dst >> t;

            u = src;
            v = dst;
            if (src > dst){
                u = dst;
                v = src;
            }

            if (u == v) continue;
            if (graph_stream[u].find(v) != graph_stream[u].end() and graph_stream[v].find(u) != graph_stream[v].end()) {
                continue;
            }

            edge_time_arrival[{u, v}] = nline;

            graph_stream[u].emplace(v);
            graph_stream[v].emplace(u);

            int du = (int) graph_stream[u].size();
            int dv = (int) graph_stream[v].size();
            int n_min = (du < dv) ? u : v;
            int n_max = (du < dv) ? v : u;
            min_neighs = graph_stream[n_min];
            int common_neighs = 0;
            for (auto neigh: min_neighs) {
                if (graph_stream[n_max].find(neigh) != graph_stream[n_max].end()) {
                    common_neighs++;
                    int entry_11 = (n_min < neigh) ? n_min : neigh;
                    int entry_12 = (n_min < neigh) ? neigh : n_min;
                    int entry_21 = (neigh < n_max) ? neigh : n_max;
                    int entry_22 = (neigh < n_max) ? n_max : neigh;

                    oracle_heaviness[{entry_11, entry_12}] += 1;
                    oracle_heaviness[{entry_21, entry_22}] += 1;

                    // -- account for triangles inside the waiting room
                    if (nline - edge_time_arrival[{entry_11, entry_12}] < wr_size)
                        edge_wr_heaviness[{entry_11, entry_12}] += 1;

                    if (nline - edge_time_arrival[{entry_21, entry_22}] < wr_size)
                        edge_wr_heaviness[{entry_21, entry_22}] += 1;

                }
            }

            int entry_31 = (u < v) ? u : v;
            int entry_32 = (u < v) ? v : u;
            oracle_heaviness[{entry_31, entry_32}] = common_neighs;
            edge_wr_heaviness[{entry_31, entry_32}] = 0;

            total_T += common_neighs;

            if (nline % 3000000 == 0) {
                printf("Processed %ld edges | Counted %ld triangles\n", nline, total_T);
            }
        }

        // -- eof: sort results

        std::cout << "Sorting the oracle and retrieving the top " << percentage_retain << " values...\n";
        std::vector<std::pair<Edge, int>> sorted_oracle;
        sorted_oracle.reserve(oracle_heaviness.size());
        for (auto &elem: oracle_heaviness) {
            sorted_oracle.emplace_back(elem.first, elem.second - edge_wr_heaviness[elem.first]);
        }

        std::sort(sorted_oracle.begin(), sorted_oracle.end(),
                  [](const std::pair<Edge, int> &a, const std::pair<Edge, int> &b) { return a.second > b.second; });

        // -- write results
        std::cout << "Done!\nWriting results...\n";
        int stop_idx = (int) (percentage_retain * (int) sorted_oracle.size());

        std::ofstream out_file(output_path);
        std::cout << "Total Triangles -> " << total_T << "\n";
        std::cout << "Full Oracle Size = " << sorted_oracle.size() << "\n";

        std::cout << "Writing top " << stop_idx << " entries...\n";

        int cnt = 0;
        for (auto elem: sorted_oracle) {
            if (cnt >= stop_idx) break;
            out_file << elem.first.first << " " << elem.first.second << " " << elem.second << "\n";
            cnt++;
        }


    } else {
        std::cerr << "Error! Unable to open oracle file " << filepath << "\n";
    }
}

/**
 * Function that builds node based MinDegreePredictor, given the graph filepath. Requires a fast pass to the stream
 * to compute node degrees.
 * @param filepath of the graph for which deriving MinDegreePredictor
 * @param percentage_retain of entries (u; deg(u)) to store sorted by deg(u)
 * @param output_path where to write MinDegreePredictor
 */
void Utils::build_node_oracle(std::string &filepath, double percentage_retain, std::string &output_path) {

    std::cout << "Building node oracle...\n";

    std::ifstream file(filepath);
    std::string line;

    emhash5::HashMap<int, int> node_map;
    // std::unordered_map<int, int> node_map;

    int u, v, t;

    if (file.is_open()) {
        long nline = 0;
        while (std::getline(file, line)) {
            nline++;
            std::istringstream iss(line);
            iss >> u >> v >> t;
            if (u == v) continue;

            if (node_map.find(u) != node_map.end())
                node_map[u] += 1;
            else
                node_map[u] = 1;

            if (node_map.find(v) != node_map.end())
                node_map[v] += 1;
            else
                node_map[v] = 1;

            if (nline % 3000000 == 0) {
                printf("Processed %ld edges\n", nline);
            }
        }

        // -- eof: sort results
        std::cout << "Sorting the oracle and retrieving the top " << percentage_retain << " values...\n";
        // convert node map to vector of pairs
        std::vector<std::pair<int, int>> sorted_oracle;
        sorted_oracle.reserve(node_map.size());
        for (auto &elem: node_map) {
            sorted_oracle.emplace_back(elem.first, elem.second);
        }

        // std::vector<std::pair<int, int>> sorted_oracle(node_map.begin(), node_map.end());
        std::sort(sorted_oracle.begin(), sorted_oracle.end(),
                  [](const std::pair<int, int> &a, const std::pair<int, int> &b) { return a.second > b.second; });

        // -- write results
        std::cout << "Done!\nWriting results...\n";
        int stop_idx = (int) (percentage_retain * (int) sorted_oracle.size());

        std::ofstream out_file(output_path);
        std::cout << "Oracle Size = " << sorted_oracle.size() << "\n";

        int cnt = 0;
        for (auto elem: sorted_oracle) {
            if (cnt > stop_idx) break;
            out_file << elem.first << " " << elem.second << "\n";
            cnt++;
        }

    } else {
        std::cerr << "Error! Unable to open oracle file " << filepath << "\n";
    }
}

