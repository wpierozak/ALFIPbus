// #include "AlfIPbus.h"
// #include <chrono>
// #include <thread>
// #include <ctime>

#define BOOST_LOG_DYN_LINK 1

#include "../inc/AlfConfig.h"
#include "../inc/AlfIPbus.h"

#include <boost/log/trivial.hpp>

#include <iostream>
#include <string>
#include <vector>

int main(int argc, const char** argv)
{
  AlfConfig cfg(argc, argv);

  AlfIPbus alf(cfg);
  alf.initLinks();
  alf.startServer();
  return 0;
}