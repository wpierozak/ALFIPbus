#ifndef SWT_UTILS_H
#define SWT_UTILS_H

#include <string>
#include <vector>
#include <stdexcept>
#include<cstdint>
#include<string>
#include <limits>
#include"Swt.h"

namespace fit_swt
{
namespace utils
{

std::vector<std::string> splitString(const std::string& text, std::string by);
char hexToChar(uint8_t hex);
uint8_t charToHex(char ch);
uint8_t stringToByte(char c1, char c2);

inline const char* findC(const char* str, char c)
{
  const char* pos = str;
  for(pos = str; *pos != '\0'; pos++){
    if(*pos == c) {
      break;
    }
  }
  return pos;
}

template< size_t N >
constexpr size_t length( char const (&)[N] )
{
  return N-1;
}

} // namespace utils
} // namespace fit_swt

#endif