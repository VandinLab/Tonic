#include <iostream>
#include "hash_table5.hpp"
#include "Tonic.h"
#include "Tonic_FD.h"
#include "Utils.h"
#include <fstream>
#include <string>
#include <chrono>

/**
 * Read stream and perform the Tonic algorithm for insertion only streams
 * @param dataset_path
 * @param algo the instantiated Tonic algorithm class
 */
void run_tonic_algo(std::string &dataset_path, Tonic &algo) {

    std::ifstream file(dataset_path);
    std::string line;
    long n_line = 0;
    int u, v, t;

    std::string oracle_type_str = algo.edge_oracle_flag_ ? "Edges" : "Nodes";

    if (file.is_open()) {
        while (true) {
            if (!std::getline(file, line)) break;
            std::istringstream iss(line);
            std::string token;
            std::getline(iss, token, ' ');
            u = std::stoi(token);
            std::getline(iss, token, ' ');
            v = std::stoi(token);
            std::getline(iss, token, ' ');
            t = std::stoi(token);
            algo.process_edge(u, v);
            if (++n_line % 5000000 == 0) {
                printf("Processed %ld edges || Estimated count T = %f\n", n_line, algo.get_global_triangles());
            }

        }
        file.close();
    } else {
        std::cerr << "Error! Unable to open file " << dataset_path << "\n";
    }


}

/**
 * Read stream and perform the Tonic FD algorithm for fully dynamic streams
 * @param dataset_path
 * @param algo the instantiated Tonic FD algorithm class
 */
void run_tonic_algo_FD(std::string &dataset_path, Tonic_FD &algo) {

    std::ifstream file(dataset_path);
    std::string line;
    long n_line = 0;
    int u, v, t, sign;
    char sign_char;
    std::string oracle_type_str = algo.edge_oracle_flag_ ? "Edges" : "Nodes";


    if (file.is_open()) {
        while (true) {
            if (!std::getline(file, line)) break;
            std::istringstream iss(line);
            std::string token;
            std::getline(iss, token, ' ');
            u = std::stoi(token);
            std::getline(iss, token, ' ');
            v = std::stoi(token);
            std::getline(iss, token, ' ');
            t = std::stoi(token);
            std::getline(iss, token, ' ');
            sign_char = token[0];
            // -- by default, assume additions
            sign = sign_char == '-' ? -1 : 1;

            algo.process_edge(u, v, t, sign);
            if (++n_line % 5000000 == 0) {
                printf("Processed %ld edges || Estimated count T = %f\n", n_line, algo.get_global_triangles());
            }

        }

        file.close();

    } else {
        std::cerr << "Error! Unable to open file " << dataset_path << "\n";
    }

}

/**
 * Write results to a csv file
 * @param name of the algorithm
 * @param estimated_T estimates of global triangles
 * @param time taken by the algorithm
 * @param output_path output file where to write results
 * @param edge_oracle_flag
 * @param alpha
 * @param beta
 * @param memory_budget
 * @param size_oracle size of the oracle in memory
 * @param time_oracle time to read the oracle
 */
void write_results(std::string name, double estimated_T, double time, std::string& output_path, bool edge_oracle_flag,
                    double alpha, double beta, long memory_budget, int size_oracle,
                    double time_oracle) {
    printf("%s Algo successfully run in time %.3f! Estimated count T = %f\n", name.c_str(), time, estimated_T);
    // -- write results
    // -- global estimates
    std::ofstream out_file(output_path + "_global_count.csv", std::ios::app);
    std::string oracle_type_str = edge_oracle_flag ? "Edges" : "Nodes";

    out_file << "Algo,Params,Oracle,SizeOracle,TimeOracle,MemEdges,GlobalTriangleCount,Time\n";
    out_file << name.c_str() << ",Alpha=" << alpha << "-Beta=" << beta << "," << oracle_type_str << "," << size_oracle
             << "," << time_oracle << "," << memory_budget << "," << std::fixed << estimated_T << "," << time << "\n";

    out_file.close();

}

/**
 * Get the base name of the executable
 * @param s the string to split
 * @return the name of the executable
 */
char *base_name(char *s)
{
    char *start;

    /* Find the last '/', and move past it if there is one.  Otherwise, return
       a copy of the whole string. */
    /* strrchr() finds the last place where the given character is in a given
       string.  Returns NULL if not found. */
    if ((start = strrchr(s, '/')) == nullptr) {
        start = s;
    } else {
        ++start;
    }
    /* If you don't want to do anything interesting with the returned value,
       i.e., if you just want to print it for example, you can just return
       'start' here (and then you don't need dup_str(), or to free
       the result). */
    return start;
}

