# IPbus interface

## General notes

- The implementation provides synchronized message exchange and periodic asynchronous message exchange. If you need periodic uploads, you can set the cycle time (in seconds) and run the timer. WARNING: For now, the timer cannot be stopped.
- Pthread library is required
- boost::asio is required
- It is adviced to use IPbusTarget class as a base class for specific usage

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

### Example
You can find an examples within the test directory. Content of the example.cpp file is included below.
```
#include <iostream>
#include<chrono>
#include<thread>
#include<ctime>
#include "IPbusInterface.h"

#define SIZE 2

int main() {
    try {
        srand(time(NULL));

        boost::asio::io_context io;
        IPbusTarget target(io,"172.20.75.175", 0, 50001);
        IPbusControlPacket packet;

        uint32_t data[SIZE] = {0x0,0x0};
        uint32_t address = 0x1004;

        packet.addTransaction(TransactionType::ipread, address, data, SIZE);
        target.transcieve(packet);

        std::cout << "\nRead...\n";
        std::cout << std::hex << data[0] << ' ' << std::hex << data[1] << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    };
    return 0;
}
```
In general, sending IPbus packet follows below schema.
1. Create a boost::asio::io_context variable (it must exist for the entire program's lifetime).
2. Create IPbusTarget object, you need to provide io_context, remote device address, and port it listens to
3. Create IPbusControlPacket object
4. Create a buffer for data
5. Add transaction to the packet: you need to provide transaction type, register address, buffer address and number of words
6. Pass packet to IPbusTarget::transcieve (called on the created IPbusTarget object)

**IMPORTANT** at the initialization IPbusTarget tries to send status packet to the device, then it waits for the response. If device is not reposnding, program will block until it receivces status packet respond. It will be handled in non-blocking way in the future.

## How to create a packet

`IPbusControlPacket` class is responsible for preparing IPbus packet to be sent. You can add transaction to packet using `addTransaction` method with following arguments:

- `TransactionType` type- transaction type (`IPbusHeaders.h`)
- `uint32_t` address - register address (or address of first register for multi-register write)
- `uint32_t` data - pointer to buffer with data
- `uint8_t` words - number of (32 bits) words

`IPbusControlPacket` provides also `addWordToWrite` for one-word write and `addNBitsToChange` for RMWbits operation.

After adding all transaction you can pass the packet to the `IPbusTarget`





