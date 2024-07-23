
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
enum errorType { networkError = 0,
                 IPbusError = 1,
                 logicError = 2 };
static const char* errorTypeName[3] = { "Network error", "IPbus error", "Logic error" };
const uint16_t maxPacket = 368; // 368 words, limit from ethernet MTU of 1500 bytes
enum errorType { networkError = 0,
                 IPbusError = 1,
                 logicError = 2 };
static const char* errorTypeName[3] = { "Network error", "IPbus error", "Logic error" };

class IPbusControlPacket
{

 public:
  /** \brief List of transactions that will be sent  */
  std::vector<Transaction> transactionsList;
  std::vector<uint32_t*> m_dataOut;
  /** \brief Size of the request specified in words */
  uint16_t requestSize = 1,
           /** \brief Size of the response specified in words */
    responseSize = 1;
  /** \brief Buffer where the request is stored */
  uint32_t request[maxPacket],
    /** \brief Buffer where the response will be saved */
    response[maxPacket];
  uint32_t dt[2];

  IPbusControlPacket()
  {
    request[0] = PacketHeader(control, 0);
  }
  ~IPbusControlPacket() {}
  IPbusControlPacket()
  {
    request[0] = PacketHeader(control, 0);
  }
  ~IPbusControlPacket() {}

  void debugPrint(std::string st);
  void debugPrint(std::string st);

  uint32_t* masks(uint32_t mask0, uint32_t mask1);

  void addTransaction(TransactionType type, uint32_t address, uint32_t* dataIN, uint32_t* dataOut, uint8_t nWords = 1);

  bool processResponse();
  /**
   * @brief resets packet data
   *
   * @details reset method clears transactionList and resets request and response sizes to 1.
   */
  void reset();
};

} // namespace ipbus
} // namespace ipbus

#endif // IPBUSCONTROLPACKET_H
