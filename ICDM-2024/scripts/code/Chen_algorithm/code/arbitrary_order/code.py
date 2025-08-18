import time
import random
from queue import PriorityQueue
import numpy as np


##################### Auxilliary functions #####################

#  function for calculate the triangles per edge
def tri_by_edges_calculator(file, delimiter = ' ', skip = 0):
	nodes = {}             # dictionary where nodes[v] is the set of neighbors of v
	tri_by_edges = {}      # dictionary where tri_by_edges[(u,v)] is the number of triangles on edge (u,v) with u < v
	start = time.time()
	i = 0

	# open file and read line by line
	with open(file) as infile:
		for line in infile:

			# IMPORTANT: need to change this line to line.split(',') etc depending on file format
			chunks = line.split(delimiter)

			v1 = int(chunks[0])
			v2 = int(chunks[1])
			
			 # check if edge is already present
			if (v2 in nodes.get(v1, {})) and (v1 in nodes.get(v2, {})):
				continue

			# update neighbors
			if v1 != v2:


				if v1 in nodes:
					nodes[v1].add(v2)
				else:
					nodes[v1] = {v2}

				if v2 in nodes:
					nodes[v2].add(v1)
				else:
					nodes[v2] = {v1}

				# get common neighbors to get triangles by edge for new ege
				common_neighbors = nodes[v1].intersection(nodes[v2])

				for u in common_neighbors:
					if u < v1:
						tri_by_edges[(u,v1)] += 1
					else:
						tri_by_edges[(v1,u)] += 1
					if u < v2:
						tri_by_edges[(u,v2)] += 1
					else:
						tri_by_edges[(v2,u)] += 1   

				if v1 < v2:
					tri_by_edges[(v1,v2)] = len(common_neighbors)
				else:
					tri_by_edges[(v2,v1)] = len(common_neighbors) 
					
	
	return tri_by_edges


def read_oracle(file, delimiter = ' ', skip = 0):

        print('Reading Oracle...')
        oracle = {}

        # open file and read line by line
        with open(file) as infile:
                for line in infile:

                        # IMPORTANT: need to change this line to line.split(',') etc depending on file format
                        chunks = line.split(delimiter)
                        v1 = int(chunks[0])
                        v2 = int(chunks[1])
                        feature = int(chunks[2])
                        u = min(v1, v2)
                        v = max(v1, v2)
                        oracle[(u, v)] = feature

        print('Done!')
        return oracle


def sample_naive_faster(file, p, delimeter=' ', skip = 0):
	
	subgraph = {} 
	
	tricount = 0
	
	with open(file) as infile:
		for k in range(skip):
			next(infile)
			
		
		for line in infile:
			
			chunks = line.split(delimeter)

			v1 = int(chunks[0])
			v2 = int(chunks[1])
			
			 # check if edge is already present
			if (v2 in subgraph.get(v1, {})) and (v1 in subgraph.get(v2, {})):
				continue
			
			if (v1 in subgraph) and (v2 in subgraph):
				tricount += len(subgraph[v1].intersection(subgraph[v2]))
			
			if np.random.rand() < p:
				
				if v1 in subgraph:
					subgraph[v1].add(v2)
				else:
					subgraph[v1] = {v2}              
				if v2 in subgraph:
					subgraph[v2].add(v1)
				else:
					subgraph[v2] = {v1}
	return tricount / (p**2), sum(len(neigh) for neigh in subgraph.values())/2


	

def update_subgraph(subgraph, v1, v2):
	if v1 in subgraph:
		subgraph[v1].add(v2)
	else:
		subgraph[v1] = {v2}              
	if v2 in subgraph:
		subgraph[v2].add(v1)
	else:
		subgraph[v2] = {v1}

