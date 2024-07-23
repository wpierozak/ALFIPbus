#include "IPbusControlPacket.h"

namespace ipbus
{

void IPbusControlPacket::debugPrint(std::string st)
{
  std::cerr << st << std::endl;
  std::cerr << "Request:\n";
  for (uint16_t i = 0; i < requestSize; ++i)
    std::cerr << std::hex << request[i] << std::endl;
  std::cerr << "\t\tResponse:" << std::endl;
  for (uint16_t i = 0; i < responseSize; ++i)
    std::cerr << "\t\t" << std::hex << response[i] << std::endl;
}

uint32_t* IPbusControlPacket::masks(uint32_t mask0, uint32_t mask1)
{
  dt[0] = mask0; // for writing 0's: AND term
  dt[1] = mask1; // for writing 1's: OR term
  return dt;
}

void IPbusControlPacket::addTransaction(TransactionType type, uint32_t address,  uint32_t* dataIn, uint32_t* dataOut, uint8_t nWords) 
{
  Transaction currentTransaction;
  request[requestSize] = TransactionHeader(type, nWords, transactionsList.size());
  currentTransaction.requestHeader = (TransactionHeader*)(request + requestSize++);
  request[requestSize] = address;
  currentTransaction.address = request + requestSize++;
  currentTransaction.responseHeader = (TransactionHeader*)(response + responseSize++);

        switch (type) {
            case data_read:
            case nonIncrementingRead:
            case configurationRead:
                currentTransaction.data = dataOut;
                responseSize += nWords;
                break;

            case data_write:
            case nonIncrementingWrite:
            case configurationWrite:
                currentTransaction.data = request + requestSize;
                for (uint8_t i=0; i<nWords; ++i) 
                {
                    request[requestSize++] = dataIn[i];
                }
                break;

            case RMWbits:
                request[requestSize++] = dataIn[0]; //AND term
                request[requestSize++] = dataIn[1]; // OR term
                currentTransaction.data = response + responseSize++;
                break;

            case RMWsum:
                request[requestSize++] = *dataIn; //addend
                currentTransaction.data = response + responseSize++;
                break;

            default:
                debugPrint("unknown transaction type");
        }
        if (requestSize > maxPacket || responseSize > maxPacket) {
            debugPrint("packet size exceeded");
            return;
        } else
        { 
            transactionsList.push_back(currentTransaction);
            m_dataOut.push_back(dataOut);
        }
}



bool IPbusControlPacket::processResponse() 
{ 
  for (uint16_t i=0; i<transactionsList.size(); ++i) 
  {
      TransactionHeader *th = transactionsList.at(i).responseHeader;
      if (th->ProtocolVersion != 2 || th->TransactionID != i || th->TypeID != transactionsList.at(i).requestHeader->TypeID) 
      {
          std::string message = "unexpected transaction header: " + std::to_string(*th) + ", expected: " + std::to_string(*transactionsList.at(i).requestHeader & 0xFFFFFFF0);
          debugPrint(message);
          return false;
      }
      if (th->Words > 0)
      { 
        switch (th->TypeID) 
        {
          case data_read:
          case nonIncrementingRead:
          case configurationRead: 
          {
            uint32_t wordsAhead = response + responseSize - (uint32_t *)th - 1;
            if (th->Words > wordsAhead) 
            { 
                if (m_dataOut.at(i) != nullptr) 
                {
                  memcpy(m_dataOut.at(i), (uint32_t *)th + 1, wordsAhead * wordSize);
                }
                  //emit successfulRead(wordsAhead); !!! 
                if (th->InfoCode == 0) 
                {
                  std::string message = "read transaction from " + std::to_string(*transactionsList.at(i).address) + " truncated " + std::to_string(wordsAhead) + "words received: " + std::to_string(th->Words);
                  debugPrint(message);
                }
                return false;
            } 
            else 
            {
                if (m_dataOut.at(i) != nullptr)
                {
                  memcpy(m_dataOut.at(i), (uint32_t *)th + 1, th->Words * wordSize);
                }
                    //emit successfulRead(th->Words); !!!
            }
          }
          break;

          case RMWbits:
          case RMWsum:
          {
            if (th->Words != 1) 
            {
              debugPrint("wrong RMW transaction");
              return false;
            }
            memcpy(m_dataOut.at(i), (uint32_t *)th + 1, wordSize);
          }
          break;

        case data_write:
        case nonIncrementingWrite:
        case configurationWrite:
          break;

        default:
          debugPrint("Unknown transaction type");
          return false;
      }
    }
    if (th->InfoCode != 0) {
      std::string message = th->infoCodeString() + ", address: " + std::to_string(*transactionsList.at(i).address + th->Words);
      debugPrint(message);
      return false;
    }
  }
  return true;
}

void IPbusControlPacket::reset()
{
    transactionsList.clear();
    m_dataOut.clear();
    requestSize = 1;
    responseSize = 1;
}

} // namespace ipbus