#!/bin/bash

# Set DIM_HOST_NODE environment variable
export DIM_HOST_NODE=172.25.75.12

# Prepare Docker network
docker network inspect alfipbus-tester-network >/dev/null 2>&1 || \
docker network create --driver=bridge --subnet=172.25.75.0/24 alfipbus-tester-network

# Run Mock container
docker run -i -d --name tester-mock --rm \
    --mount type=bind,source=$(pwd)/test/in-container-test/common-storage,target=/common-storage \
    --network alfipbus-tester-network \
    --add-host host.docker.internal:host-gateway \
    --ip 172.25.75.10 alfipbus-tester

docker exec -d -e LD_LIBRARY_PATH="/usr/lib" tester-mock /bin/bash -c \
    "/alf-ipbus-tester/build/bin/mock -c /common-storage/test-configuration.toml -f /common-storage/output/mock-log -v"

# Run ALF container
docker run -i -d --name tester-alf --rm \
    --mount type=bind,source=$(pwd)/test/in-container-test/common-storage,target=/common-storage \
    --network alfipbus-tester-network \
    --ip 172.25.75.12 \
    --add-host host.docker.internal:host-gateway alfipbus

docker exec -d -e LD_LIBRARY_PATH="/usr/lib" -e DIM_HOST_NODE=172.25.75.12 -e DIM_DNS_NODE=host.docker.internal tester-alf /bin/bash -c \
    "AlfIPbus -n ALF -l 172.25.75.10:50001 -f /common-storage/output/alf-log -t 1000 -v"

# Run Generator container
docker run -i -d --name tester-generator --rm \
    --mount type=bind,source=$(pwd)/test/in-container-test/common-storage,target=/common-storage \
    --network alfipbus-tester-network \
    --ip 172.25.75.11 \
    --add-host host.docker.internal:host-gateway alfipbus-tester 

docker exec -e LD_LIBRARY_PATH="/usr/lib" -e DIM_HOST_NODE=172.25.75.12 -e DIM_DNS_NODE=host.docker.internal tester-generator /bin/bash -c \
    "/alf-ipbus-tester/build/bin/generator -c /common-storage/test-configuration.toml -f /common-storage/output/generator-log -v"
EXIT_CODE=$?

if [ $EXIT_CODE -ne 0 ]; then
          echo "Test failed: $EXIT_CODE"
          cat $(pwd)/test/in-container-test/common-storage/output/generator-log
        else
          tail -1  $(pwd)/test/in-container-test/common-storage/output/generator-log
fi

echo Stopping containers

# Stop all containers
docker stop tester-generator
docker stop tester-alf
docker stop tester-mock

if [ $EXIT_CODE -ne 0 ]; then
    exit 1
fi
