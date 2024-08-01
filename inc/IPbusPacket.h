
#include <boost/log/trivial.hpp>
#include"IPbusHeaders.h"
#include"Memory.h"

#ifndef IPBUS_PACKET
#define IPBUS_PACKET

namespace ipbus
{
    const uint16_t maxPacket = 368;
    class IPbusPacket
    {
        public:
        uint32_t operator[](int idx)  {return m_buffer[idx];}
        uint32_t* getBuffer() { return m_buffer; }
        uint16_t getSize() const { return m_size; }

        protected:
        uint32_t m_buffer[maxPacket];
        size_t m_size;
    };
}

#endif