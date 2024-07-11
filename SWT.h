#pragma once
#include<cstdint>
#include<string>
#include <QtGlobal>
#include <QtEndian>

union half_word
{
    struct fields
    {
        quint8 first_byte;
        quint8 second_byte;
    } fields;
    quint16 data;
};

union word
{
    struct hex
    {
        quint8 first_byte;
        quint8 second_byte;
        quint8 third_byte;
        quint8 fourth_byte;
            /* data */
    } fields;
    quint32 data;
};

quint8 charToHex(char ch);
quint8 string_to_byte(char c1, char c2);


        /*      SWT     */


struct SWT
{
    quint32 data       : 16;
    quint32 submodule  : 8;
    quint32 module     : 7;
    quint32 mode       : 1;
    quint32 blank_A    : 32;
    quint32 blank_B    : 32;
    quint32 ID         : 4;
};

SWT string_to_swt(const char* str);

struct SWT_IPBUS_READY
{
    enum class Type{Read=0, Write=1};
    Type type;
    quint32 address;
    quint32 data;
};

SWT_IPBUS_READY swt_ready(SWT swt);
