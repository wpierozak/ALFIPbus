#ifndef ALF_CONFIG_H
#define ALF_CONFIG_H

#include <string>
#include <vector>
#include <stdexcept>
#include <iostream>

#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include <boost/log/trivial.hpp>

struct AlfConfig
{
  struct Link {
    std::string address;
    unsigned rport;

    Link(const std::string& arg)
    {
      size_t colonPos = arg.find(':');
      if (colonPos != std::string::npos) {
        address = arg.substr(0, colonPos);
        rport = std::stoi(arg.substr(colonPos + 1));
      } else {
        throw std::invalid_argument("Input string does not contain a colon.");
      }
    }

    operator std::string() {
      return "addr: " + address + ", rport: " + std::to_string(rport);
    }
  };

  std::string name;
  std::vector<Link> links;

  AlfConfig(int argc, const char** argv) {
    std::vector<std::string> linkArgs;

    // Declare the supported options.
    po::options_description desc("Allowed options");
    desc.add_options()
      ("help,h", "see available options")
      ("name,n", po::value<std::string>(&name), "set server name")
      ("link,l", po::value<std::vector<std::string>>(&linkArgs),
       "set the IP address and port for consecutive links (can be used multiple times)."
       "\nFormat: [IP address]:[Port number]");

    // Parse the provided arguments
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    // Handle options
    if (vm.count("help")) {
      std::cout << desc << "\n";
      exit(0);
    }

    if(!vm.count("name")) {
      std::cerr << "Error: No name provided\n";
      exit(1);
    }

    if(!vm.count("link")) {
      std::cerr << "Error: No link address provided\n";
      exit(1);
    }

    for (const auto& arg : linkArgs)
      links.push_back(arg);

    std::cout << static_cast<std::string>(*this) << "\n";
  }

  operator std::string() {
    std::string result = "ALF IPbus\nNAME: " + name + "\n";
    for(size_t i = 0; i < links.size(); i++)
      result += "LINK_" + std::to_string(i) + ": " + static_cast<std::string>(links[i]) + "\n";
    return result;
  }
};

#endif