#ifndef ALF_IPBUS_H
#define ALF_IPBUS_H

#include "AlfConfig.h"
#include "../IpbusSWT/inc/SwtLink.h"
#include <list>
#include <atomic>

class AlfIPbus
{
 public:
  AlfIPbus(const AlfConfig& cfg);
  void initLinks();

  void startServer();

  static void stop(int);

  ~AlfIPbus();

 private:
  static bool s_running;

  void mainLoop();
  AlfConfig m_cfg;
  std::list<fit_swt::SwtLink> m_swtLinks;

  boost::asio::io_context m_ioContext;
};

#endif
