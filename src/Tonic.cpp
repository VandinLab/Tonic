//
// Created by X on 09/03/24.
//

#include "Tonic.h"

/**
 * Constructor for Tonic - insertion only algorithm
 * @param random_seed
 * @param k memory budget
 * @param alpha
 * @param beta
 */
Tonic::Tonic(int random_seed, long k, double alpha, double beta) : t_(0), k_(k), alpha_(alpha),
                                                                   beta_(beta)  {

    printf("Starting Tonic Algo - alpha %.3f, beta = %.3f | Memory Budget = %ld\n", alpha, beta, k);
    WR_size_ = (long) (k_ * alpha);
    H_size_ = (long) ((k_ - WR_size_) * beta);
    SL_size_ = k_ - WR_size_ - H_size_;
    waiting_room_ = new Edge[WR_size_];
    heavy_edges_ = FixedSizePQ<Heavy_edge, heavy_edge_cmp>(H_size_);
    light_edges_sample_ = new Edge[SL_size_];
    num_edges_ = 0;
    printf("WR size = %ld, H size = %ld, SL size = %ld\n", WR_size_, H_size_, SL_size_);
    subgraph_ = emhash5::HashMap<int, emhash5::HashMap<int, bool>>();
    gen_ = std::mt19937(random_seed);
    dis_ = std::uniform_real_distribution<double>(0.0, 1.0);
    dis_int_ = std::uniform_int_distribution<int>(0, (int)SL_size_ - 1);
}


/**
 * Destructor for Tonic
 */
Tonic::~Tonic() {
    delete[] waiting_room_;
    delete[] light_edges_sample_;
    subgraph_.clear();
}

/**
 * Set the edge oracle for Tonic
 * @param edge_oracle
 */
void Tonic::set_edge_oracle(emhash5::HashMap<long, int> &edge_oracle) {
    edge_id_oracle_ = edge_oracle;
    edge_oracle_flag_ = true;
}

/**
 * Set the node oracle for Tonic
 * @param node_oracle
 */
void Tonic::set_node_oracle(emhash5::HashMap<int, int> &node_oracle) {
    node_oracle_ = node_oracle;
}

/**
* Return heaviness prediction from the node or edge oracle given the current edge (u, v)
 * @param u
 * @param v
 * @return heaviness if the edge or both nodes are found in the predictor, -1 otherwise
 */
int Tonic::get_heaviness(const int u, const int v) {
    if (edge_oracle_flag_) {
        auto id_it = edge_id_oracle_.find(edge_to_id(u, v));
        if (id_it != edge_id_oracle_.end()) {
            return id_it->second;
        } else {
            return -1;
        }
    } else {
        auto u_it = node_oracle_.find(u);
        if (u_it != node_oracle_.end()) {
            auto v_it = node_oracle_.find(v);
            if (v_it != node_oracle_.end()) {
                return std::min(u_it->second, v_it->second);
            }
        }
        return -1;
    }
}

/**
 * Generate a random double between 0 and 1
 * @return random double
 */
inline double Tonic::next_double() {
    return dis_(gen_);
}

/**
 * Return the number of nodes in the subgraph
 * @return number of nodes
 */
int Tonic::get_num_nodes() const {
    return (int) subgraph_.size();
}

/**
 * Return the number of edges in the subgraph
 * @return number of edges
 */
int Tonic::get_num_edges() const {
    return num_edges_;
}

/**
 * Return the nodes in the subgraph
 * @param nodes to fill
 */
void Tonic::get_nodes(std::vector<int> &nodes) const {
    nodes.clear();
    for (const auto &it: subgraph_) {
        nodes.push_back(it.first);
    }
}

/**
 * Return the local nodes in the subgraph
 * @param nodes to fill
 */
void Tonic::get_local_nodes(std::vector<int> &nodes) const {
    nodes.clear();
    for (const auto &it: local_triangles_cnt_) {
        nodes.push_back(it.first);
    }
}

