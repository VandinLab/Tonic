//
// Created by X on 09/03/24.
//

#include "Tonic_FD.h"

/**
 * Constructor for the WaitingRoom class used in Tonic_FD
 * @param max_size corresponding to k(alpha) in the paper
 */
Tonic_FD::WaitingRoom::WaitingRoom(long max_size) : max_size_(max_size), cur_size_(0), oldest_edge_idx_(0) {
    waiting_room_ = emhash8::HashSet<unsigned long long>(max_size);
}

/**
 * Destructor for the WaitingRoom class
 */
Tonic_FD::WaitingRoom::~WaitingRoom() { waiting_room_.clear();}

/**
 * Convert an edge (u, v) to a unique id and insert the id into the waiting room. If the maximum size is not reached,
 * increment the current size.
 * @param u
 * @param v
 */
void Tonic_FD::WaitingRoom::add_edge(int u, int v) {

    unsigned long long edge_id = (long) edge_to_wr_id(u, v);
    waiting_room_.emplace_unique(edge_id);

    if (cur_size_ < max_size_) {
        cur_size_++;
    }

}

/**
 * Pop the oldest edge from the waiting room, converts the id back to the edge, and return it
 * @return the popped oldest edge
 */
Utils::Edge Tonic_FD::WaitingRoom::pop_oldest_edge() {
    unsigned long long oldest_edge_id = *waiting_room_.begin();
    waiting_room_.erase(oldest_edge_id);
    return edge_wr_to_id(oldest_edge_id);
}

/**
 * Remove an edge (u, v) from the waiting room, used in the case of edge deletions
 * @param u
 * @param v
 * @return true if the edge was found and removed, false otherwise
 */
bool Tonic_FD::WaitingRoom::remove_edge(int u, int v) {

    unsigned long long edge_id = (long) edge_to_wr_id(u, v);
    if (waiting_room_.find(edge_id) != waiting_room_.end()) {
        waiting_room_.erase(edge_id);
        cur_size_ --;
        return true;
    }

    return false;

}


/**
 * Constructor for the Tonic_FD class.
 * @param random_seed
 * @param k memory budget
 * @param alpha
 * @param beta
 */
Tonic_FD::Tonic_FD(int random_seed, long k, double alpha, double beta) : t_(0), k_(k), alpha_(alpha),
                                                                                       beta_(beta) {

    printf("Starting Tonic Algo - alpha %.3f, beta = %.3f | Memory Budget = %ld || Random Seed = %d\n",
           alpha, beta, k, random_seed);
    WR_size_ = (long) (k_ * alpha);
    H_size_ = (long) ((k_ - WR_size_) * beta);
    SL_size_ = k_ - WR_size_ - H_size_;
    waiting_room_ = new WaitingRoom(WR_size_);
    heavy_edges_ = FixedSizePQ<Heavy_edge, heavy_edge_cmp>(H_size_);
    heavy_edges_set_ = emhash8::HashSet<unsigned long long>(H_size_);
    light_edges_sample_ = new Edge[SL_size_];
    num_edges_ = 0;
    printf("WR size = %ld, H size = %ld, SL size = %ld\n", WR_size_, H_size_, SL_size_);
    subgraph_ = emhash5::HashMap<int, emhash5::HashMap<int, bool>>(k);
    gen_ = std::mt19937(random_seed);
    dis_ = std::uniform_real_distribution<double>(0.0, 1.0);
    edge_id_to_index_ = emhash5::HashMap<long, int>(SL_size_);

}

/**
 * Destructor for the Tonic_FD class
 */
Tonic_FD::~Tonic_FD() {
    delete waiting_room_;
    delete[] light_edges_sample_;
}

/**
 * Set the edge oracle for the Tonic_FD class
 * @param edge_oracle
 */
void Tonic_FD::set_edge_oracle(emhash5::HashMap<long, int> &edge_oracle) {
    edge_id_oracle_ = edge_oracle;
    edge_oracle_flag_ = true;
}

