#!/bin/bash

# check to be inside script folder
if  [[ ! "$PWD" =~ scripts ]] ; then echo "Make sure to run this script inside <scripts> folder" && exit; fi

# check if the directory code is present
if [ ! -d "code" ]; then
    echo "Make sure to run this script after having executed download_code.sh"
    exit
fi

# -- COMPILE TONIC
echo "Compiling Tonic code..."
cd code
rm -rf Tonic-build
mkdir Tonic-build
cd Tonic-build
ls
cmake ../../..
make
cd ../../

echo "Compiling WRS code..."
# -- COMPILE WRS (if present)
cd code/waiting_room
bash package.sh && bash compile.sh
cd ../../

# -- COMPILE THINKD (if present)
echo "Compiling ThinkD code..."
cd code/thinkd
bash package.sh && bash compile.sh
cd ../../
