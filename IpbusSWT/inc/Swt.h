#ifndef SWT_H
#define SWT_H

#include <cstdint>
#include <string>

namespace fit_swt
{

union HalfWord {
  struct fields {
    uint8_t bytes[2];
  } bytes;
  uint16_t data;
};

union Word {
  struct
  {
    uint8_t bytes[4];
    /* data */
  } bytes;
  uint32_t data;
};

std::string wordToString(Word w);
std::string halfWordToString(HalfWord);

struct Swt {
  uint32_t data;
  uint32_t address;
  uint16_t mode;

  enum class TransactionType { Read,
                               Write,
                               RMWbits,
                               RMWsum,
                               BlockReadIncrement,
                               BlockReadNonIncrement };
  TransactionType getTransactionType() const;
};

Swt stringToSwt(const char* str);

} // namespace fit_swt

#endif