/**
 * Set the node oracle for the Tonic_FD class
 * @param node_oracle
 */
void Tonic_FD::set_node_oracle(emhash5::HashMap<int, int> &node_oracle) {
    node_oracle_ = node_oracle;
}

/**
 * Return heaviness prediction from the node or edge oracle given the current edge (u, v)
 * @param u
 * @param v
 * @return heaviness if the edge or both nodes are found in the predictor, -1 otherwise
 */
int Tonic_FD::get_heaviness(const int u, const int v) {
    if (edge_oracle_flag_) {
        auto id_it = edge_id_oracle_.find(edge_to_id(u, v));
        if (id_it != edge_id_oracle_.end()) {
            return id_it->second;
        } else {
            return -1;
        }
    } else {
        // -- print node u
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
inline double Tonic_FD::next_double() {
    return dis_(gen_);
}

/**
 * Return the number of nodes in the subgraph
 * @return number of nodes
 */
long Tonic_FD::get_num_nodes() const {
    return (long) subgraph_.size();
}

/**
 * Return the number of edges in the subgraph
 * @return number of edges
 */
long Tonic_FD::get_num_edges() const {
    return num_edges_;
}

/**
 * Return the nodes in the subgraph
 * @param nodes to fill
 */
void Tonic_FD::get_nodes(std::vector<int> &nodes) const {
    nodes.clear();
    for (const auto &it: subgraph_) {
        nodes.push_back(it.first);
    }
}

/**
 * Return the local nodes in the subgraph
 * @param nodes to fill
 */
void Tonic_FD::get_local_nodes(std::vector<int> &nodes) const {
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
void Tonic_FD::add_edge(const int u, const int v, bool det) {

    subgraph_[u][v] = det;
    subgraph_[v][u] = det;
    num_edges_++;

}

/**
 * Function that deletes an edge (u, v) from the subgraph, for edge deletions
 * @param u
 * @param v
 * @return returns -1 if the edge is not in the subgraph, 0 if the deleted edge is not det (SL),
 * 1 if the deleted edge is det (W or H). If present, remove directly the edge from the subgraph
 */
int Tonic_FD::edge_deletion(const int u, const int v) {
    auto u_it = subgraph_.find(u);
    if (u_it != subgraph_.end()) {
        if (u_it->second.find(v) != u_it->second.end()) {
            // -- edge uv found
            bool u_det = u_it->second[v];
            num_edges_--;
            u_it->second.erase(v);
            if (u_it->second.empty()) {
                subgraph_.erase(u);
            }

            auto v_it = subgraph_.find(v);
            if (v_it != subgraph_.end()) {
                if (v_it->second.find(u) != v_it->second.end()) {
                    // -- edge vu found
                    bool v_det = v_it->second[u];
                    v_it->second.erase(u);
                    if (v_it->second.empty()) {
                        subgraph_.erase(v);
                    }

                    // -- return 1 if edge is det, 0 if edge is not det
                    assert(u_det == v_det);
                    return (u_det ? 1 : 0);

                }
            }
        }
    }

    // -- return -1: edge not found
    return -1;
}

/**
 * Function that removes an edge (u, v) from the subgraph
 * @param u
 * @param v
 * @return true if the edge was found and removed, false otherwise
 */
bool Tonic_FD::remove_edge(const int u, const int v) {
    auto u_it = subgraph_.find(u);
    if (u_it != subgraph_.end()) {
        if (u_it->second.find(v) != u_it->second.end()) {
            // -- edge uv found
            num_edges_--;
            u_it->second.erase(v);
            if (u_it->second.empty()) {
                subgraph_.erase(u);
            }

            auto v_it = subgraph_.find(v);
            if (v_it != subgraph_.end()) {
                if (v_it->second.find(u) != v_it->second.end()) {
                    // -- edge vu found
                    v_it->second.erase(u);
                    if (v_it->second.empty()) {
                        subgraph_.erase(v);
                    }

                    return true;

                }
            }
        }

    }

    return false;
}

/**
 * Function that return the current timestamp in the stream
 * @return current timestamp
 */
inline unsigned long long Tonic_FD::get_edges_processed() const {
    return t_;
}

/**
 * Function that returns the global triangle count
 * @return the global triangle count (0 if negatives)
 */
double Tonic_FD::get_global_triangles() const {
    // return maximum of 0 and the global triangle count
    return std::max(0.0, global_triangles_cnt_);
}

/**
 * Function that returns the local triangle count for a node u
 * @param u
 * @return the local triangle count for node u (0 if negative)
 */
double Tonic_FD::get_local_triangles(const int u) const {
    auto u_it = local_triangles_cnt_.find(u);
    if (u_it != local_triangles_cnt_.end()) {
        return std::max(0.0, u_it->second);
    } else {
        return 0.0;
    }
}

/**
 * Function that counts the triangles closed by the current edge (src, dst). Increments the counters if sign is +,
 * or decrements the counters if sign is -. The function is called before the edge is sampled.
 * @param src
 * @param dst
 * @param sign
 */
void Tonic_FD::count_triangles(const int src, const int dst, const int sign) {

    emhash5::HashMap<int, bool> *u_neighs, *v_neighs;
    auto u_it = subgraph_.find(src);
    if (u_it == subgraph_.end() or u_it->second.empty()) {
        return;
    }
    u_neighs = &u_it->second;
    int du = (int) u_neighs->size();

    auto v_it = subgraph_.find(dst);
    if (v_it == subgraph_.end() or v_it->second.empty()) {
        return;
    }
    v_neighs = &v_it->second;

    int dv = (int) v_neighs->size();
    int u = src;
    int v = dst;

    if (du == 0 or dv == 0) {
        return;
    }

    if (du > dv) {
        v = src;
        u = dst;
        emhash5::HashMap<int, bool> *u_neighs_tmp = u_neighs;
        u_neighs = v_neighs;
        v_neighs = u_neighs_tmp;
    }

    double cum_cnt = 0.0;

    for (const auto &it: *u_neighs) {
        int w = it.first;

        if (w == v) {
            continue;
        }

        auto vw_it = v_neighs->find(w);
        if (vw_it != v_neighs->end()) {
            // -- triangle {u, v, w} discovered
            double increment_T = 1.0;
            if ((ell_ + d_g + d_b) > SL_size_) {
                bool vw_light = !vw_it->second;
                bool wu_light = !it.second;
                if (vw_light && wu_light) {
                    // -- both edges are light
                    increment_T =
                            ((double) (ell_ + d_g + d_b) / SL_size_) * ((double) ((ell_ + d_g + d_b - 1.0))) /
                            (SL_size_ - 1.0);
                } else if (vw_light || wu_light) {
                    // -- one edge is light
                    increment_T = ((double) (ell_ + d_g + d_b) / (double) SL_size_);
                }
            }

            cum_cnt += increment_T;

        }
    }

    // -- update counters
    if (cum_cnt > 0) {
        // -- subtract counter
        if (sign < 0) {
            cum_cnt = -cum_cnt;
        }
        global_triangles_cnt_ += cum_cnt;


    }

}

/**
 * Function that samples an edge (u, v) from the stream in the correct sets W, H or SL.
 * @param u
 * @param v
 */
void Tonic_FD::sample_edge(const int u, const int v) {

    if (H_cur_ < H_size_) {
        // -- insert current edge into H
        H_cur_++;
        int current_heaviness = get_heaviness(u, v);
        heavy_edges_.push({{u, v}, current_heaviness});
        heavy_edges_set_.insert(edge_to_id(u, v));
        return;

    } else if (waiting_room_->cur_size_ < WR_size_) {

        waiting_room_->add_edge(u, v);
        return;

    } else {

        // -- waiting room and heavy edge set are full
        // -- pop oldest and insert current edge in WR
        // -- increment light edges counter
        ell_++;

        Edge oldest_edge = waiting_room_->pop_oldest_edge();

        waiting_room_->add_edge(u, v);

        Edge uv_sample = oldest_edge;

        int current_heaviness = get_heaviness(uv_sample.first, uv_sample.second);

        Heavy_edge lightest_heavy_edge;
        int lightest_heaviness;

        if (current_heaviness > -1) {

            lightest_heavy_edge = heavy_edges_.top();
            lightest_heaviness = lightest_heavy_edge.second;
            bool found_in_H = heavy_edges_set_.find(
                    edge_to_id(lightest_heavy_edge.first.first, lightest_heavy_edge.first.second)) !=
                              heavy_edges_set_.end();

            while (!found_in_H) {
                heavy_edges_.pop();
                lightest_heavy_edge = heavy_edges_.top();
                lightest_heaviness = lightest_heavy_edge.second;
                found_in_H = heavy_edges_set_.find(
                        edge_to_id(lightest_heavy_edge.first.first, lightest_heavy_edge.first.second)) !=
                             heavy_edges_set_.end();
            }

            if (current_heaviness > lightest_heaviness ||
                (current_heaviness == lightest_heaviness && next_double() < 0.5)) {
                // -- replace the lightest heavy edge with current edge
                heavy_edges_.pop();
                heavy_edges_set_.erase(edge_to_id(lightest_heavy_edge.first.first, lightest_heavy_edge.first.second));

                heavy_edges_.push({{uv_sample.first, uv_sample.second}, current_heaviness});
                heavy_edges_set_.insert(edge_to_id(uv_sample.first, uv_sample.second));

                uv_sample = lightest_heavy_edge.first;

            }
        }

        // -- here, uv_sample will be the lightest between the current and the lightest heavy edge - (u', v') in the paper
        // -- check if the number of deletions are compensated by insertions
        if (d_g + d_b == 0) {
            // -- standard reservoir sampling
            if (SL_cur_ < SL_size_) {
                edge_id_to_index_.emplace(edge_to_id(uv_sample.first, uv_sample.second), SL_cur_);
                // assert(edge_id_to_index_.size() <= SL_size_);
                light_edges_sample_[SL_cur_++] = uv_sample;
                // -- change the edge in the subgraph
                subgraph_[uv_sample.first][uv_sample.second] = false;
                subgraph_[uv_sample.second][uv_sample.first] = false;
                return;

            } else {
                // -- all sets are full -> resort to sampling
                double p = (double) (SL_size_) / (double) ell_;
                // assert(uv_sample.first < uv_sample.second);
                if (dis_(gen_) < p) {
                    // -- edge is sampled
                    subgraph_[uv_sample.first][uv_sample.second] = false;
                    subgraph_[uv_sample.second][uv_sample.first] = false;
                    // -- evict edge uniformly at random
                    // assert(SL_cur_ == SL_size_);
                    std::uniform_int_distribution<int> dis_int = std::uniform_int_distribution<int>(0,
                                                                                                    (int) SL_cur_ - 1);
                    int replace_idx = dis_int(gen_);
                    Edge uv_replace = light_edges_sample_[replace_idx];

                    if (!remove_edge(uv_replace.first, uv_replace.second)) {
                        std::cerr << "Error! Edge uv_replace (" << uv_replace.first << ", " << uv_replace.second
                                  << ") not found in subgraph || " << replace_idx << ", " << SL_cur_ << "\n";
                        exit(1);
                    } else {

                        light_edges_sample_[replace_idx] = uv_sample;
                        edge_id_to_index_.erase((long) edge_to_id(uv_replace.first, uv_replace.second));
                        edge_id_to_index_.emplace(edge_to_id(uv_sample.first, uv_sample.second), replace_idx);

                    }

                } else {

                    if (!remove_edge(uv_sample.first, uv_sample.second)) {
                        std::cerr << "Error! Edge uv_sample not found in subgraph\n";
                        exit(1);
                    }
                    edge_id_to_index_.erase((long) edge_to_id(uv_sample.first, uv_sample.second));
                }

            }

        } else {
            // -- bad and good deletions are not compensated
            double p = (double) (d_b) / (double) (d_b + d_g);
            if (next_double() < p) {
                // assert(SL_cur_ < SL_size_);
                // -- add edge to reservoir
                edge_id_to_index_.emplace(edge_to_id(uv_sample.first, uv_sample.second), SL_cur_);
                light_edges_sample_[SL_cur_++] = uv_sample;
                // -- change the edge in the subgraph
                subgraph_[uv_sample.first][uv_sample.second] = false;
                subgraph_[uv_sample.second][uv_sample.first] = false;
                d_b--;
            } else {
                remove_edge(uv_sample.first, uv_sample.second);
                edge_id_to_index_.erase((long) edge_to_id(uv_sample.first, uv_sample.second));
                d_g--;
            }
        }

        return;

    }
}

/**
 * Function that processes an edge (src, dst) at time t with sign +1 or -1. First performs the count or deletions of
 * triangles, then check if the edge is an insertion or deletions and samples or removes the edge accordingly.
 * deletions
 * @param src
 * @param dst
 * @param t
 * @param sign
 */
void Tonic_FD::process_edge(const int src, const int dst, const int t, const int sign) {

    int u = src;
    int v = dst;
    if (src > dst) {
        u = dst;
        v = src;
    }

    // assert(u < Utils::MAX_ID_NODE and v < Utils::MAX_ID_NODE);
    current_timestamp_ = t;
    t_++;

    count_triangles(u, v, sign);
    if (sign >= 0) {
        // -- edge addition
        sample_edge(u, v);
        add_edge(u, v, true);
    } else {
        // -- edge removal
        // -- check if the edge is det or not
        int deletion_status = edge_deletion(u, v);

        bool is_in_edge_index = edge_id_to_index_.find((long) edge_to_id(u, v)) != edge_id_to_index_.end();
        if (is_in_edge_index and deletion_status == -1) {
            std::cout << "Edge (" << u << ", " << v << ", time: " << current_timestamp_
                      << ") found in edge_id_to_index_ and not found in subgraph\n";
            std::cout << "Index: " << edge_id_to_index_.find((long) edge_to_id(u, v))->second << "\n";
            std::cout << "Del status: " << deletion_status << "\n";
            exit(1);
        }

        if (deletion_status == -1) {
            // -- edge not found in subgraph: good deletion
            d_g++;
            ell_--;
        } else {
            if (deletion_status == 1) {
                // -- edge is det in subgraph: either in WR or in H
                bool is_in_WR = waiting_room_->remove_edge(u, v);

                if (!is_in_WR) {
                    heavy_edges_set_.erase(edge_to_id(u, v));
                    H_cur_--;
                }

            } else {
                // -- edge is not det in subgraph
                d_b++;
                ell_--;
                int idx = edge_id_to_index_[(long) edge_to_id(u, v)];
                if (!(idx <= SL_cur_ and idx >= 0)) {
                    std::cerr << "Error: Index out of bounds | idx: " << idx << ", SL_cur: " << SL_cur_ << "\n";
                    exit(1);
                }
                Edge edge_to_swap = light_edges_sample_[--SL_cur_];
                light_edges_sample_[idx] = edge_to_swap;
                edge_id_to_index_[(long) edge_to_id(edge_to_swap.first, edge_to_swap.second)] = idx;
                edge_id_to_index_.erase((long) edge_to_id(u, v));

            }
        }

    }

}


