#include"IPbusResponse.h"
namespace ipbus
{
    IPbusResponse::IPbusResponse()
    {
        m_buffer[0] = PacketHeader(Control, 0);
        m_transactionsNumber = 0;
        m_size = 1;
    }

    void IPbusResponse::reset(int packetID)
    {
        m_buffer[0] = PacketHeader(Control, packetID);
        m_transactionsNumber = 0;
        m_size = 1;
    }

    bool IPbusResponse::addTransaction(TransactionType type, uint32_t* dataIn, uint8_t nWords, InfoCode infoCode)
    {
        if(m_size + nWords + 1 > maxPacket)
        {
            BOOST_LOG_TRIVIAL(warning) << "Request packet size exceeded";
        }

        m_buffer[m_size++] = TransactionHeader(type, nWords, m_transactionsNumber++, infoCode);
        if(infoCode != Response)
        {
            return true;
        }
        switch(type)
        {
            case DataRead:
            case NonIncrementingRead:
            case ConfigurationRead:
            {
                memcpy(m_buffer + m_size, dataIn, nWords*wordSize);
                m_size += nWords;
            }
            break;

            case DataWrite:
            case NonIncrementingWrite:
            case ConfigurationWrite: 
            {
                if(nWords != 0 || dataIn != nullptr)
                {
                    BOOST_LOG_TRIVIAL(warning) << "Write transaction response contains only header!";
                }
            }
            break;

            case RMWbits:
            case RMWsum:
            {
                if(nWords != 1)
                {
                    BOOST_LOG_TRIVIAL(warning)  << "RMWbits/RMWsum response contains only the register value before operation";
                }
                if(dataIn == nullptr)
                {
                    BOOST_LOG_TRIVIAL(error) << "RMWbits/RMWsum response data was not provided!";
                    return false;
                }
                memcpy(m_buffer + m_size, dataIn, wordSize);
                m_size++;
            }
            break;

            default:
                BOOST_LOG_TRIVIAL(warning) << "Unknown transaction type " << type << ": no transaction was added";
                return false;
            break;
        }
        return true;
    }
}