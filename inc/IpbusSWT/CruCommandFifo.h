#include"CruCommand.h"
#include<array>
#include<stdexcept>

class CruCommandFifo
{
public:
    static constexpr uint32_t Capacity = 1024;
    void push(const CruCommand& cmd){
        if(m_currentBack == Capacity){
            throw std::out_of_range("Reached SWT fifo limit");
        }
        m_buffer[m_currentBack++] = cmd;
    }

    const CruCommand& pop() 
    {
        if(m_currentBack == 0){
            throw std::out_of_range("Swt fifo is empty!");
        }
        return m_buffer[--m_currentBack];
    }

    bool empty()
    {
        return m_currentBack == 0;
    }

    bool full()
    {
        return m_currentBack == Capacity;
    }

    CruCommand& back()
    {
        return m_buffer[m_currentBack-1];
    }
    
private:
    std::array<CruCommand,Capacity> m_buffer;
    uint32_t m_currentBack;
};
