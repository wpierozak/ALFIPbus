#include <string>
#include <boost/exception/diagnostic_information.hpp>

#include "SwtLink.h"
#include "utils.h"

namespace fit_swt
{

void SwtLink::rpcHandler()
{
  BOOST_LOG_TRIVIAL(debug) << "Received request";
  processRequest(getString());
}

void SwtLink::processRequest(const char* swtSequence)
{
  if(std::strlen(swtSequence) == 0)
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

  int64_t beg_ptr = 0;
  int64_t end_ptr = 0;
  int64_t size = 0;

  try
  {

  for(int64_t pos = 0; ; pos++)
  {
    switch(request[pos])
    {    
      case '\0':
      case '\n':
      {
        end_ptr = pos;
      }
      break;

      default:
        continue;
      break;
    }

    size = end_ptr - beg_ptr;

    if(strncmp(request + beg_ptr, "sc_reset", size) == 0)
    {

    }
    else if(strncmp(request + beg_ptr, "read", size) == 0){
      m_frames.emplace_back(Swt{EmptyMode, EmptyAddress, EmptyData});
      m_reqType.emplace_back(RequestType::Read);
    }
    else if(size == 27 && strncmp(request + beg_ptr + 22, "write", 5) == 0){
      m_frames.emplace_back(request + beg_ptr + 2);
      m_reqType.emplace_back(RequestType::Write);
    }
    else{
      char buffer[64];
      std::memcpy(buffer, request + beg_ptr, size);
      buffer[size] = '\0';
      BOOST_LOG_TRIVIAL(error) << "Invalid sequence received: " << buffer;
      return false;
    }
    if(request[pos] == '\0') break;
    beg_ptr = pos + 1;
  }

  } catch (const std::exception& e) {
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

  for (int i = 0; i < m_frames.size(); i++) {
  
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

    if (m_frames[i].data == EmptyData && m_frames[i].address == EmptyAddress && m_frames[i].mode == EmptyMode) {
      continue;
    }

    switch (m_frames[i].getTransactionType()) {
      case Swt::TransactionType::Read:
        m_request.addTransaction(ipbus::enums::transactions::Read, m_frames[i].address, &m_frames[i].data, &m_frames[i].data, 1);
        break;

      case Swt::TransactionType::Write:
        m_request.addTransaction(ipbus::enums::transactions::Write, m_frames[i].address, &m_frames[i].data, &m_frames[i].data, 1);
        break;

      case Swt::TransactionType::RMWbits:
        if (m_frames[i].mode != 2) {
          BOOST_LOG_TRIVIAL(error) << "SWT sequence failed (" << i << "): " << "RMWbits failed: first frame is not the AND frame" << std::endl;
          return false;
        }
        if (i + 1 >= m_frames.size()) {
          BOOST_LOG_TRIVIAL(error) << "SWT sequence failed (" << i << "): " << "RMWbits failed: second frame has been not received" << std::endl;
          return false;
        }
        if (m_frames[i + 1].data == EmptyData && m_frames[i + 1].address == EmptyAddress && m_frames[i + 1].mode == EmptyMode) {
          if (i + 2 >= m_frames.size()) {
            BOOST_LOG_TRIVIAL(error) << "SWT sequence failed (" << i << "): "<< "RMWbits failed: second frame has been not received" << std::endl;
            return false;
          }
          if ((m_frames[i + 2].mode) != 3) {
            BOOST_LOG_TRIVIAL(error) << "SWT sequence failed (" << i << "): " << "RMWbits failed: invalid second frame - mode: " << m_frames[i + 2].mode << std::endl;
            return false;
          }
          buffer[0] = m_frames[i].data;
          buffer[1] = m_frames[i + 2].data;
          m_request.addTransaction(ipbus::enums::transactions::RMWbits, m_frames[i].address, buffer, &m_frames[i].data);
          i += 2;
          m_lineEnd += 2;
        } else {
          if ((m_frames[i + 1].mode) != 3) {
            BOOST_LOG_TRIVIAL(error) << "SWT sequence failed (" << i << "): " << "RMWbits failed: invalid second frame - mode: " << m_frames[i + 1].mode << std::endl;
            return false;
          }
          buffer[0] = m_frames[i].data;
          buffer[1] = m_frames[i + 1].data;
          m_request.addTransaction(ipbus::enums::transactions::RMWbits, m_frames[i].address, buffer, &m_frames[i].data);
          i += 1;
          m_lineEnd += 1;
        }

        break;

      case Swt::TransactionType::RMWsum:
        m_request.addTransaction(ipbus::enums::transactions::RMWsum, m_frames[i].address, &m_frames[i].data, &m_frames[i].data);
        break;

      case Swt::TransactionType::BlockReadIncrement:
      case Swt::TransactionType::BlockReadNonIncrement:
      {
        bool success = readBlock(m_frames[i], i);
        if(!success){
          return false;
        }
        i += m_frames[i].data;
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

    std::vector<Swt> outputFrames;
    outputFrames.reserve(maxPacketPayload);
    uint32_t ipbusOutputBuffer[ipbus::maxPacket];

    uint32_t offset = frame.data % ipbus::maxPacket;

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
          outputFrames.emplace_back(Swt{frame.mode, currentAddress, ipbusOutputBuffer[idx]});
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

      readCommands += writeBlockReadResponse(outputFrames,offset);
      if(readCommands < wordRead){
          utils::ErrorMessage mess;
          mess << "Unsufficient numeber of read command to retrieve block read results; read " << wordRead << " words;";
          mess << " received " << readCommands << " read commands";
          reportError(std::move(mess));
          return false;
      }
      outputFrames.clear();
    }

    while(wordRead < frame.data){
      uint32_t sizeA = maxPacketPayload/2;
      uint32_t sizeB = maxPacketPayload - sizeA;
      m_request.addTransaction(transactionType, currentAddress, nullptr,  ipbusOutputBuffer, sizeA);
      m_request.addTransaction(transactionType, currentAddress + sizeA*increment, nullptr, ipbusOutputBuffer+sizeA, sizeB);
      if(transceive(m_request, m_response))
      {
        for( uint32_t idx = 0; idx < maxPacketPayload; idx++){
          outputFrames.emplace_back(Swt{frame.mode, currentAddress, ipbusOutputBuffer[idx]});
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

      readCommands += writeBlockReadResponse(outputFrames, ipbus::maxPacket-3);
      if(readCommands < wordRead){
          utils::ErrorMessage mess;
          mess << "Unsufficient numeber of read command to retrieve block read results; read " << wordRead << " words;";
          mess << " received " << readCommands << " read commands";
          reportError(std::move(mess));
          return false;
      }
      outputFrames.clear();
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
    if (m_reqType[i] == Read) {
      writeFrame(m_frames[i - 1]);
      m_fredResponse += "\n";
    } else {
      m_fredResponse += "0\n";
    }
  }
}

uint32_t SwtLink::writeBlockReadResponse(const std::vector<Swt> &blockResponse, uint32_t endFrameIdxOffset)
{
  if(endFrameIdxOffset){
    m_lineEnd += endFrameIdxOffset;
  }

  uint32_t wroteFrames = 0;

  for (int i = m_lineBeg; i < m_lineEnd && i < m_frames.size(); i++) {
    if (m_reqType[i] == Read) {
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
  const uint8_t* buffer = (const uint8_t*)&frame;
  m_fredResponse.push_back(utils::hexToChar(buffer[9] & 0x0F));
  for(int idx = 8; idx >= 0; idx--){
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