int main(int argc, char **argv) {

    char* project = base_name(argv[0]);

    // -- data preprocessing
    if (strcmp(project, "DataPreprocessing") == 0) {
        if (argc != 5) {
            std::cerr << "Usage: DataPreprocessing <dataset_path> <delimiter> <skip>"
                         " <output_path>\n";
            return 1;
        } else {
            std::string dataset_path(argv[1]);
            std::string delimiter (argv[2]);
            int skip = atoi(argv[3]);
            std::string output_path(argv[4]);
            auto start = std::chrono::high_resolution_clock::now();
            Utils::preprocess_data(dataset_path, delimiter, skip, output_path);
            auto stop = std::chrono::high_resolution_clock::now();
            double time = (double) ((std::chrono::duration_cast<std::chrono::milliseconds>(stop - start)).count()) / 1000;
            std::cout << "Dataset preprocessed in time: " << time << " s\n";
            return 0;
        }
    }

    // -- run exact
    if (strcmp(project, "RunExactAlgo") == 0) {
        if (argc != 4) {
            std::cerr << "Usage: RunExactAlgo <flag: 0: insertion-only stream, 1: fully-dynamic stream>"
                         " <preprocessed_dataset_path> <output_path>\n";
            return 1;
        } else {
            int flag_fd = atoi(argv[1]);
            assert(flag_fd == 0 or flag_fd == 1);
            std::string dataset_path(argv[2]);
            std::string output_path(argv[3]);
            auto start = std::chrono::high_resolution_clock::now();
            long total_T;
            if (flag_fd == 1)
                total_T = Utils::run_exact_algorithm_FD(dataset_path, output_path);
            else
                total_T = Utils::run_exact_algorithm(dataset_path, output_path);

            auto stop = std::chrono::high_resolution_clock::now();
            double time = (double) ((std::chrono::duration_cast<std::chrono::milliseconds>(stop - start)).count()) / 1000;
            printf("Exact Algorithm successfully run in time %.3f! Total count T = %ld\n", time, total_T);
            return 0;
        }
    }

    // -- build oracle
    if (strcmp(project, "BuildOracle") == 0) {
        if (argc < 5 or argc > 6) {
            std::cerr << "Usage: BuildOracle <preprocessed_dataset_path> <type = [Exact, noWR, Node]>, <percentage_retain>,"
                         " <output_path>, [<wr_size>] \n";
            return 1;
        } else {
            std::string dataset_path(argv[1]);
            std::string type_oracle(argv[2]);
            double percentage_retain = atof(argv[3]);
            std::string output_path(argv[4]);
            auto start = std::chrono::high_resolution_clock::now();
            if (strcmp(type_oracle.c_str(), "Exact") == 0) {
                Utils::build_edge_exact_oracle(dataset_path, percentage_retain, output_path);
                auto stop = std::chrono::high_resolution_clock::now();
                double time = (double) ((std::chrono::duration_cast<std::chrono::milliseconds>(stop - start)).count()) / 1000;
                printf("Exact Edge Oracle successfully run in time %.3f!\n", time);

            } else if(strcmp(type_oracle.c_str(), "noWR") == 0) {
                int wr_size = atoi(argv[5]);
                Utils::build_edge_exact_nowr_oracle(dataset_path, percentage_retain, output_path, wr_size);
                auto stop = std::chrono::high_resolution_clock::now();
                double time = (double) ((std::chrono::duration_cast<std::chrono::milliseconds>(stop - start)).count()) / 1000;
                printf("Exact-noWR Edge Oracle successfully run in time %.3f!\n", time);
            } else if (strcmp(type_oracle.c_str(), "Node") == 0) {
                    Utils::build_node_oracle(dataset_path, percentage_retain, output_path);
                    auto stop = std::chrono::high_resolution_clock::now();
                    double time = (double) ((std::chrono::duration_cast<std::chrono::milliseconds>(stop - start)).count()) / 1000;
                    printf("Node Map successfully run in time %.3f!\n", time);

                } else {
                std::cerr << "Build Oracle - Error! Type of Oracle must be Exact or Node.\n";
                return 1;
            }

            return 0;
        }
    }

    // -- create FD stream
    if (strcmp(project, "CreateFDStream") == 0) {
        if (argc != 6) {
            std::cerr << "Usage: CreateFDStream <snapshots_folder> <n_snapshots> <delimiter> <skip>"
                         " <output_path>\n";
            return 1;
        } else {
            std::string snapshots_folder(argv[1]);
            int n_snapshots = atoi(argv[2]);
            std::string delimiter (argv[3]);
            int skip = atoi(argv[4]);
            std::string output_path(argv[5]);
            auto start = std::chrono::high_resolution_clock::now();
            Utils::merge_snapshots_FD(snapshots_folder, n_snapshots, delimiter, skip, output_path);
            auto stop = std::chrono::high_resolution_clock::now();
            double time = (double) ((std::chrono::duration_cast<std::chrono::milliseconds>(stop - start)).count()) / 1000;
            std::cout << "Snapshots folder " << snapshots_folder << " merged in time: " << time << " s\n";
            return 0;
        }
    }

    // -- Tonic Algo
    if (strcmp(project, "Tonic") == 0) {
        if (argc != 10) {
            std::cerr << "Usage: Tonic <flag: 0: insertion-only stream, 1: fully-dynamic stream>"
                         " <random_seed> <memory_budget> <alpha> <beta> "
                         "<dataset_path> <oracle_path> <oracle_type = [nodes, edges]> <output_path>\n";
            return 1;
        }

        // -- read arguments
        int flag_fd = atoi(argv[1]);
        assert(flag_fd == 0 or flag_fd == 1);
        int random_seed = atoi(argv[2]);
        long memory_budget = atol(argv[3]);
        double alpha = atof(argv[4]);
        double beta = atof(argv[5]);
        // -- assert alpha, beta in (0, 1)
        if (alpha <= 0 or alpha >= 1 or beta <= 0 or beta >= 1) {
            std::cerr << "Error! Alpha and Beta must be in (0, 1)\n";
            return 1;
        }

        std::string dataset_path(argv[6]);
        std::string oracle_path(argv[7]);
        std::string oracle_type(argv[8]);
        std::string output_path(argv[9]);

        std::chrono::time_point start = std::chrono::high_resolution_clock::now();
        double time, time_oracle;
        bool edge_oracle_flag = false;
        int size_oracle;
        emhash5::HashMap<int, int> node_oracle;
        emhash5::HashMap<long, int> edge_oracle;
        if (oracle_type == "nodes") {
            if (!Utils::read_node_oracle(oracle_path, ' ', 0, node_oracle)) return 1;
            time_oracle = (double) ((std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::high_resolution_clock::now() - start)).count()) / 1000;
            printf("Node Oracle successfully read in time %.3f! Size of the oracle = %d nodes\n",
                   time_oracle, node_oracle.size());
            size_oracle = (int) node_oracle.size();
        } else if (oracle_type == "edges") {
            edge_oracle_flag = true;
            if (!Utils::read_edge_oracle(oracle_path, ' ', 0, edge_oracle)) return 1;
            time_oracle = (double) ((std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::high_resolution_clock::now() - start)).count()) / 1000;
            printf("Edge Oracle successfully read in time %.3f! Size of the oracle = %d edges\n",
                   time_oracle, edge_oracle.size());
            size_oracle = (int) edge_oracle.size();
        } else {
            std::cerr << "Error! Oracle type must be nodes or edges\n";
            return 1;
        }
        if (flag_fd == 1) {
            Tonic_FD tonic_FD_algo(random_seed, memory_budget, alpha, beta);
            if (edge_oracle_flag)
                tonic_FD_algo.set_edge_oracle(edge_oracle);
            else
                tonic_FD_algo.set_node_oracle(node_oracle);

            start = std::chrono::high_resolution_clock::now();
            run_tonic_algo_FD(dataset_path, tonic_FD_algo);
            time = (double) ((std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::high_resolution_clock::now() - start)).count()) / 1000;

            write_results(std::string("TonicFD"), tonic_FD_algo.get_global_triangles(), time,
                          output_path, edge_oracle_flag, alpha, beta, memory_budget, size_oracle, time_oracle);


        } else {
            Tonic tonic_algo(random_seed, memory_budget, alpha, beta);
            if (edge_oracle_flag)
                tonic_algo.set_edge_oracle(edge_oracle);
            else
                tonic_algo.set_node_oracle(node_oracle);

            start = std::chrono::high_resolution_clock::now();
            run_tonic_algo(dataset_path, tonic_algo);
            time = (double) ((std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::high_resolution_clock::now() - start)).count()) / 1000;

            write_results(std::string("TonicINS"), tonic_algo.get_global_triangles(), time,
                          output_path, edge_oracle_flag, alpha, beta, memory_budget, size_oracle, time_oracle);

        }
        std::cout << "Done!\n";
        return 0;
    }

    return 1;

}


