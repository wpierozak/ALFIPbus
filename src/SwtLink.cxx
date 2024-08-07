#include <string>
#include <boost/exception/diagnostic_information.hpp>

#include "SwtLink.h"
#include "utils.h"

namespace fit_swt
{

void SwtLink::rpcHandler()
{
  BOOST_LOG_TRIVIAL(info) << "Received request";
  processRequest(getString());
}

void SwtLink::processRequest(const char* swtSequence)
{
  m_fredResponse = "";
  splitLines(swtSequence);

  if (!parseFrames()) {
    sendFailure();
    return;
  }

  if (interpretFrames()) {
    sendResponse();
  } else {
    sendFailure();
  }
}

void SwtLink::splitLines(const char* swtSequence)
{
  std::string swtStr = swtSequence;
  m_lines = utils::splitString(swtStr, "\n");
}

bool SwtLink::parseFrames()
{
  m_frames.clear();
  if (m_lines[0] != "reset") {
    BOOST_LOG_TRIVIAL(error) << "Sequence parsing failed: missing reset word";
    return false;
  }
  m_lines.erase(m_lines.begin());

  for (auto line : m_lines) {
    if (line.find("read") != std::string::npos) {
      m_frames.push_back({ EmptyData, EmptyAddress, EmptyMode });
      continue;
    }

    line = line.substr(line.find("0x") + 2);
    line = line.substr(0, line.find(','));

    try {
      m_frames.emplace_back(stringToSwt(line.c_str()));
    } catch (const std::exception& e) {
      BOOST_LOG_TRIVIAL(error) << "Sequence parsing failed: " << boost::diagnostic_information(e);
      return false;
    }
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
      m_request.markPartialSwt();
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
        m_request.addTransaction(ipbus::DataRead, m_frames[i].address, &m_frames[i].data, &m_frames[i].data, 1);
        break;

      case Swt::TransactionType::Write:
        m_request.addTransaction(ipbus::DataWrite, m_frames[i].address, &m_frames[i].data, &m_frames[i].data, 1);
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
          m_request.addTransaction(ipbus::RMWbits, m_frames[i].address, buffer, &m_frames[i].data);
          i += 2;
          m_lineEnd += 2;
        } else {
          if ((m_frames[i + 1].mode) != 3) {
            BOOST_LOG_TRIVIAL(error) << "SWT sequence failed (" << i << "): " << "RMWbits failed: invalid second frame - mode: " << m_frames[i + 1].mode << std::endl;
            return false;
          }
          buffer[0] = m_frames[i].data;
          buffer[1] = m_frames[i + 1].data;
          m_request.addTransaction(ipbus::RMWbits, m_frames[i].address, buffer, &m_frames[i].data);
          i += 1;
          m_lineEnd += 1;
        }

        break;

      case Swt::TransactionType::RMWsum:
        m_request.addTransaction(ipbus::RMWsum, m_frames[i].address, &m_frames[i].data, &m_frames[i].data);
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


void SwtLink::sendResponse()
{
  m_fredResponse = "success\n" + m_fredResponse;
  setData(m_fredResponse.c_str());
}


void SwtLink::writeToResponse()
{
  for (int i = m_lineBeg; i < m_lineEnd; i++) {
    if (m_lines[i] == "read") {
      writeFrame(m_frames[i - 1]);
      m_fredResponse += "\n";
      continue;
    } else if (m_lines[i].find("write") != std::string::npos) {
      m_fredResponse += "0\n";
    }
  }
}

void SwtLink::writeFrame(Swt frame)
{
  m_fredResponse += "0x";
  HalfWord h;
  h.data = frame.mode;

  std::string mode = halfWordToString(h);
  mode = mode.substr(1);
  m_fredResponse += mode;

  Word w;
  w.data = frame.address;
  m_fredResponse += wordToString(w);
  w.data = frame.data;
  m_fredResponse += wordToString(w);
}

void SwtLink::sendFailure()
{
  BOOST_LOG_TRIVIAL(error) << "Request execution failed";
  setData("failure");
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