/**
 * Function that adds an edge (u, v) to the subgraph
 * @param u
 * @param v
 * @param det true if the edge to be inserted is deterministic (heavy or WR), false otherwise (light, in SL)
 */
void Tonic::add_edge(const int u, const int v, bool det) {
    num_edges_++;
    subgraph_[u].emplace_unique(v, det);
    subgraph_[v].emplace_unique(u, det);

}

/**
 * Function that removes an edge (u, v) from the subgraph
 * @param u
 * @param v
 */
void Tonic::remove_edge(const int u, const int v) {
    num_edges_--;
    subgraph_[u].erase(v);
    subgraph_[v].erase(u);
}

/**
 * Function that return the current timestamp in the stream
 * @return current timestamp
 */
inline unsigned long long Tonic::get_edges_processed() const {
    return t_;
}

/**
 * Function that returns the global triangle count
 * @return the global triangle count
 */
double Tonic::get_global_triangles() const {
    return global_triangles_cnt_;
}

/**
 * Function that returns the local triangle count for a node u
 * @param u
 * @return the local triangle count for node u
 */
double Tonic::get_local_triangles(const int u) const {
    auto u_it = local_triangles_cnt_.find(u);
    if (u_it != local_triangles_cnt_.end()) {
        return u_it->second;
    } else {
        return 0.0;
    }
}

/**
 * Function that counts the triangles closed by the current edge (src, dst). The function is called before the edge is
 * sampled.
 * @param src
 * @param dst
 */
void Tonic::count_triangles(const int src, const int dst) {
    emhash5::HashMap<int, bool> *u_neighs, *v_neighs;
    auto u_it = subgraph_.find(src);
    if (u_it == subgraph_.end()) {
        return;
    }
    u_neighs = &u_it->second;
    int du = (int) u_neighs->size();

    auto v_it = subgraph_.find(dst);
    if (v_it == subgraph_.end()) {
        return;
    }
    v_neighs = &v_it->second;
    int dv = (int) v_neighs->size();
    int u = src;
    int v = dst;

    if (du > dv) {
        v = src;
        u = dst;
        emhash5::HashMap<int, bool> *u_neighs_tmp = u_neighs;
        u_neighs = v_neighs;
        v_neighs = u_neighs_tmp;
    }

    double cum_cnt = 0.0;

    // -- iterate over the neighbors of u
    for (const auto &it: *u_neighs) {
        int w = it.first;
        auto vw_it = v_neighs->find(w);
        if (vw_it != v_neighs->end()) {
            // -- triangle {u, v, w} discovered
            double increment_T = 1.0;
            if (SL_cur_ > SL_size_) {
                bool vw_light = !vw_it->second;
                bool wu_light = !it.second;
                if (vw_light && wu_light) {
                    increment_T = ((double) (SL_cur_) / SL_size_) * ((double) ((SL_cur_ - 1.0))) / (SL_size_ - 1.0);
                } else if (vw_light || wu_light) {
                    increment_T = ((double) (SL_cur_) / SL_size_);
                }
            }

            cum_cnt += increment_T;
            auto w_it = local_triangles_cnt_.find(w);
            if (w_it != local_triangles_cnt_.end()) {
                w_it->second += increment_T;
            } else {
                local_triangles_cnt_.insert_unique(w, increment_T);
            }

        }
    } // end for

    // -- update counters
    if (cum_cnt > 0) {
        global_triangles_cnt_ += cum_cnt;
        auto u_local_it = local_triangles_cnt_.find(u);
        if (u_local_it != local_triangles_cnt_.end()) {
            u_local_it->second += cum_cnt;
        } else {
            local_triangles_cnt_.insert_unique(u, cum_cnt);
        }
        auto v_local_it = local_triangles_cnt_.find(v);
        if (v_local_it != local_triangles_cnt_.end()) {
            v_local_it->second += cum_cnt;
        } else {
            local_triangles_cnt_.insert_unique(v, cum_cnt);
        }
    }
}

/**
 * Function that samples an edge (u, v) from the stream in the correct sets W, H or SL.
 * @param u
 * @param v
 */
