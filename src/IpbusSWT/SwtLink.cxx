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

  m_fredResponse = "";

  if (!parseFrames(swtSequence)) {
    sendFailure();
    return;
  }

  if (interpretFrames()) {
    sendResponse();
  } else {
    sendFailure();
  }
}

bool SwtLink::parseFrames(const char* request)
{
  m_frames.clear();
  m_reqType.clear();

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

  return true;
}


bool SwtLink::interpretFrames()
{
  uint32_t buffer[2];
  m_lineBeg = 0;
  m_lineEnd = 0;

  m_request.reset();

  for (int i = 0; i < m_commands.size(); i++) {
    
    if (m_request.getSize() + m_packetPadding >= ipbus::maxPacket) {
      if(transceive(m_request, m_response))
      {
        writeToResponse();
        m_request.reset();
        m_lineBeg = i;
      }
      else
      {
        m_request.reset();
        return false;
      }
    }

    m_lineEnd++;

    if (m_commands[i].type == CruCommand::Type::Read) {
      continue;
    }

    Swt& frame = m_commands[i].frame;

    switch (frame.getTransactionType()) {
      case Swt::TransactionType::Read:
        m_request.addTransaction(ipbus::enums::transactions::Read, frame.address, &frame.data, &frame.data, 1);
        break;

      case Swt::TransactionType::Write:
        m_request.addTransaction(ipbus::enums::transactions::Write, frame.address, &frame.data, &frame.data, 1);
        break;

      case Swt::TransactionType::RMWbits:
        if (frame.mode != 2) {
          BOOST_LOG_TRIVIAL(error) << "SWT sequence failed (" << i << "): " << "RMWbits failed: first frame is not the AND frame" << std::endl;
          return false;
        }
        if (i + 1 >= m_frames.size()) {
          BOOST_LOG_TRIVIAL(error) << "SWT sequence failed (" << i << "): " << "RMWbits failed: second frame has been not received" << std::endl;
          return false;
        }
        if (m_commands[i + 1].frame.data == EmptyData && m_commands[i + 1].frame.address == EmptyAddress && m_commands[i + 1].frame.mode == EmptyMode) {
          if (i + 2 >= m_frames.size()) {
            BOOST_LOG_TRIVIAL(error) << "SWT sequence failed (" << i << "): "<< "RMWbits failed: second frame has been not received" << std::endl;
            return false;
          }
          if ((m_commands[i + 2].frame.mode) != 3) {
            BOOST_LOG_TRIVIAL(error) << "SWT sequence failed (" << i << "): " << "RMWbits failed: invalid second frame - mode: " << m_commands[i + 2].frame.mode << std::endl;
            return false;
          }
          buffer[0] = frame.data;
          buffer[1] = m_commands[i + 2].frame.data;
          m_request.addTransaction(ipbus::enums::transactions::RMWbits, frame.address, buffer, &frame.data);
          i += 2;
          m_lineEnd += 2;
        } else {
          if ((m_commands[i + 1].frame.mode) != 3) {
            BOOST_LOG_TRIVIAL(error) << "SWT sequence failed (" << i << "): " << "RMWbits failed: invalid second frame - mode: " << m_commands[i + 1].frame.mode << std::endl;
            return false;
          }
          buffer[0] = frame.data;
          buffer[1] = m_commands[i + 1].frame.data;
          m_request.addTransaction(ipbus::enums::transactions::RMWbits, frame.address, buffer, &frame.data);
          i += 1;
          m_lineEnd += 1;
        }

        break;

      case Swt::TransactionType::RMWsum:
        m_request.addTransaction(ipbus::enums::transactions::RMWsum, frame.address, &frame.data, &frame.data);
        break;

      case Swt::TransactionType::BlockReadIncrement:
      case Swt::TransactionType::BlockReadNonIncrement:
      {
        bool success = readBlock(frame, i);
        if(!success){
          return false;
        }
        i += frame.data;
      }
        break;
      default:
      break;
    }
  }

  if(transceive(m_request, m_response))
  {
    writeToResponse();
    m_request.reset();
    return true;
  }
  else
  {
    m_request.reset();
    return false;
  }

}

