#pragma once

#include"CruCommandSequence.h"
#include<array>
#include<stdexcept>

class CruCommandBuffer
{
public:
    static constexpr uint32_t Capacity = 1024;
private:
    std::array<CruCommandSequnce::Command,Capacity> m_buffer;
    uint32_t m_currentBack { 0 };

public:
    CruCommandBuffer(): m_currentBack(0), size(m_currentBack) {}
    
    inline CruCommandSequnce::Command& push(const CruCommandSequnce::Command&& cmd){
        if(m_currentBack == Capacity){
            throw std::out_of_range("Reached CRU command buffer limit");
        }
        m_buffer[m_currentBack++] = cmd;
        return m_buffer[m_currentBack - 1];
    }

    inline CruCommandSequnce::Command& operator[](uint32_t idx){
        return m_buffer[idx];
    }

    inline CruCommandSequnce::Command& operator()(uint32_t idx){
        return m_buffer[m_currentBack - 1 - idx];
    }

    inline bool empty()
    {
        return m_currentBack == 0;
    }

    inline bool full()
    {
        return m_currentBack == Capacity;
    }

    inline CruCommandSequnce::Command& back()
    {
        return m_buffer[m_currentBack-1];
    }

    inline void reset()
    {
        m_currentBack = 0;
    }
    
    const uint32_t& size;
};
