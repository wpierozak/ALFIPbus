#include "SwtElectronics.h"
#include <dim/dis.hxx>
#include <chrono>
#include <thread>
#include <ctime>
int main(int argc, const char** argv)
{

  boost::asio::io_context io_service;

  SwtElectronics target(argv[2], io_service, "172.20.75.175", 50001);
  target.debug_mode(IPbusTarget::DebugMode::Full);

  DimServer::start(argv[1]);
  for (int i = 0; i < std::stoi(argv[3]); i++) {
    std::this_thread::sleep_for(std::chrono::seconds(1000));
  }

  return 0;
}