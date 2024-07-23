#include <cstdint>
#include <string>
#include <cstring>
#include <stdexcept>
#include <boost/throw_exception.hpp>

#include "utils.h"
#include "Swt.h"

namespace fit_swt
{

/*      HEX     */

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
  return 16 * charToHex(c1) + charToHex(c2);
}

std::string wordToString(Word w)
{
  std::string result;
  result += hexToChar(w.bytes.bytes[3] >> 4);
  result += hexToChar(w.bytes.bytes[3] & 0x0F);
  result += hexToChar(w.bytes.bytes[2] >> 4);
  result += hexToChar(w.bytes.bytes[2] & 0x0F);
  result += hexToChar(w.bytes.bytes[1] >> 4);
  result += hexToChar(w.bytes.bytes[1] & 0x0F);
  result += hexToChar(w.bytes.bytes[0] >> 4);
  result += hexToChar(w.bytes.bytes[0] & 0x0F);
  return result;
}

std::string halfWordToString(HalfWord h)
{
  std::string result;
  result += hexToChar(h.bytes.bytes[1] >> 4);
  result += hexToChar(h.bytes.bytes[1] & 0x0F);
  result += hexToChar(h.bytes.bytes[0] >> 4);
  result += hexToChar(h.bytes.bytes[0] & 0x0F);
  return result;
}

/*      SWT     */

Swt stringToSwt(const char* str)
{
  if (std::strlen(str) != 19) {
    BOOST_THROW_EXCEPTION(std::runtime_error("SWT string is too short - received: " + std::to_string(strlen(str)) + " chars"));
  }

  Swt frame;

  frame.data = (static_cast<uint32_t>(stringToByte(str[11], str[12])) << 24) + (static_cast<uint32_t>(stringToByte(str[13], str[14])) << 16) + (static_cast<uint32_t>(stringToByte(str[15], str[16])) << 8) + static_cast<uint32_t>(stringToByte(str[17], str[18]));

  frame.address = (static_cast<uint32_t>(stringToByte(str[3], str[4])) << 24) + (static_cast<uint32_t>(stringToByte(str[5], str[6])) << 16) + (static_cast<uint32_t>(stringToByte(str[7], str[8])) << 8) + static_cast<uint32_t>(stringToByte(str[9], str[10]));

  frame.mode = (static_cast<uint32_t>(charToHex(str[0])) << 8) + static_cast<uint16_t>(stringToByte(str[1], str[2]));

  return frame;
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

} // namespace fit_swt
