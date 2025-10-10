#ifndef SWT_UTILS_H
#define SWT_UTILS_H

#include <string>
#include <vector>
#include <stdexcept>
#include<cstdint>
#include<string>
#include <limits>
#include <charconv>
#include"Swt.h"

namespace utils
{
  inline bool parseUnsignedInt(const char* beg, const char* end, uint32_t & dest)
  {
    return std::from_chars(beg, end, dest).ptr == end;
  }
}

namespace fit_swt
{
namespace utils
{

char hexToChar(uint8_t hex);
uint8_t charToHex(char ch);
uint8_t stringToByte(char c1, char c2);

template< size_t N >
constexpr size_t length( char const (&)[N] )
{
  return N-1;
}

} // namespace utils
} // namespace fit_swt

#endif