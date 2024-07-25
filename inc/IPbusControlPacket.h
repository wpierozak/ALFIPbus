
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
static const char* errorTypeName[3] = { "Network error", "IPbus error", "Logic error" };

class IPbusControlPacket
{

 public:
  /** \brief List of transactions that will be sent  */
  std::vector<Transaction> m_transactionsList;
  std::vector<uint32_t*> m_dataOut;
  /** \brief Size of the request specified in words */
  uint16_t m_requestSize = 1,
           /** \brief Size of the response specified in words */
    m_responseSize = 1;
  /** \brief Buffer where the request is stored */
  uint32_t m_request[maxPacket],
    /** \brief Buffer where the response will be saved */
    m_response[maxPacket];
  uint32_t m_dt[2];

  IPbusControlPacket()
  {
    m_request[0] = PacketHeader(Control, 0);
  }
  ~IPbusControlPacket() {}

  uint32_t* masks(uint32_t mask0, uint32_t mask1);

  void addTransaction(TransactionType type, uint32_t address, uint32_t* dataIn, uint32_t* dataOut, uint8_t nWords = 1);

  bool processResponse();
  /**
   * @brief resets packet data
   *
   * @details reset method clears transactionList and resets request and response sizes to 1.
   */
  void reset();
};

} // namespace ipbus

#endif // IPBUSCONTROLPACKET_H
