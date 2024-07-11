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

    frame.ID = string_to_byte(str[0], str[1]) >> 4;

    return frame;
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
    printf("ID: %u\n", swt.ID);
}



