#include "AlfIPbus.h"
#include <chrono>
#include <thread>
#include <ctime>

int main(int argc, const char** argv)
{
  if (argc < 4)
    return -1;

  AlfIPbus alf(argv[1]);
  alf.initLink(argv[2], std::stoi(argv[3]));
  alf.startServer();
  return 0;
}