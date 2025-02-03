#include <cstdint>
#include <string>
#include <cstring>
#include <stdexcept>
#include <boost/throw_exception.hpp>

#include "IpbusSWT/utils.h"
#include "IpbusSWT/Swt.h"

namespace fit_swt
{

/*      SWT     */

Swt::TransactionType Swt::type() const
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

Swt::Swt(const char* str)
{
  data = (static_cast<uint32_t>(utils::stringToByte(str[11], str[12])) << 24) 
    + (static_cast<uint32_t>(utils::stringToByte(str[13], str[14])) << 16) 
    + (static_cast<uint32_t>(utils::stringToByte(str[15], str[16])) << 8) 
    + static_cast<uint32_t>(utils::stringToByte(str[17], str[18]));

  address = (static_cast<uint32_t>(utils::stringToByte(str[3], str[4])) << 24) 
    + (static_cast<uint32_t>(utils::stringToByte(str[5], str[6])) << 16) 
    + (static_cast<uint32_t>(utils::stringToByte(str[7], str[8])) << 8) 
    + static_cast<uint32_t>(utils::stringToByte(str[9], str[10]));

  mode = static_cast<uint16_t>(utils::stringToByte(str[1], str[2]));
}

void Swt::appendToString(std::string& dest) const
{
  size_t pos = dest.size();
  dest.resize(pos + SwtStrLen);
  dest[pos++] = '0';
  dest[pos++] = 'x';
  //dest += "0x";
  const uint8_t* buffer = (const uint8_t*)&mode;
  dest[pos++] = utils::hexToChar(buffer[1] & 0x0F); // dest.push_back(utils::hexToChar(buffer[1] & 0x0F));
  dest[pos++] = utils::hexToChar(buffer[0] >> 4);// dest.push_back(utils::hexToChar(buffer[0] >> 4));
  dest[pos++] = utils::hexToChar(buffer[0] & 0x0F); //dest.push_back(utils::hexToChar(buffer[0] & 0x0F));
  buffer = (const uint8_t*)&address;
  for(int idx = 3; idx >= 0; idx--){
    dest[pos++] = utils::hexToChar(buffer[idx] >> 4); //dest.push_back(utils::hexToChar(buffer[idx] >> 4));
    dest[pos++] = utils::hexToChar(buffer[idx] & 0x0F); //dest.push_back(utils::hexToChar(buffer[idx] & 0x0F));
  }
  buffer = (const uint8_t*)&data;
  for(int idx = 3; idx >= 0; idx--){
    dest[pos++] = utils::hexToChar(buffer[idx] >> 4); // dest.push_back(utils::hexToChar(buffer[idx] >> 4));
    dest[pos++] = utils::hexToChar(buffer[idx] & 0x0F); //dest.push_back(utils::hexToChar(buffer[idx] & 0x0F));
  }
}

} // namespace fit_swt
