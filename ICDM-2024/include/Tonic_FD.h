//
// Created by X on 09/03/24.
//

#ifndef TONIC_FD
#define TONIC_FD

#include "hash_table5.hpp"
#include "hash_set8.hpp"
#include "FixedSizePQ.h"
#include "Utils.h"
#include <iostream>
#include <string>
#include <random>
#include <climits>

class Tonic_FD {

using Heavy_edge = Utils::Heavy_edge;
using Edge = Utils::Edge;

private:

    class WaitingRoom {

    using Edge = Utils::Edge;

    private:

        inline static unsigned long long edge_to_wr_id(const int u, const int v) {
            int nu = u < v ? u : v;
            int nv = u < v ? v : u;
            return static_cast<unsigned long long>(INT_MAX) * static_cast<unsigned long long>(nu) +
                   static_cast<unsigned long long>(nv);
        }

        inline static Edge edge_wr_to_id(unsigned long long id) {
            int u = (int) (id / INT_MAX);
            int v = (int) (id % INT_MAX);
            return {u, v};
        }

        // Edge* waiting_room_;
        // emhash5::HashMap<long, long> edge_to_index_;
        emhash8::HashSet<unsigned long long> waiting_room_;

    public:
        long max_size_;
        long cur_size_;
        long oldest_edge_idx_;

        WaitingRoom(long max_size);

        WaitingRoom();

        ~WaitingRoom();

        void add_edge(int u, int v);

        bool remove_edge(int u, int v);

        Edge pop_oldest_edge();

    };

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

    WaitingRoom* waiting_room_;


    // -- priority queue for all heavy edges
    FixedSizePQ<Heavy_edge, heavy_edge_cmp> heavy_edges_;
    // -- heavy edges set
    emhash8::HashSet<unsigned long long> heavy_edges_set_;

    Edge* light_edges_sample_;

    std::mt19937 gen_;
    std::uniform_real_distribution<double> dis_;


    // -- member variables
    unsigned long long t_;
    long WR_size_;
    long H_size_;
    long SL_size_;

    // -- current counters
    long H_cur_ = 0;
    long ell_ = 0;
    long SL_cur_ = 0;

    int d_g = 0, d_b = 0;
    long current_timestamp_ = 0;
    long num_edges_;

    // -- triangle estimates
    double global_triangles_cnt_ = 0.0;
    emhash5::HashMap<int, double> local_triangles_cnt_;

    // -- edge to index
    emhash5::HashMap<long, int> edge_id_to_index_;

    int get_heaviness(const int u, const int v);

    void add_edge(const int u, const int v, bool det);

    bool remove_edge(const int u, const int v);

    int edge_deletion(const int u, const int v);

    void count_triangles(const int u, const int v, const int sign);

    void sample_edge(const int u, const int v);

    inline double next_double();



public:

    inline static unsigned long long edge_to_id(const int u, const int v) {
        return Utils::edge_to_id(u, v);
    }

    long k_;
    double alpha_, beta_;
    bool edge_oracle_flag_ = false;

    Tonic_FD(int random_seed, long k, double alpha, double beta);

    ~Tonic_FD();

    void set_edge_oracle(emhash5::HashMap<long, int> &edge_oracle);

    void set_node_oracle(emhash5::HashMap<int, int> &node_oracle);

    void process_edge(const int u, const int v, const int t, const int sign);

    long get_num_nodes() const;

    long get_num_edges() const;

    double get_local_triangles(const int u) const;

    void get_nodes(std::vector<int> &nodes) const;

    double get_global_triangles() const;

    void get_local_nodes(std::vector<int> &nodes) const;

    inline unsigned long long get_edges_processed() const;


};


#endif
