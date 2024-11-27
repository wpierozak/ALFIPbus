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
        void reset(int packetID = 0);

        bool addTransaction(enums::transactions::TransactionType type, uint32_t* dataIn, uint8_t nWords, InfoCode infocode);
    };
}
#endif