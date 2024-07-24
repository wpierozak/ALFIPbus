#include "AlfIPbus.h"

AlfIPbus::AlfIPbus(std::string name)
  : m_serverName(name), m_work(true)
{
}

void AlfIPbus::initLink(std::string remoteAddress, int rport, int lport)
{
  std::string serial = m_serverName + "/SERIAL_0/LINK_";
  std::string swtSeq = "/SWT_SEQUENCE";
  int number = m_links.size();
  m_links.emplace_back(serial + std::to_string(number) + swtSeq, m_ioContext, remoteAddress, rport, lport);
}

void AlfIPbus::startServer()
{
  DimServer::start(m_serverName.c_str());
  mainLoop();
}

void AlfIPbus::mainLoop()
{
  while (m_work) {
    std::this_thread::sleep_for(std::chrono::seconds(1000));
  }
}