#include"CruCommand.h"
#include<array>
#include<stdexcept>

class CruCommandBuffer
{
public:
    static constexpr uint32_t Capacity = 1024;
private:
    std::array<CruCommand,Capacity> m_buffer;
    uint32_t m_currentBack { 0 };
    bool m_areWordsToRead;

public:
    CruCommandBuffer(): m_currentBack(0), size(m_currentBack) {}
    
    inline CruCommand& push(const CruCommand&& cmd){
        if(m_currentBack == Capacity){
            throw std::out_of_range("Reached SWT fifo limit");
        }
        m_buffer[m_currentBack++] = cmd;
        m_areWordsToRead = ((cmd.type == CruCommand::Type::Write) && (cmd.frame.responseSize() != 0));
        return m_buffer[m_currentBack-1];
    }

    inline CruCommand& operator[](uint32_t idx){
        return m_buffer[idx];
    }

    inline bool empty()
    {
        return m_currentBack == 0;
    }

    inline bool full()
    {
        return m_currentBack == Capacity;
    }

    inline CruCommand& back()
    {
        return m_buffer[m_currentBack-1];
    }

    inline CruCommand& oneBeforeLast()
    {
        return m_buffer[m_currentBack - 2];
    }

    inline bool validateLastCmd()
    {
        return (m_buffer[m_currentBack - 1].type != CruCommand::Type::Read) || (m_areWordsToRead);
    }

    inline void reset()
    {
        m_currentBack = 0;
        m_areWordsToRead = false;
    }
    
    const uint32_t& size;
};
