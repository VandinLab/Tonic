#!/bin/bash

# -- CHECK IF SCRIPT IS RUN INSIDE <scripts> FOLDER
if  [[ ! "$PWD" =~ scripts ]] ; then echo "Make sure to run this script inside <scripts> folder" && exit; fi

cd code

# -- DOWNLOAD REPO WRS
echo "Downloading WRS code..."
git clone https://github.com/kijungs/waiting_room.git

# -- DOWNLOAD REPO THINKD
echo "Downloading ThinkD code..."
git clone https://github.com/kijungs/thinkd.git


