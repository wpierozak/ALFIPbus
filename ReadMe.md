# IPbus interface

## General notes

- Implementation provides synchronize message exchange and periodic asynchronouse message exchange. If you need periodic upload
you can set cycle time (in seconds) and run the timer. WARNINING: for now, timer can not be stopped.
- Pthread library is required
- boost::asio is required
- It is adviced to use IPbusTarget class as a base class for specific usage

## IPbus specification

IPbus is a packet-based protocol. IPbus packet stacks multiple transaction, the maxium packet size is set to 1500 bytes, but it can be adjusted
(`const uint16_t maxPacket` within IPbusControlPacket.h). Full IPbus documenatation is provided [here](https://ipbus.web.cern.ch/doc/user/html/_downloads/d251e03ea4badd71f62cffb24f110cfa/ipbus_protocol_v2_0.pdf).



