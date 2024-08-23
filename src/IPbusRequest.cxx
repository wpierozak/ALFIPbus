#include"IPbusRequest.h"
#include"IPbusStatusPacket.h"
namespace ipbus
{
    IPbusRequest::IPbusRequest()
    {
        m_buffer[0] = PacketHeader(enums::packets::Control, 0);
        m_size = 1;
        m_transactionsNumber = 0;
        m_expectedResponseSize = 1;
    }

    void IPbusRequest::reset(int packetID)
    {
        m_buffer[0] = PacketHeader(enums::packets::Control, packetID);
        m_size = 1;
        m_transactionsNumber = 0;
        m_expectedResponseSize = 1;
        m_dataOut.clear();
    }

    void IPbusRequest::addTransaction(enums::transactions::TransactionType type, uint32_t address, uint32_t* dataIn, uint32_t* dataOut, uint8_t nWords)
    {
        if(m_size + nWords + 1 > maxPacket)
        {
            BOOST_LOG_TRIVIAL(error) << "Request packet size exceeded";
            return;
        }

        m_buffer[m_size++] = TransactionHeader(type, nWords, m_transactionsNumber++);
        m_buffer[m_size++] = address;
        m_expectedResponseSize++;

        switch (type)
        {
        case enums::transactions::Read:
        case enums::transactions::NonIncrementingRead:
        case enums::transactions::ConfigurationRead:
            m_expectedResponseSize += nWords;
            break;

        case enums::transactions::Write:
        case enums::transactions::NonIncrementingWrite:
        case enums::transactions::ConfigurationWrite:
            for (uint8_t i = 0; i < nWords; ++i) 
            {
                m_buffer[m_size++] = dataIn[i];
            }
            break;

        case enums::transactions::RMWbits:
            m_buffer[m_size++] = dataIn[0];
            m_buffer[m_size++] = dataIn[1];
            m_expectedResponseSize++;
            break;

        case enums::transactions::RMWsum:
            m_buffer[m_size++] = dataIn[0];
            m_expectedResponseSize++;
            break;

        default:
            BOOST_LOG_TRIVIAL(warning) << "Unknown transaction type " << type << ": no transaction was added";
            break;
        }

        if(m_expectedResponseSize > maxPacket)
        {
            BOOST_LOG_TRIVIAL(error) << "Response packet size exceeded";
            // ADD THROW!!!
            return;
        }

        m_dataOut.push_back(dataOut);
    }

    bool IPbusRequest::isStatusRequest() const
    {
        return (m_size == 16 && m_buffer[0] == (ipbus::StatusPacket()).header);
    }
}