##################### Main algo of paper #####################
def oracle_with_replacement_topk(file, p, space_limit, heavy_space, oracle, delimeter=' ', evict_light=True):

	# keep track of space used - fill till threshold is hit
	space_used = 0

	# count different types of triangles
	l1 = 0
	l2 = 0
	l3 = 0

	# subgraph of sampled edges    
	subgraph = {} 

	# set of heavy edges
	heavy_edges = set()
	heavy_queue = PriorityQueue()

	# set to tell if an edge is "early", i.e., among first space_limit # of edges
	early_edges = set()

	# set of sampled light edges
	light_edges = set()

	with open(file) as infile:
		for line in infile:
	#             print(space_used, len(heavy_edges), len(early_edges), len(light_edges))

			chunks = line.split(delimeter)

			v1 = int(chunks[0])
			v2 = int(chunks[1])

			if v1 == v2:
				continue

			# current edge
			edge = tuple(sorted((v1, v2)))


			#####################     
			# counting triangles
			#####################
			if (v1 in subgraph) and (v2 in subgraph):
				wedge_nodes = subgraph[v1].intersection(subgraph[v2])

				# get common neighbors of v1 and v2
				for node in wedge_nodes:

					key0 = tuple(sorted((node, v1)))
					key1 = tuple(sorted((node, v2)))

					# keep track of # of neighboring edges that are early or heavy since they are kept always
					n_deterministic = 0
					if key0 in early_edges or key0 in heavy_edges:
						n_deterministic += 1
					if key1 in early_edges or key1 in heavy_edges:
						n_deterministic += 1

					# if other 2 edges are both heavy/early no weighing needed
					if n_deterministic == 2:
						l3 += 1
					# if only one is heavy/early, then the other must be late light so divide by p
					elif n_deterministic == 1:
						l2 += 1
					# else both edges are late light edges so divide by p^2
					else:
						l1 += 1

			###############     
			# adding edges
			###############

			# First, add to heavy_edges if heavy_edges isn't full
			if len(heavy_edges) < heavy_space:
				update_subgraph(subgraph, v1, v2)
				heavy_edges.add(edge)
				pred_triangles = 0
				if edge in oracle:
					pred_triangles = oracle[edge]
				heavy_queue.put((pred_triangles,edge))
				space_used += 1
				continue

			# Add to heavy_edges if it is heavier than min element
			smallest_elem = heavy_queue.get()
			if edge in oracle and oracle[edge] > smallest_elem[0]:
				# Add new heavy edge
				update_subgraph(subgraph, v1,v2)
				heavy_edges.add(edge)
				heavy_queue.put((oracle[edge],edge))
				# Remove old edge (early_light -> light -> remove)
				old_edge = smallest_elem[1]
				heavy_edges.remove(old_edge)
				if space_used < space_limit:
					early_edges.add(old_edge)
					space_used += 1
				elif np.random.rand() < p:
					# need to evict an edge (early -> light)
					if len(early_edges) > 0:
						evicted = False
						while (len(early_edges) > 0) and (not evicted):
							edge_to_evict = early_edges.pop()
							# keep as late edge w.p. p
							if np.random.rand() < p:
								light_edges.add(edge_to_evict)
							# evict
							else:
								w1, w2 = edge_to_evict
								subgraph[w1].remove(w2)
								subgraph[w2].remove(w1)
								evicted = True
					else:
						if evict_light:
							edge_to_evict = light_edges.pop()
							w1, w2 = edge_to_evict
							subgraph[w1].remove(w2)
							subgraph[w2].remove(w1)
						else: # Evicting light edges, isn't allowed, just increase the space
							space_used += 1
					# add the new light edge
					light_edges.add(old_edge)
				else:
					w1, w2 = old_edge
					subgraph[w1].remove(w2)
					subgraph[w2].remove(w1)
				continue
			else:
				heavy_queue.put(smallest_elem) #put the element back

			# Next, keep edge as early is space hasn't been filled
			if space_used < space_limit:
				update_subgraph(subgraph, v1, v2)
				early_edges.add(edge)
				space_used += 1
				continue

			# Finally, keep edge as light edge w.p. p (have to evict early edge)
			elif np.random.rand() < p:
				# need to evict an edge (early -> light)
				if len(early_edges) > 0:
					evicted = False
					while (len(early_edges) > 0) and (not evicted):
						edge_to_evict = early_edges.pop()
						# keep as late edge w.p. p
						if np.random.rand() < p:
							light_edges.add(edge_to_evict)
						# evict
						else:
							w1, w2 = edge_to_evict
							subgraph[w1].remove(w2)
							subgraph[w2].remove(w1)
							evicted = True
				else:
					if evict_light:
						edge_to_evict = light_edges.pop()
						w1, w2 = edge_to_evict
						subgraph[w1].remove(w2)
						subgraph[w2].remove(w1)
					else: # Evicting light edges, isn't allowed, just increase the space
						space_used += 1

				# add the new light edge
				update_subgraph(subgraph, v1, v2)
				light_edges.add(edge)

	return l1 / (p**2) + l2 / p + l3, (l1, l2, l3)
