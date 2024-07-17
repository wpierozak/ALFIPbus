#pragma once
#include<cstdint>
#include<string>

union half_word
{
    struct fields
    {
        uint8_t first_byte;
        uint8_t second_byte;
    } fields;
    uint16_t data;
};

union word
{
    struct hex
    {
        uint8_t first_byte;
        uint8_t second_byte;
        uint8_t third_byte;
        uint8_t fourth_byte;
            /* data */
    } fields;
    uint32_t data;
};

uint8_t charToHex(char ch);
uint8_t string_to_byte(char c1, char c2);
char hexToChar(uint8_t hex);
        /*      SWT     */


struct SWT
{
    uint32_t data       : 16;
    uint32_t submodule  : 8;
    uint32_t  module     : 7;
    uint32_t mode       : 1;
    uint32_t blank_A    : 32;
    uint32_t blank_B    : 12;
    uint32_t SWT_ID     : 4;
};

SWT string_to_swt(const char* str);
SWT byte_to_swt(const uint8_t* bytes);
void swt_to_byte(SWT swt, uint8_t* bytes);

struct SWT_IPBUS_READY
{
    enum class Type{Read=0, Write=1};
    Type type;
    uint32_t address;
    uint32_t data;
};

SWT_IPBUS_READY swt_ready(SWT swt);
