#include"IPbusHeaders.h"
#include"IPbusPacket.h"
#include"Memory.h"

#ifndef IPBUS_RESPONSE
#define IPBUS_RESPONSE

namespace ipbus
{
    class IPbusResponse: public IPbusPacket
    {
        public:

        IPbusResponse();
        void reset();

        void addTransaction(TransactionType type, uint32_t address, uint32_t* dataIn, uint8_t nWords, InfoCode infocode);
        
        void setSize(uint16_t size) { m_size = size; }

    };
}
#endif