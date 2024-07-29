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
  static std::atomic<bool> s_running;

  void mainLoop();
  AlfConfig m_cfg;
  std::list<fit_swt::SwtLink> m_swtLinks;

  boost::asio::io_context m_ioContext;
};

std::atomic<bool> AlfIPbus::s_running = false;