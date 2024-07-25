// #include "AlfIPbus.h"
// #include <chrono>
// #include <thread>
// #include <ctime>

#define BOOST_LOG_DYN_LINK 1

#include "../inc/AlfConfig.h"

#include <boost/log/trivial.hpp>

#include <iostream>
#include <string>
#include <vector>

int main(int argc, const char** argv)
{
  AlfConfig(argc, argv);

  BOOST_LOG_TRIVIAL(info) << "Starting Alf";

  // AlfIPbus alf(argv[1]);
  // alf.initLink(argv[2], std::stoi(argv[3]));
  // alf.startServer();
  return 0;
}