#include <string>
#include <boost/exception/diagnostic_information.hpp>

#include "SwtLink.h"
#include "utils.h"

namespace fit_swt
{

void SwtLink::rpcHandler()
{
  processRequest(getString());
}

void SwtLink::processRequest(const char* swtSequence)
{
  std::cerr << "Spliiting lines\n";
  splitLines(swtSequence);
  std::cerr << "Parsing...\n";
  parseFrames();
  std::cerr << "Parsing...\n";
  if(interpretFrames())
  {
    execute();
  }
}

void SwtLink::splitLines(const char* swtSequence)
{
  std::string swtStr = swtSequence;
  m_lines = utils::splitString(swtStr, "\n");
}

void SwtLink::parseFrames()
{
  m_frames.clear();
  m_lines.erase(m_lines.begin());

  for (auto frame : m_lines) {
    if (frame.find("write") == std::string::npos)
      continue;

    frame = frame.substr(frame.find("0x") + 2);
    int size = frame.size();
    for (int i = frame.find(','); i < size; i++)
      frame.pop_back();

    try {
      m_frames.emplace_back(stringToSwt(frame.c_str()));
    } catch (const std::exception& e) {
      std::cerr << boost::diagnostic_information(e) << '\n';
    }

  }
}


bool SwtLink::interpretFrames()
{
  for(int i = 0; i < m_frames.size(); i++)
  {
      switch (m_frames[i].getTransactionType())
      {
      case Swt::TransactionType::Read:
        m_packet.addTransaction(ipbus::data_read, m_frames[i].address, &m_frames[i].data, 1);
        break;
      case Swt::TransactionType::Write:
        m_packet.addTransaction(ipbus::data_write,  m_frames[i].address, &m_frames[i].data, 1);
        break;
      
      case Swt::TransactionType::RMWbits:
        if( i + 1 >= m_frames.size())
        {
          return false;
        }
        if((m_frames[i+1].mode & 0x07) != 3)
        {
          return false;
        }
        if(m_rmwbitsBuffers.find(m_frames[i+1].address) != m_rmwbitsBuffers.end())
        {
          return false;
        }
        m_rmwbitsBuffers.emplace(m_frames[i].address, std::array<uint32_t,2>({m_frames[i].data, m_frames[i+1].data}));
        m_packet.addTransaction(ipbus::RMWbits, m_frames[i].address, m_rmwbitsBuffers[m_frames[i].address].data());
        break;
      
      case Swt::TransactionType::RMWsum:

        break;

      default:
        break;
      }
    }

    return true;
}

void SwtLink::execute()
{
  if (transcieve(m_packet)) {
    m_response = "success ";
    for (int i = 0; i < m_lines.size(); i++) {
      if (m_lines[i] == "read") {
        writeFrame(m_frames[i - 1]);
        m_response += "\n";
        continue;
      } else if (m_lines[i].find("write") != std::string::npos) {
        m_response += "0\n";
      }
    }
    setData(m_response.c_str());
  } else {
    setData("failure");
  }
}

void SwtLink::writeFrame(Swt frame)
{
  m_response += "0x";
  HalfWord h;
  h.data = frame.mode;

  std::string mode = halfWordToString(h);
  mode = mode.substr(1);
  m_response += mode;
  Word w;
  w.data = frame.address;
  m_response += wordToString(w);
  w.data = frame.data;
  m_response += wordToString(w);
}

} // namespace fit_swt