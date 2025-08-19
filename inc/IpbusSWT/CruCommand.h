#pragma once
#include"Swt.h"
#include "utils.h"
#include<cstring>
#include<charconv>

struct CruCommand
{
    enum class Type {ScReset = 0, Read, Write, Wait, Invalid};
    CruCommand(): type(Type::Invalid)
    {

    }
    
    CruCommand(const char* str)
    {
        if(strncmp(str, ScReset, ScResetLen) == 0)
        {
            type = Type::ScReset;
            frame.mode = 0xFFFF;
        }
        else if(strncmp(str, Read, ReadLen) == 0){
            type = Type::Read;
            frame.mode = 0xFFFF;
        }
        else if(strncmp(str + fit_swt::Swt::SwtStrLen + 1, Write, WriteLen) == 0){
            frame = fit_swt::Swt(str+2);
            type = Type::Write;
        }
        else if(strncmp(str, Wait, WaitLen) == 0 && (std::from_chars(str + WaitLen + 2, fit_swt::utils::findC(str, '\n'), frame.data).ec == std::errc())){
            type = Type::Wait;
        }
        else{
            type = Type::Invalid;
        }
    }

    Type type{Type::Invalid};
    fit_swt::Swt frame;

    inline uint32_t commandStrLen() const
    {
        switch(type)
        {
            case Type::ScReset:
                return ScResetLen;
            case Type::Read:
                return ReadLen;
            case Type::Write:
                return WriteLen + fit_swt::Swt::SwtStrLen + 1;
            case Type::Wait:
                return WaitLen + 3;
            default:
                throw std::runtime_error("Request for length of invalid line!");
        }
    }

    inline bool valid() const
    {
        return type != Type::Invalid;
    }

    static constexpr const char* ScReset = "sc_reset";
    static constexpr uint32_t ScResetLen = std::char_traits<char>::length(ScReset);
    static constexpr const char* Read = "read";
    static constexpr uint32_t ReadLen = std::char_traits<char>::length(Read);
    static constexpr const char* Write = "write";
    static constexpr uint32_t WriteLen = std::char_traits<char>::length(Write);
    static constexpr const char* Wait = "wait";
    static constexpr uint32_t WaitLen = std::char_traits<char>::length(Wait);
};


