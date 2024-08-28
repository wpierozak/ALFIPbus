#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <iostream>
#include <iomanip>
#include <sstream>

namespace logging
{
namespace bl = boost::log;
namespace ble = boost::log::expressions;

void coloring_formatter(const boost::log::record_view& record, boost::log::formatting_ostream& stream) {
  auto severity = record[boost::log::trivial::severity];
  assert(severity);

  stream << "[" << bl::extract<boost::posix_time::ptime>("TimeStamp", record) << "] [";

  switch (severity.get()) {
    case bl::trivial::severity_level::trace:
      stream << "\e[35m";
      break;
    case bl::trivial::severity_level::debug:
      stream << "\e[34m";
      break;
    case bl::trivial::severity_level::info:
      stream << "\e[32m";
      break;
    case bl::trivial::severity_level::warning:
      stream << "\e[93m";
      break;
    case bl::trivial::severity_level::error:
      stream << "\e[91m";
      break;
    case bl::trivial::severity_level::fatal:
      stream << "\e[41m";
      break;
  }

  stream << severity;

  if (severity) {
    stream << "\033[0m";
  }

  stream << "] " << record[boost::log::expressions::message];
}

void non_coloring_formatter(const boost::log::record_view& record, boost::log::formatting_ostream& stream) {
  auto severity = record[boost::log::trivial::severity];
  assert(severity);

  stream << "[" << bl::extract<boost::posix_time::ptime>("TimeStamp", record) << "] [";

  stream << severity;

  stream << "] " << record[boost::log::expressions::message];
}

void init(std::string filename = "", bool verbose = false)
{
  bl::add_common_attributes();

  if (filename == "") {
    auto consoleLog = bl::add_console_log(std::cout);
    consoleLog->set_formatter(&coloring_formatter);
  } else {
    auto fileLog = bl::add_file_log(bl::keywords::file_name = filename, bl::keywords::auto_flush = true);
    fileLog->set_formatter(&non_coloring_formatter);
  }

  if(!verbose)
    bl::core::get()->set_filter(bl::trivial::severity >= bl::trivial::info);

  BOOST_LOG_TRIVIAL(debug) << "Debug log mode enabled";
}
} // namespace logging
