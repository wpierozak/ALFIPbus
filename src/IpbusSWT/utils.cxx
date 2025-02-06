#include "IpbusSWT/utils.h"
#include <boost/throw_exception.hpp>

namespace fit_swt
{
namespace utils
{

std::vector<std::string> splitString(const std::string& text, std::string by)
{
  std::vector<std::string> result;
  std::string temp = text;

  while (temp.size()) {
    size_t index = temp.find(by);
    if (index != std::string::npos) {
      result.push_back(temp.substr(0, index));
      temp = temp.substr(index + by.length());
    } else {
      result.push_back(temp);
      temp = "";
      break;
    }
  }

  return result;
}

char hexToChar(uint8_t hex)
{
  switch (hex) {
    case 0:
      return '0';
    case 1:
      return '1';
    case 2:
      return '2';
    case 3:
      return '3';
    case 4:
      return '4';
    case 5:
      return '5';
    case 6:
      return '6';
    case 7:
      return '7';
    case 8:
      return '8';
    case 9:
      return '9';
    case 10:
      return 'A';
    case 11:
      return 'B';
    case 12:
      return 'C';
    case 13:
      return 'D';
    case 14:
      return 'E';
    case 15:
      return 'F';
    default:
      BOOST_THROW_EXCEPTION(std::runtime_error("hexToChar: Value should be within range (0-15)"));
  }
}

uint8_t charToHex(char ch)
{
  switch (ch) {
    case '0':
      return 0;
    case '1':
      return 1;
    case '2':
      return 2;
    case '3':
      return 3;
    case '4':
      return 4;
    case '5':
      return 5;
    case '6':
      return 6;
    case '7':
      return 7;
    case '8':
      return 8;
    case '9':
      return 9;
    case 'A':
    case 'a':
      return 10;
    case 'B':
    case 'b':
      return 11;
    case 'C':
    case 'c':
      return 12;
    case 'D':
    case 'd':
      return 13;
    case 'E':
    case 'e':
      return 14;
    case 'F':
    case 'f':
      return 15;
    default:
      BOOST_THROW_EXCEPTION(std::runtime_error("Invalid hexadecimal character - " + ch));
  }
}

uint8_t stringToByte(char c1, char c2)
{
  return 16 * utils::charToHex(c1) + utils::charToHex(c2);
}

} // namespace utils
} // namespace fit_swt