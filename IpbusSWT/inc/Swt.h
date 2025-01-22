#ifndef SWT_H
#define SWT_H

#include <cstdint>
#include <string>

namespace fit_swt
{

struct Swt {
  uint16_t mode;
  uint32_t address;
  uint32_t data;

  Swt() = default;
  Swt(uint16_t mode_, uint32_t address_, uint32_t data_ ):data(data_), address(address_), mode(mode_) {}
  Swt(const char* str);

  enum class TransactionType { Read,
                               Write,
                               RMWbits,
                               RMWsum,
                               BlockReadIncrement,
                               BlockReadNonIncrement };
  TransactionType getTransactionType() const;
};
} // namespace fit_swt

#endif