## Overview
The IPbus protocol facilitates packet-based communication between devices using a master-slave architecture. To minimize network communication overhead, IPbus packets are typically transported within UDP datagrams. Facilitating IPbus communication requires three key components:
- Master implementation
- Slave implementation
- UDP communication layer
In practice, the slave side is hosted on an FPGA board, while the other components are handled by the software implementation. This repository provides the master implementation along with UDP-based network communication, and it also includes a slave implementation for testing purposes. Both the master and slave implementations handle packet creation and processing internally, so there is no need to understand the exact internal structure of IPbus packets

## IPbus specification

Full IPbus documentation is provided  [here](https://ipbus.web.cern.ch/doc/user/html/_downloads/d251e03ea4badd71f62cffb24f110cfa/ipbus_protocol_v2_0.pdf).

## Building

### Requirements
- C++20
- boost::asio
- pthread

### Step-by-step
```
mkdir build
cmake3 ..
cmake3 --build .
```

## Packets
There are two classes designed for handling control packet creation: **IPbusRequest** and **IPbusResponse**. There is also special **StatusPacket** structure for simple creation of IPbus status packet.

### Packet types
There three IPbus packet types: Status, Control and Resend. The last one is used in reliability mechanisms, which is not implemented in this repository yet.

### Transaction types
| Transaction Type       | Value |
|------------------------|-------|
| Read                   | 0     |
| Write                  | 1     |
| NonIncrementingRead    | 2     |
| NonIncrementingWrite   | 3     |
| RMWbits                | 4     |
| RMWsum                 | 5     |
| ConfigurationRead      | 6     |
| ConfigurationWrite     | 7     |

### IPbusRequest
The IPbusRequest purpose is creating control packet containing mutiple transactions. In order to compose a packet, one needs to call `addTransaction` for each transaction.
```
void addTransaction(enums::transactions::TransactionType type, \\ transaction type
            uint32_t address,     \\ memory address on the target device
            uint32_t* dataIn,     \\ address of the memory block containg input data  (refer to the documentation)
            uint32_t* dataOut,    \\ address of the memory block where output data should be saved after successful operation (refer to the documentation, for example: read transaction returns values readed from device)
            uint8_t nWords = 1    \\ number of words to be changed/readed (refer to the documentation)
            );
```
IPbusRequest on initialization creates a header with default ID equals 0, `reset` method clears packet and may be used to set new ID number.
```
void reset(int packetID = 0);
```

### IPbusResponse
The IPbusResponse class is used within master-side code only as the buffer for the incoming reponse. The packet creation functionality is used only within slave code, which is provided primarily for testing purposes, but it may also be applicable in real-world scenarios. The `addTransaction` method is used to generate a response to a request from the master, with request handling being the responsibility of the IPbusSlave class.

```
\\ Returns false if provided reponse data is inconsistent, returns true otherwise
bool addTransaction(enums::transactions::TransactionType type, \\ transaction type
                uint32_t* dataIn,  \\ address of the memory block containg data that should be included within response
                uint8_t nWords,    \\ number of words (refer to the documentation)
                InfoCode infocode  \\ info code ((refer to the documentation)
                );
```

## IPbusMaster

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




