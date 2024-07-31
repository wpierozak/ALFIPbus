#ifndef ALF_CONFIG_H
#define ALF_CONFIG_H

#include <string>
#include <vector>
#include <stdexcept>
#include <iostream>

#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include <boost/log/trivial.hpp>

struct AlfConfig {
  struct Link {
    std::string address;
    unsigned rport;

    Link(const std::string& arg);

    operator std::string();
  };

  std::string name;
  std::string logFilename;
  int timeout;
  std::vector<Link> links;

  AlfConfig(int argc, const char** argv);

  operator std::string();
};

#endif