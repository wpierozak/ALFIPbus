#include"Swt.h"
#include<array>
#include<stdexcept>

namespace fit_swt
{
class SwtFifo
{
public:
    static constexpr uint32_t FifoLimit = 1024;

    uint32_t* prepareResponseFrame(const Swt& frame)
    {
        if(m_currentBack == FifoLimit){
            throw std::out_of_range("Reached SWT fifo limit");
        }
        m_buffer[m_currentBack++] = frame;
        return &m_buffer[m_currentBack-1].data;
    }
    void SwtFifo::push(const Swt& frame){
        if(m_currentBack == FifoLimit){
            throw std::out_of_range("Reached SWT fifo limit");
        }
        m_buffer[m_currentBack++] = frame;
    }
    const Swt& pop() 
    {
        if(m_currentBack == m_currentFront){
            throw std::out_of_range("Swt fifo is empty!");
        }
        return m_buffer[m_currentFront++];
    }
    bool empty(){
        return m_currentBack == m_currentFront;
    }
    uint32_t size(){
        return m_currentBack;
    }
    void clear(){
        m_currentBack = 0;
        m_currentFront = 0;
    }

private:
    std::array<Swt,FifoLimit> m_buffer;
    uint32_t m_currentBack{0};
    uint32_t m_currentFront{0};
};
}