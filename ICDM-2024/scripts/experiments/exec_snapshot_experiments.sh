#!/bin/bash

print_usage() {
  echo -e "\n!!! Run this script inside <scripts> folder!!! \nScript usage:\n"
	echo -e "Script usage:\n"
	echo -e "\t-d: Dataset Folder for graph sequence\n"
	echo -e "\t-o: Oracle exact path (for Tonic and Chen)\n"
	echo -e "\t-w: Oracle-noWR path (for Tonic)\n"
	echo -e "\t-i: MinDegreePredictor path (for Tonic) \n"
	echo -e "\t-t: Number of Trials for each parametrization\n"
	echo -e "\t-n: Name (for output saving path)\n"
}

while getopts d:o:w:i:t:n: flag; do
	case "${flag}" in
		d) DATASET_FOLDER=${OPTARG};;
		o) ORACLE_EXACT_PATH=${OPTARG};;
		w) ORACLE_NOWR_PATH=${OPTARG};;
		i) ORACLE_MIN_DEGREE_PATH=${OPTARG};;
		t) N_TRIALS=${OPTARG};;
    n) NAME=${OPTARG};;
		*) print_usage
		   exit 1 ;;
	esac
done

END=$(($RANDOM_SEED + $N_TRIALS - 1))

mkdir output
mkdir output/SnapshotExperiments
OUTPUT=output/SnapshotExperiments/$NAME
rm -rf $OUTPUT && mkdir $OUTPUT

# if you want to seed the code, you need to change source of Wrs and Chen
RANDOM_SEED=4177
END=$(($RANDOM_SEED + $N_TRIALS - 1))

FILE_EXACT=./code/Tonic-build/RunExactAlgo
OUTPUT_PATH_EXACT=$OUTPUT/output_exact_$NAME

FILE_WRS="java -cp ./code/waiting_room/WRS-2.0.jar:./code/waiting_room/fastutil-7.2.0.jar wrs.BatchIns"
OUTPUT_PATH_WRS=$OUTPUT/output_wrs_$NAME

FILE_CHEN=./code/Chen_algorithm/code/arbitrary_order/graph.py
OUTPUT_PATH_CHEN=$OUTPUT/output_chen_$NAME

FILE_TONIC=./code/Tonic-build/Tonic
OUTPUT_PATH_TONIC=$OUTPUT/output_tonic_$NAME


# LIST ALL THE FILES IN THE DATASET DIR
# read -p 'Enter the directory path: ' $DATASET_FOLDER
for DATASET_PATH in "$DATASET_FOLDER"/*; do

	# -- file rs alpha beta dataset oracle output_path exact stats
	$FILE_EXACT 0 $DATASET_PATH $OUTPUT_PATH_EXACT

	# -- Retrieve the ground truth
	last_lines=$( tail -n 3 $OUTPUT_PATH_EXACT )
	echo "$last_lines"
	chunks=($last_lines)
	# -- sets up the (integer) memory budget
	TOTAL_M=${chunks[5]}
  PERC_K=0.1
	MEMORY_BUDGET=$(echo "$PERC_K * $TOTAL_M" | bc)
	MEMORY_BUDGET=${MEMORY_BUDGET%.*}
	echo "Total number of edges: $TOTAL_M"
	echo "Memory Budget: $MEMORY_BUDGET"

	# -- WAITING ROOM SAMPLING EXECUTION
  for r in $( seq $RANDOM_SEED $END ); do
  		$FILE_WRS $DATASET_PATH $OUTPUT_PATH_WRS $MEMORY_BUDGET 0.1
  done

  # -- CHEN ALGORITHM EXECUTION
  for r in $( seq $RANDOM_SEED $END ); do
    python $FILE_CHEN --beta 0.3 --dataset $DATASET_PATH --oracle $ORACLE_EXACT_PATH --m $TOTAL_M --k $MEMORY_BUDGET --output_path $OUTPUT_PATH_CHEN
  done

  # -- TONIC EXECUTION
  for r in $( seq $RANDOM_SEED $END ); do
    $FILE_TONIC 0 $r $MEMORY_BUDGET 0.05 0.2 $DATASET_PATH $ORACLE_EXACT_PATH edges $OUTPUT_PATH_TONIC'_exact'
    $FILE_TONIC 0 $r $MEMORY_BUDGET 0.05 0.2 $DATASET_PATH $ORACLE_NOWR_PATH edges $OUTPUT_PATH_TONIC'_no_wr'
    $FILE_TONIC 0 $r $MEMORY_BUDGET 0.05 0.2 $DATASET_PATH $ORACLE_MIN_DEGREE_PATH nodes $OUTPUT_PATH_TONIC'_min_degree'
  done

done