bool Tonic::sample_edge(const int src, const int dst) {

    int u = src;
    int v = dst;

    if (src > dst) {
        u = dst;
        v = src;
    }

    if (H_cur_ < H_size_) {
        // -- insert current edge into H
        H_cur_++;
        int current_heaviness = get_heaviness(u, v);
        heavy_edges_.push({{u, v}, current_heaviness});
        return true;
    } else {
        // -- H is full -> retrieve the lightest heavy edge between current and lightest in H
        if (SL_cur_ < SL_size_) {
            Edge uv_sample = {u, v};
            int current_heaviness = get_heaviness(u, v);
            bool is_det = false;
            if (current_heaviness > -1) {
                auto lightest_heavy_edge = heavy_edges_.top();
                int lightest_heaviness = lightest_heavy_edge.second;
                if (current_heaviness > lightest_heaviness ||
                    (current_heaviness == lightest_heaviness && next_double() < 0.5)) {
                    // -- replace the lightest heavy edge with current edge
                    heavy_edges_.pop();
                    heavy_edges_.push({{u, v}, current_heaviness});
                    is_det = true;
                    subgraph_[lightest_heavy_edge.first.first][lightest_heavy_edge.first.second] = false;
                    subgraph_[lightest_heavy_edge.first.second][lightest_heavy_edge.first.first] = false;
                    uv_sample = lightest_heavy_edge.first;
                }
            }

            light_edges_sample_[SL_cur_++] = uv_sample;
            return is_det;
        } else if (WR_cur_ < WR_size_) {
            waiting_room_[WR_cur_++] = {u, v};
            return true;
        } else {
            // -- all sets are full -> resort to sampling
            SL_cur_++;
            // -- pop oldest and insert current edge in WR
            int oldest_idx = (int) (WR_cur_ % WR_size_);
            WR_cur_++;

            Edge oldest_edge = waiting_room_[oldest_idx];
            waiting_room_[oldest_idx] = {u, v};
            Edge uv_sample = oldest_edge;
            int current_heaviness = get_heaviness(uv_sample.first, uv_sample.second);
            if (current_heaviness > -1) {
                auto lightest_heavy_edge = heavy_edges_.top();
                int lightest_heaviness = lightest_heavy_edge.second;
                if (current_heaviness > lightest_heaviness ||
                    (current_heaviness == lightest_heaviness && next_double() < 0.5)) {
                    // -- replace the lightest heavy edge with current edge
                    heavy_edges_.pop();
                    heavy_edges_.push({{u, v}, current_heaviness});
                    subgraph_[lightest_heavy_edge.first.first][lightest_heavy_edge.first.second] = false;
                    subgraph_[lightest_heavy_edge.first.second][lightest_heavy_edge.first.first] = false;
                    uv_sample = lightest_heavy_edge.first;
                }
            }

            double p = (double) (SL_size_) / (double) SL_cur_;
            if (next_double() < p) {
                // -- edge is sampled
                subgraph_[uv_sample.first][uv_sample.second] = false;
                subgraph_[uv_sample.second][uv_sample.first] = false;
                // -- evict edge uniformly at random
                int replace_idx = (int) (rand() % SL_size_);
                Edge uv_replace = light_edges_sample_[replace_idx];
                remove_edge(uv_replace.first, uv_replace.second);
                light_edges_sample_[replace_idx] = uv_sample;
            } else {
                // -- edge is not resampled, just remove it from subgraph
                remove_edge(uv_sample.first, uv_sample.second);
            }
        }

        return true;

    }
}

/**
 * Function that processes an edge (src, dst). First performs the count of triangles, then samples the edge accordingly.
 * deletions
 * @param src
 * @param dst
 */
void Tonic::process_edge(const int u, const int v) {
    count_triangles(u, v);
    bool is_det = sample_edge(u, v);
    add_edge(u, v, is_det);
    t_++;
    assert(heavy_edges_.size() <= H_size_);
}
