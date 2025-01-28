#pragma once
#include"Swt.h"
#include<cstring>

struct CruCommand
{
    enum class Type {ScReset = 0, Read,Write,Invalid};
    CruCommand(const char* str)
    {
        if(strncmp(str, ScReset, ScResetLen) == 0)
        {
            type = Type::ScReset;
        }
        else if(strncmp(str, Read, ReadLen) == 0){
            type = Type::Read;
        }
        else if(strncmp(str + fit_swt::Swt::SwtStrLen + 1, Write, WriteLen) == 0){
            frame = fit_swt::Swt(str+2);
            type = Type::Write;
        }
        else{
            type = Type::Invalid;
        }
    }

    Type type;
    fit_swt::Swt frame;

    uint32_t commandStrLen() const
    {
        switch(type)
        {
            case Type::ScReset:
                return ScResetLen;
            case Type::Read:
                return ReadLen;
            case Type::Write:
                return WriteLen + fit_swt::Swt::SwtStrLen + 1;
        }
    }

    static constexpr const char* ScReset = "sc_reset";
    static constexpr uint32_t ScResetLen = std::char_traits<char>::length(ScReset);
    static constexpr const char* Read = "read";
    static constexpr uint32_t ReadLen = std::char_traits<char>::length(Read);
    static constexpr const char* Write = "write";
    static constexpr uint32_t WriteLen = std::char_traits<char>::length(Write);
};


