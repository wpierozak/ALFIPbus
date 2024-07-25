// #include "AlfIPbus.h"
// #include <chrono>
// #include <thread>
// #include <ctime>

#define BOOST_LOG_DYN_LINK 1

#include <boost/log/trivial.hpp>
#include <boost/program_options.hpp>

#include <iostream>
#include <string>
#include <vector>

namespace po = boost::program_options;

int main(int argc, const char** argv)
{
  std::string name;
  std::vector<std::string> linkAddresses;

  // Declare the supported options.
  po::options_description desc("Allowed options");
  desc.add_options()("help,h", "see available options")
    ("name,n", po::value<std::string>(&name)->default_value("ALF_IPBUS"), "set server name")
    ("link-addr,l", po::value<std::vector<std::string>>(&linkAddresses),
      "set the IP address and port for consecutive links (can be used multiple times)."
      "\nFormat: [IP address]:[Port number]");

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << desc << "\n";
    return 0;
  }

  for(auto& addr : linkAddresses) {
    std::cout << addr << '\n';
  }

  BOOST_LOG_TRIVIAL(info) << "Starting Alf";

  // if (argc < 4)
  //   return -1;

  // AlfIPbus alf(argv[1]);
  // alf.initLink(argv[2], std::stoi(argv[3]));
  // alf.startServer();
  return 0;
}