bool SwtLink::readBlock(const Swt& frame, uint32_t frameIdx)
{
  constexpr uint32_t maxPacketPayload = ipbus::maxPacket - 3;

  if(frame.data > 1024){
    BOOST_LOG_TRIVIAL(error) << "Exceeded maximum block size (1024); received request for " << frame.data << " words";
    return false;
  }
  
    if(transceive(m_request, m_response)){
        writeToResponse();
        m_request.reset();
        m_lineBeg = frameIdx + 1;
    }
    else{
      m_request.reset();
      return false;
    }

    bool increment = (frame.getTransactionType() == Swt::TransactionType::BlockReadIncrement);
    auto transactionType = (increment) ? ipbus::enums::transactions::Read : ipbus::enums::transactions::NonIncrementingRead;
    uint32_t currentAddress = frame.address;
    uint32_t wordRead = 0;
    uint32_t readCommands = 0;

    std::array<Swt,maxPacketPayload> outputFrames;
    uint32_t ipbusOutputBuffer[maxPacketPayload];

    uint32_t offset = frame.data % maxPacketPayload;

    if(offset > 0){
      uint32_t sizeA = (offset>255) ? maxPacketPayload/2: offset;
      uint32_t sizeB = offset - sizeA;
      m_request.addTransaction(transactionType, frame.address, nullptr,  ipbusOutputBuffer, sizeA);
      if(offset > 255){
        m_request.addTransaction(transactionType, frame.address + sizeA*increment, nullptr, ipbusOutputBuffer+sizeA, sizeB);
      }
      if(transceive(m_request, m_response))
      {
        for(uint32_t idx = 0; idx < offset; idx++){
          outputFrames[idx] = Swt{frame.mode, currentAddress, ipbusOutputBuffer[idx]};
          if(increment){
            currentAddress++;
          }
          wordRead++;
        }
        m_request.reset();
      }
      else{
        m_request.reset();
        return false;
      }

      readCommands += writeBlockReadResponse(outputFrames.data(),offset);
      if(readCommands < wordRead){
          utils::ErrorMessage mess;
          mess << "Unsufficient numeber of read command to retrieve block read results; read " << wordRead << " words;";
          mess << " received " << readCommands << " read commands";
          reportError(std::move(mess));
          return false;
      }

    }

    while(wordRead < frame.data){
      uint32_t sizeA = maxPacketPayload/2;
      uint32_t sizeB = maxPacketPayload - sizeA;
      m_request.addTransaction(transactionType, currentAddress, nullptr,  ipbusOutputBuffer, sizeA);
      m_request.addTransaction(transactionType, currentAddress + sizeA*increment, nullptr, ipbusOutputBuffer+sizeA, sizeB);
      if(transceive(m_request, m_response))
      {
        for( uint32_t idx = 0; idx < maxPacketPayload; idx++){
          outputFrames[idx] = Swt{frame.mode, currentAddress, ipbusOutputBuffer[idx]};
          if(increment){
            currentAddress++;
          }
          wordRead++;
        }
        m_request.reset();
      }
      else
      {
        m_request.reset();
        return false;
      }

      readCommands += writeBlockReadResponse(outputFrames.data(), ipbus::maxPacket-3);
      if(readCommands < wordRead){
          utils::ErrorMessage mess;
          mess << "Unsufficient numeber of read command to retrieve block read results; read " << wordRead << " words;";
          mess << " received " << readCommands << " read commands";
          reportError(std::move(mess));
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


void SwtLink::writeToResponse()
{
  for (int i = m_lineBeg; i < m_lineEnd && i < m_frames.size(); i++) {
    if (m_commands[i].type == CruCommand::Type::Read) {
      writeFrame(m_commands[i - 1].frame);
      m_fredResponse += "\n";
    } else {
      m_fredResponse += "0\n";
    }
  }
}

uint32_t SwtLink::writeBlockReadResponse(const Swt* blockResponse, uint32_t endFrameIdxOffset)
{
  if(endFrameIdxOffset){
    m_lineEnd += endFrameIdxOffset;
  }

  uint32_t wroteFrames = 0;

  for (int i = m_lineBeg; i < m_lineEnd && i < m_frames.size(); i++) {
    if (m_commands[i].type == CruCommand::Type::Read) {
      writeFrame(blockResponse[i-m_lineBeg]);
      m_fredResponse += "\n";
    } else {
      return wroteFrames;
    } 
    wroteFrames++;
  }

  m_lineBeg += wroteFrames;
  m_lineEnd = m_lineBeg;

  return wroteFrames;
}

void SwtLink::writeFrame(const Swt& frame)
{
  m_fredResponse += "0x";
  const uint8_t* buffer = (const uint8_t*)&frame.mode;
  m_fredResponse.push_back(utils::hexToChar(buffer[1] & 0x0F));
  m_fredResponse.push_back(utils::hexToChar(buffer[0] >> 4));
  m_fredResponse.push_back(utils::hexToChar(buffer[0] & 0x0F));
  buffer = (const uint8_t*)&frame.address;
  for(int idx = 3; idx >= 0; idx--){
    m_fredResponse.push_back(utils::hexToChar(buffer[idx] >> 4));
    m_fredResponse.push_back(utils::hexToChar(buffer[idx] & 0x0F));
  }
  buffer = (const uint8_t*)&frame.data;
  for(int idx = 3; idx >= 0; idx--){
    m_fredResponse.push_back(utils::hexToChar(buffer[idx] >> 4));
    m_fredResponse.push_back(utils::hexToChar(buffer[idx] & 0x0F));
  }
}


void SwtLink::sendFailure()
{
  BOOST_LOG_TRIVIAL(error) << "Request execution failed";
  m_fredResponse = "failure\n" + m_fredResponse;
  setData(m_fredResponse.c_str());
}

void SwtLink::setPacketPadding(int padding)
{
  m_packetPadding = padding;
}

int SwtLink::getPacketPadding() const
{
  return m_packetPadding;
}


} // namespace fit_swt