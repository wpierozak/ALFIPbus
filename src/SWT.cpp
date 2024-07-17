#pragma once
#include<cstdint>
#include<string>
#include<stdexcept>
#include"SWT.h"

        /*      HEX     */

quint8 charToHex(char ch) {
    switch (ch) {
        case '0': return 0;
        case '1': return 1;
        case '2': return 2;
        case '3': return 3;
        case '4': return 4;
        case '5': return 5;
        case '6': return 6;
        case '7': return 7;
        case '8': return 8;
        case '9': return 9;
        case 'A': case 'a': return 10;
        case 'B': case 'b': return 11;
        case 'C': case 'c': return 12;
        case 'D': case 'd': return 13;
        case 'E': case 'e': return 14;
        case 'F': case 'f': return 15;
        default: throw std::runtime_error("Invalid hexadecimal character: " + ch);
    }
}

quint8 string_to_byte(char c1, char c2)
{
    return 16*charToHex(c1) + charToHex(c2);
}


        /*      SWT     */


SWT string_to_swt(const char* str)
{
    SWT frame;
    
    half_word data; 
    data.fields = { string_to_byte(str[18], str[19]), string_to_byte(str[16],str[17]) };
    frame.data = data.data;

    frame.submodule = string_to_byte(str[14], str[15]);
    frame.module    = string_to_byte(str[12], str[13]) & 0x7F;
    frame.mode      = string_to_byte(str[12], str[13]) >> 7;

    word ignored;
    ignored.fields = {  string_to_byte(str[10], str[11]), 
                        string_to_byte(str[8], str[9]), 
                        string_to_byte(str[6], str[7]),
                        string_to_byte(str[4], str[5]) };

    frame.blank_A = ignored.data;

    ignored.fields.first_byte = string_to_byte(str[2], str[3]);
    ignored.fields.second_byte = string_to_byte(str[0], str[1]);
    ignored.data = ignored.data & 0xFFF;
    frame.blank_B = ignored.data;

    frame.SWT_ID = string_to_byte(str[0], str[1]) >> 4;

    return frame;
}

SWT byte_to_swt(const quint8* bytes)
{
    SWT frame;

    half_word data;
    data.fields = { bytes[0], bytes[1] };
    frame.data = data.data;

    frame.submodule = bytes[2];
    frame.module    = bytes[3] & 0x7F;
    frame.mode      = bytes[3] >> 7;

    word ignored;
    ignored.fields = {  bytes[4],
                        bytes[5],
                        bytes[6],
                        bytes[7] };

    frame.blank_A = ignored.data;

    ignored.fields.first_byte = bytes[8];
    ignored.fields.second_byte = bytes[9];
    ignored.data = ignored.data & 0xFFF;
    frame.blank_B = ignored.data;

    frame.SWT_ID = bytes[9] >> 4;

    return frame;
}

void swt_to_byte(SWT swt, uint8_t* bytes)
{
    bytes[0] = swt.data & 0xFF;
    bytes[1] = swt.data >> 8;
    bytes[2] = swt.submodule;
    bytes[3] = swt.module + (swt.mode << 7);
    bytes[4] = swt.blank_A & 0xFF;
    bytes[5] = (swt.blank_A >> 8) & 0xFF;
    bytes[6] = (swt.blank_A >> 16) & 0xFF;
    bytes[7] = (swt.blank_A >> 24) & 0xFF;
    bytes[8] = swt.blank_B & 0xFF;
    bytes[9] = (swt.blank_B >> 8) + (swt.SWT_ID << 4);
}

SWT_IPBUS_READY swt_ready(SWT swt)
{
    if(swt.mode == 0 )
        return { SWT_IPBUS_READY::Type::Read, (quint32(swt.module) << 8) + swt.submodule, swt.data};
    else
        return { SWT_IPBUS_READY::Type::Write, (quint32(swt.module) << 8) + swt.submodule, swt.data};
}

void printSWT(struct SWT swt) {
    printf("data: %u\n", swt.data);
    printf("submodule: %u\n", swt.submodule);
    printf("module: %u\n", swt.module);
    printf("mode: %u\n", swt.mode);
    printf("blank_A: %u\n", swt.blank_A);
    printf("blank_B: %u\n", swt.blank_B);
    printf("ID: %u\n", swt.SWT_ID);
}

char hexToChar(uint8_t hex)
{
    switch (hex)
    {
    case 0:     return '0';
    case 1:     return '1';
    case 2:     return '2';
    case 3:     return '3';
    case 4:     return '4';
    case 5:     return '5';
    case 6:     return '6';
    case 7:     return '7';
    case 8:     return '8';
    case 9:     return '9';
    case 10:    return 'A';
    case 11:    return 'B';
    case 12:    return 'C';
    case 13:    return 'D';
    case 14:    return 'E';
    case 15:    return 'F';
    }
}


