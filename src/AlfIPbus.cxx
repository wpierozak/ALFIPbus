#include "AlfIPbus.h"

AlfIPbus::AlfIPbus(const AlfConfig& cfg)
  : m_cfg(cfg), m_work(true)
{
}

void AlfIPbus::initLinks()
{
  for (size_t i = 0; i < m_cfg.links.size(); i++) {
    std::string serviceName = m_cfg.name + "/SERIAL_0/LINK_" + std::to_string(i) + "/SWT_SEQUENCE";
    std::cout << "Creating service with name " + serviceName + "\n";
    m_swtLinks.emplace_back(serviceName, m_ioContext, m_cfg.links[i].address, m_cfg.links[i].rport, 0);
    std::cout << "Created service with name " + serviceName + "\n";
  }
}

void AlfIPbus::startServer()
{
  DimServer::start(m_cfg.name.c_str());
  mainLoop();
}

void AlfIPbus::mainLoop()
{
  while (m_work) {
    std::this_thread::sleep_for(std::chrono::seconds(1000));
  }
}