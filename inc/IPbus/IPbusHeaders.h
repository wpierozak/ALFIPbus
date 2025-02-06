
#ifndef IPBUSHEADERS_H
#define IPBUSHEADERS_H

#include <cstdint>
#include <cstring>
#include <string>
#include <climits>
#include "IPbusEnums.h"

namespace ipbus
{

const uint8_t wordSize = sizeof(uint32_t); // 4 bytes

template <typename T>
T reverseBytes(T u)
{
  static_assert(CHAR_BIT == 8, "CHAR_BIT != 8");

  union {
    T u;
    unsigned char u8[sizeof(T)];
  } source, dest;

  source.u = u;

  for (size_t k = 0; k < sizeof(T); k++)
    dest.u8[k] = source.u8[sizeof(T) - k - 1];

  return dest.u;
}

struct PacketHeader {
  uint32_t packetType : 4,
    byteOrder : 4,
    packetID : 16,
    rsvd : 4,
    protocolVersion : 4;

  PacketHeader(enum enums::packets::PacketType t = enums::packets::Status, uint16_t id = 0)
  {
    packetType = t;
    byteOrder = 0xf;
    packetID = id;
    rsvd = 0;
    protocolVersion = 2;
  }

  PacketHeader(const uint32_t& word) { memcpy(this, &word, wordSize); }

  operator uint32_t() const { return *reinterpret_cast<const uint32_t*>(this); }
};

struct TransactionHeader {
  uint32_t infoCode : 4,
    typeID : 4,
    words : 8,
    transactionID : 12,
    protocolVersion : 4;
  TransactionHeader(enums::transactions::TransactionType t, uint8_t nWords, uint16_t id = 0, uint8_t code = 0xf)
  {
    infoCode = code;
    typeID = t;
    words = nWords;
    transactionID = id;
    protocolVersion = 2;
  }
  TransactionHeader(const uint32_t& word) { memcpy(this, &word, wordSize); }
  operator uint32_t() { return *reinterpret_cast<uint32_t*>(this); }
  std::string infoCodeString()
  {
    switch (infoCode) {
      case 0x0:
        return "Successful request";
      case 0x1:
        return "Bad header";
      case 0x4:
        return "IPbus read error";
      case 0x5:
        return "IPbus write error";
      case 0x6:
        return "IPbus read timeout";
      case 0x7:
        return "IPbus write timeout";
      case 0xf:
        return "Outbound request";
      default:
        return "Unknown info code";
    }
  }
};


} // namespace ipbus

#endif // IPBUSHEADERS_H
