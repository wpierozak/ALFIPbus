#include"IPbusHeaders.h"
#include"IPbusControlPacket.h"

#ifndef IPBUS_REQUEST
#define IPBUS_REQUEST

namespace ipbus
{
    class IPbusRequest
    {
        public:

        IPbusRequest();
        void reset();
        void addTransaction(TransactionType type, uint32_t address, uint32_t* dataIn, uint32_t* dataOut, uint8_t nWords = 1);

        const uint32_t& operator[](int idx) const {return m_buffer[idx];}
        const uint32_t* getBuffer() const { return m_buffer; }
        uint16_t getSize() const { return m_size; }
        uint16_t getExpectedResponseSize() const { return m_expectedResponseSize; }
        uint16_t getTransactionNumber() const { return m_transactionsNumber; }
        uint32_t* getDataOut(int idx) { return m_dataOut[idx]; }

        private:
        uint32_t m_buffer[maxPacket];
        size_t m_size;
        uint16_t m_expectedResponseSize;
        uint16_t m_transactionsNumber;
        std::vector<uint32_t*> m_dataOut;
    };
}

#endif