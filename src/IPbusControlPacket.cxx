#include "IPbusControlPacket.h"
#include <boost/log/trivial.hpp>

namespace ipbus
{

uint32_t* IPbusControlPacket::masks(uint32_t mask0, uint32_t mask1)
{
  m_dt[0] = mask0; // for writing 0's: AND term
  m_dt[1] = mask1; // for writing 1's: OR term
  return m_dt;
}

void IPbusControlPacket::addTransaction(TransactionType type, uint32_t address, uint32_t* dataIn, uint32_t* dataOut, uint8_t nWords)
{
  Transaction currentTransaction;
  m_request[m_requestSize] = TransactionHeader(type, nWords, m_transactionsList.size());
  currentTransaction.requestHeader = (TransactionHeader*)(m_request + m_requestSize++);
  m_request[m_requestSize] = address;
  currentTransaction.address = m_request + m_requestSize++;
  currentTransaction.responseHeader = (TransactionHeader*)(m_response + m_responseSize++);

  switch (type) {
    case DataRead:
    case NonIncrementingRead:
    case ConfigurationRead:
      currentTransaction.data = dataOut;
      m_responseSize += nWords;
      break;

    case DataWrite:
    case NonIncrementingWrite:
    case ConfigurationWrite:
      currentTransaction.data = m_request + m_requestSize;
      for (uint8_t i = 0; i < nWords; ++i) {
        m_request[m_requestSize++] = dataIn[i];
      }
      break;

    case RMWbits:
      m_request[m_requestSize++] = dataIn[0]; // AND term
      m_request[m_requestSize++] = dataIn[1]; // OR term
      currentTransaction.data = m_response + m_responseSize++;
      break;

    case RMWsum:
      m_request[m_requestSize++] = *dataIn; // addend
      currentTransaction.data = m_response + m_responseSize++;
      break;

    default:
      BOOST_LOG_TRIVIAL(warning) << "Unknown transaction type " << type << ": no transaction was added";
  }
  if (m_requestSize > maxPacket || m_responseSize > maxPacket) {
    BOOST_LOG_TRIVIAL(error) << "Packet size exceeded";
    return;
  } else {
    m_transactionsList.push_back(currentTransaction);
    m_dataOut.push_back(dataOut);
  }
}

bool IPbusControlPacket::processResponse()
{
  for (uint16_t i = 0; i < m_transactionsList.size(); ++i) {
    TransactionHeader* th = m_transactionsList.at(i).responseHeader;
    if (th->protocolVersion != 2 || th->transactionID != i || th->typeID != m_transactionsList.at(i).requestHeader->typeID) {
      std::string message = "Unexpected transaction header: " + std::to_string(*th) + ", expected: " + std::to_string(*m_transactionsList.at(i).requestHeader & 0xFFFFFFF0);
      BOOST_LOG_TRIVIAL(error) << message;
      return false;
    }
    if (th->words > 0) {
      switch (th->typeID) {
        case DataRead:
        case NonIncrementingRead:
        case ConfigurationRead: {
          uint32_t wordsAhead = m_response + m_responseSize - (uint32_t*)th - 1;
          if (th->words > wordsAhead) {
            if (m_dataOut.at(i) != nullptr) {
              memcpy(m_dataOut.at(i), (uint32_t*)th + 1, wordsAhead * wordSize);
            }
            // emit successfulRead(wordsAhead); !!!
            if (th->infoCode == 0) {
              std::string message = "Read transaction from " + std::to_string(*m_transactionsList.at(i).address) + " truncated " + std::to_string(wordsAhead) + "words received: " + std::to_string(th->words);
              BOOST_LOG_TRIVIAL(warning) << message;
            }
            return false;
          } else {
            if (m_dataOut.at(i) != nullptr) {
              memcpy(m_dataOut.at(i), (uint32_t*)th + 1, th->words * wordSize);
            }
            // emit successfulRead(th->words); !!!
          }
        } break;

        case RMWbits:
        case RMWsum: {
          if (th->words != 1) {
            BOOST_LOG_TRIVIAL(error) << "Invalid RMW transaction";
            return false;
          }
          memcpy(m_dataOut.at(i), (uint32_t*)th + 1, wordSize);
        } break;

        case DataWrite:
        case NonIncrementingWrite:
        case ConfigurationWrite:
          break;

        default:
          BOOST_LOG_TRIVIAL(error) << "Unknown transaction type: response cannot be processed";
          return false;
      }
    }
    if (th->infoCode != 0) {
      std::string message = th->infoCodeString() + ", address: " + std::to_string(*m_transactionsList.at(i).address + th->words);
      BOOST_LOG_TRIVIAL(error) << "Transaction response error: " << message;
      return false;
    }
  }
  return true;
}

void IPbusControlPacket::reset()
{
  m_transactionsList.clear();
  m_dataOut.clear();
  m_requestSize = 1;
  m_responseSize = 1;
}

} // namespace ipbus