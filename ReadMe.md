# IPbus interface

## General notes

- The implementation provides synchronized message exchange and periodic asynchronous message exchange. If you need periodic uploads, you can set the cycle time (in seconds) and run the timer. WARNING: For now, the timer cannot be stopped.
- Pthread library is required
- boost::asio is required
- It is adviced to use IPbusTarget class as a base class for specific usage

## IPbus specification

IPbus is a packet-based protocol. An IPbus packet stacks multiple transactions, with the maximum packet size set to 1500 bytes, though it can be adjusted (const uint16_t maxPacket within IPbusControlPacket.h). Full IPbus documentation is provided  [here](https://ipbus.web.cern.ch/doc/user/html/_downloads/d251e03ea4badd71f62cffb24f110cfa/ipbus_protocol_v2_0.pdf).

## How to create a packet

`IPbusControlPacket` class is responsible for preparing IPbus packet to be sent. You can add transaction to packet using `addTransaction` method with following arguments:

- `TransactionType` type- transaction type (`IPbusHeaders.h`)
- `uint32_t` address - register address (or address of first register for multi-register write)
- `uint32_t` data - pointer to buffer with data
- `uint8_t` words - number of (32 bits) words

`IPbusControlPacket` provides also `addWordToWrite` for one-word write and `addNBitsToChange` for RMWbits operation.

After adding all transaction you can pass the packet to the `IPbusTarget`





