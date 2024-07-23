#include "SwtLink.h"
#include <dim/dic.hxx>
#include <chrono>
#include <thread>

using namespace fit_swt;

int main(int argc, const char** argv)
{
  char bad[] = "BAD";
  DimInfo subscribe(argv[1], bad);
  std::string cafe = argv[3];
  cafe += ",";
  cafe += argv[4];
  std::cout << "Sending comand..." << DimClient::sendCommand(argv[2], cafe.c_str()) << std::endl;
  for (int i = 0; i < 5; i++) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::string respone = subscribe.getString();
    std::cout << "Service status: " << respone << std::endl;
  }
  return 0;
}