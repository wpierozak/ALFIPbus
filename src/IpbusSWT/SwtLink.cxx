#include <string>
#include <boost/exception/diagnostic_information.hpp>
#include<algorithm>
#include<iterator>
#include "IpbusSWT/SwtLink.h"
#include "IpbusSWT/utils.h"

namespace fit_swt
{

void SwtLink::rpcHandler()
{
  BOOST_LOG_TRIVIAL(debug) << "Received request";
  resetState();
  processRequest(getString());
}

void SwtLink::processRequest(const char* swtSequence)
{
  m_sequenceLen = getSize() - 1;
  if(m_sequenceLen == 0)
  {
    BOOST_LOG_TRIVIAL(warning) << "Received empty request";
    sendFailure();
  }

  if (!parseFrames(swtSequence)) {
    sendFailure();
    return;
  }
  
  bool success = false;
  try{
    success = interpretFrames();
  }
  catch(std::exception& e){
    BOOST_LOG_TRIVIAL(error) << e.what();
  }

  if (success) {
    sendResponse();
  } else {
    sendFailure();
  }
}

bool SwtLink::parseFrames(const char* request)
{
  const char* currentLine = request;
  const char* end = request + m_sequenceLen;

  try
  {
    for(; currentLine < end; )
    {
      m_commands.emplace_back(currentLine);
      if(m_commands.back().type == CruCommand::Type::Invalid){
        const char* delimiter = std::find(currentLine, end,'\n');
        std::string message;
        std:copy(currentLine, delimiter, std::back_inserter(message));
        BOOST_LOG_TRIVIAL(error) << "Sequence parsing failed: Invalid line:" << message;
        return false;
      }
      currentLine += m_commands.back().commandStrLen()+1;      
    }
  }
   catch (const std::exception& e) {
    BOOST_LOG_TRIVIAL(error) << "Sequence parsing failed: " << boost::diagnostic_information(e);
    return false;
  }

  m_commandsNumber = m_commands.size();
  return true;
}

void SwtLink::resetState()
{
  m_fredResponse.clear();
  m_request.reset();
  m_commands.clear();
  m_current = m_lastWritten = 0;
}


bool SwtLink::interpretFrames()
{
  uint32_t buffer[2];

  for (; m_current < m_commandsNumber; m_current++) {
    
    if ((m_request.getSize() + PacketPadding >= ipbus::maxPacket) || m_commands[m_current].frame.isBlock()) {
      if(executeTransactions() == false){
        return false;
      }
    }

    if (m_commands[m_current].type != CruCommand::Type::Write) {
      continue;
    }

    Swt& frame = m_commands[m_current].frame;

    switch (frame.type()) {
      case Swt::TransactionType::Read:
        m_request.addTransaction(ipbus::enums::transactions::Read, frame.address, &frame.data, &frame.data, 1);
        break;

      case Swt::TransactionType::Write:
        m_request.addTransaction(ipbus::enums::transactions::Write, frame.address, &frame.data, &frame.data, 1);
        break;

      case Swt::TransactionType::RMWbits:
        if (frame.mode != 2) {
          BOOST_LOG_TRIVIAL(error) << "SWT sequence failed (" << m_current << "): " << "RMWbits failed: first frame is not the AND frame" << std::endl;
          return false;
        }
        if (m_current + 1 >= m_commandsNumber) {
          BOOST_LOG_TRIVIAL(error) << "SWT sequence failed (" << m_current << "): " << "RMWbits failed: second frame has been not received" << std::endl;
          return false;
        }
        if (m_commands[m_current + 1].type == CruCommand::Type::Read) {
          if (m_current + 2 >= m_commandsNumber) {
            BOOST_LOG_TRIVIAL(error) << "SWT sequence failed (" << m_current << "): "<< "RMWbits failed: second frame has been not received" << std::endl;
            return false;
          }
          if ((m_commands[m_current + 2].frame.mode) != 3) {
            BOOST_LOG_TRIVIAL(error) << "SWT sequence failed (" << m_current << "): " << "RMWbits failed: invalid second frame - mode: " << m_commands[m_current + 2].frame.mode << std::endl;
            return false;
          }
          buffer[0] = frame.data;
          buffer[1] = m_commands[m_current + 2].frame.data;
          m_request.addTransaction(ipbus::enums::transactions::RMWbits, frame.address, buffer, &frame.data);
          m_current += 2;
        } else {
          if ((m_commands[m_current + 1].frame.mode) != 3) {
            BOOST_LOG_TRIVIAL(error) << "SWT sequence failed (" << m_current << "): " << "RMWbits failed: invalid second frame - mode: " << m_commands[m_current + 1].frame.mode << std::endl;
            return false;
          }
          buffer[0] = frame.data;
          buffer[1] = m_commands[m_current + 1].frame.data;
          m_request.addTransaction(ipbus::enums::transactions::RMWbits, frame.address, buffer, &frame.data);
          m_current += 1;
        }

        break;

      case Swt::TransactionType::RMWsum:
        m_request.addTransaction(ipbus::enums::transactions::RMWsum, frame.address, &frame.data, &frame.data);
        break;

      case Swt::TransactionType::BlockReadIncrement:
      case Swt::TransactionType::BlockReadNonIncrement:
      {
        bool success = readBlock(frame, m_current);
        if(!success){
          return false;
        }
      }
        break;
      default:
      break;
    }
  }
  return executeTransactions();
}

bool SwtLink::executeTransactions()
{
  bool success = transceive(m_request, m_response);
  if(success){
    processExecutedCommands();
  }
  m_request.reset();
  return success;
}

bool SwtLink::readBlock(const Swt& frame, uint32_t frameIdx)
{
  constexpr uint32_t maxPacketPayload = ipbus::maxPacket - 3;

  if(frame.data > 1024){
    BOOST_LOG_TRIVIAL(error) << "Exceeded maximum block size (1024); received request for " << frame.data << " words";
    return false;
  }

    m_current++;
    processExecutedCommands();

    bool increment = (frame.type() == Swt::TransactionType::BlockReadIncrement);
    auto transactionType = (increment) ? ipbus::enums::transactions::Read : ipbus::enums::transactions::NonIncrementingRead;
    uint32_t currentAddress = frame.address;
    uint32_t wordRead = 0;
    uint32_t readCommands = 0;

    uint32_t ipbusOutputBuffer[maxPacketPayload];

    while(wordRead < frame.data){
      uint32_t wordLeft = frame.data - wordRead;
      uint32_t sizeA = ( (wordLeft > maxPacketPayload/2) ? maxPacketPayload/2 : wordLeft );
      uint32_t size = sizeA;
      m_request.addTransaction(transactionType, currentAddress, nullptr,  ipbusOutputBuffer, sizeA);
      if(sizeA < wordLeft && wordLeft < maxPacketPayload){
        uint32_t sizeB = wordLeft - sizeA;
        m_request.addTransaction(transactionType, currentAddress + sizeA*increment, nullptr, ipbusOutputBuffer+sizeA, sizeB);
        size += sizeB;
      } else if(sizeA < wordLeft){
        uint32_t sizeB = maxPacketPayload - sizeA;
        m_request.addTransaction(transactionType, currentAddress + sizeA*increment, nullptr, ipbusOutputBuffer+sizeA, sizeB);
        size += sizeB;
      }

      if(transceive(m_request, m_response))
      {
        uint32_t curr = m_current;
        for( uint32_t idx = 0 ; idx < size; idx++){
          m_fifo.push(Swt{frame.mode, currentAddress, ipbusOutputBuffer[idx]});
          if(increment){
            currentAddress++;
          }
        }
        m_request.reset();
      }
      else
      {
        m_request.reset();
        return false;
      }
      processExecutedCommands();
    }

    return true;
}

void SwtLink::sendResponse()
{
  BOOST_LOG_TRIVIAL(debug) << "Request successfully processed - sending response";
  m_fredResponse = "success\n" + m_fredResponse;
  setData(m_fredResponse.c_str());
}


void SwtLink::processExecutedCommands()
{
  for (;m_lastWritten < m_current; m_lastWritten++) {
      if (m_commands[m_lastWritten].type == CruCommand::Type::Read) {
        while(m_fifo.empty() == false){
          m_fifo.pop().appendToString(m_fredResponse);
          m_fredResponse += "\n";
        }
        
    } else if(m_commands[m_lastWritten].type == CruCommand::Type::Write){
        updateFifoState(m_commands[m_lastWritten].frame);
        m_fredResponse += "0\n";
    } else if(m_commands[m_lastWritten].type == CruCommand::Type::ScReset){
      m_fifo.clear();
    }
  }
}

void SwtLink::updateFifoState(const Swt& frame)
{
  switch(frame.mode)
  {
    case 0:
    case 2:
    case 4:
    m_fifo.push(frame);
    break;
    default:
    break;
  }
}

void SwtLink::sendFailure()
{
  BOOST_LOG_TRIVIAL(error) << "Request execution failed";
  m_fredResponse = "failure\n" + m_fredResponse;
  setData(m_fredResponse.c_str());
}

} // namespace fit_swt