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