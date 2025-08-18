#include <string>
#include <boost/exception/diagnostic_information.hpp>
#include<algorithm>
#include<iterator>
#include "IpbusSWT/SwtLink.h"
#include "IpbusSWT/utils.h"

namespace fit_swt
{

void SwtLink::runStatusThread()
{
  if(isStausCheckInProgress() == false){
    m_checkStatusParallel = std::make_unique<std::thread>(
      [&]{
        while(!isIPbusOK() && !m_terminate) {
          checkStatus();
        }
      });
    m_checkStatusParallel->detach();
  }
}

void SwtLink::rpcHandler()
{
  BOOST_LOG_TRIVIAL(debug) << "Received request";
  resetState();
  
  if(isIPbusOK() == true){
    processRequest(getString());
  } else{
    BOOST_LOG_TRIVIAL(error) << "Device is not available!";
    sendFailure();
  }
  
  if(isIPbusOK() == false){
    runStatusThread();
  }
}

void SwtLink::processRequest(const char* swtSequence)
{
  if(getSize() <= 1){
    BOOST_LOG_TRIVIAL(debug) << "Received empty request";
    sendResponse();
    return;
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
}

void SwtLink::resetState()
{
  m_fredResponse.clear();
  m_request.reset();
  m_cmdBuffer.reset();
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

  m_fifo.clear();

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
      for(uint32_t idx = 0 ; idx < size; idx++){
        updateFifoState(Swt{frame.mode, currentAddress, ipbusOutputBuffer[idx]});
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

void SwtLink::executeReadCommand()
{
  do{
    m_fifo.pop().appendToString(m_fredResponse);
    m_fredResponse += "\n";
  }while(m_fifo.empty() == false);
  m_fifo.clear();
}

void SwtLink::processExecutedCommands()
{
  for(uint32_t idx = 0; idx < m_cmdBuffer.size; idx++) {
      switch(m_cmdBuffer[idx].type) {
        case CruCommand::Type::Read: {
          executeReadCommand();
        }
          break;
        case CruCommand::Type::Write: {
          updateFifoState(m_cmdBuffer[idx].frame);
          m_fredResponse += "0\n";
        }
          break;
        case CruCommand::Type::Wait: {
          m_fredResponse += "1\n";
        }
        default:
          break;
      }
  }
  m_cmdBuffer.reset();
}

void SwtLink::updateFifoState(const Swt& frame)
{
  switch(frame.mode)
  {
    case 0:
    case 2:
    case 4:
    case 8:
    case 9:
    try
    {  
      m_fifo.push(frame);
    }
    catch(const std::exception& e)
    {
      BOOST_LOG_TRIVIAL(warning) << "Fifo is full!";
    }
    
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
  currentLine += (m_cmdBuffer.push(CruCommand(currentLine)).valid()) ? (m_cmdBuffer.back().commandStrLen() + 1) : 0;
  return m_cmdBuffer.back();
}

bool SwtLink::parseSequence(const char* request)
{
  const char* currentLine = request;
  const char* end = request + getSize() - 1;

  bool expectRmwOr = false;
  bool failure = false;
  
  uint32_t buffer[2];

  while(!failure && currentLine < end){
    if(isIPbusPacketFull()){
      failure = !(executeTransactions());
    }
    CruCommand& cmd = parseNextCommand(currentLine);
    switch (cmd.type){
    case CruCommand::Type::ScReset: {
      m_fifo.clear();
    }
      break;
    case CruCommand::Type::Read: {
      failure = m_cmdBuffer.validateLastCmd();
    }
      break;
    case CruCommand::Type::Write:{
      switch (cmd.frame.type()){
      case Swt::TransactionType::Read:{
        Swt &frame = cmd.frame;
        m_request.addTransaction(ipbus::enums::transactions::Read, frame.address, &frame.data, &frame.data, 1);
      }
        break;
      case Swt::TransactionType::Write:{
        Swt &frame = cmd.frame;
        m_request.addTransaction(ipbus::enums::transactions::Write, frame.address, &frame.data, &frame.data, 1);
      }
        break;
      case Swt::TransactionType::RMWsum:{
        Swt &frame = cmd.frame;
        m_request.addTransaction(ipbus::enums::transactions::RMWsum, frame.address, &frame.data, &frame.data);
      }
        break;
      case Swt::TransactionType::RMWbitsAnd:{
        expectRmwOr = true;
        continue;
      }
        break;
      case Swt::TransactionType::RMWbitsOr:{
        if(validateRmwTransaction(cmd.frame, expectRmwOr) == false){
          failure = true;
        } else {
          expectRmwOr = false; 
          buffer[0] = m_cmdBuffer.oneBeforeLast().frame.data;
          buffer[1] = cmd.frame.data;
          m_request.addTransaction(ipbus::enums::transactions::RMWbits, cmd.frame.address, buffer, &m_cmdBuffer.oneBeforeLast().frame.data);
        } 
      }
        break;
      case Swt::TransactionType::BlockReadIncrement:
      case Swt::TransactionType::BlockReadNonIncrement:{
        if(executeTransactions() == false){
          failure = true;
        } else {
          failure = !(readBlock(cmd.frame));
        }
      }
        break;
      default:
        break;
      }
    }
      break;
    case CruCommand::Type::Invalid: {
      BOOST_LOG_TRIVIAL(error) << "Sequence parsing failed: Invalid line:" 
                              << std::string_view(currentLine, std::find(currentLine, end,'\n'));
      failure = true;
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

  if(failure == false){
      failure = !(executeTransactions());
  }
  return !failure;
}


} // namespace fit_swt