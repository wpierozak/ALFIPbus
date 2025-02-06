
#include <boost/log/trivial.hpp>
#include"IPbusHeaders.h"
#include"Memory.h"

#ifndef IPBUS_PACKET
#define IPBUS_PACKET

namespace ipbus
{
    enum InfoCode{Response=0x0,
                BadHeader=0x1,
                ErrorRead=0x4,
                ErrorWrite=0x5,
                TimeoutRead=0x6,
                TimeoutWrite=0x7,
                Request=0xf};
                
    const uint16_t maxPacket = 368;
    class IPbusPacket
    {
        public:
        uint32_t& operator[](int idx) {return m_buffer[idx];}
        uint32_t* getBuffer() { return m_buffer; }

        uint32_t operator[](int idx) const  {return m_buffer[idx];}
        const uint32_t* getBuffer() const { return m_buffer; }

        uint16_t getSize() const { return m_size; }
        uint16_t getTransactionNumber() const { return m_transactionsNumber; } // Not works when request/response is received!

        void setSize(uint16_t size) { m_size = size; }

        protected:
        uint32_t m_buffer[maxPacket];
        size_t m_size;

        uint16_t m_transactionsNumber;
    };
}

#endif