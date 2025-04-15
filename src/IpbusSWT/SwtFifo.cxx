#include"IpbusSWT/SwtFifo.h"

namespace fit_swt
{
    void SwtFifo::push(const Swt& frame){
        if(m_currentBack == FifoLimit){
            throw std::out_of_range("Reached SWT fifo limit");
        }
        m_buffer[m_currentBack++] = frame;
    }
}