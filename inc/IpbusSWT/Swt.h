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

  enum class TransactionType { Read,
                               Write,
                               RMWbits,
                               RMWsum,
                               BlockReadIncrement,
                               BlockReadNonIncrement };
  uint32_t getResponseSize();
  TransactionType getTransactionType() const;
};
} // namespace fit_swt

#endif