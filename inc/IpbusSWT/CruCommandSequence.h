#pragma once
#include"Swt.h"
#include "utils.h"
#include<cstring>
#include<charconv>


class CruCommandSequnce
{
    public:
    CruCommandSequnce(const char* sequence);

    struct Command
    {
        enum class Type {Read = 0, Write, ReadCnt, Wait, ScReset, Invalid = -1};

        static constexpr const char* ReadStr = "read";
        static constexpr size_t ReadStrLen = std::char_traits<char>::length(ReadStr);
        static constexpr const char* WriteStr = "write";
        static constexpr size_t WriteStrLen = std::char_traits<char>::length(WriteStr);
        static constexpr const char* ReadCntStr = "read_cnt";
        static constexpr size_t ReadCntStrLen = std::char_traits<char>::length(ReadCntStr);
        static constexpr const char* WaitStr = "wait";
        static constexpr size_t WaitStrLen = std::char_traits<char>::length(WaitStr);
        static constexpr const char* ScResetStr = "sc_reset";
        static constexpr size_t ScResetStrLen = std::char_traits<char>::length(ScResetStr);
        static constexpr size_t SwtFrameLen = 21;

        Command(): type(Type::Invalid) {}
        Command(const char* beg, size_t command, size_t len);

        Type type;
        union {
            fit_swt::Swt frame;
            uint32_t wordsToRead;
            uint32_t waitTime;
            uint32_t timeout;
            uint32_t words[sizeof(fit_swt::Swt) / sizeof(uint32_t)];
            uint8_t bytes[sizeof(fit_swt::Swt)];
        } data;

        private:
        bool parsedArgUnsignedInteger(const char* beg, size_t command, uint32_t& dest)
        {
            return std::from_chars(beg, beg + command - 1, dest).ptr == (beg + command - 1);
        }

    };
    
    Command getNextCmd();
    bool isNextCmd();

    private:
    const char* const m_seqBegin;
    const char* m_position;
};