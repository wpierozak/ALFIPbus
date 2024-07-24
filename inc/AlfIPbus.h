#include "SwtLink.h"
#include <list>

class AlfIPbus
{
 public:
  AlfIPbus(std::string name);
  void initLink(std::string remoteAddress, int rport, int lport = 0);

  void startServer();

  bool m_work;

 private:
  void mainLoop();
  std::string m_serverName;
  std::list<fit_swt::SwtLink> m_links;

  boost::asio::io_context m_ioContext;
};