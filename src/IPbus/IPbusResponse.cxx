#include"IPbus/IPbusResponse.h"
namespace ipbus
{
    IPbusResponse::IPbusResponse()
    {
        m_buffer[0] = PacketHeader(enums::packets::Control, 0);
        m_transactionsNumber = 0;
        m_size = 1;
    }

    void IPbusResponse::reset(int packetID)
    {
        m_buffer[0] = PacketHeader(enums::packets::Control, packetID);
        m_transactionsNumber = 0;
        m_size = 1;
    }

    bool IPbusResponse::addTransaction(enums::transactions::TransactionType type, uint32_t* dataIn, uint8_t nWords, InfoCode infoCode)
    {
        if(m_size + nWords + 1 > maxPacket)
        {
            BOOST_LOG_TRIVIAL(warning) << "Response packet size exceeded";
        }

        m_buffer[m_size++] = TransactionHeader(type, nWords, m_transactionsNumber++, infoCode);
        if(infoCode != Response)
        {
            return true;
        }
        switch(type)
        {
            case enums::transactions::Read:
            case enums::transactions::NonIncrementingRead:
            case enums::transactions::ConfigurationRead:
            {
                memcpy(m_buffer + m_size, dataIn, nWords*wordSize);
                m_size += nWords;
            }
            break;

            case enums::transactions::Write:
            case enums::transactions::NonIncrementingWrite:
            case enums::transactions::ConfigurationWrite: 
            {
                if(dataIn != nullptr)
                {
                    BOOST_LOG_TRIVIAL(warning) << "Write transaction response should contain only header!";
                }
            }
            break;

            case enums::transactions::RMWbits:
            case enums::transactions::RMWsum:
            {
                if(nWords != 1)
                {
                    BOOST_LOG_TRIVIAL(warning)  << "RMWbits/RMWsum response should contain only the register value before operation";
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