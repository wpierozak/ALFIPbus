#!/bin/bash

print_failure() {
    echo "Error: $1 failed. Exiting."
    exit 1
}

# Clear previous output logs
rm -f $(pwd)/test/in-container-test/common-storage/output/*-log 

# Build docker images
#docker build -f ENV.Dockerfile -t alf-ipbus-env .
#if [ $? -ne 0 ]; then
#    print_failure "alfipbus build"
#fi

docker build --no-cache -f BLDR.Dockerfile -t alf-ipbus-bldr .
if [ $? -ne 0 ]; then
    print_failure "alfipbus build"
fi

