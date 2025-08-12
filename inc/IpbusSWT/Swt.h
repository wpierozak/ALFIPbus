#ifndef SWT_H
#define SWT_H

#include <cstdint>
#include <string>

namespace fit_swt
{

struct Swt {
  uint32_t data;
  uint32_t address;
  uint16_t mode;

  Swt() = default;
  Swt(uint16_t mode_, uint32_t address_, uint32_t data_ ):data(data_), address(address_), mode(mode_) {}
  Swt(const char* str);

  enum class TransactionType { Read = 0,
                               Write = 1,
                               RMWbitsAnd = 2,
                               RMWbitsOr = 3,
                               RMWsum = 4,
                               BlockReadIncrement = 8,
                               BlockReadNonIncrement = 9 };
  TransactionType type() const;
  bool isBlock() const
  {
    return mode == 8 || mode == 9;
  }
  uint32_t responseSize() const
  {
    switch(mode)
    {
      case 1:
      case 3:
        return 0;
      case 0:
      case 2:
      case 4:
        return 1;
      case 8:
      case 9:
        return data;
    }
    return 0;
  }
  void appendToString(std::string& dest) const;
  static constexpr uint32_t SwtStrLen = 21;
};
} // namespace fit_swt

#endif