//
// Created by X on 09/03/24.
//

#ifndef TONIC_TONIC_H
#define TONIC_TONIC_H

#include "hash_table5.hpp"
#include "FixedSizePQ.h"
#include <iostream>
#include <string>
#include <random>

using Edge = std::pair<int, int>;
using Heavy_edge = std::pair<Edge, int>;


class Tonic {

private:

    emhash5::HashMap<int , emhash5::HashMap<int, bool>> subgraph_;

    // -- heavy edge comparator -> return lightest edge
    struct heavy_edge_cmp {
        bool operator()(const Heavy_edge &a, const Heavy_edge &b) const {
            return a.second > b.second;
        }
    };

    // -- oracles
    emhash5::HashMap<int, int> node_oracle_;
    emhash5::HashMap<long, int> edge_id_oracle_;

    // -- sets for storing edges
    Edge* waiting_room_;
    FixedSizePQ<Heavy_edge, heavy_edge_cmp> heavy_edges_;
    Edge* light_edges_sample_;

    std::mt19937 gen_;
    std::uniform_real_distribution<double> dis_;
    std::uniform_int_distribution<int> dis_int_;

    // -- member variables
    unsigned long long t_;

    long WR_size_;
    long H_size_;
    long SL_size_;

    // -- current counters
    long WR_cur_ = 0;
    long H_cur_ = 0;
    long SL_cur_ = 0;

    int num_edges_;

    // -- triangle estimates
    double global_triangles_cnt_ = 0.0;
    emhash5::HashMap<int, double> local_triangles_cnt_;

    int get_heaviness(const int u, const int v);

    void add_edge(const int u, const int v, bool det);

    void remove_edge(const int u, const int v);

    void count_triangles(const int u, const int v);

    bool sample_edge(const int u, const int v);

    inline double next_double();




public:

    long k_;
    double alpha_;
    double beta_;
    bool edge_oracle_flag_ = false;

    constexpr static unsigned long long MAX_ID_NODE = 75000000;

    inline static unsigned long long edge_to_id(const int u, const int v) {
        int nu = (u < v ? u : v);
        int nv = (u < v ? v : u);
        return (MAX_ID_NODE) * static_cast<unsigned long long>(nu) +
               static_cast<unsigned long long>(nv);
    }

    Tonic(int random_seed, long k, double alpha, double beta);

    ~Tonic();

    void set_edge_oracle(emhash5::HashMap<long, int> &edge_oracle);

    void set_node_oracle(emhash5::HashMap<int, int> &node_oracle);

    void process_edge(const int u, const int v);

    int get_num_nodes() const;

    int get_num_edges() const;

    double get_local_triangles(const int u) const;

    void get_nodes(std::vector<int> &nodes) const;

    double get_global_triangles() const;

    void get_local_nodes(std::vector<int> &nodes) const;

    inline unsigned long long get_edges_processed() const;


};


#endif
