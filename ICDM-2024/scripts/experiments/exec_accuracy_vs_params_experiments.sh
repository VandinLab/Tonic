#!/bin/bash

print_usage() {
	echo -e "\n!!! Run this script inside <scripts> folder!!! \nScript usage:\n"
	echo -e "\t-d: Dataset path\n"
	echo -e "\t-o: Oracle exact path (for Tonic and Chen)\n"
	echo -e "\t-w: Oracle-noWR path (for Tonic)\n"
	echo -e "\t-i: MinDegreePredictor path (for Tonic) \n"
	echo -e "\t-k: Memory Budget\n"
	echo -e "\t-m: Total Number of Edges of Dataset (for Chen)\n"
	echo -e "\t-t: Number of Trials for each parametrization\n"
	echo -e "\t-n: Name (for output saving path)\n"
}

while getopts d:o:w:i:k:m:t:n: flag; do
	case "${flag}" in
		d) DATASET_PATH=${OPTARG};;
		o) ORACLE_EXACT_PATH=${OPTARG};;
		w) ORACLE_NOWR_PATH=${OPTARG};;
		i) ORACLE_MIN_DEGREE_PATH=${OPTARG};;
		k) MEMORY_BUDGET=${OPTARG};;
		m) TOTAL_EDGES=${OPTARG};;
		t) N_TRIALS=${OPTARG};;
		n) NAME=${OPTARG};;
		*) print_usage
		   exit 1 ;;
	esac
done

echo "Running Accuracy vs Params Experiments for $DATASET_PATH | Memory Budget=$MEMORY_BUDGET, Total Edges=$TOTAL_EDGES"


mkdir output
mkdir output/AccuracyVsParams
OUTPUT=output/AccuracyVsParams/$NAME
rm -rf $OUTPUT && mkdir $OUTPUT

# if you want to seed the code, you need to change source of Wrs and Chen
RANDOM_SEED=4177
END=$(($RANDOM_SEED + $N_TRIALS - 1))

echo "Runnnig WRS with alpha=[0.1, 0.15, 0.19, 0.23, 0.24, 0.28, 0.3, 0.36]"
# -- WAITING ROOM SAMPLING EXECUTION
FILE_WRS="java -cp ./code/waiting_room/WRS-2.0.jar:./code/waiting_room/fastutil-7.2.0.jar wrs.BatchIns"
OUTPUT_PATH_WRS=$OUTPUT/output_wrs_$NAME

for ALPHA in 0.1 0.15 0.19 0.23 0.24 0.28 0.3 0.36
do
	for r in $( seq $RANDOM_SEED $END )
	do
		$FILE_WRS $DATASET_PATH $OUTPUT_PATH_WRS $MEMORY_BUDGET $ALPHA
	done
done

# -- CHEN ALGORITHM EXECUTION
FILE_CHEN=./code/Chen_algorithm/code/arbitrary_order/graph.py
OUTPUT_PATH_CHEN=$OUTPUT/output_chen_$NAME


for BETA in 0.1 0.15 0.19 0.23 0.24 0.28 0.3 0.36
do
	for r in $( seq $RANDOM_SEED $END )
	do
		echo Chen
		python $FILE_CHEN --beta $BETA --dataset $DATASET_PATH --oracle $ORACLE_EXACT_PATH --m $TOTAL_EDGES --k $MEMORY_BUDGET --output_path $OUTPUT_PATH_CHEN
	done
done


# -- TONIC EXECUTION
# -- select which implementation
FILE_TONIC=./code/Tonic-build/Tonic
OUTPUT_PATH_TONIC=$OUTPUT/output_tonic_$NAME

for ALPHA in 0.05 0.1 0.15 0.2 
do
	for BETA in 0.05 0.1 0.15 0.2
	do
		for r in $( seq $RANDOM_SEED $END )
		do
		        $FILE_TONIC 0 $r $MEMORY_BUDGET $ALPHA $BETA $DATASET_PATH $ORACLE_EXACT_PATH edges $OUTPUT_PATH_TONIC'_exact'
		        $FILE_TONIC 0 $r $MEMORY_BUDGET $ALPHA $BETA $DATASET_PATH $ORACLE_NOWR_PATH edges $OUTPUT_PATH_TONIC'_no_wr'
		        $FILE_TONIC 0 $r $MEMORY_BUDGET $ALPHA $BETA $DATASET_PATH $ORACLE_MIN_DEGREE_PATH nodes $OUTPUT_PATH_TONIC'_min_degree'
		
		done
	done
done
