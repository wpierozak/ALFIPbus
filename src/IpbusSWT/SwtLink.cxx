#include <string>
#include <boost/exception/diagnostic_information.hpp>
#include<algorithm>
#include<iterator>
#include "IpbusSWT/SwtLink.h"
#include "IpbusSWT/utils.h"
#include "IpbusSWT/CruCommandSequence.h"
#include "IpbusSWT/CruCommandExecutor.h"

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
    m_fredResponse += "\n" ;
    m_fredResponse += e.what();
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

void SwtLink::sendResponse()
{
  BOOST_LOG_TRIVIAL(debug) << "Request successfully processed - sending response";
  m_fredResponse = "success\n" + m_fredResponse;
  setData(m_fredResponse.c_str());
}

void SwtLink::sendFailure()
{
  BOOST_LOG_TRIVIAL(error) << "Request execution failed";
  m_fredResponse = "failure\n" + m_fredResponse;
  setData(m_fredResponse.c_str());
}

bool SwtLink::executeTransactions()
{
  bool success = transceive(m_request, m_response);
  if(success){
    CruCommandExecutor::execute(m_cmdBuffer, m_fifo, m_fredResponse);
  }
  m_request.reset();
  return success;
}

bool SwtLink::parseSequence(const char* request)
{
  CruCommandSequnce sequence(request);
  bool expectRmwOr = false;
  bool failure = false;
  bool executeTransactionsOnNextRead = false;
  
  uint32_t buffer[2];
  uint32_t wordsToReadByNextCmd = 0x0;

  while(!failure && sequence.isNextCmd()){
    if(isIPbusPacketFull()){
      failure = !(executeTransactions());
    }

    CruCommandSequnce::Command& cmd  = m_cmdBuffer.push(sequence.getNextCmd());
    uint32_t* responseData = nullptr;

    switch (cmd.type){
    case CruCommandSequnce::Command::Type::ScReset: {
      m_fifo.clear();
      wordsToReadByNextCmd = 0x0;
    }
      break;

    case CruCommandSequnce::Command::Type::Read: {
      if(wordsToReadByNextCmd == 0x0) {
        throw std::runtime_error(std::string(CruCommandSequnce::Command::ReadStr) + ": there is no data in SWT FIFO to read!");
      } else if (m_cmdBuffer(1).type == CruCommandSequnce::Command::Type::ScReset) {
        throw std::runtime_error(std::string(CruCommandSequnce::Command::ReadStr) + ": there is no data in SWT FIFO to read!");
      } else if (m_cmdBuffer(1).type == CruCommandSequnce::Command::Type::Read) {
        throw std::runtime_error(std::string(CruCommandSequnce::Command::ReadStr) + ": there is no data in SWT FIFO to read!");
      } 
      if(executeTransactionsOnNextRead){
        failure = !(executeTransactions());
        executeTransactionsOnNextRead = false;
      }
      wordsToReadByNextCmd = 0x0;
    }
      break;

    case CruCommandSequnce::Command::Type::ReadCnt: {
      if (wordsToReadByNextCmd == 0x0) {
        throw std::runtime_error(std::string(CruCommandSequnce::Command::ReadCntStr) + ": there is no data in SWT FIFO to read!");
      } else if (wordsToReadByNextCmd != cmd.data.wordsToRead) {
        throw std::runtime_error(std::string(CruCommandSequnce::Command::ReadCntStr) + ": mismatch between expected words to read and words in SWT FIFO!");
      }
      if(executeTransactionsOnNextRead){
        failure = !(executeTransactions());
        executeTransactionsOnNextRead = false;
      }
      wordsToReadByNextCmd = 0x0;
    }
      break;

    case CruCommandSequnce::Command::Type::Write: {
      switch (cmd.data.frame.type()){
        case Swt::TransactionType::Read: {
          responseData = m_fifo.prepareResponseFrame(cmd.data.frame);
          m_request.addTransaction(ipbus::enums::transactions::Read, cmd.data.frame.address, &cmd.data.frame.data, responseData, 1);
        }
          break;
        case Swt::TransactionType::Write: {
          (void) m_fifo.prepareResponseFrame(cmd.data.frame);
          m_request.addTransaction(ipbus::enums::transactions::Write, cmd.data.frame.address, &cmd.data.frame.data, nullptr, 1);
        }
          break;
        case Swt::TransactionType::RMWsum: {
          responseData = m_fifo.prepareResponseFrame(cmd.data.frame);
          m_request.addTransaction(ipbus::enums::transactions::RMWsum, cmd.data.frame.address, &cmd.data.frame.data, responseData);
        }
          break;
        case Swt::TransactionType::RMWbitsAnd: {
          expectRmwOr = true;
          wordsToReadByNextCmd += cmd.data.frame.responseSize();
          continue;
        }
          break;
        case Swt::TransactionType::RMWbitsOr: {
          if (expectRmwOr == false) {
            throw std::runtime_error("SWT - RMW bits: received RMW OR without preceeding RMW AND!");
          } else if (cmd.data.frame.address != m_cmdBuffer(1).data.frame.address) {
            throw std::runtime_error("SWT - RMW bits: received RMW OR on address different than in preceeding RMW AND!");
          }
          expectRmwOr = false;
          responseData = m_fifo.prepareResponseFrame(cmd.data.frame);
          buffer[0] = m_cmdBuffer(1).data.frame.data;
          buffer[1] = cmd.data.frame.data;
          m_request.addTransaction(ipbus::enums::transactions::RMWbits, cmd.data.frame.address, buffer, responseData);
        }
          break;
        case Swt::TransactionType::BlockReadIncrement:
        case Swt::TransactionType::BlockReadNonIncrement:{
          if(executeTransactions() == false){
            failure = true;
          } else {
            failure = !(readBlock(cmd.data.frame));
            executeTransactionsOnNextRead = true;
          }
        }
          break;
        default:
          break;
      }// SWT switch
      wordsToReadByNextCmd += cmd.data.frame.responseSize();
    } 
      break;

    default:
      break;
    }

    if(expectRmwOr == true){
        throw std::runtime_error("SWT - RMW bits: received RMW AND without following RMW OR!");
    }  
  } // CRU command switch

  if(failure == false){
      failure = !(executeTransactions());
  }
  return !failure;
}

bool SwtLink::readBlock(const Swt& frame)
{
  constexpr uint32_t maxPacketPayload = ipbus::maxPacket - 3;

  if(frame.data > 1023){
    BOOST_LOG_TRIVIAL(error) << "Exceeded maximum block size (1023); received request for " << frame.data << " words";
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
      for(uint32_t idx = 0 ; idx < size; idx++){
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

} // namespace fit_swt