#include"IPbusHeaders.h"
#include"IPbusControlPacket.h"
#include"Memory.h"

#ifndef IPBUS_RESPONSE
#define IPBUS_RESPONSE

namespace ipbus
{
    class IPbusResponse
    {
        public:

        IPbusResponse();
        void reset();

        void addTransaction(TransactionType type, uint32_t address, uint32_t* dataIn, uint8_t nWords, InfoCode infocode);
        
        uint32_t& operator[](int idx) {return m_buffer[idx];}
        
        uint32_t* getBuffer() { return m_buffer; }
        uint16_t getSize() const { return m_size; }
        void setSize(uint16_t size) { m_size = size; }

        private:
        uint32_t m_buffer[maxPacket];
        uint16_t m_size;
    };
}
#endif