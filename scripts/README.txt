Instructions for reproducing experiments of paper:
"Fast and Accurate Triangle Counting in Graph Streams Using Predictions"

1 - exec the script "download_code.sh" to download the code of the algorithms used in the experiments;
    NOTE: the code for Chen is already inside the "code" folder, since the original code does not take arguments from the command line, we modified it to do so. The original code can be found at:
    "https://openreview.net/attachment?id=8in_5gN9I0&name=supplementary_material"

2 - exec the script "compile.sh" to compile the code (if necessary). This script will also bring the binaries of Tonic inside the "code" folder;

3 - follow the instructions in the README.md file for downloading the datasets, preprocessing them, building the oracles, and running the experiments.
    NOTE: you may need to change delimiter on:
    * WRS (BatchIns.java and BatchDel.java, line 51),
    * ThinkD (BatchAcc.java, line 47), which uses tab as default delimiter, while we preprocess the datasets using space as delimiter.
    * For fully-dynamic streams, Tonic read rows as (u v t sign), while ThinkD and WRSDel read rows as (u v sign). You need to change the code to read the correct format. The sign is also interpreted differently (as char).

4 - after having set up all the environments correctly, you can run the experiments by inside the "experiments" folder.
    You might run each experiment individually by running the scripts inside the "experiments" folder. We provided bash sources for running experiments for ranges of parameters (Fig. 1 of the paper), for ranges of memory budget allowed (Fig. 2 of the paper), and for snapshot sequences of input graph streams (Fig. 4 of the paper).
    NOTE: if you want to save results for several trials, you need to change the code for WRS and ThinkD allowing to append results to the output file.
