#!/bin/bash

print_failure() {
    echo "Error: $1 failed. Exiting."
    exit 1
}

docker build --no-cache -f BLDR.Dockerfile -t alf-ipbus-bldr .
if [ $? -ne 0 ]; then
    print_failure "alfipbus build"
fi

docker build --no-cache -f  $(pwd)/test/in-container-test/alf-run.Dockerfile -t alfipbus .
if [ $? -ne 0 ]; then
    print_failure "alfipbus build"
fi

docker build -f ./test/in-container-test/tester.Dockerfile -t alfipbus-tester .
if [ $? -ne 0 ]; then
    print_failure "alfipbus-tester build"
fi