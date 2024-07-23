#include "SwtLink.h"
#include <dim/dis.hxx>
#include <chrono>
#include <thread>
#include <ctime>

using namespace fit_swt;
int main(int argc, const char** argv)
{

  boost::asio::io_context io_service;

  SwtLink target(argv[2], io_service, "172.20.75.175", 50001);
  target.debugMode(ipbus::IPbusTarget::DebugMode::Full);

  DimServer::start(argv[1]);
  for (int i = 0; i < std::stoi(argv[3]); i++) {
    std::this_thread::sleep_for(std::chrono::seconds(1000));
  }

  return 0;
}