#include"IPbusHeaders.h"
#include"IPbusPacket.h"
#include"IPbusPacket.h"

#ifndef IPBUS_REQUEST
#define IPBUS_REQUEST

namespace ipbus
{
    class IPbusRequest: public IPbusPacket
    {
        public:

        IPbusRequest();
        void reset(int packetID = 0);
        void addTransaction(TransactionType type, uint32_t address, uint32_t* dataIn, uint32_t* dataOut, uint8_t nWords = 1);

        uint16_t getExpectedResponseSize() const { return m_expectedResponseSize; }
        uint32_t* getDataOut(int idx) { return m_dataOut[idx]; }

        private:

        uint16_t m_expectedResponseSize;
        std::vector<uint32_t*> m_dataOut;
    };
}

#endif