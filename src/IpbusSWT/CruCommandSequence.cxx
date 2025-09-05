#include "IpbusSWT/CruCommandSequence.h"
#include "IpbusSWT/utils.h"
#include <charconv>

CruCommandSequnce::CruCommandSequnce(const char* sequence): m_seqBegin(sequence), m_position(sequence)
{

}

CruCommandSequnce::Command CruCommandSequnce::getNextCmd()
{
    size_t command{0};
    size_t len{0};
    for(const char* tmp = m_position; *tmp != '\0'; tmp++){
        if(*tmp == ',') {
            command = (tmp - m_position) + 1;
        } else if (*tmp == '\n' || *tmp == '\0'){
            len = tmp - m_position;
            break;
        }       
    }
    Command next(m_position, command, len);
    m_position += (*(m_position + len) != '\0') ? len + 1 : len;
    return next;
}

bool CruCommandSequnce::isNextCmd()
{
    return (*m_position != '\0' && *(m_position + 1) != '\0');
}

CruCommandSequnce::Command::Command(const char* beg, size_t command, size_t len)
{
    switch (len - command)
    {
    case ReadStrLen: // same len for WaitStr
        if (strncmp(beg + command, ReadStr, ReadStrLen) == 0) {
            type = Type::Read;
        } else if (strncmp(beg + command, WaitStr, WaitStrLen) == 0) {
            type = Type::Wait;
        } else {
            type = Type::Invalid;
        }
    break;
    case WriteStrLen:
        if (strncmp(beg + command, WriteStr, WriteStrLen) == 0) {
            type = Type::Write;
        } else {
            type = Type::Invalid;
        }
        break;
    case ReadCntStrLen: // same len for ScResetStr
        if(strncmp(beg + command, ReadCntStr, ReadCntStrLen) == 0) {
            type = Type::ReadCnt;
        } else if (strncmp(beg + command, ScResetStr, ScResetStrLen) == 0) {
            type = Type::ScReset;
        } else {
            type = Type::Invalid;
        }
        break;
    default:
        type = Type::Invalid;
        break;
    }

    switch(type)
    {
        case Type::Read: {
            if(command != 0 && parsedArgUnsignedInteger(beg, command, data.timeout) == false) {
                std::string argument(beg, beg + command);
                throw std::runtime_error("Invalid argument to " + std::string(ReadStr) + ": " + argument);
            }
        }
            break;
        case Type::Wait:  {
            if(command == 0 || (command != 0 && parsedArgUnsignedInteger(beg, command, data.waitTime) == false)){
                std::string argument(beg, beg + command);
                throw std::runtime_error("Invalid argument to " + std::string(WaitStr) + ": " + argument);
            }
        }
            break;
        case Type::ReadCnt: {
            if(command == 0 || (command != 0 && parsedArgUnsignedInteger(beg, command, data.wordsToRead) == false)){
                std::string argument(beg, beg + command);
                throw std::runtime_error("Invalid argument to " + std::string(ReadCntStr) + ": " + argument);
            }
        }
            break;
        case Type::Write: {
            if(command != SwtFrameLen + 1) {
                std::string argument(beg, beg + command);
                throw std::runtime_error("Invalid argument to " + std::string(WriteStr) + ": " + argument);
            }
            try{
                data.frame = fit_swt::Swt(beg + 2);
            } catch(std::exception & e){
                std::string argument(beg, beg + command - 1);
                throw std::runtime_error("Invalid argument to " + std::string(WriteStr) + ": " + argument + std::string("; Exception: ") + e.what());
            }
        }
            break;
        case Type::Invalid: {
            std::string errorMessage(beg, beg + len);
            throw std::runtime_error("Invalid command: " + errorMessage);
        }
        default:
            break;
    }
}
