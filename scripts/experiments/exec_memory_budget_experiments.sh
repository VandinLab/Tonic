#!/bin/bash

#!/bin/bash

print_usage() {
	echo -e "\n!!! Run this script inside <scripts> folder!!! \nScript usage:\n"
	echo -e "\t-f: Flag for FD streams (1 runs FD and accounts for ThinkD and WRSdel, 0 runs insertion-only and accounts for WRSins and Chen)\n"
	echo -e "\t-d: Dataset path\n"
	echo -e "\t-o: Oracle exact path (for Tonic and Chen)\n"
	echo -e "\t-w: Oracle-noWR path (for Tonic)\n"
	echo -e "\t-i: MinDegreePredictor path (for Tonic) \n"
	echo -e "\t-k: Memory Budget of start (Sequence: [k, 3k, 5k, 10k])\n"
	echo -e "\t-m: Total Number of Edges of Dataset (for Chen)\n"
	echo -e "\t-t: Number of Trials for each parametrization\n"
	echo -e "\t-n: Name (for output saving path)\n"

}

while getopts f:d:o:w:i:k:m:t:n: flag; do
	case "${flag}" in
	  f) FD_FLAG=${OPTARG};;
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

# check if FD flag is equal to 1 or 0
if [ $FD_FLAG -eq 1 ]
then
  echo "Running fully-dynamic Streams Experiments for $DATASET_PATH"
else
  echo "Running insertion-only Streams Experiments for $DATASET_PATH"
fi

MEMORY_SEQ=($(( MEMORY_BUDGET )) $(( 3*MEMORY_BUDGET )) $(( 5*MEMORY_BUDGET )) $(( 10*MEMORY_BUDGET )) )
echo "Memory Budget Sequence: "
for i in "${MEMORY_SEQ[@]}"; do
  echo "$i"
done

mkdir output
mkdir output/MemoryBudget
OUTPUT=output/MemoryBudget/$NAME
rm -rf $OUTPUT && mkdir $OUTPUT

# if you want to seed the code, you need to change source of Wrs and Chen
RANDOM_SEED=4177
END=$(($RANDOM_SEED + $N_TRIALS - 1))


if [ $FD_FLAG -eq 0 ]
then
  FILE_WRS="java -cp ./code/waiting_room/WRS-2.0.jar:./code/waiting_room/fastutil-7.2.0.jar wrs.BatchIns"
  OUTPUT_PATH_WRS=$OUTPUT/output_wrs_$NAME'_ins'
else
  FILE_WRS="java -cp ./code/waiting_room/WRS-2.0.jar:./code/waiting_room/fastutil-7.2.0.jar wrs.BatchDel"
  OUTPUT_PATH_WRS=$OUTPUT/output_wrs_$NAME'_del'
fi


for current_budget in "${MEMORY_SEQ[@]}"
do
	echo "Current Memory Budget: $current_budget"
  for r in $( seq $RANDOM_SEED $END )
  do
    echo "Wrs with Memory Budget: $current_budget | Alpha: 0.1| Random Seed: $r"
    $FILE_WRS $DATASET_PATH $OUTPUT_PATH_WRS $current_budget 0.1
  done
done

if [ $FD_FLAG -eq 0 ]
then
  # -- CHEN ALGORITHM EXECUTION
  FILE_CHEN=./code/Chen_algorithm/code/arbitrary_order/graph.py
  OUTPUT_PATH_CHEN=$OUTPUT/output_chen_$NAME
  for current_budget in "${MEMORY_SEQ[@]}"; do
    for r in $( seq $RANDOM_SEED $END )
    do
      python $FILE_CHEN --beta 0.3 --dataset $DATASET_PATH --oracle $ORACLE_EXACT_PATH --m $TOTAL_EDGES --k $current_budget --output_path $OUTPUT_PATH_CHEN
    done
  done
else
  # ThinkD Acc
  FILE_THINKD="java -cp ./code/thinkd/ThinkD-2.0.jar:./code/thinkd/fastutil-7.2.0.jar thinkd.BatchAcc"
  OUTPUT_PATH_THINKD=$OUTPUT/output_thinkd_$NAME
  for current_budget in "${MEMORY_SEQ[@]}"; do
    for r in $( seq $RANDOM_SEED $END )
    do
      $FILE_THINKD $DATASET_PATH $OUTPUT_PATH_THINKD $MEMORY_BUDGET 1
    done
  done
fi

# -- TONIC EXECUTION
# -- select which implementation
FILE_TONIC=./code/Tonic-build/Tonic
OUTPUT_PATH_TONIC=$OUTPUT/output_tonic_$NAME

for current_budget in "${MEMORY_SEQ[@]}"; do
  for r in $( seq $RANDOM_SEED $END )
  do
    # proposed parametrization: alpha=0.05, beta=0.2
    $FILE_TONIC $FD_FLAG $r $current_budget 0.05 0.2 $DATASET_PATH $ORACLE_EXACT_PATH edges $OUTPUT_PATH_TONIC'_exact'
    $FILE_TONIC $FD_FLAG $r $current_budget 0.05 0.2 $DATASET_PATH $ORACLE_NOWR_PATH edges $OUTPUT_PATH_TONIC'_no_wr'
    $FILE_TONIC $FD_FLAG $r $current_budget 0.05 0.2 $DATASET_PATH $ORACLE_MIN_DEGREE_PATH nodes $OUTPUT_PATH_TONIC'_min_degree'
  done
done