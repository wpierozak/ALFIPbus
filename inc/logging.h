#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

#include <iostream>

namespace logging
{
  namespace bl = boost::log;

  void init()
  {
    bl::core::get()->set_filter(
    bl::trivial::severity >= bl::trivial::info);
  }
} // namespace logging