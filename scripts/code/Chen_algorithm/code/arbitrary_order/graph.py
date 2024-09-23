#### Example for oregon dataset ###

from code import *
import os
import argparse
import numpy as np
import time

if __name__ == '__main__':
    # get oracle using graph 1 -> run on graph 2
    parser = argparse.ArgumentParser()
    parser.add_argument('--dataset', type=str, help='Filepath of dataset to be preprocessed')
    parser.add_argument('--oracle', type=str, help='Filepath of the oracle to read')
    parser.add_argument('--m', type=int, help='True number of edges in the stream')
    parser.add_argument('--k', type=int, help='Memory budget')
    parser.add_argument('--beta', type=float, help='Beta param')
    parser.add_argument('--output_path', type=str, help='output path for writing results')

    args = parser.parse_args()

    start = time.time()
    tri_by_edges_oracle = read_oracle(args.oracle)
    time_read_oracle = time.time() - start
    oracle_size = len(tri_by_edges_oracle)
    print(f'Oracle successfully read in {time_read_oracle} | Oracle Size = {oracle_size}!')
    true_m = args.m  # true # of edges

    # set space
    space = args.k
    print('Space:', space)
    frac = args.beta  # .3 fraction of space for heavy edges
    heavy_space = int(frac * space)
    file = args.dataset
    p_topk = (space - heavy_space) / (true_m - heavy_space)  # set sampling prob in our alg

    out_path = args.output_path

    # -- write on file for plots
    out_file = open(out_path + "_global_count.csv", 'a')
    print(f'Starting Chen algorithm for counting triangles with beta = {args.beta}')
    start = time.time()
    triangle_estimate, (l1, l2, l3) = oracle_with_replacement_topk(file, p_topk, space, heavy_space, tri_by_edges_oracle)
    total_time = time.time() - start
    print(f'Triangle estimate: {triangle_estimate}, in time: {total_time}\n')
    
    out_file.write(f'Chen_algo,Beta={args.beta},Edges,{oracle_size},{time_read_oracle},{space},{triangle_estimate},'
                   f'{total_time}\n')
