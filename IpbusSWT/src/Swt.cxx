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

std::string wordToString(Word w)
{
  std::string result;
  result += utils::hexToChar(w.bytes.bytes[3] >> 4);
  result += utils::hexToChar(w.bytes.bytes[3] & 0x0F);
  result += utils::hexToChar(w.bytes.bytes[2] >> 4);
  result += utils::hexToChar(w.bytes.bytes[2] & 0x0F);
  result += utils::hexToChar(w.bytes.bytes[1] >> 4);
  result += utils::hexToChar(w.bytes.bytes[1] & 0x0F);
  result += utils::hexToChar(w.bytes.bytes[0] >> 4);
  result += utils::hexToChar(w.bytes.bytes[0] & 0x0F);
  return result;
}

std::string halfWordToString(HalfWord h)
{
  std::string result;
  result += utils::hexToChar(h.bytes.bytes[1] >> 4);
  result += utils::hexToChar(h.bytes.bytes[1] & 0x0F);
  result += utils::hexToChar(h.bytes.bytes[0] >> 4);
  result += utils::hexToChar(h.bytes.bytes[0] & 0x0F);
  return result;
}

/*      SWT     */

Swt::TransactionType Swt::getTransactionType() const
{
  switch (mode) {
    case 0:
      return TransactionType::Read;
    case 1:
      return TransactionType::Write;
    case 2:
    case 3:
      return TransactionType::RMWbits;
    case 4:
      return TransactionType::RMWsum;
    case 8:
      return TransactionType::BlockReadIncrement;
    case 9:
      return TransactionType::BlockReadNonIncrement;
    default:
      BOOST_THROW_EXCEPTION(std::runtime_error("Unknown transaction type " + std::to_string(mode)));
  }
}

Swt stringToSwt(const char* str)
{
  Swt frame;

  frame.data = (static_cast<uint32_t>(utils::stringToByte(str[11], str[12])) << 24) + (static_cast<uint32_t>(utils::stringToByte(str[13], str[14])) << 16) + (static_cast<uint32_t>(utils::stringToByte(str[15], str[16])) << 8) + static_cast<uint32_t>(utils::stringToByte(str[17], str[18]));

  frame.address = (static_cast<uint32_t>(utils::stringToByte(str[3], str[4])) << 24) + (static_cast<uint32_t>(utils::stringToByte(str[5], str[6])) << 16) + (static_cast<uint32_t>(utils::stringToByte(str[7], str[8])) << 8) + static_cast<uint32_t>(utils::stringToByte(str[9], str[10]));

  frame.mode = static_cast<uint16_t>(utils::stringToByte(str[1], str[2]));

  return frame;
}

} // namespace fit_swt
