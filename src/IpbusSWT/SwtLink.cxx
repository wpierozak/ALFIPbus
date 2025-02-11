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
  if(getSize() <= 1)
  {
    BOOST_LOG_TRIVIAL(warning) << "Received empty request";
    sendFailure();
  }

  bool success = false;
  try{
    success = parseSequence(swtSequence);
  }
  catch(std::exception& e){
    BOOST_LOG_TRIVIAL(error) << e.what();
  }

  if (success) {
    sendResponse();
  } else {
    sendFailure();
  }
  resetState();
}

void SwtLink::resetState()
{
  m_fredResponse.clear();
  m_request.reset();
  m_cmdFifoSize = 0;
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

bool SwtLink::readBlock(const Swt& frame)
{
  constexpr uint32_t maxPacketPayload = ipbus::maxPacket - 3;

  if(frame.data > 1024){
    BOOST_LOG_TRIVIAL(error) << "Exceeded maximum block size (1024); received request for " << frame.data << " words";
    return false;
  }

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
      for( uint32_t idx = 0 ; idx < size; idx++){
        m_fifo.push(Swt{frame.mode, currentAddress, ipbusOutputBuffer[idx]});
        if(increment){
          currentAddress++;
        }
      }
      wordRead += size;
      m_request.reset();
    }
    else
    {
      m_request.reset();
      return false;
    }
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
  for(uint32_t idx = 0; idx < m_cmdFifoSize; idx++) {
    if (m_commands[idx].type == CruCommand::Type::Read) {
        while(m_fifo.empty() == false){
          m_fifo.pop().appendToString(m_fredResponse);
          m_fredResponse += "\n";
        }
        m_fifo.clear();
    } else if(m_commands[idx].type == CruCommand::Type::Write){
      updateFifoState(m_commands[idx].frame);
      m_fredResponse += "0\n";
    } 
  }
  m_cmdFifoSize = 0;
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

CruCommand& SwtLink::parseNextCommand(const char* &currentLine)
{
  m_commands[m_cmdFifoSize++] = CruCommand(currentLine);

  if(m_commands[m_cmdFifoSize-1].type == CruCommand::Type::Invalid){
    return m_commands[m_cmdFifoSize-1];
  }

  currentLine += m_commands[m_cmdFifoSize-1].commandStrLen()+1;      
  return m_commands[m_cmdFifoSize-1];
}

std::string SwtLink::parseInvalidLine(const char* currentLine, const char*end)
{
  const char* delimiter = std::find(currentLine, end,'\n');
  std::string message;
  std:copy(currentLine, delimiter, std::back_inserter(message));
  return message;
}

bool SwtLink::parseSequence(const char* request)
{
  const char* currentLine = request;
  const char* end = request + getSize() - 1;

  bool expectRmwOr = false;
  bool failure = false;

  while(!failure && currentLine < end)
    {
      CruCommand& cmd = parseNextCommand(currentLine);
      if(isIPbusPacketFull()){
        if(executeTransactions() == false){
          failure = true;
        }
      }
      if(cmd.type == CruCommand::Type::Invalid){
        BOOST_LOG_TRIVIAL(error) << "Sequence parsing failed: Invalid line:" << parseInvalidLine(currentLine,end);
        failure = true;
      }
      if(cmd.type == CruCommand::Type::Read){
        if(m_fifo.empty()){
          continue;
        } else{
          processExecutedCommands();
        }
      } else if(cmd.type == CruCommand::Type::ScReset){
        m_fifo.clear();
      } else {
        switch (cmd.frame.type())
        {
        case Swt::TransactionType::Read:
        {
          Swt &frame = cmd.frame;
          m_request.addTransaction(ipbus::enums::transactions::Read, frame.address, &frame.data, &frame.data, 1);
        }
          break;
        case Swt::TransactionType::Write:
        {
          Swt &frame = cmd.frame;
          m_request.addTransaction(ipbus::enums::transactions::Write, frame.address, &frame.data, &frame.data, 1);
        }
          break;
        case Swt::TransactionType::RMWsum:
        {
          Swt &frame = cmd.frame;
          m_request.addTransaction(ipbus::enums::transactions::RMWsum, frame.address, &frame.data, &frame.data);
        }
          break;
        case Swt::TransactionType::RMWbitsAnd:
        {
          expectRmwOr = true;
          continue;
        }
        break;
        case Swt::TransactionType::RMWbitsOr:
        {
          uint32_t buffer[2];
          if(expectRmwOr == false){
            BOOST_LOG_TRIVIAL(error) << "Received RMW OR before RMW AND!";
            failure = true;
          } else if(m_commands[m_cmdFifoSize-1].type == CruCommand::Type::Read){
            buffer[0] = m_commands[m_cmdFifoSize-2].frame.data;
            buffer[1] = cmd.frame.data;
            m_request.addTransaction(ipbus::enums::transactions::RMWbits, cmd.frame.address, buffer, &m_commands[m_cmdFifoSize-2].frame.data);
          }else{
            buffer[0] = m_commands[m_cmdFifoSize-1].frame.data;
            buffer[1] = cmd.frame.data;
            m_request.addTransaction(ipbus::enums::transactions::RMWbits, cmd.frame.address, buffer, &m_commands[m_cmdFifoSize-1].frame.data);
          }
          expectRmwOr = false; 
        }
        break;
        case Swt::TransactionType::BlockReadIncrement:
        case Swt::TransactionType::BlockReadNonIncrement:
        {
          if(executeTransactions() == false){
            failure = true;
          }else{
            readBlock(cmd.frame);
          }
        }
        break;
        default:
          break;
        }
        if(expectRmwOr == true){
          BOOST_LOG_TRIVIAL(error) << "Missing RMW OR after RMW AND!";
          failure = true;
        }  
      } 
  }
  if(executeTransactions() == false){
      failure = true;
  }
  return !failure;
}


} // namespace fit_swt