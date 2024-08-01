
#ifndef IPBUSCONTROLPACKET_H
#define IPBUSCONTROLPACKET_H
#include "IPbusHeaders.h"
#include <vector>
#include <iostream>
#include <vector>
#include <iostream>

namespace ipbus
{

const uint16_t maxPacket = 368; // 368 words, limit from ethernet MTU of 1500 bytes
enum ErrorType { NetworkError = 0,
                 IPbusError = 1,
                 logicError = 2 };

class IPbusControlPacket
{
 public:

  IPbusControlPacket(IPbusMode mode = IPbusMode::Master)
  {
    m_request[0] = PacketHeader(Control, 0);
    m_mode = mode;
  }
  ~IPbusControlPacket() {}

  void addTransaction(TransactionType type, uint32_t address, uint32_t* dataIn, uint32_t* dataOut, uint8_t nWords = 1);

  uint16_t getRequestSize() const { return m_requestSize; }
  uint16_t getResponseSize() const { return m_responseSize; }

  bool processResponse();
  void reset();

  IPbusMode getMode() { return m_mode; }
  void setMode(IPbusMode mode) { m_mode = mode; }

  std::vector<Transaction> m_transactionsList;
  std::vector<uint32_t*> m_dataOut;
  uint16_t m_requestSize = 1,
          m_responseSize = 1;
  uint32_t m_request[maxPacket],
          m_response[maxPacket];

  IPbusMode m_mode;
};

} // namespace ipbus

#endif // IPBUSCONTROLPACKET_H
