#include "AlfConfig.h"
#include "../IpbusSWT/inc/SwtLink.h"
#include <list>

class AlfIPbus
{
 public:
  AlfIPbus(const AlfConfig& cfg);
  void initLinks();

  void startServer();

  bool m_work;

 private:
  void mainLoop();
  AlfConfig m_cfg;
  std::list<fit_swt::SwtLink> m_swtLinks;

  boost::asio::io_context m_ioContext;
};