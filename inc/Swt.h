#pragma once
#include<cstdint>
#include<string>

union HalfWord
{
    struct fields
    {
        uint8_t bytes[2];
    } bytes;
    uint16_t data;
};

union Word
{
    struct
    {
        uint8_t bytes[4];
            /* data */
    } bytes;
    uint32_t data;
};

uint8_t charToHex(char ch);
uint8_t stringToByte(char c1, char c2);
char hexToChar(uint8_t hex);
std::string wordToString(Word w);
std::string halfWordToString(HalfWord);

        /*      SWT     */


struct Swt
{
    uint32_t data;
    uint32_t address;
    uint16_t mode;

    enum class TransactionType{Read, Write};
    TransactionType getTransactionType() 
    { 
        switch (mode & 0x03)
        {
            case 0: 
                return TransactionType::Read;
            case 1: 
                return TransactionType::Write;
            default: 
                return TransactionType::Read;
        }
    }
};

Swt stringToSwt(const char* str);
