#ifndef SWT_UTILS_H
#define SWT_UTILS_H

#include <string>
#include <vector>
#include <stdexcept>
#include<cstdint>
#include<string>
#include"Swt.h"

namespace fit_swt
{
namespace utils
{

std::vector<std::string> splitString(const std::string& text, std::string by);
char hexToChar(uint8_t hex);
uint8_t charToHex(char ch);
uint8_t stringToByte(char c1, char c2);

template<typename T>
std::string toASCII(const T& integer)
{
    const uint8_t* buffer = (const uint8_t*) &buffer;
    std::string result;
    for(int i = sizeof(T)-1; i >= 0; i--){
        result.push_back(hexToChar(buffer[i] >> 4));
        result.push_back(hexToChar(buffer[i] & 0xF));
    }
    return result;
}

template< size_t N >
constexpr size_t length( char const (&)[N] )
{
  return N-1;
}

class ErrorMessage
  {
    public:
    ErrorMessage(uint32_t frameIdx, fit_swt::Swt frame, std::string message);
    ErrorMessage(std::string message="");

    operator std::string() const{
      return mess;
    }

    ErrorMessage& operator<<(const std::string& ss){
      mess += ss;
      return *this;
    }
    ErrorMessage& operator<<(uint32_t uu){
      mess += std::to_string(uu);
      return *this;
    }

    private:
    std::string mess;
};

} // namespace utils
} // namespace fit_swt

#endif