# IPbus interface

## IPbus specification

IPbus is a packet-based protocol. An IPbus packet stacks multiple transactions, with the maximum packet size set to 1500 bytes, though it can be adjusted (const uint16_t maxPacket within IPbusControlPacket.h). Full IPbus documentation is provided  [here](https://ipbus.web.cern.ch/doc/user/html/_downloads/d251e03ea4badd71f62cffb24f110cfa/ipbus_protocol_v2_0.pdf).

## Building

### Requirements
- C++20
- boost::asio
- pthread

### Step-by-step

If you already pulled repository, then you need to:
```
mkdir build
cd build
cmake3 ..
cmake3 --build .
```

## Documentation

### Introduction
The IPbus library provides both master and slave implementations. The `IPbusMaster` class is a complete master-side implementation, ready for use in IPbus communication. On the other hand, `IPbusSlave` is a software mock of the FPGA implementation and can be used for testing purposes.


### Example
You can find an examples within the test directory. Content of the example.cpp file is included below.
```
#include <iostream>
#include <chrono>
#include <thread>
#include <ctime>
#include "IPbusMaster.h"
#include "IPbusRequest.h"
#include "IPbusResponse.h"

int main() {
    try {
        srand(time(NULL));

        boost::asio::io_context io;
        IPbusMaster target(io,"172.20.75.175", 0, 50001);
        IPbusRequest request;
        IPbusResponse response;

        uint32_t data[SIZE] = {0x0,0x0};
        uint32_t address = 0x1004;

        request.addTransaction(TransactionType::DataRead, address, data, SIZE);
        target.transceive(request, response);

        std::cout << "\nRead...\n";
        std::cout << std::hex << data[0] << ' ' << std::hex << data[1] << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    };
    return 0;
}
```
In general, sending IPbus packet as master follows below schema.
1. Create a boost::asio::io_context variable (it must exist for the entire program's lifetime).
2. Create IPbusMaster object, you need to provide io_context, remote device address, and port it listens to
3. Create IPbusRequest and IPbusResponse
4. Create a buffer for data
5. Add transaction to the request: you need to provide transaction type, register address, buffer address and number of words
6. Pass request and response to IPbusMaster::transceive




