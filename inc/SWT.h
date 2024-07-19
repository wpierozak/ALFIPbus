#pragma once
#include<cstdint>
#include<string>

union half_word
{
    struct fields
    {
        uint8_t bytes[2];
    } bytes;
    uint16_t data;
};

union word
{
    struct
    {
        uint8_t bytes[4];
            /* data */
    } bytes;
    uint32_t data;
};

uint8_t charToHex(char ch);
uint8_t string_to_byte(char c1, char c2);
char hexToChar(uint8_t hex);
std::string word_to_string(word w);
std::string half_word_to_string(half_word);

        /*      SWT     */


struct SWT
{
    uint32_t data;
    uint32_t address;
    uint16_t mode;

    enum class TransactionType{READ, WRITE};
    TransactionType getTransactionType() 
    { 
        switch (mode & 0x03)
        {
            case 0: default: return TransactionType::READ;
            case 1: return TransactionType::WRITE;
        }
    }
};

SWT string_to_swt(const char* str);
