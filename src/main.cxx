#define BOOST_LOG_DYN_LINK 1

#include "AlfConfig.h"
#include "AlfIPbus.h"
#include "logging.h"

#include <boost/log/trivial.hpp>

#include <iostream>
#include <string>
#include <vector>

int main(int argc, const char** argv)
{
  logging::init();

  AlfConfig cfg(argc, argv);

  AlfIPbus alf(cfg);
  alf.initLinks();
  alf.startServer();
  return 0;
}