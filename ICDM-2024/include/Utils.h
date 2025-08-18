#ifndef TONIC_UTILS_H
#define TONIC_UTILS_H

#include "hash_table5.hpp"
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <unordered_set>
#include <filesystem>
#include <random>


class Utils {

public:

    constexpr static unsigned long long MAX_ID_NODE = 100000000;

    inline static unsigned long long edge_to_id(const int u, const int v) {
        int nu = (u < v ? u : v);
        int nv = (u < v ? v : u);
        return (MAX_ID_NODE) * static_cast<unsigned long long>(nu) +
               static_cast<unsigned long long>(nv);
    }

    struct hash_edge {
        size_t operator()(const std::pair<int, int> &p) const {
            return edge_to_id(p.first, p.second);
        }
    };

    using Edge = std::pair<int, int>;
    using Heavy_edge = std::pair<Edge, int>;
    using EdgeTimestamped = std::pair<Edge, long>;
    using EdgeSigned = std::pair<EdgeTimestamped, int>;
    using EdgeStream = std::unordered_map<Edge, long, hash_edge>;


    static long run_exact_algorithm(std::string &dataset_filepath,  std::string &output_path);

    static long run_exact_algorithm_FD(std::string &dataset_filepath, std::string &output_path);

    static bool read_node_oracle(std::string &oracle_filename, char delimiter, int skip,
                                 emhash5::HashMap<int, int> &node_oracle);

    static bool read_edge_oracle(std::string &oracle_filename, char delimiter, int skip,
                                 emhash5::HashMap<long, int> &edge_id_oracle);

    static void preprocess_data(const std::string &dataset_path, std::string &delimiter,
                                int skip, std::string &output_path);

    static std::pair<EdgeStream, long> preprocess_data_FD(const std::string &dataset_path, std::string &delimiter,
                                                       int skip);

    static void merge_snapshots_FD(std::string &filepath, int n_snapshots, std::string &delimiter, int line_to_skip,
                                   std::string &output_path);

    static void build_edge_exact_oracle(std::string &filepath, double percentage_retain,
                                  std::string &output_path);

    static void build_edge_exact_nowr_oracle(std::string &filepath, double percentage_retain,
                                        std::string &output_path, int wr_size);

    static void build_node_oracle(std::string &filepath, double percentage_retain,
                                        std::string &output_path);

};


#endif
