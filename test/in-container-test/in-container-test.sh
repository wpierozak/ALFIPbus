#!bin/bash

print_failure() {
    echo "Error: $1 failed. Exiting."
    exit 1
}

dns&

# Building docker images
docker build  -f tester-container.Dockerfile -t alfipbus-tester .
if [ $? -ne 0 ]; then
    print_failure "alfipbus-tester build"
fi

docker build -f ../../Dockerfile -t alfipbus .
if [ $? -ne 0 ]; then
    print_failure "alfipbus build"
fi


# Preparing docker network
docker network inspect alfipbus-tester-network >/dev/null 2>&1 || \docker network create --driver=bridge --subnet=172.25.75.0/16 alfipbus-tester-network

# Run mock
docker run -i -d --name tester-mock --rm --mount type=bind,source=./common-storage,target=/common-storage --network alfipbus-tester-network --ip 172.25.75.10 alfipbus-tester
docker exec -d tester-mock  /bin/bash -c "/home/alf-ipbus-tester/build/bin/mock -c /common-storage/test-configuration.toml -f /common-storage/output/mock-log -v"

# Run ALF
docker run -i -d --name tester-alf --rm --mount type=bind,source=./common-storage,target=/common-storage --network alfipbus-tester-network --ip 172.25.75.12 --add-host host.docker.internal:host-gateway alfipbus
docker exec -d -e LD_LIBRARY_PATH="/usr/local/lib"  -e DIM_DNS_NODE=host.docker.internal tester-alf /bin/bash -c "AlfIPbus -n ALF -l 172.25.75.10:50001 -f /common-storage/output/alf-log -t 1000 -v"

# Run generator
docker run -i -d --name tester-generator --rm --mount type=bind,source=./common-storage,target=/common-storage --network alfipbus-tester-network --ip 172.25.75.11 --add-host host.docker.internal:host-gateway alfipbus-tester 
docker exec -d -e LD_LIBRARY_PATH="/usr/local/lib" -e DIM_DNS_NODE=host.docker.internal  tester-generator  /bin/bash -c "/home/alf-ipbus-tester/build/bin/generator -c /common-storage/test-configuration.toml -f /common-storage/output/generator-log -v"

sleep 10

# Stop generator
docker stop tester-generator
# Stop ALF
docker stop tester-alf
# Stop mock
docker stop tester-mock

