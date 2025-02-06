#ifndef IPBUSSTATUS_H
#define IPBUSSTATUS_H

#include <cstdint>
#include <cstring>
#include <string>
#include <climits>
#include"IPbusHeaders.h"

namespace ipbus
{

struct StatusPacket {
  PacketHeader header = reverseBytes<uint32_t>(uint32_t(PacketHeader(enums::packets::Status))); // 0x200000F1: {0xF1, 0, 0, 0x20} -> {0x20, 0, 0, 0xF1}
  uint32_t mtu = 0,
           nResponseBuffers = 0,
           nextPacketID = 0;
  uint8_t trafficHistory[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
  uint32_t controlHistory[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
};

}

#endif