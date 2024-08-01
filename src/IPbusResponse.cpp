#include"IPbusResponse"
namespace ipbus
{
    IPbusResponse::IPbusResponse()
    {
        m_size = 0;
    }

    void IPbusResponse::reset()
    {
        m_size = 0;
    }

    void IPbusResponse::addTransaction(TransactionType type, uint32_t address, uint32_t* dataIn, uint8_t nWords, InfoCode infocode)
    {

    }
}