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

void init()
{
  bl::add_common_attributes();

  bl::add_file_log(bl::keywords::file_name = "alf_ipbus.log",
                   bl::keywords::auto_flush = true,
                   bl::keywords::format = (ble::stream
                                           << "["
                                           << ble::format_date_time<boost::posix_time::ptime>("TimeStamp", "%H:%M:%S.%f")
                                           << "] "
                                           << "[" << std::setw(7) << bl::trivial::severity << "]:\t"
                                           << ble::smessage));

  bl::add_console_log(std::cout,
                      bl::keywords::format = (ble::stream
                                              << "["
                                              << ble::format_date_time<boost::posix_time::ptime>("TimeStamp", "%H:%M:%S.%f")
                                              << "] "
                                              << "[" << std::setw(7) << bl::trivial::severity << "]:\t"
                                              << ble::smessage));

#ifndef DEBUG
  bl::core::get()->set_filter(
    bl::trivial::severity >= bl::trivial::info);
#endif
}
} // namespace logging